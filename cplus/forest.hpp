
#include<vector>
#include "matrix.hpp"
#include "classifier.hpp"
using std::vector

class Forest : public Classifier {
    private:
        int n_trees;
        int n_features;
        vector<TreeNode> trees;
    public:
        Forest();
}