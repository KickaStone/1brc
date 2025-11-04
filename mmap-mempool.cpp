#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <thread>
#include <vector>
#include <algorithm>

#include "myhashtable2.h"

using namespace std;

#ifndef NTHREADS
#define NTHREADS 16
#endif

struct WeatherStation
{
    uint32_t cnt = 0;
    int totalTemp = 0;
    int maxTemp = numeric_limits<int>::min();
    int minTemp = numeric_limits<int>::max();
};

inline int parse(char *data, int &len, int &v)
{
    int neg = 1;
    len = 0;
    v = 0;
    int idx = 0;
    // 解析名称
    while (data[idx++] != ';') len++;

    // 解析温度值
    while (data[idx] != '\n' && data[idx] != '\0')
    {
        if (data[idx] == '-')
        {
            neg = -1;
            idx++;
            continue;
        }
        if (data[idx] == '.')
        {
            idx++;
            continue;
        }
        v = v * 10 + (data[idx] - '0');
        idx++;
    }
    v *= neg;
    if (data[idx] == '\n') idx++;
    return idx;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }
    string fn = argv[1];

    int fd = open(fn.c_str(), O_RDONLY);
    if (fd == -1)
    {
        cout << "Error opening file" << endl;
        return 1;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        cout << "Error getting file size" << endl;
        close(fd);
        return 1;
    }

    size_t file_size = sb.st_size;
    char *mapped = (char *)mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED)
    {
        cout << "Error mapping file" << endl;
        close(fd);
        return 1;
    }

    vector<HashTable<WeatherStation>> v;
    v.reserve(NTHREADS);
    for (int i = 0; i < NTHREADS; i++) {
        v.emplace_back(10000);
    }
    vector<thread> threads;

    size_t left[NTHREADS], right[NTHREADS];
    double d = file_size * 1.0 / NTHREADS;
    for (int i = 0; i < NTHREADS; i++)
    {
        left[i] = (size_t)(d * i);
        if (i)
        {
            size_t t = left[i];
            while (mapped[t] != '\n') t++;
            left[i] = t + 1;
            right[i - 1] = t + 1; // mapped[t] = '\n'
        }
    }
    right[NTHREADS - 1] = file_size;

    for (auto i = 0; i < NTHREADS; i++)
    {
        threads.emplace_back([&, i]()
                             {
            auto &stations = v[i];
            char *name = nullptr, *data = nullptr;
            int total = 0, len = 0, value = 0;
            while(left[i] + total < right[i]){
                data = &mapped[left[i] + total];
                name = data;
                total += parse(data, len, value);
                auto ptr = stations.try_emplace(name, len);
                if(ptr != nullptr){
                    ptr->cnt += 1;
                    ptr->totalTemp += value;
                    ptr->maxTemp = max(ptr->maxTemp, value);
                    ptr->minTemp = min(ptr->minTemp, value);
                }else{
                    printf("Error: %s insert failed\n", name);
                }
            } });
    }

    for (auto &t : threads)
        t.join();
    munmap(mapped, file_size);
    close(fd);

    HashTable<WeatherStation> records(10000);
    for (size_t k = 0; k < NTHREADS; k++)
    {
        auto &mp = v[k];
        for (size_t i = 0; i < mp.unique_cnt; i++)
        {
            auto key = mp.keys[i].first;
            auto len = mp.keys[i].second;
            auto record = mp.find(key, len);
            auto ptr = records.try_emplace(key, len);
            if (ptr != nullptr && record != nullptr)
            {
                ptr->totalTemp += record->totalTemp;
                ptr->maxTemp = max(ptr->maxTemp, record->maxTemp);
                ptr->minTemp = min(ptr->minTemp, record->minTemp);
                ptr->cnt += record->cnt;
            }
            else
            {
                throw std::runtime_error("Error");
                printf("Error: %s insert failed, record:%p, ptr:%p\n", mp.keys[i].first, record, ptr);
            }
        }
    }

    vector<char*> names(records.unique_cnt);
    for (size_t i = 0; i < records.unique_cnt; i++)
    {
        names[i] = const_cast<char*>(records.keys[i].first);
    }
    sort(names.begin(), names.end(), [](auto &a, auto &b) { return strcmp(a, b) < 0; });

    for (auto &name : names)
    {
        auto ptr = records.find(name, strlen(name));
        if (ptr != nullptr)
        {
            printf("%s=%.1f/%.1f/%.1f, ", name, ptr->minTemp * 0.1, ptr->totalTemp * 0.1 / ptr->cnt, ptr->maxTemp * 0.1);
        }
        else
        {
            printf("Error: %s not found\n", name);
        }
    }
    cout << endl;
    return 0;
}
