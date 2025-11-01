#include <chrono>
#include <fstream>
#include <iostream>
#include <unordered_map>

using namespace std;

const string MEASUREMENT_FILE = "./dataset/measurements-go.txt";

struct WeatherStation {
    string id;
    double meanTemperature{};
};
unordered_map<string, WeatherStation> stations;

int main(){
    auto start = chrono::high_resolution_clock::now();
    ifstream input(MEASUREMENT_FILE, std::ios::in);
    if (!input.is_open()) {
        cout << "Error opening file" << endl;
        return 1;
    }   
    
    string line;
    while (getline(input, line)) {
        string id = line.substr(0, line.find(';'));
        double temperature = stod(line.substr(line.find(';') + 1));
        if (stations.find(id) == stations.end()) {
            stations[id] = WeatherStation{id, temperature};
        } else {
            stations[id].meanTemperature = (stations[id].meanTemperature + temperature) / 2;
        }
    }
    input.close();
    auto finish = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(finish - start);
    cout << "Time taken: " << duration.count() << " milliseconds" << endl;
    return 0;
}