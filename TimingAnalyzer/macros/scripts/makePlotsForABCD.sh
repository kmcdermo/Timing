#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
xbin=${1:-""}
xblind=${2:-""}
ybin=${3:-""}
yblind=${4:-""}
outdir=${5:-"madv2_v3/full_chain/results_ABCD"}
is_blind=${6:-"true"}
save_meta_data=${7:-0}
do_cleanup=${8:-"true"}

## other global config (derived)
xboundary=$( echo "${xbin}" | sed -r 's/_/ /g' )
yboundary=$( echo "${ybin}" | sed -r 's/_/ /g' )

## make tmp config for plotting
plot_config="met_${ybin}_vs_time_${xbin}_config.${inTextExt}"
> "${plot_config}"

## fill tmp plot config
./scripts/fill2DConfig.sh "${plot_config}" "${xboundary}" "${yboundary}" "${xblind}" "${yblind}"

## make tmp misc config
misc_config="misc_ABCD.${inTextExt}"
> "${misc_config}"

## fill tmp misc config
echo "scale_to_bin_widths=0" >> "${misc_config}"

## set blinding
if [[ "${is_blind}" == "true" ]]
then
    echo "blind_data=1" >> "${misc_config}"
    blind_data=1
else
    echo "blind_data=0" >> "${misc_config}"
    blind_data=0
fi

## use only SR
echo "${SR}" | while read -r label infile insigfile sel
do    
    ## make plot
    echo "Making 2D plot with ${label}"

    ./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${plot_config}" "${misc_config}" "${MainEra}" ${save_meta_data} "${plotfiletext}" "${outdir}/${outplot2Ddir}"
done

## delete tmp files
if [[ "${do_cleanup}" == "true" ]]
then
    rm "${plot_config}"
    rm "${misc_config}"
fi

## end message
echo "Finished making 2D plot"
