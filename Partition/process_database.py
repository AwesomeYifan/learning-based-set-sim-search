from collections import defaultdict
import functools
import random

def compare(set1, set2):
    length = min(len(set1), len(set2))
    for i in range(length):
        if set1[i] < set2[i]:
            return -1
        elif set1[i] > set2[i]:
            return 1
    if len(set1) < len(set2):
        return -1
    else:
        return 1

def clean_database(database): 
    # map tokens to 0, 1, 2,...
    token_dict = dict()
    new_token_id = 0
    for a_set in database:
        for token in a_set:
            if token not in token_dict.keys():
                token_dict[token] = new_token_id
                new_token_id += 1
    print("num. tokens: " + str(new_token_id))
    new_database = []
    for a_set in database:
        new_database.append([token_dict[token] for token in a_set])
    return new_database

def sort_database(database):
    database = sorted(database, key=functools.cmp_to_key(compare))
    return database

def read_database(path):
    database = []
    with open(path, "r") as read_file:
        while True:
            line = read_file.readline()
            if not line:
                break
            temp_line = [int(v) for v in line.strip().split(" ")]
            temp_line.sort()
            database.append(temp_line)
    return database

dir = "datasets/kosarak/"
database = read_database(dir + "origin.dat")
database = clean_database(database)
database = sort_database(database)

with open(dir + "all.dat", 'w') as write_file:
    for _set in database:
        write_file.write(' '.join([str(v) for v in _set]) + "\n")
