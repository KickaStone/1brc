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

using namespace std;

#ifndef NTHREADS
#define NTHREADS 16
#endif

struct WeatherStation {
    uint32_t cnt = 0;
    string id = "";
    double totalTemp = 0;
    double maxTemp = numeric_limits<double>::min();
    double minTemp = numeric_limits<double>::max();
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
            v[i].reserve(1000);
            auto &stations = v[i];

            auto k = left[i];
            auto r = right[i];
            
            // 从k开始
            char buf[100];
            size_t buf_i = 0;
            double t;
            string name;
            
            while(mapped[k] != '\0' && k < r){
                buf[buf_i] = mapped[k];
                t = atof(buf);
                buf_i++;

                if(mapped[k] == ';'){
                    buf[buf_i-1] = '\0';
                    name = string(buf);
                    buf_i = 0;
                }

                if(mapped[k] == '\n'){
                    buf[buf_i-1] = '\0';
                    t = atof(buf);
                    buf_i = 0;
                    if (stations.find(name) == stations.end()) {
                        stations[name] = WeatherStation();
                    }
                    stations[name].cnt += 1;
                    stations[name].id = name;
                    stations[name].totalTemp += t;
                    stations[name].maxTemp = max(stations[name].maxTemp, t);
                    stations[name].minTemp = min(stations[name].minTemp, t);
                }
                k++;
            }
        });
    }

    for(auto &t :threads) t.join();

    munmap(mapped, file_size);
    close(fd);

    // map<string, WeatherStation> stations;
    // stations.reserve(1000);
    
    // for (auto &hashmap: v) {
    //     for (auto &p: hashmap) {
    //         if (stations.find(p.first) == stations.end()) {
    //             stations[p.first] = WeatherStation();
    //             stations[p.first].id = p.first;
    //         }
            
    //         WeatherStation &mp_st = stations[p.first];
    //         mp_st.totalTemp += hashmap[p.first].totalTemp;
    //         mp_st.maxTemp = max(mp_st.maxTemp, hashmap[p.first].maxTemp);
    //         mp_st.minTemp = min(mp_st.minTemp, hashmap[p.first].minTemp);
    //         mp_st.cnt += hashmap[p.first].cnt;
    //         // st.insert(p.first);
    //     }
    // }

    // auto print = [](WeatherStation &ws, const char * c) {
    //     printf("%s%s=%.1f/%.1f/%.1f", c, ws.id.c_str(), ws.totalTemp / ws.cnt, ws.minTemp, ws.maxTemp);
    // };

    // for(auto &p : stations) {
    //     print(p.second, ", ");
    // }

    for(auto &stations : v){
        cout << stations.size() << endl;
    }

    return 0;
}
