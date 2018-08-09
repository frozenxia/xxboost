import numpy as np

from xgboost_model import Xgboost
from sklearn import preprocessing

def get_data(file_path):
    data = np.loadtxt(file_path, delimiter=',',converters={33: lambda x:int(x == '?'), 34: lambda x:int(x)-1 } )  
    sz = data.shape  
    
    train = data[:int(sz[0] * 0.7), :] # take row 1-256 as training set  
    test = data[int(sz[0] * 0.7):, :]  # take row 257-366 as testing set  
    
    train_X = train[:,0:33]  
    train_Y = np.expand_dims(train[:, 34],axis=1) 
    
    
    test_X = test[:,0:33]  
    test_Y = np.expand_dims(test[:, 34],axis=1)   
    return train_X,train_Y,test_X,test_Y



def train_and_predict():
    file_path = "data/dermatology.data.txt"

    train_x,train_y,test_x,test_y = get_data(file_path)

    print(train_x[:1])
    print(train_y[:1])
    enc = preprocessing.OneHotEncoder()
    enc.fit(train_y)
    train_y = enc.transform(train_y).toarray()
    print(train_x[:1])
    print(train_y[:1])
   

    xgb = Xgboost()
  
    xgb.fit(train_x,train_y)

    y_pred = xgb.preedict(test_x)

    print(y_pred)
    print(test_y)
train_and_predict()

def l2_loss(y_sets):
    # print(data_sets)
    if len(y_sets) == 0 :
        return 0
    avg = np.sum(y_sets,axis=0)/ len(y_sets)
    print(avg)
    return np.sum(pow(y_sets-avg,2))

# npa = np.array([[1,2,3,4],[5,6,7,9.0],[5,6,7,9.0]])

# print(l2_loss(npa))



# enc = preprocessing.OneHotEncoder()
# enc.fit([[3],[0],[2],[0]])
 
# array = enc.transform([[3]]).toarray()
 
# print (array)