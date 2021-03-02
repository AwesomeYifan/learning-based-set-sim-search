#ifndef RTREE_H
#define RTREE_H
#include "RNode.h"
#include "MEASURE.h"
#include "PARAM.h"
#include <map>
#include <vector>
#include <chrono>
#include <ctime>
#include <queue>
#include <string>
#include <fstream>
#include <sstream>
typedef pair<int, vector<int>> leaf_item;
typedef pair<float, multiset<int>> entry;

struct DualTrans{
    vector<multiset<int>> database;
    vector<set<int>> global_groups;
    vector<set<int>> global_dual_groups;
    RNode* root;
    DualTrans() = default;
    DualTrans(string path);
    // void insert(leaf_item item);
    void check_correctness();
    vector<int> get_rep(multiset<int> trans);
    int findKNN(multiset<int> trans, int k);
    void findDeltaNN(multiset<int> trans, double delta);
    vector<set<int>> getGreedyGroups(map<int, int> token_freqs);
    vector<set<int>> getDualGroups(vector<set<int>> groups, map<int, int> token_freqs);
    void write_groups(vector<set<int>> group1, vector<set<int>> group2);
    map<int, int> read_data(string path);
    double testKNN(int k);
    double testDeltaNN(float d);
    double compute_UB(RNode* node, vector<int> coords);
    int get_size_in_MB();
};
DualTrans::DualTrans(string path) {
    root = new RNode;
    
    map<int, int> token_freqs = read_data(path);
    global_groups = getGreedyGroups(token_freqs);
    global_dual_groups = getDualGroups(global_groups, token_freqs);
    // write_groups(global_groups, global_dual_groups);
    // cout<<"tokens written";
    // cin.get();
    for(int i = 0; i < database.size(); i++) {
        
        leaf_item item({i, get_rep(database[i])});
        root = root->insert(item);
        
    }
   
}
int DualTrans::get_size_in_MB(){
    int result = root->get_size_in_bytes();
    for(auto g : global_groups) {
        result += g.size() * 4 * 2;
    }
    return result / 1024 / 1024;
}
void DualTrans::write_groups(vector<set<int>> greedyGroup, vector<set<int>> dualgroup) {
    ofstream fout("/Users/yifan/Projects/L2P/datasets/email/token-groups");
    for(auto group : greedyGroup) {
        for(auto itr = group.begin(); itr != group.end(); itr++) {
            fout<<*itr;
            if(itr != --group.end()) {
                fout<<' ';
            }
        }
        fout<<endl;
    }
    // for(auto group : dualgroup) {
    //     for(auto itr = group.begin(); itr != group.end(); itr++) {
    //         fout<<*itr;
    //         if(itr != --group.end()) {
    //             fout<<' ';
    //         }
    //     }
    //     fout<<endl;
    // }
}


