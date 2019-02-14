#!/bin/bash

##################
## Source First ##
##################
source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
outdir=${1:-"madv2_v3/full_chain/ABCD"}
is_blind=${2:-"true"}
ws_filename=${3:-"ws_final.root"}
savemetadata=${4:-0}
docleanup=${5:-"true"}
x=${6-""}
y=${7-""}
# x1=${6-""}
# x2=${7-""}
# y1=${8-""}
# y2=${9-""}

##################################################################################################
##                                       Scan outline                                           ##
##                                                                                              ##
## 1) Loop over all 2D bin combos and compute significance for each MC                          ##
##    A) Run macro to produce signal MC and data plots, skip bkgd (scale DOWN)                  ##
##    B) If blind data, set C to B*D/A, otherwise use real data                                 ##
##    C) Run macro to compute significance of each signal MC, dump into huge text file          ##
##                                                                                              ##
## 2) Select highest significance for each signal MC and store in output file                   ##
##    A) Run macro to read text file: map<Signal Point,vector<Significance,Xbin,Ybin,FileName>  ##
##    B) Sort vectors in map by highest significance, keeping lower xy combos                   ##
##    C) Combine all MC + data hists into one root file (scale UP)                              ##
##                                                                                              ##
## 3) Plot final significances                                                                  ##
##                                                                                              ##
## 4) Clean up the tmp stuff                                                                    ##
##################################################################################################

############
## Step 1 ##
############

## config
outplot2Ddir="plots2D"

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

## common info for titles
plot_title="p_{T}^{miss} [GeV] vs Leading Photon Weighted Cluster Time [ns]"
x_title="Leading Photon Weighted Cluster Time [ns]"
y_title="p_{T}^{miss} [GeV]"

## make tmp config for plotting
plot="met_${y}_vs_time_${x}_box"
#plot="met_${y1}_${y2}_vs_time_${x1}_${x2}_box"
plot_config="${plot}_config.${inTextExt}"
> "${plot_config}"

## fill tmp plot config
echo "plot_title=${plot_title}" >> "${plot_config}"
echo "x_title=${x_title}" >> "${plot_config}"
echo "x_var=phoweightedtimeLT120_0" >> "${plot_config}"
echo "x_var_data=+phoweightedtimeLT120SHIFT_0" >> "${plot_config}"
echo "x_var_sign=+phoweightedtimeLT120SHIFT_0+phoweightedtimeLT120SMEAR_0" >> "${plot_config}"
echo "x_bins=VARIABLE -2 ${x} 25" >> "${plot_config}"
#echo "x_bins=VARIABLE -2 ${x1} ${x2} 25" >> "${plot_config}"
echo "y_title=${y_title}" >> "${plot_config}"
echo "y_var=t1pfMETpt" >> "${plot_config}"
echo "y_bins=VARIABLE 0 ${y} 3000" >> "${plot_config}"
#echo "y_bins=VARIABLE 0 ${y1} ${y2} 3000" >> "${plot_config}"
echo "z_title=Events/ns/GeV" >> "${plot_config}"
echo "blinding=(${x},+Inf,${y},+Inf)" >> "${plot_config}"
#echo "blinding=(${x2},+Inf,${y2},+Inf)" >> "${plot_config}"

## loop over inputs (in reality, just SR) --> produce plots + significance dump
for input in "${inputs[@]}"
do
    echo ${!input} | while read -r label infile insigfile sel varwgtmap
    do
        ## tmp out name
	outtext="${plot}_${label}"
	
	echo "Computing significance for ${outtext}"
	
        ## make plot
	./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${varwgtconfigdir}/empty.${inTextExt}" "${plot_config}" "${misc_config}" "${MainEra}" ${savemetadata} "${outtext}" "${outdir}/${outplot2Ddir}"

	cp "${outtext}.root" "${ws_filename}"
    done # end read of input
done # end loop over input settings

## delete tmp files
if [[ "${docleanup}" == "true" ]]
then
    rm "${plot_config}" "${misc_config}"
fi

## end message
echo "Finished significance scan"
