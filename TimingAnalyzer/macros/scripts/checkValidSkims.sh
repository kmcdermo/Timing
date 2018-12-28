#!/bin/bash

## source first 
source scripts/common_variables.sh

## config
pdname=${1:-"SinglePhoton"}
inskimdir=${2:-"madv2_v1"}

## check valid skims
root -l -b -q checkValidSkims.C\(\"${pdname}\",\"${inskimdir}\"\)

## Final message
echo "Finished CheckingValidSkims"