vector<int> DualTrans::get_rep(multiset<int> trans) {
    vector<int> rep;
    for(int i = 0; i < dim; i++) {
        int count1 = 0;
        int count2 = 0;
        for(auto token : trans) {
            if(global_groups[i].find(token) != global_groups[i].end()) {
                count1++;
            }
            if(global_dual_groups[i].find(token) != global_dual_groups[i].end()) {
                count2++;
            }
        }
        rep.push_back(count1);
        rep.push_back(count2);
    }
    return rep;
}
map<int, int> DualTrans::read_data(string path_to_sets) {
    map<int, int> token_freqs;
    ifstream in(path_to_sets);
    string line;
    while(getline(in, line)) {
        istringstream line_stream(line);
        string temp;
        multiset<int> temp_set;
        while(line_stream>>temp) {
            int token = stoi(temp);
            temp_set.insert(token);
            if(token_freqs.find(token) != token_freqs.end()) {
                token_freqs.find(token)->second++;
            }
            else {
                token_freqs.insert({token, 1});
            }
        }
        database.push_back(temp_set);
    }
    in.close();
    return token_freqs;
}
vector<set<int>> DualTrans::getGreedyGroups(map<int, int> token_freqs) {
    multimap<int, int> freq_tokens;
    for(auto itr = token_freqs.begin(); itr != token_freqs.end(); itr++) {
        freq_tokens.insert({itr->second, itr->first});
    }
    
    vector<set<int>> groups(dim);
    vector<int> freq_counts(dim);
    for(auto itr = freq_tokens.rbegin(); itr != freq_tokens.rend(); itr++) {
        int min_idx = 0;
        int min_count = INT_MAX;
        for(int i = 0; i < freq_counts.size(); i++) {
            if(freq_counts[i] < min_count) {
                min_count = freq_counts[i];
                min_idx = i;
            }
        }
        groups[min_idx].insert(itr->second);
        freq_counts[min_idx] += itr->first;
    }
    
    return groups;
}
vector<set<int>> DualTrans::getDualGroups(vector<set<int>> groups, map<int, int> token_freqs) {
    vector<set<int>> dual_groups(dim);
    vector<int> freq_counts(dim);
    for(auto current_group : groups) {
        map<int, int> temp_token_freqs;
        for(int token : current_group) {
            temp_token_freqs.insert({token, token_freqs.find(token)->second});
        }
        vector<set<int>> splitted_group = getGreedyGroups(temp_token_freqs);
        vector<int> skip(dim);
        for(auto current_sub_group : splitted_group) {
            int min_idx = 0;
            int min_count = INT_MAX;
            for(int i = 0; i < freq_counts.size(); i++) {
                if(skip[i]) {
                    continue;
                }
                if(freq_counts[i] < min_count) {
                    min_count = freq_counts[i];
                    min_idx = i;
                }
            }
            for(int token : current_sub_group) {
                dual_groups[min_idx].insert(token);
                freq_counts[min_idx] += token_freqs.find(token)->second;
            }
            skip[min_idx] = 1;
        }
    }
    return dual_groups;
}
int DualTrans::findKNN(multiset<int> trans, int k) {
    vector<int> rep = get_rep(trans);
    priority_queue<pair<float, RNode*>> candidates;
    candidates.push({1.0, root});
    priority_queue<pair<float, multiset<int>>, vector<pair<float, multiset<int>>>, std::greater<pair<float, multiset<int>>> > result;
    int check_count = 0;
    while(candidates.size() > 0) {
        RNode* next = candidates.top().second;
        candidates.pop();
        if(next->children.size() > 0) {
            for(auto child : next->children) {
                double ub = compute_UB(child, rep);
                candidates.push({ub, child});
            }
        }
        else {
            for(auto item : next->data) {
                check_count++;
                double sim = Measure::computeSim(trans, database[item.first]);
                if(result.size() >= k && sim < result.top().first) {
                    continue;
                }
                result.push({sim, database[item.first]});
                if(result.size() > k)
                    result.pop();
            }
        }
        if(result.size() == k && result.top().first >= candidates.top().first) {
            break;
        }
    }
    return check_count;
    // return result;
}
void DualTrans::findDeltaNN(multiset<int> trans, double delta) {
    vector<int> rep = get_rep(trans);
    multimap<double, RNode*> candidates;
    candidates.insert({1.0, root});
    vector<multiset<int>> result;
    while(candidates.size() > 0) {
        if(candidates.rbegin()->first < delta) {
            break;
        }
        RNode* next = candidates.rbegin()->second;
        candidates.erase(prev(candidates.end()));
        if(next->children.size() > 0) {
            for(auto child : next->children) {
                double ub = compute_UB(child, rep);
                candidates.insert({ub, child});
            }
        }
        else {
            for(auto item : next->data) {
                double sim = Measure::computeSim(trans, database[item.first]);
                if(sim >= delta) {
                    result.push_back(database[item.first]);
                }
            }
        }
    }
    // return result;
}
double DualTrans::testKNN(int k) {
    float query_ratio = 1000.0 / database.size();
    int query_size = 0;
    int check_count = 0;
    auto start = chrono::system_clock::now();
    for(auto &query_set : database) {
        float rand_v = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            //cout<<rand_v<<endl;
            if(rand_v > query_ratio)
                continue;
            check_count += findKNN(query_set, k);
            query_size++;
    }
    auto end = chrono::system_clock::now();
    double total_time = (end-start).count();
    double avg_time_milliseconds = total_time/query_size/1000;
    double avg_check_count = double(check_count)/query_size/database.size();
    cout<<"average search time: "<<avg_time_milliseconds<<endl;
    // cout<<"average check percentage: "<<avg_check_count<<endl;
    return avg_time_milliseconds;
}
double DualTrans::testDeltaNN(float d) {
    float query_ratio = 1000.0 / database.size();
    int query_size = 0;
    auto start = chrono::system_clock::now();
    for(auto &query_set : database) {
        float rand_v = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
            //cout<<rand_v<<endl;
            if(rand_v > query_ratio)
                continue;
            findDeltaNN(query_set, d);
            query_size++;
    }
    auto end = chrono::system_clock::now();
    double total_time = (end-start).count();
    double avg_time_milliseconds = total_time/query_size/1000;
    cout<<"average search time: "<<avg_time_milliseconds<<endl;
    return avg_time_milliseconds;
}
double DualTrans::compute_UB(RNode* node, vector<int> point) {
    int num_common_tokens = 0;
    int num_total_tokens = 0;
    for(int i = 0; i < dim; i++) {
        if(point[i] < node->range[i][0]) {
            num_common_tokens += point[i];
            num_total_tokens += node->range[i][0];
        }
        else if(point[i] > node->range[i][1]) {
            num_common_tokens += node->range[i][1];
            num_total_tokens += point[i];
        }
        else {
            num_common_tokens += point[i];
            num_total_tokens += point[i];
        }
    }
    double ub1 = double(num_common_tokens) / num_total_tokens;

    num_common_tokens = 0;
    num_total_tokens = 0;
    for(int i = dim; i < 2*dim; i++) {
        if(point[i] < node->range[i][0]) {
            num_common_tokens += point[i];
            num_total_tokens += node->range[i][0];
        }
        else if(point[i] > node->range[i][1]) {
            num_common_tokens += node->range[i][1];
            num_total_tokens += point[i];
        }
        else {
            num_common_tokens += point[i];
            num_total_tokens += point[i];
        }
    }
    double ub2 = double(num_common_tokens) / num_total_tokens;
    return min(ub1, ub2);
}
#endif