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

        return addition_info + "{split feature : %s,split value : %s ,left node : %s ,right node : %s }" %(str(self.split_feature,self.condition_value,left_desc,right_desc))




    