#!/bin/sh

for sample in dyll
do
    echo "Doing sample: " ${sample}
    root -l -b -q "macros/addwgtsum.C("\"${sample}\"")"
    echo "" 
done
