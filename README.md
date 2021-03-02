# learning-based-set-sim-search
The repository is being updated.
## Structure
Folder `Partition` contains the implementation of all partitioning methods (L2P, PAR_G, PAR_C, PAR_H) and embedding methods (PTR, PCA, MDS, doc2vec). Folder `Search` contains the implementation of set similarity search methods, including LES3, DualTrans, InvIdx, and the Brute-force method.

## Datasets
The links to the five datasets used in the paper:
* KOSARAK: http://fimi.uantwerpen.be/data/
* LIVEJ: http://socialnetworks.mpi-sws.org/data-imc2007.html
* AOL: http://www.cim.mcgill.ca/~dudek/206/Logs/AOL-user-ct-collection/
* FS: https://snap.stanford.edu/data/com-Friendster.html
* PMC: http://academictorrents.com/details/06d6badd7d1b0cfee00081c28fddd5e15e106165
* 
Process the raw dataset downloaded from above websites with `Partition/process_database.py`, which includes tokenization (so that tokens are labeled 1, 2, 3,...) and sorting (details given in Section 7.1, paragraph `initialization`), and a file named `all.dat` will be produced. The KOSARAK dataset and the sampled KOSARAK used in Section 7.3 are given in folder `datasets`.
## To reproduce the results
### L2P
`Partition/trainL2P.py` implements the proposed learn2partition method. It takes two input files: `all.dat` which is the dataset to be partitioned, and `all.dat-representation` which include the embedding representations of the sets to be partitioned. The output of the program includes a list of files `all.dat-group-X`, where X are non-negative intergers, containing the partitioning results at level X of the cascade framework.
### Test embedding
We give our partitioning results corresponding to different embedding methods (`PTR`, `PCA`, `MDS`, and `doc2vec`) in folder `datasets/kosarak-sampled/`. You can execute file `Partition/check_GPO.py` to compare the quality of different embedding methods. Or you can generate the embedding the train the model for partitioning from scratch. Simply execute the file `Partition/test_embedding.py` and four set representation files with name `all.dat-rep-XXX` will be produced in folder `datasets/kosarak-sampled/`, where `XXX` denotes `PTR`, `PCA`, `MDS`, or `doc2vec`. Use a file as the input representation file of `Partition/trainL2P.py` and get the corresponding partitioning results.
### Test partitioning
We give our partitioning results corresponding to different partitioning methods (`L2P`, `PAR_G`, `PAR_C`, `PAR_H`) in folder `datasets/kosarak-sampled/`. Use file `Search/test_partition.cpp` to compare the quality of different partitioning methods. You can also perform the partitioning from scratch. The partitioning results of L2P can be produced by file `Partition/trainL2P.py`. The partitioning results of PAR_G, PAR_C, PAR_H can be produced by file `Partition/test_partitioners.py`.
### Test search
Execute file `Search/test_search.cpp` to get the results (index size and search time) of all set similarity search methods (`LES3`, `DualTrans`, `InvIdx`, `Brute-Force`).
