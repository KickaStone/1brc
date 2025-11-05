#include <chrono>
#include <fstream>
#include <iostream>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include <future>
#include <set>
#include <limits>

#ifndef NTHREADS
#define NTHREADS 16
#endif
#define NSTATIONS 1000

using namespace std;


mutex mu;
ifstream myfile;

struct WeatherStation {
    uint32_t cnt = 0;
    string id = "";
    double totalTemp = 0;
    double maxTemp = numeric_limits<double>::min();
    double minTemp = numeric_limits<double>::max();
};

int main(int argc, char *argv[]) {

    if(argc != 2) {
        cout << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }
    string fn = argv[1];

    auto start = chrono::high_resolution_clock::now();
    myfile.open(fn);
    if (!myfile.is_open()) {
        return 1;
    }

    vector<thread> threads;
    vector<unordered_map<string, WeatherStation> > v(NTHREADS);

    for (auto &mp : v) {
        mp.reserve(NSTATIONS);
    }

    for (int i = 0; i < NTHREADS; i++) {
        threads.emplace_back([i, &v]() {
            string line;
            auto &stations = v[i];
            while (true) {
                {
                    lock_guard<mutex> lock(mu);
                    if (!getline(myfile, line)) {
                        break;
                    }
                }
                auto idx = line.find(';');
                string id = line.substr(0, idx);
                double t = stod(line.substr(idx + 1));
                if (stations.find(id) == stations.end()) {
                    stations[id] = WeatherStation();
                }
                stations[id].id = id;
                stations[id].totalTemp += t;
                stations[id].maxTemp = max(stations[id].maxTemp, t);
                stations[id].minTemp = min(stations[id].minTemp, t);
            }
        });

    }

    for (auto &t: threads) t.join();

    unordered_map<string, WeatherStation> stations;
    stations.reserve(NSTATIONS);

    set<string> st;
    for (auto &hashmap: v) {
        for (auto &p: hashmap) {
            if (stations.find(p.first) == stations.end()) {
                stations[p.first] = WeatherStation();
                stations[p.first].id = p.first;
            }

            WeatherStation &mp_st = stations[p.first];
            mp_st.totalTemp += hashmap[p.first].totalTemp;
            mp_st.maxTemp = max(mp_st.maxTemp, mp_st.totalTemp);
            mp_st.minTemp = min(mp_st.minTemp, mp_st.totalTemp);
            mp_st.cnt += hashmap[p.first].cnt;
            st.insert(p.first);
        }
    }

    auto print = [](WeatherStation &ws, const char * c) {
        printf("%s%s=%.1f/%.2f/%.1f", c, ws.id.c_str(), ws.totalTemp / ws.cnt, ws.minTemp, ws.maxTemp);
    };

    // 
    bool flag = false;
    for(auto &s : st){
        if(flag) {
            print(stations[s], ", ");
            flag = true;
        }else{
            print(stations[s], "");
        }
    }

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;
    cout << "Time: " << chrono::duration_cast<chrono::seconds>(elapsed).count() << " seconds." << endl;
    return 0;
}
