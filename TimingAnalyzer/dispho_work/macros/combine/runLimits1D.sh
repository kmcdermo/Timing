#!/bin/bash

indir=${1:-"output"}
infilename=${2:-"AsymLim"}
outtext=${3:-"limit1D"}

root -l -b -q runLimits1D.C\(\"${indir}\",\"${infilename}\",\"${outtext}\"\)