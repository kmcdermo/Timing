#!/bin/sh

while read line; do    
    root -l -b -q myrate.C\(\"${line}\",3\)
done < disphopaths.txt