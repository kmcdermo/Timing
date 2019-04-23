#!/bin/bash

##################
## Source First ##
##################
source scripts/common_variables.sh

############
## Config ##
############

## command line inputs
outdirbase=${1:-"ntuples_v5p1/time_study_091018/single_object_${base_time_var}"}
savemetadata=${2:-0}

## other config
nodir="no_corrs"
tofdir="tof_corrs"
shiftdir="shift_corrs"
smeardir="smear_corrs"

filedump="tmp_infiles.${inTextExt}"

####################
## Raw time plots ##
####################

## lauch plots with no corrections
echo "Making time related plots with no corrections"
./scripts/makeTimePlots.sh "${outdirbase}/${nodir}" ${savemetadata} "false" "false" "false" "false"

## plot raw TOF
echo "Making raw TOF plots"
./scripts/makeCorrPlots.sh "${outdirbase}/${tofdir}" ${savemetadata} "TOF"

############################
## Make shift corrections ##
############################

## lauch plots with no corrections
echo "Making time related plots with TOF corrections"
./scripts/makeTimePlots.sh "${outdirbase}/${tofdir}" ${savemetadata} "true" "false" "false" "true" "${filedump}"

## launch time adjuster: use only Zee selection
echo "${Zee}" | while read -r label infile insigfile sel
do
    echo "Running time adjuster for computing shift corrections for: ${label}" 
    ./scripts/runTimeAdjuster.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${filedump}" "${base_adjust_var}" "${base_time_var}" 1 0 ${savemetadata}
done

## rm tmp file
rm "${filedump}"

## plot raw shift
echo "Making raw shift plots"
./scripts/makeCorrPlots.sh "${outdirbase}/${shiftdir}" ${savemetadata} "SHIFT"

############################
## Make smear corrections ##
############################

## lauch plots with tof, shift, and smear corrections
echo "Making time related plots with TOF + SHIFT corrections"
./scripts/makeTimePlots.sh "${outdirbase}/${shiftdir}" ${savemetadata} "true" "true" "false" "true" "${filedump}"

## launch time adjuster: use only Zee selection
echo "${Zee}" | while read -r label infile insigfile sel
do
    echo "Running time adjuster for computing smear corrections for: ${label}" 
    ./scripts/runTimeAdjuster.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${filedump}" "${base_adjust_var}" "${base_time_var}" 0 1 ${savemetadata}
done

## rm tmp file
rm "${filedump}"

## plot raw shift
echo "Making raw smear plots"
./scripts/makeCorrPlots.sh "${outdirbase}/${smeardir}" ${savemetadata} "SMEAR"

#################
## Final plots ##
#################

## lauch plots with TOF + SHIFT + SMEAR corrections
echo "Making time related plots with TOF + SHIFT + SMEAR corrections"
./scripts/makeTimePlots.sh "${outdirbase}/${smeardir}" ${savemetadata} "true" "true" "true" "false"

## Final prep dir
echo "Final prep outdir"
PrepOutDir "${topdir}/${disphodir}/${outdirbase}"

## all done
echo "Finished full chain of time study"
