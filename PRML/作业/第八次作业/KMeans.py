import numpy as np

Red = np.array([6.2, 3.2])
Green = np.array([6.6, 3.7])
Blue = np.array([6.5, 3.0])

R = np.array([
      [6.2, 2.8],
      [6.7, 3.1]])
K = Blue
for i in R:
      K[0] += i[0]
      K[1] += i[1]
print(K[0]/(len(R)+1), K[1]/(len(R)+1))

