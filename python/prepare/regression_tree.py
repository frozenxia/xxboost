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


class TreeNode(object):
    def __init__(self):
        pass
    def update_predict_value(self,value):
        self.params = value

    def predict(self,instance_value):
        return self.params

    def describe(self):
        return str(self.params)


def build_tree(instance_sets,depth,max_depth,loss_function,y_index=0):
    tree = Tree()
    if depth >= max_depth:
        tree_node = TreeNode()
        sum1 = sum([x[y_index] for x in instance_sets])
        tree_node.update_predict_value(sum1/len(instance_sets))
        tree.tree_node = tree_node
    else:
        loss = -1
        select_attribute = None
        select_left_set = []
        select_right_set = []
        select_split_value = None
        for attribute_idx in range(0,len(instance_sets[0])):
            if attribute_idx == y_index :
               continue

            split_value_sets = set([x[attribute_idx]  for x in instance_sets])

            print('svts',split_value_sets)
            for sv in split_value_sets:
                
                left_split_set = []
                right_split_set = []
                for x in instance_sets:
                    if x[attribute_idx] < sv :
                        left_split_set.append(x)
                    else:
                        right_split_set.append(x)
                
                sum_loss = loss_function(left_split_set,y_index) + loss_function(right_split_set,y_index)

                print('sum_loss',sum_loss)
                if loss < 0 or sum_loss < loss:
                    select_attribute =  attribute_idx
                    select_split_value = sv
                    select_left_set  = left_split_set
                    select_right_set = right_split_set
                    loss = sum_loss
                
        # update set    
        tree.split_feature = select_attribute
        tree.condition_value = select_split_value
        tree.left_tree = build_tree(select_left_set,depth+1,max_depth,loss_function,y_index)
        tree.right_tree = build_tree(select_right_set,depth+1,max_depth,loss_function,y_index)
    return tree


# l2 loss
def compute_loss(data_sets,y_index):
    print(data_sets)
    if len(data_sets) == 0 :
        return 0
    avg = sum([x[y_index] for x in data_sets])/ len(data_sets)
    return sum([pow(x[y_index]-avg,2) for x in data_sets])






    