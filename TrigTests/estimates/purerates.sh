#!/bin/sh

while read line; do    
    root -l -b -q myrate.C\(\"${line}\"\)
done < disphopaths.txt