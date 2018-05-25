#!/bin/bash

indir=${1}
files=${2}
puwgtfile=${3}

root -l -b -q computePUWeights.C\(\"${indir}\",\"${files}\",\"${puwgtfile}\"\)
