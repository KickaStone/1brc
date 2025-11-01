#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <unordered_map>

using namespace std;

struct WeatherStation {
    string id;
    double meanTemperature;
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <filename>" << endl;
        return 1;
    }
    string fn = argv[1];

    auto start = chrono::high_resolution_clock::now();
    
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
    
    unordered_map<string, WeatherStation> stations;
    stations.reserve(2000);
    int count = 0;
    size_t pos = 0;
    
    while (pos < file_size) {
        size_t line_start = pos;
        while (pos < file_size && mapped[pos] != '\n') {
            pos++;
        }
        
        size_t line_end = pos;
        if (line_end > line_start) {
            size_t semicolon_pos = line_start;
            while (semicolon_pos < line_end && mapped[semicolon_pos] != ';') {
                semicolon_pos++;
            }
            
            string id(mapped + line_start, semicolon_pos - line_start);
            double temperature = stod(string(mapped + semicolon_pos + 1, line_end - semicolon_pos - 1));
            
            if (stations.find(id) == stations.end()) {
                stations[id] = WeatherStation{id, temperature};
            } else {
                stations[id].meanTemperature = (stations[id].meanTemperature + temperature) / 2;
            }
            
            count++;
            if (count % 10000000 == 0) {
                cout << "Processed " << count << " lines" << endl;
            }
        }
        pos++;
    }
    
    munmap(mapped, file_size);
    close(fd);
    
    auto finish = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(finish - start);
    cout << "Time taken: " << duration.count() << " milliseconds" << endl;


    return 0;
}