import os, glob, sys
import array

files = []

with open('disphopaths.txt') as paths:
    for line in paths:
        lsplit = line.split()
        files.append(lsplit[0]+'.txt')
        
output = open('disphocsv.txt','w')

for file in files:
    with open(file) as f:
        for line in f:
            output.write(file+' '+line)



output.close()
