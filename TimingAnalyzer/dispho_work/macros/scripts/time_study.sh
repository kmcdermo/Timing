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
rangelow=${3:-"2"}
rangeup=${4:-"2"}

## other config
filedump="tmp_infiles.${inTextExt}"

## make generic plots
echo "Making 1D plots"
./scripts/make1Dplots.sh "${outdirbase}/basic_plots" "standard_plots_zee" "false"

## lauch plots without shift/smear corrections
echo "Making time related plots without shift nor smear corrections"
./scripts/makeTimePlots.sh "${outdirbase}/no_corrs" "false" "false" "${fittype}" "${rangelow}" "${rangeup}" "true" "${filedump}"

## launch time adjuster, over each input selection
for input in "${inputs[@]}"
do echo ${!input} | while read -r label infile insigfile sel varwgtmap
    do
	echo "Running time adjuster for computing shift corrections for: ${label}" 
 	./scripts/runTimeAdjuster.sh "${infile}" "${insigfile}" "${filedump}" 1 0
    done
done

## rm tmp file
rm "${filedump}"

## lauch plots with shift corrections
echo "Making time related plots with shift corrections"
./scripts/makeTimePlots.sh "${outdirbase}/shift_corrs" "true" "false" "${fittype}" "${rangelow}" "${rangeup}" "true" "${filedump}"

## launch time adjuster, over each input selection
for input in "${inputs[@]}"
do echo ${!input} | while read -r label infile insigfile sel varwgtmap
    do
	echo "Running time adjuster for computing smear corrections for: ${label}"
 	./scripts/runTimeAdjuster.sh "${infile}" "${insigfile}" "${filedump}" 0 1
    done
done

## rm tmp file
rm "${filedump}"

## lauch plots with shift corrections
echo "Making time related plots with corrections"
./scripts/makeTimePlots.sh "${outdirbase}/shift_smear_corrs" "true" "true" "${fittype}" "${rangelow}" "${rangeup}" "false"

## final prep dir
echo "Final prep outdir"
PrepOutDir "${topdir}/${disphodir}/${outdirbase}"

## all done
echo "Finished full chain of time study"
