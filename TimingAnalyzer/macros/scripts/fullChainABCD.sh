#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"madv2_v3/full_chain"}
is_blind=${2:-"true"}
use_obs=${3:-"false"}
savemetadata=${4:-0}
docleanup=${5:-"true"}

## for safety
echo "Compiling ahead of time"
./scripts/compile.sh

## xs, ys
declare -a xs=(0 0.5 1 1.5 2 3 5)
declare -a ys=(50 100 150 200 300 500 1000)

for x in "${xs[@]}"
do
    for y in "${ys[@]}"
    do
	echo "Making analysis plots + limits for ${x},${y}"
	./scripts/makeAnalysisABCD.sh "${outdir}/x_${x}_y_${y}" "${is_blind}" "${use_obs}" ${savemetadata} "${docleanup}" "${x}" "${y}"
    done
done

## x1_x2s, y1_y2s
#declare -a x1_x2s=("0 1.5" "0 2" "0 5" "1 1.5" "1 2" "1 5" "1.5 3" "1.5 5" "2 5")
#declare -a y1_y2s=("50 100" "50 200" "50 500" "100 200" "100 500" "200 500")

## make analysis plots + limits
# for x1_x2 in "${x1_x2s[@]}"
# do echo ${x1_x2} | while read -r x1 x2
#     do
# 	for y1_y2 in "${y1_y2s[@]}"
# 	do echo ${y1_y2} | while read -r y1 y2
# 	    do
# 		echo "Making analysis plots + limits for ${x1}-${x2},${y1}-${y2}"
# 		./scripts/makeAnalysisABCD.sh "${outdir}/x_${x1}_${x2}_y_${y1}_${y2}" "${is_blind}" "${use_obs}" ${savemetadata} "${docleanup}" "${x1}" "${x2}" "${y1}" "${y2}"
# 	    done
# 	done
#     done
# done

## final prep dir
echo "Final prep outdir"
PrepOutDir "${topdir}/${disphodir}/${outdir}"

## all done
echo "Finished full chain of analysis (ABCD)"
