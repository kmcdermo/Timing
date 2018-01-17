#!/bin/bash

infilename=${1:-"met_vs_time.root"}

root -b -q -l runFit.C\(\"${infilename}\"\)
