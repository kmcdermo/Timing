#!/bin/bash

basedir="/eos/user/k/kmcdermo/www"
dir="dispho/plots/madv2_v3/test/ABCD"
outdir="${basedir}/${dir}"
mkdir -p "${outdir}"

gjets_data="skims/v3/orig_2phosCR/gjets.root Data_Tree NONE Data_GJetsCR"
qcd_data="skims/v3/orig_2phosCR/qcd.root Data_Tree NONE Data_QCDCR"
declare -a inputs=(gjets_data qcd_data)

declare -a time_bins=(1 2 3)
declare -a met_bins=(50 75 100 150 200)

textfile="MET_v_Time.csv"
> ${textfile}
echo "Sample,Time,MET,obsA,obsB,obsC,obsD,c1{obsB/obsA},c2{obsD/obsA},predC{obsA*c1*c2},pullObsC{(obsC-predC)/obsCunc},pullPredC{(obsC-predC)/predCunc},pullC{(obsC-predC)/sqrt(obsCunc^2+predCunc^2)}" >> ${textfile}

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

mv "${textfile}" "${outdir}"

pushd "${basedir}"
./copyphp.sh "${dir}"
popd
