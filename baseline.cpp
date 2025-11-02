#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <algorithm>
#include <ostream>

using namespace std;

struct Record{
    u_int64_t cnt;
    double sum;
    float min;
    float max;
};

using DB = unordered_map<string, Record>;

DB process_input(istream &in){
    DB db;

    string station;
    string value;

    while(getline(in, station, ';') && getline(in, value, '\n')){
        float fp_value = stof(value);

        auto it = db.find(station);
        if(it == db.end()){
            db.emplace(station, Record{1, fp_value, fp_value, fp_value});
            continue;
        }

        it->second.min = min(it->second.min, fp_value);
        it->second.max = max(it->second.max, fp_value);
        it->second.sum += fp_value;
        it->second.cnt++;
    }

    return db;
}

void format_output(const DB &db){
    vector<string> names;
    names.reserve(db.size());
    transform(db.begin(), db.end(), back_inserter(names), [](const auto& pair) {return pair.first; });
    sort(names.begin(), names.end());
    for(string &x : names){
        auto r = db.at(x);
        printf("%s=%.1f/%.1f/%.1f, ", x.c_str(), r.min, r.sum/r.cnt, r.max);
    }
}

int main(int argc, char* argv[]){
    if(argc < 2) {
        printf("Usage: %s <db_name>\n", argv[0]);
    }

    std::ifstream ifile(argv[1], ifile.in);
    if(!ifile.is_open()){
        throw runtime_error("Failed to open the input file.");
    }
    DB db = process_input(ifile);
    format_output(db);
    return 0;
}