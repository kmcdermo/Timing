#!/bin/bash

infilename=${1:-"skim.root"}
outfiletext=${2:-"dump"}

root -l -b -q test_macros/cutflowDumper.C\(\"${infilename}\",\"${outfiletext}\"\)
