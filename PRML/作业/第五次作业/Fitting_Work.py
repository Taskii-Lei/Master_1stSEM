## •自己编程实现课堂上 Polynomial Curve Fitting的例子，体会过拟合。
import random
import math
import matplotlib.pyplot as plt
import numpy as np

N = 10
x = np.arange(-1, 1, 0.1)
noise_sigma = 0.08
y = []
for i in x:
    p = math.sin(2*math.pi*i)+random.random()*noise_sigma
    y.append(p)
plt.scatter(x, y, c='b')

## 真实曲线
x_r = np.arange(-1, 1, 0.01)
y_r = []
for i in x_r:
    y_r.append(math.sin(2 * math.pi * i))
plt.plot(x_r,y_r,c='b')

order = [0,1,2,5,9,15,20]
for i in order:
    an = np.polyfit(x, y, i)  # 用3次多项式拟合
    yvals = np.polyval(an, x_r)
    plt.plot(x_r, yvals, label ="order="+str(i))
plt.ylim([-2,2])
plt.legend()
plt.show()
