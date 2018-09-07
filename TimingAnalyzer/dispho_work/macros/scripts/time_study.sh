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

## other config
nodir="no_corrs"
tofdir="tof_corrs"
shiftdir="shift_corrs"
smeardir="smear_corrs"

filedump="tmp_infiles.${inTextExt}"

#################
## Run scripts ##
#################

## 1) Plot generic kinematics, time before corrections
## 2) Make shift corrections
## 3) Make smear corrections
## 4) Final plots + delta time plots with all corrections

##############################
## Plots before corrections ##
##############################

## make generic plots
echo "Making 1D generic plots"
./scripts/make1Dplots.sh "${outdirbase}/basic_plots" "standard_plots_zee" "false"

## lauch plots with no corrections
echo "Making time related plots without TOF, SHIFT, SMEAR corrections"
./scripts/makeTimePlots.sh "${outdirbase}/${nodir}" "false" "false" "false" "true" "${filedump}"

## launch delta time plots with TOF+SHIFT+SMEAR corrections
echo "Making deltaT related plots with TOF, SHIFT, SMEAR corrections"
./scripts/makeDeltaTimePlots.sh "${outdirbase}/${smeardir}" "true" "true" "true"

## plot raw TOF
echo "Making raw TOF plots"
./scripts/makeCorrPlots.sh "${outdirbase}/${tofdir}" "TOF"

############################
## Make shift corrections ##
############################

## lauch plots with TOF corrections
echo "Making time related plots without SHIFT, SMEAR corrections; with TOF corrections"
./scripts/makeTimePlots.sh "${outdirbase}/${tofdir}" "true" "false" "false" "true" "${filedump}"

## launch time adjuster, over each input selection
for input in "${inputs[@]}"
do echo ${!input} | while read -r label infile insigfile sel varwgtmap
    do
 	echo "Running time adjuster for computing shift corrections for: ${label}" 
  	./scripts/runTimeAdjuster.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${filedump}" 1 0
    done
done

## rm tmp file
rm "${filedump}"

## plot raw shift
echo "Making raw shift plots"
./scripts/makeCorrPlots.sh "${outdirbase}/${tofdir}" "SHIFT"

############################
## Make smear corrections ##
############################

## lauch plots with TOF+SHIFT corrections
echo "Making time related plots without SMEAR corrections; with TOF, SHIFT corrections"
./scripts/makeTimePlots.sh "${outdirbase}/${shiftdir}" "true" "true" "false" "true" "${filedump}"

## launch time adjuster, over each input selection
for input in "${inputs[@]}"
do echo ${!input} | while read -r label infile insigfile sel varwgtmap
    do
	echo "Running time adjuster for computing smear corrections for: ${label}"
	./scripts/runTimeAdjuster.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${filedump}" 0 1
    done
done

## rm tmp file
rm "${filedump}"

## plot raw smear
echo "Making raw smear plots"
./scripts/makeCorrPlots.sh "${outdirbase}/${tofdir}" "SMEAR"

##########################
## Final plots and such ##
##########################

## lauch plots with TOF+SHIFT+SMEAR corrections
echo "Making time related plots with TOF, SHIFT, SMEAR corrections"
./scripts/makeTimePlots.sh "${outdirbase}/${smeardir}" "true" "true" "true" "false"

## launch delta time plots with TOF+SHIFT+SMEAR corrections
echo "Making deltaT related plots with TOF, SHIFT, SMEAR corrections"
./scripts/makeDeltaTimePlots.sh "${outdirbase}/${smeardir}" "true" "true" "true"

## final prep dir
echo "Final prep outdir"
PrepOutDir "${topdir}/${disphodir}/${outdirbase}"

## all done
echo "Finished full chain of time study"
