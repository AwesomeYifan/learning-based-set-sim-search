#ifndef RNODE_H
#define RNODE_H
// #include "Item.h"
#include "PARAM.h"
#include "MEASURE.h"
#include <float.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>
#include <set>
typedef pair<int, vector<int>> leaf_item;
using namespace std;
struct RNode{
    vector<vector<int>> range;
    RNode* parent;
    vector<RNode*> children;
    vector<leaf_item> data;
    RNode(){
        parent = nullptr;
        vector<int> temp;
        temp.push_back(INT_MAX);
        temp.push_back(0);
        for(int i = 0; i < 2*dim; i++) {
            range.push_back(temp);
        }
    };
    inline bool operator==(const RNode& rhs)
    {
        return this == &rhs;
    }
    RNode* insert(leaf_item item);
    RNode* insert(RNode* node);
    void clear();
    void split();
    int check();
    int count();
    int get_size_in_bytes();
    double dist_to_point(vector<int> point);
    double dist_to_rec(vector<vector<int>> rec);
    // double compute_UB(vector<double> point);
    void print();
    void print_range();
};
double dist_between_points(leaf_item item1, leaf_item item2) {
    double dist = 0;
    for(int i = 0; i < item1.second.size(); i++) {
        dist += abs(item1.second[i] - item2.second[i]);
    }
    return dist;
}
double dist_between_nodes(RNode n1, RNode n2) {
    double dist = 0;
    for(int i = 0; i < n1.range.size(); i++) {
        
        if(n1.range[i][0] > n2.range[i][1]) {
            dist += n1.range[i][0] - n2.range[i][1];
        }
       
        else if(n2.range[i][0] > n1.range[i][1]) {
            dist += n2.range[i][0] - n1.range[i][1];
        }
        
        else if(n1.range[i][0] < n2.range[i][1] && n1.range[i][1] > n2.range[i][0]) {
            dist += n2.range[i][0] - n1.range[i][1];
        }
        
        else if(n2.range[i][0] < n1.range[i][1] && n2.range[i][1] > n1.range[i][0]) {
            dist += n1.range[i][0] - n2.range[i][1];
        }
        
        else if(n1.range[i][0] < n2.range[i][0] && n1.range[i][1] > n2.range[i][1]) {
            dist += n2.range[i][1] - n2.range[i][0];
        }
        
        else {
            dist += n1.range[i][1] - n1.range[i][0];
        }
    }
    return dist;
}
int RNode::get_size_in_bytes() {
    int result = range.size() * 2 * 4;
    for(RNode* node : children) {
        result += node->get_size_in_bytes();
    }
    result += data.size() * 4;
    result += children.size() * 8;
    result += parent == nullptr? 0 : 8;
    return result;
}

// double compute_euc_dist(vector<double> p1, vector<double> p2) {
//     double result = 0;
//     for(int i = 0; i < p1.size(); i++) {
//         result += pow(p1[i] - p2[i], 2);
//     }
//     return sqrt(result);
// }

// double compute_rec_dist(vector<vector<double>> rec1, vector<vector<double>> rec2) {
//     double area1 = 1;
//     for(int i = 0; i < rec1.size(); i++) {
//         area1 *= (rec1[i][1] - rec1[i][0]);
//     }
//     double area2 = 1;
//     for(int i = 0; i < rec2.size(); i++) {
//         area2 *= (rec2[i][1] - rec2[i][0]);
//     }
//     double area3 = 1;
//     for(int i = 0; i < rec1.size(); i++) {
//         area3 *= (max(rec1[i][1], rec2[i][1]) - min(rec1[i][0], rec2[i][0]));
//     }
//     return area3 - area1 - area2;
// }
RNode* RNode::insert(leaf_item item) {
    for(int i = 0; i < range.size(); i++) {
        range[i][0] = min(range[i][0], item.second[i]);
        range[i][1] = max(range[i][1], item.second[i]);
    }
    if(children.size() > 0) {
        double min_dist = INT_MAX;
        RNode* node_to_insert = nullptr;
        for(auto node : children) {
            double dist = node->dist_to_point(item.second);
            if(dist < min_dist) {
                min_dist = dist;
                node_to_insert = node;
            }
        }
        node_to_insert->insert(item);
    }
    else {
        data.push_back(item);
        if(data.size() > 2 * rnode_min_size) {
            split();
        }
    }
    RNode* result = this;
    while(result->parent != nullptr) {
        result = result->parent;
    }
    return result;
}

RNode* RNode::insert(RNode* node) {
    children.push_back(node);
    for(int i = 0; i < 2*dim; i++) {
        range[i][0] = min(range[i][0], node->range[i][0]);
        range[i][1] = max(range[i][1], node->range[i][1]);
    }
    RNode* result = this;
    while(result->parent != nullptr) {
        result = result->parent;
    }
    return result;
}
void RNode::clear() {
    if(children.size() > 0) {
        children.clear();
    }
    if(data.size() > 0) {
        data.clear();
    }
    range.clear();
    vector<int> temp;
    temp.push_back(INT_MAX);
    temp.push_back(0);
    for(int i = 0; i < 2*dim; i++) {
        range.push_back(temp);
    }
}

