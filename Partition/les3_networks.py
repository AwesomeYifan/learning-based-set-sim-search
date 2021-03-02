import torch.nn as nn
import torch.nn.functional as F
import torch
import les3_Parameters as Parameters
import random

class EmbeddingNetMLP(nn.Module):
    def __init__(self, input_dim, output_dim):
        super(EmbeddingNetMLP, self).__init__()
        self.net = nn.Sequential(nn.Linear(input_dim, Parameters.n_neurons, bias=True),
                                 nn.ReLU(),
                                 nn.Linear(Parameters.n_neurons, Parameters.n_neurons, bias=True),
                                 nn.ReLU(),
                                 nn.Linear(Parameters.n_neurons, output_dim, bias=True),
                                 nn.Sigmoid(),
                                 )
        
    def forward(self, x):
        output = self.net(x)
        return output

    def get_embedding(self, x):
        return self.forward(x)

# class EmbeddingNetMLP(nn.Module):
#     def __init__(self, input_dim, output_dim):
#         super(EmbeddingNetMLP, self).__init__()
#         self.net = nn.Sequential(nn.Linear(input_dim, output_dim, bias=True),
#                                  nn.Sigmoid(),
#                                  )
        
#     def forward(self, x):
#         output = self.net(x)
#         return output

#     def get_embedding(self, x):
#         return self.forward(x)


class SiameseNet(nn.Module):
    def __init__(self, embedding_net):
        super(SiameseNet, self).__init__()
        self.embedding_net = embedding_net

    def forward(self, x1, x2):
        output1 = self.embedding_net(x1)
        output2 = self.embedding_net(x2)
        return output1, output2

    def get_embedding(self, x):
        return self.embedding_net(x)
