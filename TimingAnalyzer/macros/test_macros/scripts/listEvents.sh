#!/bin/bash

gjets="skims/v2/orig_2phosCR/gjets.root GJetsCR" 
qcd="skims/v2/orig_2phosCR/qcd.root QCDCR"
declare -a inputs=(gjets qcd)

basedir="/eos/user/k/kmcdermo/www"
dir="dispho/plots/madv2_v2/test/adish_abcd_v1"
outdir=${basedir}/${dir}
mkdir -p ${outdir}

for input in "${inputs[@]}"
do
    echo ${!input} | while read -r filename label
    do
	textfilename="${label}_all_regions.txt"
	> ${textfilename}

	outfilename="${label}_all_regions.root"

	root -l -b -q test_macros/list_of_events.C\(\"${filename}\",\"${textfilename}\",\"${outfilename}\"\)

	mv ${textfilename} ${outfilename} ${outdir}
    done
done

pushd ${basedir}
./copyphp.sh ${dir}
popd
