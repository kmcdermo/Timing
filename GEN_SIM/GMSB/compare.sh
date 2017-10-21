#!/bin/bash

dir="CTaus"
mkdir -p ${dir}

tlamb=tmp_lambda.txt

for lamb in 50 100 200 300 400 500 600 700 800 900 1000 1100 1200 1300 1400 1500
do
    file=${dir}/Lambda${lamb}TeV_widths.txt
    for ctau in 0.001 0.01 0.1 1 5 10 25 50 100 200 300 500 750 1000 1500 2000 2500 3000
    do
	width=$(grep "DECAY   1000022" SLHA/GMSB_Lambda${lamb}TeV_CTau${ctau}cm.slha | cut -d ' ' -f6)
	echo ${ctau} ${width} >> ${file}
    done
    root -l -b -q getctaus.C\(${lamb},\"${file}\"\)
    
    echo ${lamb} >> ${tlamb}
done

root -l -b -q plotdiffs.C\(\"${tlamb}\"\)
rm ${tlamb}