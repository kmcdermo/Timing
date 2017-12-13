#!/bin/bash

indir=${1}
files=${2}
wgtfile=${3}

root -b -q -l runSumWeights.C\(\"${indir}\",\"${files}\",\"${wgtfile}\"\)
