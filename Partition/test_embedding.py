from enum import auto
import math
import random
import numpy as np
from sklearn.decomposition import PCA
from mds import MDS,landmark_MDS
from gensim.models.doc2vec import Doc2Vec, TaggedDocument
import time
#token id must start from 0

def read_database(path):
    database = []
    with open(path, "r") as read_file:
        while True:
            line = read_file.readline()
            if not line:
                break
            temp_line = [int(v) for v in line.strip().split(" ")]
            database.append(temp_line)
    return database

def read_database_doc(path):
    database = []
    with open(path, "r") as read_file:
        while True:
            line = read_file.readline()
            if not line:
                break
            temp_line = line.strip().split(" ")
            database.append(temp_line)
    return database

def partition_based_rep(numTokens):
    reps_of_tokens = list()
    mid_value = numTokens / 2
    num_iterations = int(math.ceil(math.log(numTokens, 2)))
    for t in range(numTokens):
        rep_of_t = []
        for i in range(num_iterations):
            indicator = int(math.floor(t / (mid_value / math.pow(2, i)))) % 2
            if indicator == 0:
                rep_of_t.append(1)
                rep_of_t.append(0)
            else:
                rep_of_t.append(0)
                rep_of_t.append(1)
        reps_of_tokens.append(rep_of_t)
    return reps_of_tokens

def write_reps(path, reps_of_tokens):
    max_rep_num = 1
    counter = 0
    with open(path) as csv_file:
        while True:
            line = csv_file.readline()
            if not line:
                break
            transaction = [int(v) for v in line.strip().split(" ")]
            counter += 1
            for i in range(len(reps_of_tokens[0])):
                sum = 0
                for token in transaction:
                    if reps_of_tokens[int(token)][i] == 1:
                        sum += 1
                max_rep_num = max(max_rep_num, sum)

    with open(path + '-rep-PTR', 'w') as writeFile:
        idx = 0
        with open(path) as csv_file:
            while True:
                line = csv_file.readline()
                if not line:
                    break
                transaction = [int(v) for v in line.strip().split(" ")]
                # print('set ' + str(idx))
                rep = []
                for i in range(len(reps_of_tokens[0])):
                    sum = 0
                    for token in transaction:
                        if reps_of_tokens[int(token)][i] == 1:
                            sum += 1
                    rep.append(sum/max_rep_num)
                writeFile.write(str(idx) + ',' + ' '.join(str(_rep) for _rep in rep) + '\n')
                idx += 1
    
def compute_jac_dist(set1, set2):
    common_tokens = set(set1).intersection(set(set2))
    dist = 1 - len(common_tokens) / (len(set1) + len(set2) - len(common_tokens))
    return dist

def build_landmark_matrix(database):
    num_landmarks = 5000
    landmarks = random.sample(range(0,len(database)),num_landmarks)
    distance_matrix = list()
    for i in landmarks:
        dists = list()
        for j in range(0, len(database)):
            dist = compute_jac_dist(database[i], database[j])
            dists.append(dist)
        distance_matrix.append(dists)
    landmarks = np.array(landmarks)
    distance_matrix = np.array(distance_matrix)
    return landmarks, distance_matrix

def build_PTR_rep(path):
    reps_of_tokens = partition_based_rep(num_tokens)
    write_reps(path, reps_of_tokens)

def build_PCA_rep(path):
    database = np.zeros((num_sets, num_tokens))
    read_file = open(path, 'r')
    lines = read_file.readlines()
    set_id = 0
    for line in lines:
        tokens = line.split()
        for token in tokens:
            database[set_id][int(token)] += 1
        set_id += 1
    encoding_length = 2*int(math.ceil(math.log(num_tokens, 2)))
    pca = PCA(n_components=encoding_length, svd_solver='auto')
    results = pca.fit_transform(database)
    write_file = open(path + '-rep-PCA', 'w')
    for i in range(len(results)):
        write_file.write(str(i) + "," + ' '.join([str(v) for v in results[i]]) + "\n")
    write_file.close()

def build_MDS_rep(path):
    database = read_database(path)
    landmarks, distance_matrix = build_landmark_matrix(database)
    encoding_length = 2*int(math.ceil(math.log(num_tokens, 2)))
    results = landmark_MDS(distance_matrix,landmarks,encoding_length)
    write_file = open(path + '-rep-MDS', 'w')
    for i in range(len(results)):
        write_file.write(str(i) + "," + ' '.join([str(v) for v in results[i]]) + "\n")
    write_file.close()


def build_doc2vec_rep(path):
    database = read_database_doc(path)
    tagged_database = [TaggedDocument(doc, [i]) for i, doc in enumerate(database)]
    encoding_length = 2*int(math.ceil(math.log(num_tokens, 2)))
    model = Doc2Vec(tagged_database, vector_size=encoding_length, min_count=1)
    write_file = open(path + '-rep-doc2vec', 'w')
    for i in range(len(database)):
        vector = model.infer_vector(database[i])
        write_file.write(str(i) + "," + ' '.join([str(v) for v in vector]) + "\n")
    write_file.close()

# kos 5p
num_sets = 49482
num_tokens = 18735

path = "datasets/kosarak-5percent/all.dat"
start_time = time.time()
build_PTR_rep(path)
# build_PCA_rep(path)
# build_MDS_rep(path)
# build_doc2vec_rep(path)
end_time = time.time()
total_time = end_time - start_time
print(total_time)