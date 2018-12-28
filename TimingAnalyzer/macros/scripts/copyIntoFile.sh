#!/bin/bash

## config
sampleconfig=${1}
cutflowconfig=${2}
infilename=${3}
outfilename=${4}

## run macro
root -l -b -q copyIntoFile.C\(\"${sampleconfig}\",\"${cutflowconfig}\",\"${infilename}\",\"${outfilename}\"\)

## Final message
echo "Finished Copying Into:" ${outfilename} "from:" ${infilename}