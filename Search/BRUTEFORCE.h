//
//  BRUTEFORCE.h
//  LES3
//
#ifndef BRUTEFORCE_h
#define BRUTEFORCE_h
#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <chrono>
#include <time.h>
#include <queue>
#include <ctime>
#include <set>
#include <unordered_set>
#include <map>
#include <sstream>
#include <filesystem>
#include "MEASURE.h"
using namespace std;
typedef pair<float, multiset<int>> entry3;

struct BRUTEFORCE {
    vector<multiset<int>> database;
    BRUTEFORCE() = default;
    BRUTEFORCE(string path) {
        // cout<<"Reading data..."<<endl;
        auto start = chrono::system_clock::now();
        ifstream in(path);
        string line;
        while(getline(in, line)) {
            istringstream line_stream(line);
            string temp;
            multiset<int> temp_set;
            while(getline(line_stream, temp, ' ')) {
                temp_set.insert(stoi(temp));
            }
            database.push_back(temp_set);
        }
        in.close();
        auto end = chrono::system_clock::now();
        double total_time = (end-start).count()/1000;
        // cout<<"Time spent reading data: "<<total_time<<endl;
    }
    float testKNN(int k);
    float testDeltaNN(float delta);
    void constructSimGraph(int k, string path);
    float disk_based_knn_search(string path, multiset<int> query_set, int k);
    float disk_based_delta_search(string path, multiset<int> query_set, float delta);
};

float BRUTEFORCE::testKNN(int k) {
    clock_t t;
    float query_ratio = 1000.0 / database.size();
    int query_size = 0;
    Measure measure;
    auto start = chrono::system_clock::now();
    for(auto &query_set : database) {
        float rand_v = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if(rand_v > query_ratio)
            continue;
        priority_queue<pair<float, multiset<int>>, vector<pair<float, multiset<int>>>, std::greater<pair<float, multiset<int>>> > result;
        query_size++;
        for(auto &candidate_set : database) {
            float sim = measure.computeSim(query_set, candidate_set);
            if(result.size() >= k && sim < result.top().first)
                continue;
            result.push({sim, candidate_set});
            if(result.size() > k)
                result.pop();
        }
    }
    auto end = chrono::system_clock::now();
    double total_time = (end-start).count();
    double avg_time_milliseconds = total_time/query_size/1000;
    cout<<"Average search time: "<<avg_time_milliseconds<<endl;
    return avg_time_milliseconds;
}

float BRUTEFORCE::testDeltaNN(float delta) {
    float query_ratio = 1000.0 / database.size();
    int query_size = 0;
    Measure measure;
    auto start = chrono::system_clock::now();
    for(auto &query_set : database) {
        float rand_v = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if(rand_v > query_ratio)
            continue;
        vector<multiset<int>> result;
        query_size++;
        for(auto &candidate_set : database) {
            float sim = measure.computeSim(query_set, candidate_set);
            if(sim >= delta)
                result.push_back(candidate_set);
        }
    }
    auto end = chrono::system_clock::now();
    double total_time = (end-start).count();
    double avg_time_milliseconds = total_time/query_size/1000;
    cout<<avg_time_milliseconds<<endl;
    return avg_time_milliseconds;
}


float BRUTEFORCE::disk_based_knn_search(string path, multiset<int> query_set, int k) {
    Measure measure;
    ifstream in(path);
    string line;
    auto start = chrono::system_clock::now();
    priority_queue<entry3, vector<entry3>, greater<entry3> > result; 
    int idx = -1;
    while(getline(in, line)) {
        idx++;
        istringstream line_stream(line);
        string temp;
        multiset<int> temp_set;
        while(getline(line_stream, temp, ' ')) {
            temp_set.insert(stoi(temp));
        }
        float sim = measure.computeSim(query_set, temp_set);
        if(result.size() >= k && sim < result.top().first)
            continue;
        result.push({sim, temp_set});
        if(result.size() > k)
            result.pop();
    }
    in.close();
    auto end = chrono::system_clock::now();
    double total_time = (end-start).count()/1000;
    cout<<"brute-force takes "<<total_time<<endl;
    return total_time;
}

float BRUTEFORCE::disk_based_delta_search(string path, multiset<int> query_set, float delta) {
    Measure measure;
    ifstream in(path);
    string line;
    auto start = chrono::system_clock::now();
    vector<multiset<int>> result;
    int idx = -1;
    while(getline(in, line)) {
        idx++;
        istringstream line_stream(line);
        string temp;
        multiset<int> temp_set;
        while(getline(line_stream, temp, ' ')) {
            temp_set.insert(stoi(temp));
        }
        float sim = measure.computeSim(query_set, temp_set);
        if(sim >= delta)
            result.push_back(temp_set);
    }
    in.close();
    auto end = chrono::system_clock::now();
    double total_time = (end-start).count()/1000;
    cout<<"brute-force takes "<<total_time<<endl;
    return total_time;
}
#endif /* BRUTEFORCE_h */
