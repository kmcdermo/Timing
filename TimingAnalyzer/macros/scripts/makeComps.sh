#!/bin/bash

## source first
source ./scripts/common_variables.sh

## command line input
indir=${1:-"compare_input"}
plot=${2:-"met_zoom"}
savemetadata=${3:-0}
outdir=${4:-"ntuples_v5/checks_v4/era_comp"}

for era in "${eras[@]}"
do
    # make config
    tmp_config="compare_config.txt"
    > ${tmp_config}

    # fill config
    echo "ratio_type=DIVONLY" >> "${tmp_config}"
    echo "ratio_y_title=After/Before" >> "${tmp_config}"
    echo "ratio_min_y=0" >> "${tmp_config}"
    echo "ratio_max_y=2" >> "${tmp_config}"

    echo "file_name_1=${indir}/after.root" >> "${tmp_config}"
    echo "file_name_2=${indir}/before.root" >> "${tmp_config}"

    echo "hist_name_1=${era}_Hist" >> "${tmp_config}"
    echo "hist_name_2=${era}_Hist" >> "${tmp_config}"

    echo "label_1=After" >> "${tmp_config}"
    echo "label_2=Before" >> "${tmp_config}"

    # run macro
    ./scripts/runPlotComparator.sh "${tmp_config}" "${era}" ${savemetadata} "${era}_${plot}" "${outdir}"

    # tidy up
    rm -rf "${tmp_config}"
done

# final message
echo "Finished MakingComps"
