import numpy as np
class Tree(object):
    def __init__(self):
        self.split_feature = None
        self.condition_value = None
        self.left_tree = None
        self.right_tree = None
        self.tree_node = None
    def predict(self,instance_value):
        if self.tree_node != None:
            return self.tree_node.predict(instance_value)
        if instance_value[self.split_feature] < self.condition_value:
            return self.left_tree.predict(instance_value)
        return self.right_tree.predict(instance_value) 

    
    def describe(self,addition_info = ""):
        if self.tree_node != None:
            return self.tree_node.describe()
        
        left_desc  = self.left_tree.describe()
        right_desc = self.right_tree.describe()

        return addition_info + "{split feature : %s,split value : %s ,left node : %s ,right node : %s }" %(str(self.split_feature),str(self.condition_value),left_desc,right_desc)


class RegressionTree(object):
    def __init__(self,max_depth,min_sample_split,min_impurity):
        self.tree  = None
        self.max_depth = max_depth
        self.min_sample_split = min_sample_split
        self.min_impurity = min_impurity
        pass
    def fit(self,X,Y):
        # print(X.shape,Y.shape)
        # xy = np.concatenate((Y,X),axis=1)
        # print(xy[:1])
        self.tree = self.build_tree(X,Y,0,self.max_depth,self.get_loss_function())
    def describe(self):
        return self.tree.describe()
    def predict(self,X):
        if self.tree == None:
            raise Exception("please fit tree before predict")
        y_pred = []
        for x in X:
            y_pred.append(self.tree.predict(x))
        return y_pred

    def get_loss_function(self,function_type='l2_loss'):
        def l2_loss(y_sets):
            # print(data_sets)
            if len(y_sets) == 0 :
                return 0
            avg = np.sum(y_sets,axis=0)/ len(y_sets)
            return np.sum(pow(y_sets-avg,2))

        loss_func = None
        
        if function_type == 'l2_loss':
            loss_func =  l2_loss
        return loss_func

    def build_tree(self,x_sets,y_sets,depth,max_depth,loss_function):
        tree = Tree()
        if depth >= max_depth:
            tree_node = TreeNode()
            sum1 = np.sum(y_sets,axis=0)
            tree_node.update_predict_value(sum1/len(y_sets))
            tree.tree_node = tree_node
        else:
            loss = -1
            select_attribute = None
            select_left_set = []
            select_right_set = []

            select_left_set_y = []
            select_right_set_y = []

            select_split_value = None
            for attribute_idx in range(0,len(x_sets[0])):
                split_value_sets = set([x[attribute_idx]  for x in x_sets])

                # print('svts',split_value_sets)
                for sv in split_value_sets:
                    
                    left_split_set = []
                    right_split_set = []

                    left_split_set_y = []
                    right_split_set_y = []


                    for idx in range(len(x_sets)):
                        x = x_sets[idx]
                        if x[attribute_idx] < sv :
                            left_split_set.append(x)
                            left_split_set_y.append(y_sets[idx])
                        else:
                            right_split_set.append(x)
                            right_split_set_y.append(y_sets[idx])
                    sum_loss = loss_function(left_split_set_y) + loss_function(right_split_set_y)
                

                    # print('sum_loss',sum_loss)
                    if loss < 0 or sum_loss < loss:
                        select_attribute =  attribute_idx
                        select_split_value = sv
                        select_left_set  = left_split_set
                        select_right_set = right_split_set

                        select_left_set_y = left_split_set_y
                        
                        select_right_set_y = right_split_set_y
                        loss = sum_loss

            if len(select_left_set) == 0 or len(select_right_set) == 0 :
                tree_node = TreeNode()
                sum1 = np.sum(y_sets,axis=0)
                tree_node.update_predict_value(sum1/len(y_sets))
                tree.tree_node = tree_node
            else:
                # update set    
                tree.split_feature = select_attribute
                tree.condition_value = select_split_value
                tree.left_tree = self.build_tree(select_left_set,select_left_set_y,depth+1,max_depth,loss_function)
                tree.right_tree = self.build_tree(select_right_set,select_right_set_y,depth+1,max_depth,loss_function)
        return tree

class TreeNode(object):
    def __init__(self):
        pass
    def update_predict_value(self,value):
        self.params = value

    def predict(self,instance_value):
        return self.params

    def describe(self):
        return str(self.params)











    