from regression_tree import *

class GBTD(object):
    def __init__(self,iteration_num):
        self.tree_sets = []
        self.iteration_num = iteration_num
        self.max_deepth = 0
        self.gamma = 0.1

    def describe(self):
        return 'split'.join([x.describe() for x in self.tree_sets])

    def fit(self,data_sets,y_index=0):
        loss_function = None
        train_datasets = data_sets
        for i in range(self.iteration_num):
            ## build tree for every iteration 
            tree = build_tree(instance_sets = train_datasets,depth=0,max_depth=self.max_deepth,loss_function=loss_function,y_index=0)
            for x in train_datasets:
                x[y_index] -= self.gamma * tree.predict(x)
            self.tree_sets.append(tree)
    
    def predict(self,data,y_index=0):
        sum = 0.0
        for tr in self.tree_sets:
            sum += self.gamma *  tr.predict(data)     
        return sum
