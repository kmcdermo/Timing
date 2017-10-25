#!/bin/bash

indir=${1:-"output"}
outdir=${2:-"eff_areas"}

mkdir -p ${outdir}

declare -a arr=("phoecaliso_EB_OOT_v_nvtx_quant_0.90"
                "phohcaliso_EB_OOT_v_nvtx_quant_0.90"
		"photrkiso_EB_OOT_v_nvtx_quant_0.90")
for i in "${arr[@]}"
do
    cp ${indir}/photon/iso_v_nvtx/effarea/lin/${i}.png ${outdir}    
    grep ${i} ${indir}/effareas.txt >> ${outdir}/effareas.txt
done

cp ${indir}/standard/effarea/lin/rho_v_nvtx_mean.png ${outdir}



