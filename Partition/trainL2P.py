import random
import time
import os
import pandas as pd
import torch
import torch.optim as optim
from torch.optim import lr_scheduler
from les3_datasets import SiameseDataset
from les3_losses import ContrastiveLoss
from les3_networks import SiameseNet, EmbeddingNetMLP
from les3_trainer import fit_siamese

path = "datasets/kosarak/all.dat"
num_sets = 990002

def initialize():
    num_init_groups = 128
    write_file = open(path + "-group-0", 'w')
    avg_group_size = int(num_sets / num_init_groups)
    for i in range(num_sets):
        write_file.write(str(i))
        if (i+1) % avg_group_size == 0:
            write_file.write("\n")
        else:
            write_file.write(' ')
    write_file.close()

def write_direct_group_file(group_file_suffix, trans):
    file = path + "-group-" + str(group_file_suffix)
    with open(file, 'a') as writeFile:
        writeFile.write(' '.join([str(v) for v in trans]) + "\n")

def compute_dist(point1, point2):
    common_tokens = set(point1).intersection(set(point2))
    result = 1.0 - 2.0 * len(common_tokens) / (len(point1) + len(point2))
    return result

def generate_rep_file():
    if len(trans) < partition_threshold:
        return
    # with open(path + "-representations-" + str(suffix), 'w') as writeFile:
    with open(path + "-representations-temp", 'w') as writeFile:
        for line in representations:
            writeFile.write(line + "\n")

def read_required_representations(trans):
    trans = set(trans)
    file = path + '-representations'
    with open(file, 'r') as readFile:
        current_line = -1
        while True:
            line = readFile.readline()
            current_line += 1
            #print(current_line)
            if not line:
                break
            if current_line in trans:
                representations.append(line.strip())
    return representations

def read_required_transactions(trans):
    trans = set(trans)
    with open(path) as csv_file:
        current_line = -1
        while True:
            line = csv_file.readline()
            current_line += 1
            #print(current_line)
            if not line:
                break
            if current_line in trans:
                transactions.append([int(v) for v in line.strip().split(" ")])

def generate_training_file(group_suffix):
    if len(trans) < partition_threshold:
        write_direct_group_file(group_suffix, trans)
        return
    file = path + "-training-temp"
    all_trans_identical = True
    sample_ratio = 200.0 / len(trans)
    # sample_ratio = 0.5
    with open(file, 'w') as writeFile:
        writeFile.write("P1,P2,dist\n")
        for i in range(len(transactions)):
            if random.random() > sample_ratio:
                continue
            for j in range(len(transactions)):
                if random.random() > sample_ratio or i == j:
                    continue
                two_trans_dist = compute_dist(transactions[i], transactions[j])
                
                if two_trans_dist != 0:
                    all_trans_identical = False
                writeFile.write(representations[i].split(",")[1] + ","
                                + representations[j].split(",")[1] + ","
                                + str(two_trans_dist) + "\n")
    if all_trans_identical:
        write_direct_group_file(group_suffix, trans)
        os.remove(file)

def cascade_training(new_group_suffix, DATASET):

    LEARNING_FILE = DATASET + '-training-temp'
    REPRESENTATION_FILE = DATASET + '-representations-temp'
    GROUP_FILE = DATASET + '-group-' + str(new_group_suffix)

    #file does not exist if the corresponding group is too small to partition
    #in which case the entire group is already written into the new file
    if not os.path.isfile(LEARNING_FILE):
        return
    n_epoch = 3
   

    params = {'batch_size': 256,
              'shuffle': True}
    loss_fn = ContrastiveLoss()

    # Load training set
    train_df = pd.read_csv(LEARNING_FILE, delimiter=',', encoding="utf-8-sig")

    training_samples = train_df[['P1', 'P2']]
    training_labels = train_df[['dist']]
    
    input_dim = len(training_samples['P1'][0].split())

    training_samples = training_samples.values
    training_labels = training_labels.values

    siamese_train_dataset = SiameseDataset(training_samples, training_labels)  
    siamese_train_loader = torch.utils.data.DataLoader(siamese_train_dataset, **params)

    embedding_net = EmbeddingNetMLP(input_dim, 1)

    model = SiameseNet(embedding_net)
    
    lr = 1e-2
    optimizer = optim.Adam(model.parameters(), lr=lr)

    scheduler = lr_scheduler.StepLR(optimizer, 8, gamma=0.1, last_epoch=-1)
    log_interval = 100
    patience = 3
    fit_siamese(siamese_train_loader, model, loss_fn, optimizer, scheduler, patience, n_epoch, log_interval)
    
    avg_prediction = 0.5
    first_set = []
    second_set = []
    with open(REPRESENTATION_FILE, 'r') as csv_file:
        lines = csv_file.readlines()
        for line in lines:
            line = line.split(",")
            id = line[0]
            rep = line[1]
            vec = [float(k) for k in rep.split(" ")]
            vec = torch.FloatTensor(vec)
            group = model.get_embedding(vec).data.numpy()[0]
            if group > avg_prediction:
                first_set.append(id)
            else:
                second_set.append(id)

    all_items = []
    if len(first_set) == 0:
        all_items = second_set.copy()
    elif len(second_set) == 0:
        all_items = first_set.copy()
    if len(first_set) == 0 or len(second_set) == 0:
        first_set.clear()
        second_set.clear()
        for item in all_items:
            if random.uniform(0, 1)< 0.5:
                first_set.append(item)
            else:
                second_set.append(item)

    
    # print(str(len(first_set)) + "-" + str(len(second_set)))
    with open(GROUP_FILE, 'a') as writeFile:
        if len(first_set) > 0:
            writeFile.write(' '.join(first_set) + "\n")
        if len(second_set) > 0:
            writeFile.write(' '.join(second_set) + "\n")
            
partition_threshold = 50
initialize()
with open(path+"-training_time", 'w') as cost_file:
    for current_group_suffix in range(0,5):
        start_time = time.time()
        if os.path.isfile(path + "-group-" + str(current_group_suffix + 2)):
            continue
        if os.path.isfile(path + "-group-" + str(current_group_suffix + 1)):
            os.remove(path + "-group-" + str(current_group_suffix + 1))
        next_group_suffix = current_group_suffix + 1
        # suffix = int(pow(2, current_group_suffix) - 1)
        file = path + "-group-" + str(current_group_suffix)
        with open(file, 'r') as readFile:
            while True:
                line = readFile.readline()
                if not line:
                    break
                trans = [int(id) for id in line.split(" ")]
                representations = list()
                transactions = list()
                read_required_representations(trans)
                read_required_transactions(trans)
                generate_rep_file()
                generate_training_file(next_group_suffix)
                # print("training model " + str(suffix))
                cascade_training(new_group_suffix=next_group_suffix, DATASET=path)
                
        training_time = time.time() - start_time
        cost_file.write(str(current_group_suffix) + "," +str(training_time) + "\n")
        cost_file.flush()