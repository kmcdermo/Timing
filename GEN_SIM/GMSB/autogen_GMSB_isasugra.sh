#!/bin/bash

for lambda in 50 100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400 1500
do
    lamb=$((${lambda}*1000))
    mess=$((${lamb}*2)) # messenger mass is set to 2 * lambda in SPS8

    while read -r line 
    do
	IFS=' ' read -r -a array <<< ${line}
	ctau=${array[0]}
	cgrav=${array[1]}

 	echo -e "temp.txt\n"GMSB_Lambda${lambda}TeV_CTau${ctau}cm.slha"\n/\n2\n"${lamb}" "${mess}" 1 15 1 173.1 "${cgrav}"\n0\n/" | ./isasugra.x
	rm temp.txt
    done < Lambda/Lambda${lambda}TeV_cgrav.txt
done

mkdir -p "SLHA"
mv GMSB_Lambda*TeV_CTau*cm.slha SLHA
