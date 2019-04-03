#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
nom_skim=${1:-"skims/v4/syst_nom/signals_sr.root"}
unc_skim=${2:-"skims/v4/pho_sc_up/signals_sr.root"}

xbin=${3:-""}
xblind=${4:-""}
ybin=${5:-""}
yblind=${6:-""}

plotlabel=${7:-"x_${xbin}_y_${ybin}"}
outdir=${8:-"madv2_v4/uncs/pho_scale_up"}
save_meta_data=${9:-0}
do_cleanup=${10:-"true"}

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
echo "skip_data=1" >> "${misc_config}"
echo "skip_bkgd_mc=1" >> "${misc_config}"

## make plot for nominal first
echo "Making 2D plot for nominal"

./scripts/runTreePlotter2D.sh "NO_DATA.root" "${nom_skim}" "${cutconfigdir}/always_true.${inTextExt}" "${plot_config}" "${misc_config}" "${MainEra}" ${save_meta_data} "${plotfiletext}_${plotlabel}" "${outdir}/${nom2Ddir}"

## make plot for systematic second
echo "Making 2D plot for systematic test"

./scripts/runTreePlotter2D.sh "NO_DATA.root" "${unc_skim}" "${cutconfigdir}/always_true.${inTextExt}" "${plot_config}" "${misc_config}" "${MainEra}" ${save_meta_data} "${plotfiletext}_${plotlabel}" "${outdir}/${unc2Ddir}"

## delete tmp files
if [[ "${do_cleanup}" == "true" ]]
then
    rm "${plot_config}"
    rm "${misc_config}"
fi

## end message
echo "Finished making 2D plot"
