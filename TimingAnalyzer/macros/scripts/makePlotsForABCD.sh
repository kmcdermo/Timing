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
outfiletext=${5:-"met_vs_time"}
outdir=${6:-"madv2_v3/full_chain/results_ABCD"}
is_blind=${7:-"true"}
save_meta_data=${8:-0}
do_cleanup=${9:-"true"}

## other global config (derived)
outplot2Ddir="plots2D"
xboundary=$( echo "${xbin}" | sed -r 's/_/ /g' )
yboundary=$( echo "${ybin}" | sed -r 's/_/ /g' )

## make tmp misc config
misc_config="misc_ABCD.${inTextExt}"
> "${misc_config}"

## fill tmp misc config
echo "skip_bkgd_mc=1" >> "${misc_config}"
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

## make tmp config for plotting
plot_config="met_${ybin}_vs_time_${xbin}_config.${inTextExt}"
> "${plot_config}"

## common info for titles
xtitle="Leading Photon Weighted Cluster Time [ns]"
ytitle="p_{T}^{miss} [GeV]"

## fill tmp plot config
echo "plot_title=${ytitle} vs. ${xtitle}" >> "${plot_config}"
echo "x_title=${xtitle}" >> "${plot_config}"
echo "x_var=phoweightedtimeLT120_0" >> "${plot_config}"
echo "x_var_data=+phoweightedtimeLT120SHIFT_0" >> "${plot_config}"
echo "x_var_sign=+phoweightedtimeLT120SHIFT_0+phoweightedtimeLT120SMEAR_0" >> "${plot_config}"
echo "x_bins=VARIABLE -2 ${xboundary} 25" >> "${plot_config}"
echo "y_title=${ytitle}" >> "${plot_config}"
echo "y_var=t1pfMETpt" >> "${plot_config}"
echo "y_bins=VARIABLE 0 ${yboundary} 3000" >> "${plot_config}"
echo "z_title=Events/ns/GeV" >> "${plot_config}"
echo "blinding=(${xblind},+Inf,${yblind},+Inf)" >> "${plot_config}"

## use only SR
echo "${SR}" | while read -r label infile insigfile sel
do    
    ## make plot
    echo "Making 2D plot with ${label}"

    ./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${plot_config}" "${misc_config}" "${MainEra}" ${save_meta_data} "${outfiletext}" "${outdir}/${outplot2Ddir}"
done

## delete tmp files
if [[ "${do_cleanup}" == "true" ]]
then
    rm "${plot_config}"
    rm "${misc_config}"
fi

## end message
echo "Finished making 2D plot"
