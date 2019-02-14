#!/bin/bash

## source
source scripts/common_variables.sh

## Command Line Input
inlimitdir=${1:-"input"}
ws_filename=${2:-"ws_inputs.root"}
ws_name=${3:-"workspace"}
sample=${4:-"GMSB"}
tmplog_file=${5:-"tmp.log"}
outfile_name=${6:-"ws_final.root"}

## make workspace
root -l -b -q writeToWS.C\(\"${inlimitdir}\",\"${ws_filename}\",\"${ws_name}\",\"${sample}\",\"${tmplog_file}\",\"${outfile_name}\"\)

## final message
echo "Finished WritingToWS for sample: ${sample}"
