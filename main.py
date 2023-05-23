from math import *

T = 9.81 * 0.0500
T = [ T , T*2, T*3 ]
f = [20.86, 29.11, 36.01]
v = [19.1, 27.5, 33.1]
p = 0.001340
for i in range(3):
    print( (f[i] * 0.950 - v[i])/v[i]*100 )
    #print(f"T:{T[i]}")
    #print( sqrt( T[i] / p ) )