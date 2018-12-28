#!/bin/bash

basedir="/eos/user/k/kmcdermo/www"
dir="dispho/plots/madv2_v2/test/newmet/adish_abcd_v1"
outdir=${basedir}/${dir}
mkdir -p ${outdir}

gjets_data="skims/v2/orig_2phosCR/gjets.root Data_Tree NONE Data_GJetsCR"
gjets_gmsb_l100_ctau200="skims/v2/orig_2phosCR/signals_gjets.root GMSB_L100_CTau200_Tree MC(1) GMSB_L100_CTau200_GJetsCR"
gjets_gmsb_l300_ctau200="skims/v2/orig_2phosCR/signals_gjets.root GMSB_L300_CTau200_Tree MC(1) GMSB_L300_CTau200_GJetsCR"
gjets_gmsb_l500_ctau200="skims/v2/orig_2phosCR/signals_gjets.root GMSB_L500_CTau200_Tree MC(1) GMSB_L500_CTau200_GJetsCR"
declare -a inputs=(gjets_data gjets_gmsb_l100_ctau200 gjets_gmsb_l300_ctau200 gjets_gmsb_l500_ctau200)

seedtime="phoseedtime_0 Seed"
wgttime="phoweightedtimeLT120_0 Weighted"
declare -a times=(seedtime wgttime)

oldmet="t1pfMETpt Old"
newmet="hackedMETpt New"
declare -a mets=(oldmet newmet)

for time in "${times[@]}"
do
    echo ${!time} | while read -r timevar timelabel
    do
	for met in "${mets[@]}"
	do
	    echo ${!met} | while read -r metvar metlabel
	    do
		textfile="${metlabel}MET_v_${timelabel}Time.csv"
		> ${textfile}
		echo "Sample,A,B,C,D,B/A(=c1),C/D,D/A(=c2),C/B,PredC(A*c1*c2),PredCUnc" >> ${textfile}

		for input in "${inputs[@]}"
		do
		    echo ${!input} | while read -r filename treename selection label
		    do
			root -l -b -q test_macros/fast2D_with_correlation.C\(\"${filename}\",\"${treename}\",\"${selection}\",\"${label}\",\"${timevar}\",\"${timelabel}\",\"${metvar}\",\"${metlabel}\",\"${textfile}\",\"${outdir}\"\)
		    done
		done

		mv ${textfile} ${outdir}
	    done
	done
    done
done

pushd ${basedir}
./copyphp.sh ${dir}
popd
