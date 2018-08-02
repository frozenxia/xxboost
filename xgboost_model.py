import numpy as np
from regression_tree import RegressionTree
class Xgboost(object):

    def __init__ (self,n_estimators = 200,
                    learning_rate = 0.01,
                    min_sample_split = 2,
                    min_impurity = 1e-7,
                    max_depth = 2):
        self.n_estimators = n_estimators
        self.learning_rate = learning_rate
        self.min_impurity = min_impurity
        self.min_sample_split = min_sample_split
        self.max_depth = max_depth
        self.trees = []

    def fit(self,X,y):
        y_pred = y
        for i in self.n_estimators:
            tree = RegressionTree(self.max_depth,self.min_sample_split,self.min_impurity)
            tree.fit(X,y_pred)
            update_pred = tree.predict(X)
            y_pred -= np.multiply(self.learning_rate,update_pred)
            self.trees.append(tree)
    
    def preedict(self,X):
        y_pred = None
        for tree in self.trees:
            update_pred = tree.predict(X)
            if y_pred is None:
                y_pred = np.zeros_like(update_pred)
            y_pred += np.multiply(self.learning_rate,update_pred)
        ## softmax
        y_pred  = np.exp(y_pred)/np.sum(np.exp(y_pred),axis=1,keepdims=True)
        y_pred = np.argmax(y_pred,axis=1)
        return y_pred
        