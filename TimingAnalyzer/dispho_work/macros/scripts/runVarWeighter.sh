#!/bin/bash

## source first
source scripts/common_variables.sh

## config
varwgtconfig=${1:-"${varwgtconfigdir}/qcd_met.${inTextExt}"}

## first make plot
root -l -b -q runVarWeighter.C\(\"${varwgtconfig}\"\)

## Final message
echo "Finished VarWeighting for:" ${varwgtconfig}
