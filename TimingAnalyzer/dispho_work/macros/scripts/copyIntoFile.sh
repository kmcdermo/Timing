#!/bin/bash

sampleconfig=${1}
cutflowconfig=${2}
infilename=${3}
outfilename=${4}

root -l -b -q copyIntoFile.C\(\"${sampleconfig}\",\"${cutflowconfig}\",\"${infilename}\",\"${outfilename}\"\)