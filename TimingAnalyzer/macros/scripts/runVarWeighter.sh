#!/bin/bash

## source first
source scripts/common_variables.sh

## config
varwgtconfig=${1:-"${varwgtconfigdir}/qcd_met.${inTextExt}"}
savemetadata=${2:-0}

## first make plot
root -l -b -q runVarWeighter.C\(\"${varwgtconfig}\",${savemetadata}\)

## Final message
echo "Finished VarWeighting for:" ${varwgtconfig}
