#!/bin/bash

## SETUP
basename=GMSB_L200TeV_CTau400cm_930
path=/store/user/kmcdermo/${basename}/${basename}_step0
output=${basename}_xsecs.txt

## GET LOGS FROM EOS
timestamp=$(eos ls ${path})
xrdcp -r root://eoscms/${path}/${timestamp}/0000/log . 

## HOW MANY?
njobs=$( ls cmsRun_*.log.tar.gz | wc -l)

## EXTRACT XSEC FROM LOGS
for i in `seq 1 ${njobs}`
do
    tar -zxvf cmsRun_${i}.log.tar.gz
    rm -rf cmsRun_${i}.log.tar.gz
    rm cmsRun-stderr-${i}.log
    rm FrameworkJobReport-${i}.xml

    xsec=$( grep "final cross section" cmsRun-stdout-${i}.log | cut -d " " -f 7 )
    exsec=$( grep "final cross section" cmsRun-stdout-${i}.log | cut -d " " -f 9 )
    
    echo ${xsec} ${exsec} >> ${output}

    rm cmsRun-stdout-${i}.log
done

## COMPUTE AVERAGE XSEC AND DUMP
root -l -b -q computeAverageXSec.C\(\"${output}\"\)

## CLEANUP
rm ${output}
