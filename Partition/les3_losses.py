import torch
import torch.nn as nn
from torch.autograd import Variable

class ContrastiveLoss(nn.Module):
    def __init__(self, inV = 1):
        super(ContrastiveLoss, self).__init__()
        self.aplha = inV

    def forward(self, output1, output2, force, left_sizes, right_sizes):

        output1 = output1.type(torch.FloatTensor)
        output2 = output2.type(torch.FloatTensor)
        force = force.type(torch.FloatTensor)
        # left_sizes = left_sizes.type(torch.FloatTensor)
        # right_sizes = right_sizes.type(torch.FloatTensor)
        # weights = left_sizes + right_sizes

        tensor_0_5 = torch.Tensor(output1.size())
        tensor_0_5 = tensor_0_5.fill_(0.5)
        left_0 = torch.le(output1, tensor_0_5)
        right_0 = torch.le(output2, tensor_0_5)
       
        common_group = torch.eq(left_0, right_0).type(torch.FloatTensor)
        
        tensor_diff = (tensor_0_5 - (output1 - output2).abs()).type(torch.FloatTensor)
        tensor_diff = tensor_diff * common_group
        
        # tensor_005 = torch.Tensor(output1.size())
        # tensor_005 = tensor_005.fill_(0.05)
        # loss = force * tensor_diff * weights * tensor_005
        loss = force * tensor_diff

        return loss.mean()
