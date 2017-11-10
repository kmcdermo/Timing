#!/bin/bash

basename=GMSB_L200TeV_CTau400cm_930
path=/store/user/kmcdermo/${basename}/${basename}_step0
timestamp=$(eos ls ${path})
xrdcp -r root://eoscms/${path}/${timestamp}/0000/log . 

njobs=$( ls cmsRun_*.log.tar.gz | wc -l)

for i in `seq 1 ${njobs}`
do
    tar -zxvf cmsRun_${i}.log.tar.gz
    rm -rf cmsRun_${i}.log.tar.gz
    rm cmsRun-stderr-${i}.log
    rm FrameworkJobReport-${i}.xml
done
