#!/bin/bash

for lambda in 100 150 200 250 300 350 400
do
    lamb=$((${lambda}*1000))
    mess=$((${lamb}*2)) # messenger mass is set to 2 * lambda in SPS8

    while read -r line 
    do
	IFS=' ' read -r -a array <<< ${line}
	ctau=${array[0]}
	cgrav=${array[1]}

 	echo -e "temp.txt\n"GMSB_Lambda${lambda}TeV_CTau${ctau}cm.slha"\n/\n2\n"${lamb}" "${mess}" 1 15 1 175 "${cgrav}"\n0\n/" | ./isasugra.x
	rm temp.txt
    done < Lambda${lambda}_cgrav.txt
done
