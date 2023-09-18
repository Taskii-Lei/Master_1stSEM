"""
**********************************************************
**  copyright@huanglei; taskii0401@163.com  **************
**  Class:702; Stu-Num: 2022E8013282156  *****************
**  built in 2022.09.06 Tue.  ****************************
**********************************************************
*********************  测试说明  **************************
## 需要实现定义好变量，例如三元函数就需要定义三个 symblos 并组成向量
## x1,x2,x3 = sympy.symbols("x1 x2 x3")
## x = [x1,x2,x3]
## 如果需要判断测例属于哪列，直接让 x 为该向量即可，类似 x = [1,2,3]
## x = [1,2,3]
## 运行函数 Bayes_Gaussion_with2cls(w1, w2, P_w1, P_w2,x)
## 对二维可以进行画图，运行 plot_2d(w1,w2,bc, new_p=None) 函数
## plot_2d 函数最后一个参数代表待测试点，默认为None
**********************************************************
"""

import numpy as np
import sympy
import matplotlib.pyplot as plt

def Bayes_Gaussion_with2cls(w1, w2, P_w1, P_w2,x)\
        ->sympy.tensor.array.dense_ndim_array.ImmutableDenseNDimArray:
    m1 = np.mean(w1, axis=0)
    m2 = np.mean(w2, axis=0)
    print("\n The mean vector(m1) of the samples in category w1 is: \n", m1)
    print("\n The mean vector(m2) of the samples in category w2 is: \n", m2)
    C1 = np.zeros((len(m1), len(m1)))
    C2 = np.zeros((len(m2), len(m2)))
    for i in range(len(w1)):
        c_i = np.dot((w1 - m1)[i].reshape(len(m1), 1), [(w1 - m1)[i]])
        C1 += c_i
    ## w1 和 w2 中的学习示例个数可能不同
    for i in range(len(w2)):
        c_i = np.dot((w2 - m2)[i].reshape(len(m2), 1), [(w2 - m2)[i]])
        C2 += c_i
    C1 = C1 / len(w1)
    C2 = C2 / len(w2)
    print("\n The Covariance matrix(C1) in category w1 is: \n", C1)
    print("\n The Covariance matrix(C2) in category w2 is: \n", C2)
    inv_C1 = np.linalg.inv(C1)
    inv_C2 = np.linalg.inv(C2)
    det_C1= np.abs(np.linalg.det(C1))
    det_C2 = np.abs(np.linalg.det(C2))
    g1 = np.log(P_w1) - np.log(det_C1) / 2 - ((x - m1).dot(inv_C1)).dot((x - m1).reshape(len(m2), 1))/2
    g2 = np.log(P_w2) - np.log(det_C2) / 2 - ((x - m2).dot(inv_C2)).dot((x - m2).reshape(len(m2), 1))/2
    Bayes_Classfier = sympy.simplify(g1-g2)[0]
    if(type(x[0]) == int or type(x[0]) == float):
        print("\n The Value of Bayesian classification surface equation is: \n", Bayes_Classfier)
        if (Bayes_Classfier > 0): print("This Vector belongs to w1 \n")
        if (Bayes_Classfier < 0): print("This Vector belongs to w2 \n")
        if (Bayes_Classfier == 0): print("This Vector lays on the classification surface\n")
    else :
        print("\n The Bayesian classification surface equation is: \n", sympy.simplify(Bayes_Classfier))
        return Bayes_Classfier

def plot_2d(w1,w2,bc, new_p=None):
    low = min(min(min(w1)), min(min(w2)))
    high = max(max(max(w1)), max(max(w2)))
    fig = plt.figure()
    if(new_p):plt.scatter(new_p[0], new_p[1], c="black",label="Test Point")
    for i in range(len(w1)):
        p1 = w1[i]
        p2 = w2[i]
        if (i == 0):
            plt.scatter(p1[0], p1[1], c="r", label="w1")
            plt.scatter(p2[0], p2[1], c="b", label="w2")
        plt.scatter(p1[0], p1[1], c="r")
        plt.scatter(p2[0], p2[1], c="b")

    y1 = x1 - (low - 1)
    y2 = x1 - (high + 1)
    i1, j1 = sympy.solve([bc, y1], [x1, x2]).items()
    i2, j2 = sympy.solve([bc, y2], [x1, x2]).items()
    plt.plot([i1[1], j1[1]], [i2[1], j2[1]], "g-", label="Bayesian classification surface")
    plt.legend(loc="best")
    plt.show()
    plt.close()

print("=====================  测例 1 ====================")
w1 = [[0,0],[2,0],[2,2],[0,2]]
w2 = [[4,4],[6,4],[6,6],[4,6]]
P_w1, P_w2 = 1/2, 1/2
x1, x2 = sympy.symbols("x1 x2")
x = [x1, x2]
bc = Bayes_Gaussion_with2cls(w1, w2, P_w1, P_w2, x)
print("============ 测试 y = [1,2] 属于哪类 ==============")
y = [1, 2]
Bayes_Gaussion_with2cls(w1, w2, P_w1, P_w2, y)
## 画出决策面
plot_2d(w1, w2, bc, y)

print("=====================  测例 2 ====================")

w1 = [[0,0,0],[1,0,1],[1,0,0],[1,1,0]]
w2 = [[0,0,1],[0,1,1],[1,1,1],[0,1,0]]

x1,x2,x3 = sympy.symbols("x1 x2 x3")
x = [x1,x2,x3]
P_w1, P_w2 = 1/2, 1/2
a = Bayes_Gaussion_with2cls(w1, w2, P_w1, P_w2,x)

