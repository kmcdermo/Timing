#!/bin/bash

label=${1:-"ex1pho_x_1.5_y_200"}

## labels
filename_stat="stat_unc.root"
filename_tot="tot_unc.root"

## base command
base_exe="combine -M FitDiagnostics datacardABCD_${label}.txt --forceRecreateNLL"

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
root -l -b -q test_macros/printDiagnostics.C\(\"${filename_stat}\",\"${filename_tot}\",\"${label}\"\) >& "${tmp}"

rm "${tmp}"
rm "${filename_stat}"
rm "${filename_tot}"

## final message
echo "Finishing running diagnostics"
