
def get_dist(set1, set2):
    common_tokens = set(set1).intersection(set(set2))
    all_tokens = set(set1).union(set(set2))
    dist = 1 - len(common_tokens) / len(all_tokens)
    
    return dist
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
database = read_database("./datasets/kosarak-sampled/all.dat")

for tech in ['PTR']:
    print("****************************")
    print(tech)
    for i in [3,4,5,6,7]:
        read_file = open("./datasets/kosarak-sampled/" + tech + "/all.dat-group-" + str(i), 'r')
        
        lines = read_file.readlines()
        groups = []
        for line in lines:
            items = line.split()
            items_int = [int(v) for v in items]
            groups.append(items_int)
        
        gpo = 0
        for group in groups:
            for x in group:
                for y in group:
                    gpo += get_dist(database[x],database[y])
        print(str(i) + ":" + str(gpo))