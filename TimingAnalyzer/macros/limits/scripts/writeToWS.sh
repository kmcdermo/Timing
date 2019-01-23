#!/bin/bash

## source
source scripts/common_variables.sh

## Command Line Input
inlimitdir=${1:-"input"}
ws_filename=${2:-"ws_final.root"}
ws_name=${3:-"workspace"}
tmplog_filename=${4:-"tmp.log"}
sample=${5:-"GMSB"}
dir=${6:-"madv2_v3/full_chain/ABCD/card_input"}

## make workspace
root -l -b -q writeToWS.C\(\"${inlimitdir}\",\"${ws_filename}\",\"${ws_name}\",\"${tmplog_filename}\",\"${sample}\"\)

## make out dirs
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir ${fulldir}

## copy log file
cp "${tmplog_filename}" "${fulldir}"

## final message
echo "Finished WritingToWS for sample: ${sample}"
