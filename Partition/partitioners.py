import numpy as np
import random

class PAR_A:
    def __init__(self, num_clusters):
        self.num_clusters = num_clusters
        self.labels_ = None
    
    def find_groups_to_merge(self, database, groups):
        min_group_id = 0
        min_group_size = len(groups[0])
        for i in range(len(groups)):
            if len(groups[i]) < min_group_size:
                min_group_id = i
                min_group_size = len(groups[i])
        other_group_id = 0
        min_sum_dist = np.inf
        for i in range(len(groups)):
            if i == min_group_id:
                continue
            temp_group_ids = groups[i].copy()
            temp_group_ids.extend(groups[min_group_id])
            new_group = Group()
            new_group.initialize(database, temp_group_ids)
            if min_sum_dist > new_group.sum_dist:
                other_group_id = i
                min_sum_dist = new_group.sum_dist
        return min(min_group_id, other_group_id), max(min_group_id, other_group_id)

    def fit(self, database):
        all_ids = list(range(len(database)))
        random.shuffle(all_ids)

        groups = [[i] for i in range(len(database))]
       

        while len(groups) > self.num_clusters:
            print(len(groups))

            g1, g2 = self.find_groups_to_merge(database, groups)
            
            new_group = groups[g1].copy()
            new_group.extend(groups[g2])
            
            del groups[g2]
            del groups[g1]
            groups.append(new_group)

        labels = [0]*len(database)
        for i in range(self.num_clusters):
            for set_id in groups[i].all_sets:
                labels[set_id] = i
        self.labels = labels
    
    def get_groups(self):
        groups = [[] for _ in range(self.num_clusters)]
        for i in range(len(self.labels)):
            group_id = self.labels[i]
            set_id = i
            groups[group_id].append(set_id)
        return groups

class PAR_D:
    def __init__(self, num_clusters):
        self.num_clusters = num_clusters
        self.labels_ = None
    
    def find_group_to_split(self, groups):
        max_dist = 0
        group_id = random.choice(list(range(len(groups))))
        for i in range(len(groups)):
            dist = groups[i].sum_dist
            if dist > max_dist:
                max_dist = dist
                group_id = i
        return group_id

    def fit(self, database):
        all_ids = list(range(len(database)))
        random.shuffle(all_ids)

        init_group = Group()
        init_group.initialize(database, all_ids)
        groups = [init_group]
       

        while len(groups) < self.num_clusters:
            print(len(groups))

            group_to_split = self.find_group_to_split(groups)
            
            star_set_id = groups[group_to_split].get_a_random_set()
            
            temp = Group()
            temp.initialize(database, [star_set_id])
            groups.append(temp)
            new_group = len(groups) - 1
           
            
            possible_sets_to_move = groups[group_to_split].all_sets.copy()
            for i in possible_sets_to_move:
                gpo_decrease = groups[group_to_split].get_overall_dist(database, i)
                gpo_increase = groups[new_group].get_overall_dist(database, i)
                if gpo_decrease > gpo_increase:
                    groups[group_to_split].remove(i)
                    groups[new_group].insert(i)
            
            groups[group_to_split].update_sum_dist(database)
            groups[new_group].update_sum_dist(database)
            random.shuffle(groups[group_to_split].all_sets)
            random.shuffle(groups[new_group].all_sets)
        
        labels = [0]*len(database)
        for i in range(self.num_clusters):
            for set_id in groups[i].all_sets:
                labels[set_id] = i
        self.labels = labels
    
    def get_groups(self):
        groups = [[] for _ in range(self.num_clusters)]
        for i in range(len(self.labels)):
            group_id = self.labels[i]
            set_id = i
            groups[group_id].append(set_id)
        return groups

