#!/bin/bash

gjets="skims/v1/orig_2phosCR/gjets.root GJetsCR" 
qcd="skims/v1/orig_2phosCR/qcd.root QCDCR"
declare -a inputs=(gjets qcd)

basedir="/eos/user/k/kmcdermo/www"
dir="dispho/plots/madv2_v1/test/adish_abcd_v1"
outdir=${basedir}/${dir}
mkdir -p ${outdir}

for input in "${inputs[@]}"
do
    echo ${!input} | while read -r filename label
    do
	textfilename="${label}_C_region_v1.txt"
	> ${textfilename}

	root -l -b -q list_of_events.C\(\"${filename}\",\"${textfilename}\"\)

	mv ${textfilename} ${outdir}
    done
done

pushd ${basedir}
./copyphp.sh ${dir}
popd
