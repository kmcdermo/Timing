#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
outdir=${1:-"madv2_v3/full_chain/ABCD/combine_input"}

is_blind=${2:-"true"}
use_systematics=${3:-"false"}
do_cleanup=${4:-"true"}

## Derived Config
plotfilename="${plotfiletext}.root"

## use prediction or real data? 
if [[ "${is_blind}" == "true" ]]
then
    blind_data=1
else
    blind_data=0
fi

## use systematics in making datacards?
if [[ "${use_systematics}" == "true" ]]
then
    include_systematics=1
else
    include_systematics=0
fi

## ABCD info
bininfoname="bininfo.${inTextExt}"
ratioinfoname="ratioinfo.${inTextExt}"
binratioinfoname="binratioinfo.${inTextExt}"

## datacard+ws ninfo
systfilename="${systconfigdir}/systematics.${inTextExt}"

######################
## Make ABCD Config ##
######################

./scripts/runABCDGenerator.sh "${plotfilename}" "${bininfoname}" "${ratioinfoname}" "${binratioinfoname}" "${outdir}/${incombdir}"

################################
## Write Results to Datacards ##
################################

./scripts/runCombinePreparer.sh "${plotfilename}" "${bininfoname}" "${ratioinfoname}" "${binratioinfoname}" "${systfilename}" "${datacardname}" ${blind_data} ${include_systematics} "${outdir}/${incombdir}"

#######################################
## Move WS and Datacards to Tmp File ##
#######################################

mv "${datacardname}"*".${inTextExt}" "${inlimitdir}"

###########################
## Clean Up If Requested ##
###########################

if [[ "${do_cleanup}" == "true" ]]; then
    echo "Cleaning up tmp ABCD configs"
    rm "${bininfoname}"
    rm "${ratioinfoname}"
    rm "${binratioinfoname}"
fi

###################
## Final Message ##
###################

echo "Finished MakingDatacards (ABCD)"