void RNode::split() {
    RNode* sibling = new RNode;
    auto gen = std::bind(std::uniform_int_distribution<>(0,1),std::default_random_engine());
    if(data.size() > 0) {
        vector<leaf_item> all_points = data;
        clear();
        //find the furthest pair
        leaf_item center_1 = all_points[0];
        leaf_item center_2 = all_points[1];
        int max_dist = 0;
        for(int i = 0; i < all_points.size() - 1; i++) {
            for(int j = i + 1; j < all_points.size(); j++) {
                double dist = dist_between_points(all_points[i], all_points[j]);
                if(dist > max_dist) {
                    max_dist = dist;
                    center_1 = all_points[i];
                    center_2 = all_points[j];
                }
            }
        }
        
        insert(center_1);
        
        sibling->insert(center_2);

        //split the remaining points
        for(leaf_item item : all_points) {
            if(item.first == center_1.first || item.first == center_2.first) {
                continue;
            }
            double dist_1 = dist_between_points(item, center_1);
            double dist_2 = dist_between_points(item, center_2);
            
            if(dist_1 < dist_2) {
                insert(item);
            }
            else if(dist_1 > dist_2) {
                sibling->insert(item);
            }
            else {
                bool b = gen();
                if(b) { insert(item); }
                else {sibling->insert(item);}
            }
            
            // if(data.size() >= rnode_min_size) {
                
            //     for(int i = 0; i < all_points.size(); i++) {
                    
            //         sibling->insert(all_points[i]);
                   
            //     }
            //     //cout<<data.size()<<","<<sibling->data.size()<<endl;
            //     break;
                
            // }
            // else if(sibling->data.size() >= rnode_min_size) {
                
            //     for(int i = 0; i < all_points.size(); i++) {
                    
            //         insert(all_points[i]);
                    
            //     }
                
            //     break;
            // }
        }
        
    }
    else {
        
        vector<RNode*> all_children = children;
        clear();
        //find the pair of furthest children
        RNode* center_1 = all_children[0];
        RNode* center_2 = all_children[1];
        int max_dist = 0;
        for(int i = 0; i < all_children.size() - 1; i++) {
            for(int j = i + 1; j < all_children.size(); j++) {
                double dist = dist_between_nodes(*(all_children[i]), *(all_children[j]));
                if(dist > max_dist) {
                    max_dist = dist;
                    center_1 = all_children[i];
                    center_2 = all_children[j];
                }
            }
        }
        
        
        insert(center_1);
        
        sibling->insert(center_2);
        
        //split the remaining child nodes
        for(RNode* node : all_children) {
            if(node == center_1 || node == center_2) {
                continue;
            }
            double dist_1 = dist_between_nodes(*center_1, *node);
            double dist_2 = dist_between_nodes(*center_2, *node);
            
            if(dist_1 < dist_2) {
                insert(node);
            }
            else if(dist_1 > dist_2) {
                sibling->insert(node);
            }
            else {
                bool b = gen();
                if(b) { insert(node); }
                else {sibling->insert(node);}
            }
            
            // if(children.size() >= rnode_min_size) {
                
            //     for(int i = 0; i < all_children.size(); i++) {
                   
            //         sibling->insert(all_children[i]);
                   
            //     }
                
            //     break;
            // }
            // else if(sibling->children.size() >= rnode_min_size) {
                
            //     for(int i = 0; i < all_children.size(); i++) {
                    
            //         insert(all_children[i]);
            //     }
                
            //     break;
            // }
        }
    }
    if(parent != nullptr) {
        sibling->parent = parent;
        parent->insert(sibling);
        if(parent->children.size() > 2 * rnode_min_size) {
            parent->split();
        }
    }
    else {
        parent = new RNode;
        sibling->parent = parent;
        parent->insert(this);
        parent->insert(sibling);
    }
}

double RNode::dist_to_point(vector<int> point) {
    double dist = 0;
    for(int i = 0; i < point.size(); i++) {
        double temp = 0;
        if(point[i] < range[i][0]) {
            temp = pow(point[i] - range[i][0], 2);
        }
        else if(point[i] > range[i][1]) {
            temp = pow(point[i] - range[i][1], 2);
        }
        dist += temp;
    }
    return sqrt(dist);
}
// double RNode::dist_to_rec(vector<vector<double>> rec) {
//     return compute_rec_dist(range, rec);
// }

int RNode::check() {
    if(children.size() > 0) {
        int sum = 0;
        for(auto child : children) {
            sum += child->check();
        }
        return sum;
    }
    else {
        return data.size();
    }
}
int RNode::count() {
    int result = 1;
    for(auto node : children) {
        result += node->count();
    }
    return result;
}
void RNode::print() {
    cout<<"[";
    if(data.size() > 0) {
        for(auto p : data) {
            cout<<p.first<<",";
        }
    }
    else {
        for(auto child : children) {
            child->print();
        }
    }
    cout<<"]";
}
void RNode::print_range() {
    for(auto this_dim : range) {
        for(auto d : this_dim) {
            cout<<d<<" ";
        }
        cout<<endl;
    }
    for(auto child : children) {
        child->print_range();
    }
}

#endif