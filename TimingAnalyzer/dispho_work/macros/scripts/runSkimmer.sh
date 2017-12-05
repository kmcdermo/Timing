#!/bin/bash

indir=${1}
outdir=${2}
filename=${3}

echo "Working on file:" ${filename}
root -b -q -l runSkimmer.C\(\"${indir}\",\"${outdir}\",\"${filename}\"\)
