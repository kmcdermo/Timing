#!/bin/bash

indir=${1}
outdir=${2}
filename=${3}
sumwgts=${4}
redophoid=${5:-0}

root -b -q -l runSkimmer.C\(\"${indir}\",\"${outdir}\",\"${filename}\",${sumwgts},${redophoid}\)
