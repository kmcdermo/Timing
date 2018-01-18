#!/bin/bash

infilename=${1:-"met_vs_time.root"}
outfilename=${2:-"fitresults.root"}

root -b -q -l runFitter.C\(\"${infilename}\",\"${outfilename}\"\)

cp xfit_2D.png yfit_2D.png fit_projX.png fit_projY.png ${outfilename} ~/www/quick/
