#!/bin/bash

for era in 2017B 2017C 2017DE
do
    for eff in 0 1 2
    do
	root -l -b -q "runHLTPlots.C("\"SP\",\"${era}\",${eff}")"
    done
done
