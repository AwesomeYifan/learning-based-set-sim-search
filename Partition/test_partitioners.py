from functools import partial
from sklearn_extra.cluster import KMedoids
from sklearn.cluster import DBSCAN, AgglomerativeClustering
import numpy as np
from scipy import sparse
import time
import networkx as nx
import subprocess
from partitioners import PAR_H, PAR_C

def read_database(path):
    database = []
    with open(path + 'all.dat', "r") as read_file:
        while True:
            line = read_file.readline()
            if not line:
                break
            temp_line = [int(v) for v in line.strip().split(" ")]
            temp_line = set(temp_line)
            database.append(temp_line)
    return database

def write_graph(path):
    # tranform into hyper graph
    G = nx.Graph()
    read_file = open(path + "10NN", 'r')
    
    set_id = 0
    while True:
        line = read_file.readline()
        if not line:
            break
        G.add_node(set_id + 1)
        items = line.split()
        neighbors = [int(v) for v in items]
        for neighbor_id in neighbors:
            if set_id == neighbor_id:
                continue
            G.add_node(neighbor_id + 1)
            G.add_edge(set_id + 1, neighbor_id + 1)
        set_id += 1
    # print(len(G))
    cycles = [s for s in nx.find_cliques(G)]
    
    with open(path + "graph_cut_10NN", 'w') as write_file:
        write_file.write(str(len(cycles)) + " " + str(set_id))
        for cycle in cycles:
            write_file.write("\n" + ' '.join([str(v) for v in cycle]))
           
def organize_groups(path):
    read_file = open(path + "graph_cut_10NN.part970.epsilon0.03.seed-1.KaHyPar", 'r')
    lines = read_file.readlines()
    groups = dict()
    set_id = 0
    for line in lines:
        group_id = int(line)
        if group_id not in groups:
            groups[group_id] = []
        groups[group_id].append(set_id)
        set_id += 1
    
    with open(path + "PAR_G", 'w') as write_file:
        for K, list_of_sets in groups.items():
            for set_id in list_of_sets:
                write_file.write(str(set_id) + " ")
            write_file.write("\n")

path = './datasets/kosarak/'
database = read_database(path)
# num_sets = len(database)
num_clusters = 970 # note the number is 970, as L2P also has 970 groups due to early stop (see section 7.1, the last paragraph) 

# PAR_C
start_time = time.time()
partitioner = PAR_C(num_clusters)
partitioner.fit(database)

groups = partitioner.get_groups()
f = open(path + 'PAR_C', 'w')
for set_ids in groups:
    f.write(' '.join([str(v) for v in set_ids]) + '\n')
f.close()
total_time = time.time() - start_time
print(total_time)

# PAR_H
start_time = time.time()
partitioner = PAR_H(num_clusters)
partitioner.fit(database)

groups = partitioner.get_groups()
f = open(path + 'PAR_H', 'w')
for set_ids in groups:
    f.write(' '.join([str(v) for v in set_ids]) + '\n')
f.close()
total_time = time.time() - start_time
print(total_time)

#PAR_G
start_time = time.time()
write_graph(path)
program='./kahypar-master/KaHyPar-bin'
subprocess.call([program, '-h', path + 'graph_cut_10NN','-w','true','-k','970','-e','0.03','-o','cut','-m','recursive','-p','./kahypar-master/config/cut_rKaHyPar_sea20.ini'])
total_time = time.time() - start_time
organize_groups(path)
print(total_time)