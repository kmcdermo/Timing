#!/bin/bash

## SETUP
basename=GMSB_L250TeV_CTau0p1cm_step0
path=/eos/cms/store/group/phys_egamma/soffi/displacedPhotons/GEN-SIM_2304218/${basename}/${basename}
output=${basename}_xsecs.txt

echo "Copying log tar files"

## GET LOGS FROM EOS
timestamp=$(ls ${path})
cp ${path}/${timestamp}/0000/log/cmsRun_*.log.tar.gz .

echo "Extracting xsecs..."

## EXTRACT XSEC FROM LOGS
for file in cmsRun_*.log.tar.gz
do
    i=$(echo ${file} | sed 's/.*cmsRun_\(.*\)\.log\.tar\.gz/\1/')

    echo "Untarring" ${file}
    tar -zxf ${file}
    rm -rf ${file}
    rm cmsRun-stderr-${i}.log
    rm FrameworkJobReport-${i}.xml

    echo "Grepping for xsec"
    xsec=$( grep "final cross section" cmsRun-stdout-${i}.log | cut -d " " -f 7 )
    exsec=$( grep "final cross section" cmsRun-stdout-${i}.log | cut -d " " -f 9 )
    echo ${xsec} ${exsec} >> ${output}

    rm cmsRun-stdout-${i}.log
done

## COMPUTE AVERAGE XSEC AND DUMP
echo "Computing total xsec"
root -l -b -q computeAverageXSec.C\(\"${output}\"\)

## CLEANUP
rm ${output}
