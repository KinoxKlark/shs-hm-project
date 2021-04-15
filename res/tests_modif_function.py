# -*- coding: utf-8 -*-
"""
Created on Wed Apr 14 18:02:27 2021

@author: samue
"""

import numpy as np
import matplotlib.pyplot as plt

alpha = 1

def up(x):
    # return x+1e-3
    # return np.sqrt(x)
    # return np.sqrt(np.sqrt(x))
    # return x**2
    return (1-alpha)*x + alpha*np.sqrt(x)

def down(x):
    #return x-1e-3
    # return x**2
    # return 1-np.sqrt(1-(1-np.sqrt(1-x)))
    # return 1-(1-x)**2
    return (1-alpha)*x + alpha*(1-np.sqrt(1-x))

vals = np.random.uniform(size=(10000,))
vals2 = np.copy(vals)

nb = 1000
means = np.zeros((nb,))
stds = np.zeros((nb,))
suites = np.zeros((nb,*vals.shape))
suites2 = np.zeros((nb,*vals.shape))

for i in range(nb):
  means[i] = vals.mean()
  stds[i] = vals.std()
  
  suites[i,:] = vals
  suites2[i,:] = vals2
    
  if np.random.uniform(size=1) < .5:
      vals = down(vals)
  else:
      vals = up(vals)
      
  mask = np.random.uniform(size=vals2.shape) < .5
  vals2[mask] = down(vals2[mask])
  vals2[~mask] = up(vals2[~mask])
  
print(vals[-1])
    
plt.figure()
plt.subplot(1,2,1)
plt.plot(means)
plt.plot(stds)

plt.subplot(1,2,2)
plt.plot(suites2[0:50,0:-1:1000], color='red')
plt.plot(suites[0:50,0:-1:1000], color='blue')