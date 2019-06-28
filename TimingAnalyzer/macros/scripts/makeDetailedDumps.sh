#!/bin/bash

for category in ex1pho in2pho
do
    for ctau in 10 100 1000 10000
    do
	root -l -b -q dumpDetailedEffs.C\(\"${category}\",\"${ctau}\"\)
    done
done
