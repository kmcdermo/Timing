#!/bin/bash

infilename=${1:-"signal_skims.root"}
outtext=${2:-"signal_efficiency"}

root -l -b -q computeSignalEfficiency.C\(\"${infilename}\",\"${outtext}\"\)
