//
//  LES3.h
//  LES3
//

#ifndef LES3_h
#define LES3_h

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <ctime>
#include <queue>
#include <set>
#include <unordered_set>
#include <map>
#include <sstream>
#include <filesystem>
#include "MEASURE.h"
#include "TGM_C.h"
using namespace std;
// typedef pair<float, multiset<int>> entry;
// typedef pair<float, int> entry_construct;
struct LES3 {
    // TGM tgm;
    
    int num_groups;
    TGM_C tgm;
    Measure measure;
    int total_num_sets;
    // vector<set<int>> database;
    vector<vector<multiset<int>>> grouped_database;
    vector<multiset<int>> database;
    vector<vector<int>> groups;
    LES3() = default;
    LES3(string path_to_sets, string path_to_groups) {
        preprocess(path_to_sets, path_to_groups);
        tgm.construct(path_to_sets, path_to_groups);
    };
    
    int findKNN(multiset<int> &query_set, int k);
    int findDeltaNN(multiset<int> &query_set, float delta);
    void preprocess(string path_to_sets, string path_to_groups);
    void insert_helper(multiset<int> &set_to_insert);
    int get_size_in_MB();
    void insert(string path, float insert_ratio);
    double testKNN(int result_size);
    double testDeltaNN(float delta);
   
};
void LES3::preprocess(string path_to_sets, string path_to_groups) {
    // vector<multiset<int>> database;
    ifstream in(path_to_sets);
    string line;
    while(getline(in, line)) {
        istringstream line_stream(line);
        string temp;
        multiset<int> temp_set;
        while(line_stream>>temp) {
            temp_set.insert(stoi(temp));
        }
        database.push_back(temp_set);
    }
    in.close();
    total_num_sets = database.size();

    in.open(path_to_groups);
    int group_id = 0;
    while(getline(in, line)) {
        vector<int> current_group;
        istringstream line_stream(line);
        string temp;
        while(line_stream>>temp) {
            int set_id = stoi(temp);
            current_group.push_back(set_id);
        }
        groups.push_back(current_group);
        group_id++;
    }
   
    //construct grouped database
    for(int i = 0; i < groups.size(); i++) {
        vector<multiset<int>> this_group;
        for(int set_id : groups[i]) {
            this_group.push_back(database[set_id]);
        }
        grouped_database.push_back(this_group);
    }
    in.close();
    // database.clear();
    
    // num_groups = groups.size();
    // cout<<"num groups: "<<num_groups<<endl;
   
}
int LES3::get_size_in_MB() {
    return tgm.get_size();
}
int LES3::findKNN(multiset<int> &query_set, int k) {
    int check_counts = 0;
    priority_queue<pair<float, int> > candidate_groups;
    for(unsigned j = 0; j < num_groups; j++) {
        float ub = tgm.getUB(query_set, j);
        candidate_groups.push({ub, j});
    }
    
    priority_queue<pair<float, multiset<int>>, vector<pair<float, multiset<int>>>, std::greater<pair<float, multiset<int>>> > result;
    while(!candidate_groups.empty()) {
        if(result.size() == k && result.top().first >= candidate_groups.top().first) {
            break;
        }
        for(auto &candidate_set : grouped_database[candidate_groups.top().second]) {
            check_counts++;
            
            float sim = measure.computeSim(query_set, candidate_set);
            if(result.size() >= k && sim < result.top().first)
                continue;
            
            result.push({sim,candidate_set});
            if(result.size() > k)
                result.pop();
        }
        candidate_groups.pop();
    }
    // cout<<check_counts<<endl;
    return check_counts;
}

int LES3::findDeltaNN(multiset<int> &query_set, float delta) {
    int check_counts = 0;
    priority_queue<pair<float, int> > candidate_groups;
    for(unsigned j = 0; j < num_groups; j++) {
        float ub = tgm.getUB(query_set, j);
        candidate_groups.push({ub, j});
    }
    vector<multiset<int>> result;
    while(!candidate_groups.empty() && candidate_groups.top().first >= delta) {
        // total_checked_count++;
        for(auto &candidate_set : grouped_database[candidate_groups.top().second]) {
            check_counts++;
           
            float sim = measure.computeSim(query_set, candidate_set);
            if(sim >= delta)
                result.push_back(candidate_set);
        }
        candidate_groups.pop();
    }
    return check_counts;
}


double LES3::testKNN(int result_size) {
    float query_ratio = 1000.0 / total_num_sets;
    int query_size = 0;
    unsigned total_checked_count = 0;
   
    auto start = chrono::system_clock::now();
    for(int i = 0; i < groups.size(); i++) {
        for(auto &query_set : grouped_database[i]) {
            float rand_v = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            if(rand_v > query_ratio)
                continue;
            query_size++;
            total_checked_count += findKNN(query_set, result_size);
        }   
    }
    auto end = chrono::system_clock::now();
    double total_time = (end-start).count();
    double avg_time_milliseconds = total_time/query_size/1000;
    // float avg_checked_count = total_checked_count / query_size;
   
    // cout<<"average checked sets: "<<avg_checked_count<<endl;
    
    cout<<"average search time: "<<avg_time_milliseconds<<endl;
    return avg_time_milliseconds;
}

double LES3::testDeltaNN(float delta) {
    float query_ratio = 1000.0 / total_num_sets;
    int query_size = 0;
    auto start = chrono::system_clock::now();
    unsigned total_checked_count = 0;
    // double ub_sum = 0;
    for(int i = 0; i < groups.size(); i++) {
        for(auto &query_set : grouped_database[i]) {

            float rand_v = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            if(rand_v > query_ratio)
                continue;
            query_size++;
            total_checked_count += findDeltaNN(query_set, delta);
        }
    }
    auto end = chrono::system_clock::now();
    double total_time = (end-start).count();
    double avg_time_milliseconds = total_time/query_size/1000;
    unsigned avg_checked_count = total_checked_count / query_size;
    // double avg_ub_sum = ub_sum / query_size;
    // cout<<"average checked sets: "<<avg_checked_count<<endl;
    // cout<<"average upper bounds: "<<avg_ub_sum<<endl;
    cout<<"average search time: "<<avg_time_milliseconds<<endl;
    return avg_time_milliseconds;
}

#endif /* LES3_h */
