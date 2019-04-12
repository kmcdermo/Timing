#!/bin/bash

dir="CTaus"
mkdir -p ${dir}

tlamb=tmp_lambda.txt

for lamb in 100 150 200 250 300 350 400 500 600
do
    file=${dir}/Lambda${lamb}TeV_widths.txt
    for ctau in 0.001 0.01 0.1 0.5 1 5 10 50 100 200 400 600 800 1000 1200 5000 10000
    do
	width=$(grep "DECAY   1000022" SLHA/GMSB_Lambda${lamb}TeV_CTau${ctau}cm.slha | cut -d ' ' -f6)
	echo ${ctau} ${width} >> ${file}
    done
    root -l -b -q getctaus.C\(${lamb},\"${file}\"\)
    
    echo ${lamb} >> ${tlamb}
done

root -l -b -q plotdiffs.C\(\"${tlamb}\"\)
rm ${tlamb}