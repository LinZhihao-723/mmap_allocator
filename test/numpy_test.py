import numpy as np
z=np.random.random((3000,3000))
z1=np.random.random((3000,3000))
z2=np.random.random((3000,3000))
for x in range(25):
    print(x)
    z3=np.random.random((3000,3000))
    z4=np.random.random((3000,3000))
    z1+=z3*z4
print(z.sum())
