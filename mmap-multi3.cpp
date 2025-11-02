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

using namespace std;

#ifndef NTHREADS
#define NTHREADS 16
#endif

struct WeatherStation {
    uint32_t cnt = 0;
    int totalTemp = 0;
    int maxTemp = numeric_limits<int>::min();
    int minTemp = numeric_limits<int>::max();
};


int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }
    string fn = argv[1];
    
    int fd = open(fn.c_str(), O_RDONLY);
    if (fd == -1) {
        cout << "Error opening file" << endl;
        return 1;
    }
    
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        cout << "Error getting file size" << endl;
        close(fd);
        return 1;
    }
    
    size_t file_size = sb.st_size;
    char* mapped = (char*)mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        cout << "Error mapping file" << endl;
        close(fd);
        return 1;
    }

    vector<unordered_map<string, WeatherStation>> v(NTHREADS);
    vector<thread> threads;
    
    size_t left[NTHREADS], right[NTHREADS];
    double d = file_size * 1.0 / NTHREADS;
    for(int i = 0; i <NTHREADS; i++){
        left[i] = (size_t)(d * i);
        if(i){
            // 向前探处一个回车，作为当前一个开始位置和上一个结束
            size_t t = left[i];
            while(mapped[t] != '\n') t++;
            left[i] = t+1;
            right[i-1] = t+1; // mapped[t] = '\n' 
        }
    }
    right[NTHREADS-1] = file_size;

    for(auto i = 0; i < NTHREADS; i++){
        threads.emplace_back([&,i](){
            auto &stations = v[i];
            stations.reserve(1000);

            auto k = left[i];
            auto r = right[i];
            
            // 从k开始
            char buf[100];
            size_t buf_i = 0;
            int t = 0;
            bool is_name = true;
            bool is_negative = false;
            while(mapped[k] != '\0' && k < r){
                t = 0;
                while(mapped[k] != '\n'){
                    if(is_name){
                        if(mapped[k] == ';'){
                            is_name = false;
                            buf[buf_i] = '\0';
                            buf_i = 0;
                            k++;
                            continue;
                        }
                        buf[buf_i++] = mapped[k++];
                    }else{
                        if(mapped[k] == '-') {is_negative = true; k++; continue;}
                        if(mapped[k] == '.') {k++; continue;}
                        t = t * 10 + (mapped[k++] - '0');
                    }
                }

                if(is_negative) t = -t;
                auto [it, success] =  stations.try_emplace(string(buf));
                it->second.cnt += 1;
                it->second.totalTemp += t;
                it->second.maxTemp = max(it->second.maxTemp, t);
                it->second.minTemp = min(it->second.minTemp, t);
                k++;
            }
        });
    }

    for(auto &t :threads) t.join();
    munmap(mapped, file_size);
    close(fd);

    unordered_map<string, WeatherStation> records;
    records.reserve(1000);
    for(auto &stations : v){
        for(auto &p : stations){
            auto [it, success] = records.try_emplace(p.first);
            auto &ws = it->second;
            ws.totalTemp += p.second.totalTemp;
            ws.maxTemp = max(ws.maxTemp, p.second.maxTemp);
            ws.minTemp = min(ws.minTemp, p.second.minTemp);
            ws.cnt += p.second.cnt;
        }
    }

    vector<string> names;
    names.reserve(records.size());
    transform(records.begin(), records.end(), back_inserter(names), [](auto &p) { return p.first; });
    sort(names.begin(), names.end());

    for(auto &name : names){
        auto &ws = records[name];
        printf("%s=%.1f/%.1f/%.1f, ", name.c_str(), ws.totalTemp * 0.1 / ws.cnt, ws.minTemp * 0.1, ws.maxTemp * 0.1);
    }
    return 0;
}
