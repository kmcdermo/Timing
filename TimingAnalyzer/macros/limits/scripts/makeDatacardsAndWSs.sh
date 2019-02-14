#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
inlimitdir=${1:-"input"}
ws_filename=${2:-"ws_inputs.root"}
outdir=${3:-"madv2_v3/full_chain/ABCD/combine_input"}
docleanup=${4:-"true"}
ws_outfile_base=${5:-"ws"}

## global info
ws_basename="workspace"

#####################################
## Write Results to Datacards + WS ##
#####################################

for lambda in 100 150 200 250 300 350 400 500 600
do
    echo "Submitted lambda: ${lambda}"
    ./scripts/makeDatacardsAndWSsSub.sh "${inlimidir}" "${ws_filename}" "${ws_basename}" "${ws_outfile_base}" "${lambda}" "${docleanup}" & 
done

wait

################################
## Copy Cards + Final WS File ##
################################

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy all files from input
cp "${ws_outfile_base}_L"*.root "${fulldir}"
cp "${inlimitdir}/${ws_filename}" "${fulldir}"
cp "${inlimitdir}/${base_datacardABCD}"*".${inTextExt}" "${fulldir}"

###################
## Final Message ##
###################

echo "Finished MakingDatacardsAndWSs (ABCD)"
