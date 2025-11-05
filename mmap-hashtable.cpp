#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <unordered_map>
#include <thread>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include "myhashtable.h"

using namespace std;
using namespace hash1;

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

inline int parse(char *&data, char *&name, int &len, int &v)
{
    char *start = data; // 记录起始位置
    int neg = 1;
    name = data;
    len = 0;
    v = 0;

    // 解析名称
    while (*data != ';')
    {
        len++;
        data++;
    }
    data++; // 跳过 ';'

    // 解析温度值
    while (*data != '\n' && *data != '\0')
    {
        if (*data == '-')
        {
            neg = -1;
            data++;
            continue;
        }
        if (*data == '.')
        {
            data++;
            continue;
        }
        v = v * 10 + (*data - '0');
        data++;
    }
    v *= neg;

    // 跳过当前行的 '\n'，移动到下一行
    if (*data == '\n')
        data++;

    // cout << string(name, len) << "=" << v << endl;
    return data - start; // 返回解析的字符总数（包括\n）
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

    vector<HashTable<WeatherStation>> v(NTHREADS, HashTable<WeatherStation>(10000));
    vector<thread> threads;

    size_t left[NTHREADS], right[NTHREADS];
    double d = file_size * 1.0 / NTHREADS;
    for (int i = 0; i < NTHREADS; i++)
    {
        left[i] = (size_t)(d * i);
        if (i)
        {
            // 向前探处一个回车，作为当前一个开始位置和上一个结束
            size_t t = left[i];
            while (mapped[t] != '\n')
                t++;
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
            char *name = nullptr;
            int total = 0;
            int len = 0;
            int t = 0;
            char *data = mapped;
            while(left[i] + total < right[i]){
                total += parse(data, name, len, t);
                auto ptr = stations.try_emplace(name, len);
                if(ptr != nullptr){
                    ptr->cnt += 1;
                    ptr->totalTemp += t;
                    ptr->maxTemp = max(ptr->maxTemp, t);
                    ptr->minTemp = min(ptr->minTemp, t);
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
    for (auto &hashtable : v)
    {
        for (size_t i = 0; i < hashtable.unique_cnt; i++)
        {
            auto record = hashtable.find(hashtable.keys[i].first, hashtable.keys[i].second);
            auto ptr = records.try_emplace(hashtable.keys[i].first, hashtable.keys[i].second);
            if (ptr != nullptr)
            {
                ptr->totalTemp += record->totalTemp;
                ptr->maxTemp = max(ptr->maxTemp, record->maxTemp);
                ptr->minTemp = min(ptr->minTemp, record->minTemp);
                ptr->cnt += record->cnt;
            }
            else
            {
                printf("Error: %s insert failed\n", hashtable.keys[i].first);
            }
        }
    }

    vector<string> names;
    for (size_t i = 0; i < records.unique_cnt; i++)
    {
        names.push_back(string(records.keys[i].first, records.keys[i].second));
    }
    sort(names.begin(), names.end());

    for (auto &name : names)
    {
        auto ptr = records.find(name.c_str(), name.size());
        if (ptr != nullptr)
        {
            printf("%s=%.1f/%.1f/%.1f, ", name.c_str(), ptr->minTemp * 0.1, ptr->totalTemp * 0.1 / ptr->cnt, ptr->maxTemp * 0.1);
        }
        else
        {
            printf("Error: %s not found\n", name.c_str());
        }
    }
    cout << endl;
    return 0;
}
