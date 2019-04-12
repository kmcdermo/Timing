#!/bin/bash

indir="input/GMSB"

while IFS=" " read -r lambda ctau xsec BR
do
    name="L${lambda}_CTau${ctau}"
    nohup cmsRun dispho.py globalTag="94X_mc2017_realistic_v17" isGMSB=True xsec="${xsec}" BR="${BR}" inputFileNames="${indir}/${name}.txt" outputFileName="${name}.root" >& "${name}.log" &
done < "${indir}/config.txt"
