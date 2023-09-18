"""
**********************************************************
**  copyright@huanglei; taskii0401@163.com  **************
**  Class:702; Stu-Num: 2022E8013282156  *****************
**  built in 2022.09.21 Wen.  ****************************
**********************************************************
*********************  测试说明  **************************
## 直接输入 样本 w1 和 样本 w2 ，并定义 C
## 运行函数 Sensor(w1, w2, C)
**********************************************************
"""
import numpy as np
import sympy


def Sensor(w1, w2, C):
    for i in range(len(w1)):
        w1[i] = w1[i] + [1]
    for i in range(len(w2)):
        w2[i] = w2[i] + [1]
    print("增广矩阵w1： ", w1)
    a = -1 * np.eye(len(w2[0]))
    oppo_w2 = np.dot(w2, a).tolist()
    print("负增广矩阵w2： ", oppo_w2)
    total_samples = w1
    total_samples.extend(oppo_w2)
    total_samples = np.array(total_samples)
    w = np.zeros(len(w2[0]))
    wrong_sum = len(total_samples)
    epoch = 0
    while (wrong_sum != 0):
        wrong_sum = len(total_samples)
        epoch += 1
        print("--------------------- the ", epoch, "_th epoch --------------------")
        for i in range(len(total_samples)):
            i_th_sum = sum(np.multiply(w, total_samples[i]))
            if i_th_sum > 0:
                print("w(", i, ") > 0, w(", i + 1, ") = w(", i, ") =", w)
                wrong_sum -= 1
                continue
            else:
                w = w + C * total_samples[i]
                print("w(", i, ") <= 0, w(", i + 1, ") = w(", i, ") + C*x_", i, "=", w)
                continue
    return w


print("**********************************************************")
print("*********************** 测例一 ****************************")
print("**********************************************************")
w1 = [[0, 0], [0, 1]]
w2 = [[1, 0], [1, 1]]
C = 1
Sensor(w1, w2, C)

print("**********************************************************")
print("*********************** 测例二 ****************************")
print("**********************************************************")
w1 = [[0, 0, 0], [1, 0, 0], [1, 0, 1], [1, 1, 0]]
w2 = [[0, 0, 1], [0, 1, 1], [0, 1, 0], [1, 1, 1]]
x1, x2, x3 = sympy.symbols("x1 x2 x3")
C = 1
w = Sensor(w1, w2, C)
print("最终该解向量为：", w, "\n相应判别函数为：d(x) =", np.dot(w, [x1, x2, x3, 1]))
