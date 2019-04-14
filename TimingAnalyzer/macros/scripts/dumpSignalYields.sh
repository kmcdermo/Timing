#!/bin/bash

infiletext=${1:-""}
outfiletext=${2:-""}

root -l -b -q dumpSignalYields.C\(\"${infiletext}\",\"${outfiletext}\"\)