class PAR_C:
    def __init__(self, num_clusters):
        self.num_clusters = num_clusters
        self.labels_ = None
        self.max_iter = 100
    
    def find_group_to_split(self, groups):
        max_dist = 0
        group_id = random.choice(list(range(len(groups))))
        for i in range(len(groups)):
            dist = groups[i].sum_dist
            if dist > max_dist:
                max_dist = dist
                group_id = i
        return group_id


    def fit(self, database):
        set_ids_by_group = [[] for _ in range(self.num_clusters)]
        for i in range(len(database)):
            group_id = random.randint(0, self.num_clusters-1)
            set_ids_by_group[group_id].append(i)
        groups = []
        for set_ids in set_ids_by_group:
            group = Group()
            group.initialize(database, set_ids)
            groups.append(group)

        for round in range(self.max_iter):
            removes = [[] for _ in range(self.num_clusters)]
            inserts = [[] for _ in range(self.num_clusters)]
            move_occurred = False

            count = 0
            for i in range(len(groups)):
            
                # print("group " + str(count))
                count+=1

                for set_id in groups[i].all_sets:
                    
                    gpo_decrease = groups[i].get_overall_dist(database, set_id)
                    j = i
                    while j == i:
                        j = random.choice(list(range(self.num_clusters)))
                    
                    gpo_increase = groups[j].get_overall_dist(database, set_id)
                   
                    if gpo_increase < gpo_decrease:
                        removes[i].append(set_id)
                        inserts[j].append(set_id)
                        move_occurred = True
                
            if not move_occurred:
                break
            for i in range(self.num_clusters):
                for set_id in removes[i]:
                    groups[i].remove(set_id)
                for set_id in inserts[i]:
                    groups[i].insert(set_id)
            
            sum_dist = 0
            for group in groups:
                group.update_sum_dist(database)
                sum_dist += group.sum_dist
            print("round " + str(round) + ", GPO: " + str(sum_dist))
        
        labels = [0]*len(database)
        for i in range(self.num_clusters):
            for set_id in groups[i].all_sets:
                labels[set_id] = i
        self.labels = labels
    
    def get_groups(self):
        groups = [[] for _ in range(self.num_clusters)]
        for i in range(len(self.labels)):
            group_id = self.labels[i]
            set_id = i
            groups[group_id].append(set_id)
        return groups

class Group:
    def __init__(self):
        self.all_sets = [] # save ids
        self.sum_dist = 0
        self.approx_factor = 0.01
        
    def initialize(self, database, set_ids):
        self.all_sets = set_ids.copy()
        sets_to_check = self.get_sets_to_check()
        overall_dist = 0
        for i in sets_to_check:
            for j in sets_to_check:
                num_common_tokens = len(database[i].intersection(database[j]))
                overall_dist += (1 - 2 * num_common_tokens / (len(database[i]) + len(database[j])))
        overall_dist = overall_dist / len(sets_to_check)**2 * len(self.all_sets)**2
        self.sum_dist = overall_dist
    
    def remove(self, set_id):
        self.all_sets.remove(set_id)

    def update_sum_dist(self, database):
        sets_to_check = self.get_sets_to_check()
        overall_dist = 0
        for i in sets_to_check:
            for j in sets_to_check:
                num_common_tokens = len(database[i].intersection(database[j]))
                overall_dist += (1 - 2 * num_common_tokens / (len(database[i]) + len(database[j])))
        overall_dist = overall_dist / len(sets_to_check)**2 * len(self.all_sets)**2
        self.sum_dist = overall_dist


    def insert(self, set_id):
        self.all_sets.append(set_id)
        
        
    def get_overall_dist(self, database, set_id):
        sets_to_check = self.get_sets_to_check()
        overall_dist = 0
        
        for i in sets_to_check:
            num_common_tokens = len(database[i].intersection(database[set_id]))
            overall_dist += (1 - 2 * num_common_tokens / (len(database[i]) + len(database[set_id])))
        overall_dist = overall_dist / len(sets_to_check) * len(self.all_sets)
        return overall_dist
    
    def get_a_random_set(self):
        return random.choice(self.all_sets)
    
    def get_sets_to_check(self):
        num_sets_to_check = int(len(self.all_sets) * self.approx_factor)
        num_sets_to_check = max(100, num_sets_to_check)
        num_sets_to_check = min(len(self.all_sets), num_sets_to_check)
        # num_sets_to_check = min(len(self.all_sets), 100)
        sets_to_check = random.sample(self.all_sets, k=num_sets_to_check)
        return sets_to_check
        