import numpy as np
import torch
from torch.utils.data import Dataset

class SiameseDataset(Dataset):
    def __init__(self, X, labels):
        self.X = X
        self.labels = labels
        
    def __len__(self):
        # return len(self.X)
        return len(self.labels)

    def __getitem__(self, idx):
        left = torch.FloatTensor([float(i) for i in str(self.X[idx][0]).split()])
        #print(left)
        right = torch.FloatTensor([float(i) for i in str(self.X[idx][1]).split()])
        # force = float(self.labels[idx])
        force = torch.FloatTensor(self.labels[idx])
        
        return (left, right), force
