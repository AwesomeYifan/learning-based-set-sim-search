//
//  MEASURE.h
//  LES3
//

#ifndef MEASURE_h
#define MEASURE_h
#include <set>
#include <unordered_set>
#include <vector>
#include <cmath>
#include <forward_list>
// #include "roaring.hh"
// #include "roaring.c"
using namespace std;

struct Measure {
    
    static inline float computeSim(multiset<int> &set1, multiset<int> &set2) {
        float num_common_items = 0;
        auto itr1 = set1.begin();
        auto itr2 = set2.begin();
        while(itr1 != set1.end() && itr2 != set2.end()) {
            if(*itr1 < *itr2) ++itr1;
            else if(*itr1 > *itr2) ++ itr2;
            else {
                ++num_common_items;
                ++itr1;
                ++itr2;
            }
        }
        return num_common_items/(set1.size() + set2.size() - num_common_items);
    };
    
};

#endif /* MEASURE_h */
