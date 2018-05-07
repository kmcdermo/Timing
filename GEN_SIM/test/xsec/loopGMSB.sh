#!/bin/bash

tmpoutput="tmpxsec.txt"
fulloutput="all_xsecs.txt"

for lamb in 100 150 200 250 300 350 400
do
    for ctau in 0p1 #10 200 400 600 800 1000 1200
    do
	basename=GMSB_L${lamb}TeV_CTau${ctau}cm_step0
	path=/eos/cms/store/group/phys_egamma/soffi/displacedPhotons/GEN-SIM_2304218/${basename}/${basename}

	./getXSecFromLogs.sh ${path} ${tmpoutput}
	line=$(head -n 1 ${tmpoutput})
	    
	sctau=${ctau}
	if [[ "${sctau}" == "0p1" ]] ; then
	    sctau="0.1"
	fi

	echo ${lamb} ${sctau} ${line} >> ${fulloutput}

	rm ${tmpoutput}
    done
done