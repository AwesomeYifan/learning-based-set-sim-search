
#ifndef INVERTED_INDEX_h
#define INVERTED_INDEX_h

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <ctime>
#include <queue>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <map>
#include <regex>
#include <cmath>
#include <sstream>
#include <filesystem>
#include "math.h"
#include "MEASURE.h"

using namespace std;

typedef pair<int, map<int, vector<int>>> block;
typedef pair<float, multiset<int>> entry;

struct InvertedIndex{
    string path;
    
    vector<vector<block>> inverted_index;
    Measure measure;
    vector<multiset<int>> database;
    int num_tokens;
    float step_size = 0.4;
    InvertedIndex() = default;
    InvertedIndex(string path) {
        read_database(path);
        construct_index();
       
    };
    void read_database(string path);
    void construct_index();
   
    int find_knn(multiset<int> query, int k);
    
    vector<int> find_candidates(multiset<int> query, float delta);
    
    void find_deltann(multiset<int> query, float delta);
    void testKNN(int k);
    void testDeltaNN(float delta);
    void print_a_set(multiset<int> query);
    int get_size_in_MB();
    
};
int InvertedIndex::get_size_in_MB() {
    int num_elements = 0;
    for(auto blocks : inverted_index) {
        for(auto block : blocks) {
            num_elements++;
            for(auto itr = block.second.cbegin(); itr != block.second.cend(); itr++) {
                num_elements++;
                num_elements += itr->second.size();
            }
           
        }
    }
    int size_in_MB = num_elements * 4 / 1024 / 1024;
    return size_in_MB;
}
void InvertedIndex::read_database(string path_to_sets){
    set<int> all_tokens;
    ifstream fin(path_to_sets);
    string line;
    while(getline(fin, line)) {
        istringstream line_stream(line);
        string temp;
        multiset<int> temp_set;
        while(line_stream>>temp) {
            int token = stoi(temp);
            all_tokens.insert(token);
            temp_set.insert(token);
        }
        database.push_back(temp_set);
    }
    fin.close();
    num_tokens = all_tokens.size();
}
void InvertedIndex::construct_index() {
    vector<map<int, map<int, vector<int>>>> raw_inverted_index(num_tokens);
    for(int set_id = 0; set_id < database.size(); set_id++){
        auto a_set = database[set_id];
        int set_size = a_set.size();
        int pos = 1;
        for(auto a_token : a_set) {
            if(raw_inverted_index[a_token].find(set_size) == raw_inverted_index[a_token].end()) {
                map<int, vector<int>> temp;
                raw_inverted_index[a_token].insert({set_size, temp});
            }
            if(raw_inverted_index[a_token][set_size].find(pos) == raw_inverted_index[a_token][set_size].end()) {
                vector<int> temp;
                raw_inverted_index[a_token][set_size].insert({pos, temp});
            }
            raw_inverted_index[a_token][set_size][pos].push_back(set_id);
            pos++;
        }
    }
    for(auto raw_blocks_of_token : raw_inverted_index) {
        vector<block> blocks_of_token;
        for(auto block_itr = raw_blocks_of_token.cbegin(); block_itr != raw_blocks_of_token.cend(); block_itr++) {
            int set_size = block_itr->first;
            auto pos_setid_of_block = block_itr->second;
            block this_block({set_size, pos_setid_of_block});
            blocks_of_token.push_back(this_block);
        }
        inverted_index.push_back(blocks_of_token);
    }
    
}

vector<int> InvertedIndex::find_candidates(multiset<int> query, float delta) {
    if(delta <= 0) {
        vector<int> result(database.size());
        iota(result.begin(), result.end(), 0);
        return result;
    }
    vector<int> results;
    int probe_length = int(floor(query.size() * (1.0 - delta) / (1.0 + delta))) + 1;
    probe_length = probe_length < query.size()? probe_length : query.size();
    auto token_itr = query.cbegin();
    for(int i = 1; i <= probe_length; i++) {
        int length_filter = int(floor((query.size() - (i - 1) * (delta + 1)) / (delta + 0.000000001)));
        for(auto block_itr = inverted_index[*token_itr].cbegin(); block_itr != inverted_index[*token_itr].cend(); block_itr++) {
            if(block_itr->first > length_filter) {
                break;
            }
            int pos_stop_at = int(ceil((block_itr->first + delta + 1 - delta * query.size()) / (1 + delta)));
            auto block = block_itr->second;
            for(auto set_itr = block.cbegin(); set_itr != block.cend(); set_itr++) {
                if(set_itr->first > pos_stop_at) {
                    break;
                }
                
                results.insert(results.end(), set_itr->second.begin(), set_itr->second.end());
            }
        }
        token_itr++;
    }
    return results;
}
void InvertedIndex::find_deltann(multiset<int> query, float delta) {
    vector<int> candidates = find_candidates(query, delta);
    
    // cout<<candidates.size()<<endl;
    vector<multiset<int>> result;
    for(auto set_id : candidates) {
        float sim = measure.computeSim(query, database[set_id]);
        if(sim >= delta) {
            result.push_back(database[set_id]);
        }
    }
}
int InvertedIndex::find_knn(multiset<int> query, int k) {
    
    float delta = 1.0;
    set<int> already_checked_sets;
    priority_queue<pair<float, multiset<int>>, vector<pair<float, multiset<int>>>, std::greater<pair<float, multiset<int>>> > result;
    while(delta > 0) {
        vector<int> candidates = find_candidates(query, delta);
        for(int set_id : candidates) {
            if(already_checked_sets.find(set_id) != already_checked_sets.end()) {
                continue;
            }
            already_checked_sets.insert(set_id);
            float sim = measure.computeSim(query, database[set_id]);
            if(result.size() >= k && sim < result.top().first) {
                continue;
            }
            result.push({sim, database[set_id]});
            if(result.size() > k) {
                result.pop();
            }
        }
        if(result.size() >= k && result.top().first >= delta) {
            return already_checked_sets.size();
            // return;
        }
        else {
            delta -= step_size;
        }
    }
    return 1;
}
void InvertedIndex::testKNN(int k) {
    float query_ratio = 1000.0 / database.size();
    int query_size = 0;
    int check_count = 0;
    auto start = chrono::system_clock::now();
    for(auto query : database) {
        float rand_v = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if(rand_v > query_ratio)
            continue;
        query_size++;
        check_count += find_knn(query, k);
    }
    auto end = chrono::system_clock::now();
    double total_time = (end-start).count();
    double avg_time_milliseconds = total_time/query_size/1000;
    cout<<"average search time: "<<avg_time_milliseconds<<endl;
    check_count /= query_size;
    // cout<<"total check count: "<<check_count<<endl;
}

void InvertedIndex::testDeltaNN(float delta) {
    float query_ratio = 1000.0 / database.size();
    int query_size = 0;
    auto start = chrono::system_clock::now();
    for(auto query : database) {
        float rand_v = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if(rand_v > query_ratio)
            continue;
        query_size++;
        find_deltann(query, delta);
    }
    auto end = chrono::system_clock::now();
    double total_time = (end-start).count();
    double avg_time_milliseconds = total_time/query_size/1000;
    cout<<"average search time: "<<avg_time_milliseconds<<endl;
}
#endif