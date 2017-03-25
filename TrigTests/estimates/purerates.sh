#!/bin/sh

while read line; do    
    root -l -b -q myrate.C\(3,\"${line}\"\)
done < disphopaths.txt