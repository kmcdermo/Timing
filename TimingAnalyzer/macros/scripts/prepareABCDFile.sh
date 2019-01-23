#!/bin/bash

## source first
source scripts/common_variables.sh

## config
signif_dump=${1:-"tmp_dump.txt"}
signif_list=${2:-"tmp_list.txt"}
ws_filename=${3:-"ws_final.root"}
dir=${4:-"plots/full_chain/ABCD"}

## run macro
root -l -b -q prepareABCDFile.C\(\"${signif_dump}\",\"${signif_list}\",\"${ws_filename}\"\)

## make out dirs
fulldir="${topdir}/${disphodir}/${dir}"
PrepOutDir "${fulldir}"

## cp all to outdir
cp "${signif_dump}" "${signif_list}" "${ws_filename}" "${fulldir}"

## Final message
echo "Finished PreparingABCDFile"
