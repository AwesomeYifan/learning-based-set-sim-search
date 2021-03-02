//
//  TGM.h
//  LES3
//
#ifndef TGM_C_h
#define TGM_C_h
#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <chrono>
#include <ctime>
#include <set>
#include "roaring.hh"
#include "roaring.c"
#include <map>
#include <sstream>
#include <filesystem>
#include "MEASURE.h"
#include "PARAM.h"

struct TGM_C {
    vector<Roaring> bit_map;//group-token
    void construct(string path_to_sets, string path_to_groups);
    
    float getUB(multiset<int> query_set, int group_id);
    int insert(multiset<int> set_to_insert);
    int get_size();
};

void TGM_C::construct(string path_to_sets, string path_to_groups){
    vector<vector<int>> database;
    ifstream fin(path_to_sets);
    string line;
    while(getline(fin, line)) {
        istringstream sin(line);
        string temp;
        vector<int> temp_set;
        while(sin>>temp) {
            temp_set.push_back(stoi(temp));
        }
        database.push_back(temp_set);
    }
    fin.close();
    vector<vector<int>> groups;
    fin = ifstream(path_to_groups);
    while(getline(fin, line)) {
        istringstream sin(line);
        string temp;
        vector<int> a_group;
        while(sin>>temp) {
            a_group.push_back(stoi(temp));
        }
        groups.push_back(a_group);
    }
    
    for(int group_id = 0; group_id < groups.size(); group_id++) {
        Roaring r;
        for(int set_id : groups[group_id]) {
            for(int token : database[set_id])
                r.add(token);
        }
        bit_map.push_back(r);
    }
    
}
int TGM_C::get_size() {
    int total_size = 0;
    for(auto r : bit_map) {
        total_size += r.getSizeInBytes();
    }
    return total_size/1024/1024;
}
float TGM_C::getUB(multiset<int> query_set, int group_id) {
    // return Measure::computeUB(query_set, bit_map, group_id);
    int num_common_tokens = 0;
    for(int token : query_set) {
        if(bit_map[group_id].contains(token))
            num_common_tokens += 1;
    }
    return ((float)num_common_tokens) / query_set.size();
}

int TGM_C::insert(multiset<int> set_to_insert) {
    float max_UB = 0.0;
    int group_with_max_UB = 0;
    for(int i = 0; i < num_groups; i++) {
        float ub = getUB(set_to_insert, i);
        if(ub > max_UB) {
            max_UB = ub;
            group_with_max_UB = i;
        }
    }
    
    for(auto token : set_to_insert) {
        bit_map[group_with_max_UB].add(token);
    }
    return group_with_max_UB;
}
#endif /* TGM_h */
