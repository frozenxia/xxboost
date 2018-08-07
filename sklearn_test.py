import numpy as np
from sklearn.tree import DecisionTreeRegressor
import matplotlib.pyplot as plt
from xgboost_model import Xgboost


from regression_tree import RegressionTree

# Create a random dataset
rng = np.random.RandomState(1)
X = np.sort(5 * rng.rand(80, 1), axis=0)
y = np.sin(X).ravel()
y[::5] += 3 * (0.5 - rng.rand(16))

# print(X,y)


# Fit regression model
regr_1 = DecisionTreeRegressor(max_depth=2)
regr_2 = DecisionTreeRegressor(max_depth=5)
regr_1.fit(X, y)
regr_2.fit(X, y)

# Predict

# regr_3 = RegressionTree(max_depth=5,min_sample_split=2,min_impurity=1)



X_test = np.arange(0.0, 5.0, 0.01)[:, np.newaxis]
y_1 = regr_1.predict(X_test)
y_2 = regr_2.predict(X_test)


def predict_xgboost(nestimator):
    regr_3 =  Xgboost(n_estimators=nestimator)
    y_3= [[y0] for y0 in y]
    regr_3.fit(X,y_3)
    y_31 = regr_3.predict(X_test)
    y_31 = [y0[0] for y0 in y_31]
    return y_31
# print(y_31)




# Plot the results
plt.figure()
plt.scatter(X, y, s=20, edgecolor="black",
            c="darkorange", label="data")
# plt.plot(X_test, y_1, color="cornflowerblue",
#          label="max_depth=2", linewidth=2)
plt.plot(X_test, y_2, color="yellowgreen", label="max_depth=5", linewidth=2)
# print(regr_1.)
plt.plot(X_test, predict_xgboost(10), color="green", label="max_depth=5", linewidth=2)

plt.plot(X_test, predict_xgboost(40), color="red", label="max_depth=5", linewidth=2)
plt.xlabel("data")
plt.ylabel("target")
plt.title("Decision Tree Regression")
plt.legend()
plt.show()

# print(regr_3.describe())
from sklearn.externals.six import StringIO  
from IPython.display import Image  
from sklearn.tree import export_graphviz
import pydotplus
dot_data = StringIO()
export_graphviz(regr_2, out_file=dot_data,  
                filled=True, rounded=True,
                special_characters=True)
graph = pydotplus.graph_from_dot_data(dot_data.getvalue())  
# Image(graph.create_png())
graph.write_pdf('te.pdf')
