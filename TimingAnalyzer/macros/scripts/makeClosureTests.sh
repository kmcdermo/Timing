#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Config
outfiletext=${1:-"met_vs_time"}
outdir=${2:-"madv2_v4/uncs/closure"}

save_meta_data=${3:-0}
do_cleanup=${4:-"true"}

## plot config
plot_config="${plotconfigdir}/met_vs_time_fine.${inTextExt}"

## misc config
misc_config="misc_time_met_fine.${inTextExt}"
> "${misc_config}"
echo "skip_bkgd_mc=1" >> "${misc_config}"
echo "skip_signal=1" >> "${misc_config}"

## output text file (used by all inputs)
outtextfile="${outfiletext}.${outTextExt}"
> "${outtextfile}"
echo "Control Region [Data],Time Boundary [ns],MET Boundary [GeV],obsA,obsB,obsC,obsD,c1{obsB/obsA},c2{obsD/obsA},predC{obsA*c1*c2},Percent Diff (1-predC/obsC),pullC{(obsC-predC)/sqrt(obsCunc^2+predCunc^2)}" >> "${outtextfile}"

###########################
## Main Loop Over Inputs ##
###########################

for input in "${inputs[@]}"
do
    echo "${!input}" | while read -r label infile insigfile sel
    do
	## first make the 2D hist
	./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "NOSIGNALS.root" "${cutconfigdir}/always_true.${inTextExt}" "${plot_config}" "${misc_config}" "${MainEra}" ${save_meta_data} "${outfiletext}_${label}" "${outdir}"

	## run extractor
	./scripts/extractClosureUncertainty.sh "${label}" "${outfiletext}" "${outdir}"
    done
done

##########################
## Final Copy to OutDir ##
##########################

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy log file!
cp "${outtextfile}" "${fulldir}"

###################
## Final Cleanup ##
###################

if [[ "${do_cleanup}" == "true" ]]
then
    rm "${misc_config}"
    ./scripts/cleanup.sh
fi

###################
## Final Message ##
###################

echo "Finished Making Closure Tests!"
