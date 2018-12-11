#!/bin/bash

gjets="skims/v2/orig_2phosCR/gjets.root NONE GJetsCR" 
qcd="skims/v2/orig_2phosCR/qcd.root NONE QCDCR"
dyll="skims/v2/zee/skim.root phoisEB_0&&phopt_0>40&&phoisEB_1&&phopt_1>40&&hltDiEle33MW DYCR"
declare -a inputs=(gjets qcd dyll)

basedir="/eos/user/k/kmcdermo/www"
dir="dispho/plots/madv2_v2/test/adish_abcd_v7"
outdir=${basedir}/${dir}
mkdir -p ${outdir}

textfile="output.csv"
> ${textfile}
echo "Sample,A,B,C,D,B/A(=c1),C/D,D/A(=c2),C/B,PredC(A*c1*c2),PredCUnc" >> ${textfile}

for input in "${inputs[@]}"
do
    echo ${!input} | while read -r filename selection label
    do
	root -l -b -q test_macros/fast2D_with_correlation.C\(\"${filename}\",\"${selection}\",\"${label}\",\"${textfile}\",\"${outdir}\"\)
    done
done

mv ${textfile} ${outdir}

pushd ${basedir}
./copyphp.sh ${dir}
popd
