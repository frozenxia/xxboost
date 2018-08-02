from regression_tree import *
import numpy as np
import pandas as pd
# from sklearn.tree import DecisionTreeRegressor

def gen_data():
    X = [ x for x in range(10) ]

    Y = []
    for x in X:
        if x < 3:
            Y.append([1,x])
        elif x < 6:
            Y.append([2,x])
        elif x < 10:
            Y.append([3,x])

    return Y    

    # print(Y)
    # print(X)

    # return np.concatenate((np.expand_dims(Y,axis=0),np.expand_dims(X,axis=0)),axis=0)


dt = gen_data()
print(dt)
            
tr = build_tree(dt,0,2,compute_loss)
print(tr.describe())
for i in range(13):
    print(tr.predict([1,i]))