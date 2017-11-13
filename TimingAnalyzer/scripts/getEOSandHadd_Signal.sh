#!/bin/bash

base=${1}

gmsb="GMSB_L200TeV_CTau400cm_930"
dir=tmp
mkdir -p ${dir}

path=/store/user/kmcdermo/${gmsb}/${base}_${gmsb}
timestamp=$(eos ls ${path})
xrdcp -r root://eoscms/${path}/${timestamp}/0000/ ${dir}

hadd -O -k tree.root ${dir}/*.root
rm -rf ${dir}/*.root
