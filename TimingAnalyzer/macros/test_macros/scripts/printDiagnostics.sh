#!/bin/bash

label=${1:-"ex1pho_x_1.5_y_200"}
maskC=${2:-0} # if 0, mask bin C

## labels
filename_stat="stat_unc.root"
filename_tot="tot_unc.root"

## tmp mask names
mask_tmp="test_datacard"

## base command
if (( ${maskC} == 0 ))
then
    base_exe="combine -M FitDiagnostics datacardABCD_${label}.txt --forceRecreateNLL"
elif (( ${maskC} == 1 ))
then
    echo "Making temporary workspace to mask binC"
    combineCards.py "datacardABCD_${label}.txt" -S > "${mask_tmp}.txt"
    text2workspace.py "${mask_tmp}.txt" --channel-masks

    base_exe="combine -M FitDiagnostics ${mask_tmp}.root --setParameters mask_ch1_Bin3_2017=1"
else
    echo "maskC can only be 0 (full datacard) or 1 (mask binC)! Value: ${maskC} is no good! Exiting..."
    exit
fi

## tmp file
tmp="tmp.log"

## combine: stat only
echo "Running combine: stat only for ${label}"
${base_exe} -S 0 >& "${tmp}"

rm "${tmp}"
rm "combine_logger.out"
rm "higgsCombineTest.FitDiagnostics.mH120.root"
mv "fitDiagnostics.root" "${filename_stat}"

## combine: tot
echo "Running combine: stat + syst for ${label}"
${base_exe} >& "${tmp}"

rm "${tmp}"
rm "combine_logger.out"
rm "higgsCombineTest.FitDiagnostics.mH120.root"
mv "fitDiagnostics.root" "${filename_tot}"

## run macro
echo "Run macro to printout yields"
root -l -b -q printDiagnostics.C\(\"${filename_stat}\",\"${filename_tot}\",\"${label}\",${maskC}\) >& "${tmp}"

rm "${tmp}"
rm "${filename_stat}"
rm "${filename_tot}"

## cleanup maskC
if (( ${maskC} == 1 ))
then
    rm "${mask_tmp}.txt"
    rm "${mask_tmp}.root"
fi

## final message
echo "Finishing running diagnostics"
