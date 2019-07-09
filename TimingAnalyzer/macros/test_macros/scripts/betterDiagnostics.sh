#!/bin/bash

## input
category=${1:-"in2pho_x_1.5_y_200"}

## main function
function RunCombine()
{
    local category=${1}
    local label=${2}
    local exe=${3}

    local tmp_log="tmp.log"
    
    echo "Running combine for: ${category} with: ${label}"
    ${exe} >& "${tmp_log}"
    
    rm "${tmp_log}"
    rm "combine_logger.out"
    rm "higgsCombineTest.FitDiagnostics.mH120.root"
    mv "fitDiagnostics.root" "${label}.root"
}

## labels
fullC="fullC"
maskC="maskC"
tmp_log="tmp.log"
tmp_ws="tmp_ws"

## convert to workspace
echo "Making temporary workspace (include channel masks)"
combineCards.py "datacardABCD_${category}.txt" -S > "${tmp_ws}.txt"
text2workspace.py "${tmp_ws}.txt" --channel-masks &> "${tmp_log}"

## base combine command
exe="combine -M FitDiagnostics ${tmp_ws}.root --saveShapes --saveWithUnc"

## full binC
RunCombine "${category}" "${fullC}" "${exe}"

## mask binC
exe+=" --setParameters mask_ch1_Bin3_2017=1"
RunCombine "${category}" "${maskC}" "${exe}"

## run macro
echo "Run macro to printout yields"
root -l -b -q betterDiagnostics.C\(\"${fullC}\",\"${maskC}\",\"${category}\"\) &> "${tmp_log}"

rm "${tmp_log}"
rm "${tmp_ws}.root"
rm "${tmp_ws}.txt"
rm "${fullC}.root"
rm "${maskC}.root"

## final message
echo "Finishing running diagnostics"
