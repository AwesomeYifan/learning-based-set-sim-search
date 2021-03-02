//
//  main.cpp
//  LES3
//

#include <iostream>
#include <fstream>
#include <vector>
#include <regex>
#include <chrono>
#include <ctime>
#include <set>
#include <map>
#include <sstream>
#include <filesystem>
#include "LES3.h"
#include "BRUTEFORCE.h"
#include "DualTrans.h"
#include "PARAM.h"
#include "InvertedIndex.h"
using namespace std;

void check_les3(string path_to_sets, string path_to_groups, vector<int> ks, vector<float> ds) {
    LES3 les3(path_to_sets, path_to_groups);
    cout<<"LES3"<<endl;
    cout<<"LES3 size: "<<les3.get_size_in_MB()<<endl;
    for(int k : ks) {
        cout<<k<<endl;
        les3.testKNN(k);
    }
    for(float d : ds) {
        cout<<d<<endl;
        les3.testDeltaNN(d);
    }
}

void check_dt(string path_to_sets, vector<int> ks, vector<float> ds) {
    DualTrans dt(path_to_sets);
    cout<<"DualTrans"<<endl;
    cout<<"DT size: "<<dt.get_size_in_MB()<<endl;
    for(int k : ks) {
        cout<<k<<endl;
        dt.testKNN(k);
    }
    for(float d : ds) {
        cout<<d<<endl;
        dt.testDeltaNN(d);
    }
}

void check_ii(string path_to_sets, vector<int> ks, vector<float> ds) {
    InvertedIndex invidx(path_to_sets);
    
    cout<<"Inverted index"<<endl;
    cout<<"invidx size: "<<invidx.get_size_in_MB()<<endl;
    for(int k : ks) {
        cout<<k<<endl;
        invidx.testKNN(k);
    }
    for(float d : ds) {
        cout<<d<<endl;
        invidx.testDeltaNN(d);
    }
}

void check_bf(string path_to_sets, vector<int> ks, vector<float> ds) {
    cout<<"Brute force"<<endl;
    BRUTEFORCE bf(path_to_sets);
    bf.testKNN(2);
}

int main(int argc, const char * argv[]) {
    
    string path_to_sets = "../datasets/kosarak/all.dat";
    
    string path_to_groups = "../datasets/kosarak/LES3";
    
    vector<int> ks({2,10,50,100});
    vector<float> ds({0.9,0.7,0.5,0.3,0.1});
    
    check_les3(path_to_sets, path_to_groups, ks, ds);
    check_ii(path_to_sets, ks, ds);
    check_dt(path_to_sets, ks, ds);
    check_bf(path_to_sets, ks, ds);

}