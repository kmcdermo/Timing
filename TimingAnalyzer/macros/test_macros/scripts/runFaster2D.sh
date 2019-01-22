#!/bin/bash

basedir="/eos/user/k/kmcdermo/www"
dir="dispho/plots/madv2_v3/test/ABCD_lv"
outdir="${basedir}/${dir}"
mkdir -p "${outdir}"

# gjets="skims/v3/orig_2phosCR/gjets.root Data_Tree NONE GJets"
# qcd="skims/v3/orig_2phosCR/qcd.root Data_Tree NONE QCD"
# dyll="skims/v3/orig_2phosCR/dyll.root Data_Tree NONE DYLL"

gjets="skims/v3/orig_2phosCR/gjets.root Data_Tree (nelLowM==0&&nmuLowM==0) GJets"
qcd="skims/v3/orig_2phosCR/qcd.root Data_Tree (nelLowM==0&&nmuLowM==0) QCD"
dyll="skims/v3/orig_2phosCR/dyll.root Data_Tree NONE DYLL"

declare -a inputs=(gjets qcd dyll)

declare -a time_bins=(1 2)
declare -a met_bins=(100 150 200)

textfile="MET_v_Time_lv.csv"
> ${textfile}
echo "Control Region [Data],Time Boundary [ns],MET Boundary [GeV],obsA,obsB,obsC,obsD,c1{obsB/obsA},c2{obsD/obsA},predC{obsA*c1*c2},pullObsC{(obsC-predC)/obsCunc},pullObsCLow{(obsC-predC)/obsCuncLow},pullObsCUp{(obsC-predC)/obsCuncUp},pullPredC{(obsC-predC)/predCunc},pullPredCLow{(obsC-predC)/predCuncLow},pullPredCUp{(obsC-predC)/predCuncUp},pullC{(obsC-predC)/sqrt(obsCunc^2+predCunc^2)},pullCLow{(obsC-predC)/sqrt(obsCuncLow^2+predCuncLow^2)},pullCUp{(obsC-predC)/sqrt(obsCuncUp^2+predCuncUp^2)}" >> ${textfile}

for time_bin in "${time_bins[@]}"
do
    for met_bin in "${met_bins[@]}"
    do	
	for input in "${inputs[@]}"
	do
	    echo ${!input} | while read -r filename treename selection label
	    do
		root -l -b -q test_macros/faster2D.C\(\"${filename}\",\"${treename}\",\"${selection}\",\"${label}\",${time_bin},${met_bin},\"${textfile}\",\"${outdir}\"\)
	    done
	done
    done
done

cp "${textfile}" "${outdir}"

pushd "${basedir}"
./copyphp.sh "${dir}"
popd
