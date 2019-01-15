#!/bin/bash

basedir="/eos/user/k/kmcdermo/www"
dir="dispho/plots/madv2_v3/test/ABCD"
outdir="${basedir}/${dir}"
mkdir -p "${outdir}"

gjets_data="skims/v3/orig_2phosCR/gjets.root Data_Tree NONE Data_GJetsCR"
qcd_data="skims/v3/orig_2phosCR/gjets.root Data_Tree NONE Data_QCDCR"
declare -a inputs=(gjets_data qcd_data)

declare -a time_bins=(2 3)
declare -a met_bins=(100 200)

for time_bin in "${time_bins[@]}"
do
    for met_bin in "${met_bins[@]}"
    do
	textfile="MET${met_bin}_v_Time${time_bin}.csv"
	> ${textfile}
	echo "Sample,A,B,C,D,B/A(=c1),D/A(=c2),PredC(A*c1*c2)" >> ${textfile}
	
	for input in "${inputs[@]}"
	do
	    echo ${!input} | while read -r filename treename selection label
	    do
		root -l -b -q test_macros/faster2D.C\(\"${filename}\",\"${treename}\",\"${selection}\",\"${label}\",\"${time_bin}\",\"${timelabel}\",\"${met_bin}\",\"${textfile}\",\"${outdir}\"\)
	    done
	done
	
	mv "${textfile}" "${outdir}"
    done
done

pushd "${basedir}"
./copyphp.sh "${dir}"
popd
