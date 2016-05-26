#!/bin/sh

for sample in dyll
do
    echo "Doing sample: " ${sample}
    path="input/MC/"${sample}
    root -l -b -q "macros/addwgtsum.C("\"${path}\"")"
    echo "" 
done