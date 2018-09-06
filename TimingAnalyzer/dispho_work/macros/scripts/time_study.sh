#!/bin/bash

##################
## Source First ##
##################
source scripts/common_variables.sh

############
## Config ##
############

## command line inputs
outdirbase=${1:-"plots/ntuples_v5/time_study"}
fittype=${2:-"Gaus1core"}
rangelow=${3:-"3"}
rangeup=${4:-"3"}

## other config
filedump="tmp_infiles.txt"

## make generic plots
./scripts/make1Dplots.sh "${outdirbase}/basic_plots" "standard_plots_zee" "false"

## lauch plots without corrections
./scripts/makeTimePlots.sh "${outdirbase}/no_corrs" "false" "${fittype}" "${rangelow}" "${rangeup}" "true" "${filedump}"

## launch time adjuster, over each input selection
for input in "${inputs[@]}"
do echo ${!input} | while read -r label infile insigfile sel varwgtmap
    do
	./scripts/runTimeAdjuster.sh "${infile}" "${insigfile}" "${filedump}"
    done
done

## rm tmp file
rm "${filedump}"

## lauch plots with corrections
./scripts/makeTimePlots.sh "${outdirbase}/with_corrs" "true" "${fittype}" "${rangelow}" "${rangeup}" "false"
