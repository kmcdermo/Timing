#!/bin/bash

# config
infilename=${1:-"skims/v4p1/final/categories/exclusive_1pho.root"}
intreename=${2:-"Data_Tree"}
selection=${3:-"(run==306423&&lumi==220&&event==388707708)"}
outfilename=${4:-"binC_ex1pho.root"}

# run macro
root -l -b -q copyEvents.C\(\"${infilename}\",\"${intreename}\",\"${selection}\",\"${outfilename}\"\)

# Final message
echo "Done copying events!"
