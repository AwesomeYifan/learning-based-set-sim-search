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

void check_par_g(string path_to_sets, string path_to_groups) {
    LES3 les3(path_to_sets, path_to_groups);
    cout<<"PAR_G"<<endl;
    les3.testKNN(10);
}
void check_par_c(string path_to_sets, string path_to_groups) {
    LES3 les3(path_to_sets, path_to_groups);
    cout<<"PAR_C"<<endl;
    les3.testKNN(10);
}
void check_par_h(string path_to_sets, string path_to_groups) {
    LES3 les3(path_to_sets, path_to_groups);
    cout<<"PAR_H"<<endl;
    les3.testKNN(10);
}
void check_l2p(string path_to_sets, string path_to_groups) {
    LES3 les3(path_to_sets, path_to_groups);
    cout<<"L2P"<<endl;
    les3.testKNN(10);
}

int main(int argc, const char * argv[]) {
    
    string path_to_sets = "../datasets/kosarak/all.dat";
    
    string path_to_groups = "../datasets/kosarak/LES3";
    
    check_par_g(path_to_sets, path_to_groups);
   

}