#!/bin/bash

##################
## Source First ##
##################
source scripts/common_variables.sh

############
## Config ##
############

skipdata=${1:-0}
skipbkgdmc=${2:-0}
skipsignal=${3:-0}
savemetadata=${4:-0}
filedump="tmp_infiles.${inTextExt}"

############################
## Make shift corrections ##
############################

> "${filedump}"
echo "EB_Full=/eos/cms/store/group/phys_exotica/displacedPhotons/nTuples/2017/analysis/calib/phoE_0_Zee_EB_Full_ShiftInput_230419.root" >> "${filedump}"
echo "EE_Full=/eos/cms/store/group/phys_exotica/displacedPhotons/nTuples/2017/analysis/calib/phoE_0_Zee_EB_Full_ShiftInput_230419.root" >> "${filedump}"

## launch time adjuster, over each input selection
for input in "${inputs[@]}"
do echo ${!input} | while read -r label infile insigfile sel
    do
  	echo "Running time adjuster for computing shift corrections for: ${label}" 
   	./scripts/runTimeAdjuster.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${filedump}" "${base_adjust_var}" "${base_time_var}" 1 0 ${skipdata} ${skipbkgdmc} ${skipsignal} ${savemetadata}
    done
done

## rm tmp file
rm "${filedump}"

############################
## Make smear corrections ##
############################

> "${filedump}"
echo "EB_Full=/eos/cms/store/group/phys_exotica/displacedPhotons/nTuples/2017/analysis/calib/phoE_0_Zee_EB_Full_SmearInput_230419.root" >> "${filedump}"
echo "EE_Full=/eos/cms/store/group/phys_exotica/displacedPhotons/nTuples/2017/analysis/calib/phoE_0_Zee_EB_Full_SmearInput_230419.root" >> "${filedump}"

## launch time adjuster, over each input selection
for input in "${inputs[@]}"
do echo ${!input} | while read -r label infile insigfile sel
    do
  	echo "Running time adjuster for computing smear corrections for: ${label}" 
   	./scripts/runTimeAdjuster.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${filedump}" "${base_adjust_var}" "${base_time_var}" 0 1 ${skipdata} ${skipbkgdmc} ${skipsignal} ${savemetadata}
    done
done

## rm tmp file
rm "${filedump}"

## all done
echo "Finished full chain of time adjusting only"
