#!/bin/bash

## source
source scripts/common_variables.sh

## Command Line Input
tmplimitdir=${1:-"tmp"}
inplotsfile=${2:-"met_vs_time_box_SR.root"}
name=${3:-"GMSB"}
tmpwsfile=${4:-"${base_wsfileABCD}_${name}.root"}

./scripts/writeToWS.sh "${tmplimitdir}" "${inplotsfile}" "${name}" "${tmpwsfile}"
