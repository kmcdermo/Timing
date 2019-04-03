#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
plot_config=${1:-"met_vs_time.txt"}
xboundary=${2:-""}
yboundary=${3:-""}
xblind=${4:-""}
yblind=${5:-""}

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
echo "z_title=Events" >> "${plot_config}"
echo "blinding=(${xblind},+Inf,${yblind},+Inf)" >> "${plot_config}"

## final message
echo "Quickly made 2D Plot Config"
