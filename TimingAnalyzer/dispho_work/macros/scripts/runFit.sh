#!/bin/bash

infilename=${1:-"met_vs_time.root"}
outfilename=${2:-"fitresults.root"}

root -b -q -l runFit.C\(\"${infilename}\",\"${outfilename}\"\)

cp xframe_projfit.png yframe_projfit.png ~/www/quick/
