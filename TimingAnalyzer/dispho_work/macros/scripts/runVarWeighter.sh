#!/bin/bash

varwgtconfig=${1:-"varwgt_config/qcd_met.txt"}

## first make plot
root -l -b -q runVarWeighter.C\(\"${varwgtconfig}\"\)

## Final message
echo "Finished VarWeighting for:" ${varwgtconfig}
