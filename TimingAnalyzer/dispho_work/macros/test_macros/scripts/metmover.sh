#!/bin/bash

gjets="skims/v2/orig_2phosCR/gjets.root Data_Tree GJetsCR"
declare -a inputs=(gjets)

basedir="/eos/user/k/kmcdermo/www"
dir="dispho/plots/madv2_v2/test/newmet"
outdir=${basedir}/${dir}
mkdir -p ${outdir}

for input in "${inputs[@]}"
do
    echo ${!input} | while read -r filename treename label
    do
	textfilename="${label}_METcounters.txt"
	> ${textfilename}

	outfiletext="${label}_METcomp"

	root -l -b -q test_macros/met_move.C\(\"${filename}\",\"${treename}\",\"${textfilename}\",\"${outfiletext}\"\)

	mv ${textfilename} ${outfiletext}.png ${outdir}
    done
done

pushd ${basedir}
./copyphp.sh ${dir}
popd
