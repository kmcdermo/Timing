#!/bin/bash

temp="temp.txt"
log="slha.log"

for lambda in 100 150 200 250 300 350 400 500 600
do
    echo "Working on Lambda = ${lambda} TeV"

    lamb=$((${lambda}*1000))
    mess=$((${lamb}*2)) # messenger mass is set to 2 * lambda in SPS8

    while read -r line 
    do
	IFS=' ' read -r -a array <<< ${line}
	ctau=${array[0]}
	cgrav=${array[1]}

 	echo -e "${temp}\n"GMSB_Lambda${lambda}TeV_CTau${ctau}cm.slha"\n/\n2\n"${lamb}" "${mess}" 1 15 1 173.1 "${cgrav}"\n0\n/" | ./isasugra.x >& "${log}"
	rm temp.txt
    done < Lambda/Lambda${lambda}TeV_cgrav.txt
done

mkdir -p "SLHA"
mv GMSB_Lambda*TeV_CTau*cm.slha SLHA
