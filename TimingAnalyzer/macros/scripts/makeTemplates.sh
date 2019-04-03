#!/bin/bash

## source
source scripts/common_variables.sh

## input from command line
outfiletext=${1:-"met_vs_time_templates"}
outdir=${2:-"ootVID_v3/std/templates"}

x_label=${3:-"Time"}
x_low=${4:-"-1"} # Time ns
x_high=${5:-"1"} # Time ns

y_label=${6:-"MET"}
y_low=${7:-"0"} # MET GeV
y_high=${8:-"100"} # MET GeV

## global config
plot="${plotconfigdir}/met_vs_time_fine.${inTextExt}" ## constant size bins with super fine bin widths
savemetadata=1
calibdir="/eos/cms/store/group/phys_exotica/displacedPhotons/nTuples/2017/analysis/calib"

## make misc_config
misc="misc_config.${inTextExt}"
> "${misc}"

echo "scale_to_bin_widths=0" >> "${misc}"
echo "blind_data=1" >> "${misc}"

signals_to_plot="signals_to_plot="
for lambda in 100 200 300
do 
    for ctau in 0p1 200 1000
    do
	signals_to_plot+="GMSB_L${lambda}_CTau${ctau} "
    done
done

echo "${signals_to_plot}" >> "${misc}"

## make 2D Hist: use SR only
echo "${SR}" | while read -r label infile insigfile sel
do
    echo "Doing 2D plot for: ${label}"
    
    ./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${plot}" "${misc}" "Full" ${savemetadata} "${outfiletext}" "${outdir}"
done

## make templates
echo "Making Templates from 2D"
root -l -b -q generateTemplates.C\(\"${outfiletext}\",\"${misc}\",\"${x_label}\",\"${x_low}\",\"${x_high}\",\"${y_label}\",\"${y_low}\",\"${y_high}\"\)

## copy to calib dir
cp "${outfiletext}.root" "${calibdir}"

## copy the latest to outdir
fulldir="${topdir}/${disphodir}/${outdir}"

for canvscale in "${canvscales[@]}"
do
    for ext in "${exts[@]}"
    do
	cp "${outfiletext}_projX_${canvscale}.${ext}" "${fulldir}"
	cp "${outfiletext}_projY_${canvscale}.${ext}" "${fulldir}"
    done
done

cp "${outfiletext}_projX.${outTextExt}" "${fulldir}"
cp "${outfiletext}_projY.${outTextExt}" "${fulldir}"
cp "${outfiletext}.root" "${fulldir}"

## rm tmp file
rm "${misc}"

## end message
echo "All done!"
