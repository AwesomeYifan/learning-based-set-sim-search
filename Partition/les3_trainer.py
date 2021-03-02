import numpy as np
from les3_pytorchtools import EarlyStopping
import les3_Parameters as Parameters
# import matplotlib.pyplot as plt

def fit_siamese(train_loader, model, loss_fn, optimizer, scheduler, patience, n_epochs, log_interval, metrics=[],
        start_epoch=0):

    for epoch in range(0, start_epoch):
        scheduler.step()
    early_stopping = EarlyStopping(patience=patience, verbose=True)
    for epoch in range(start_epoch, n_epochs):
        
        # Train stage
        train_loss, metrics = train_siamese(train_loader, model, loss_fn, optimizer, log_interval, metrics)
        scheduler.step()
        Parameters.epoch += 1

        message = 'Epoch: {}/{}. Train set: Average loss: {:.4f}'.format(epoch + 1, n_epochs, train_loss)
        for metric in metrics:
            message += '\t{}: {}'.format(metric.name(), metric.value())

        for metric in metrics:
            message += '\t{}: {}'.format(metric.name(), metric.value())
        print(message)

def train_siamese(train_loader, model, loss_fn, optimizer, log_interval, metrics):
    for metric in metrics:
        metric.reset()

    model.train()
    losses = []
    total_loss = 0

    for batch_idx, (data, target) in enumerate(train_loader):

        target = target if len(target) > 0 else None
        if not type(data) in (tuple, list):
            data = (data,)
        

        optimizer.zero_grad()
        outputs = model(*data)

        if type(outputs) not in (tuple, list):
            outputs = (outputs,)

        loss_inputs = outputs
        if target is not None:
            target = (target,)
            loss_inputs += target

        loss_outputs = loss_fn(*loss_inputs)
        loss = loss_outputs[0] if type(loss_outputs) in (tuple, list) else loss_outputs
        losses.append(loss.item())
        total_loss += loss.item()
        loss.backward()
        optimizer.step()

        if batch_idx % log_interval == 0:
            message = 'Train: [{}/{} ({:.0f}%)] Loss: {:.6f}'.format(
                batch_idx * len(data[0]), len(train_loader.dataset),
                100. * batch_idx / len(train_loader), np.mean(losses))
            for metric in metrics:
                message += '\t{}: {}'.format(metric.name(), metric.value())

            print(message)
            losses = []

    total_loss /= (batch_idx + 1)
    return total_loss, metrics
