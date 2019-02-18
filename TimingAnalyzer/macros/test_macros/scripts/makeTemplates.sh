#!/bin/bash

## source
source scripts/common_variables.sh

## input from command line
outfiletext=${1:-"met_vs_time_templates"}
outdir=${2:-"madv2_v3/checks_v12"}

## global config
plot="met_vs_time_fine"
savemetadata=1

calibdir="/eos/cms/store/group/phys_exotica/displacedPhotons/nTuples/2017/analysis/calib"

## make misc_config
misc="misc_config.${inTextExt}"
> "${misc}"

echo "scale_to_bin_widths=0" >> "${misc}"
echo "blind_data=1" >> "${misc}"

## make 2D Hist
for input in "${inputs[@]}"
do
    if [[ "${input}" != "SR" ]]
    then
	continue
    fi

    echo ${!input} | while read -r label infile insigfile sel varwgtmap
    do
	echo "Doing 2D plot for: ${label}"
	
	./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${varwgtconfigdir}/empty.${inTextExt}" "${plotconfigdir}/${plot}.${inTextExt}" "${misc}" "Full" ${savemetadata} "${outfiletext}" "${outdir}"
    done
done

## make templates
echo "Making Templates from 2D"
root -l -b -q test_macros/makeTemplates.C\(\"${outfiletext}\"\)

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
cp "${outfiletext}.root" "${fulldir}"

## rm tmp file
rm "${misc}"

## end message
echo "All done!"
