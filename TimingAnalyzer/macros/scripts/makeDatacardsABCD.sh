#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
plotfilename=${1:-"met_vs_time.root"}
datacardname=${2:-"datacardABCD"}
outdir=${3:-"madv2_v3/full_chain/ABCD/combine_input"}
inlimitdir=${4:-"input"}
is_blind=${5:-"true"}
do_cleanup=${6:-"true"}

## ABCD info
bininfoname="bininfo.${inTextExt}"
ratioinfoname="ratioinfo.${inTextExt}"
binratioinfoname="binratioinfo.${inTextExt}"

## datacard+ws ninfo
systfilename="${systconfigdir}/systematics.${inTextExt}"

## derived config
if [[ "${is_blind}" == "true" ]]
then
    blind_data=1
else
    blind_data=0
fi

######################
## Make ABCD Config ##
######################

./scripts/runABCDGenerator.sh "${plotfilename}" "${bininfoname}" "${ratioinfoname}" "${binratioinfoname}" "${outdir}"

################################
## Write Results to Datacards ##
################################

./scripts/runCombinePreparer.sh "${plotfilename}" "${bininfoname}" "${ratioinfoname}" "${binratioinfoname}" "${systfilename}" "${datacardname}" ${blind_data} "${outdir}"

#######################################
## Move WS and Datacards to Tmp File ##
#######################################

mv "${datacardname}"*".${inTextExt}" "${inlimitdir}"
mv "${wsfilename}" "${inlimitdir}"

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

echo "Finished MakingWSsAndDatacards (ABCD)"
