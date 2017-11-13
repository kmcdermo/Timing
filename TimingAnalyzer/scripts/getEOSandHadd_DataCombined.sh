#!/bin/bash

dataset=${1}
base=${2}

for label in "B v1" "B v2" "C v1" "C v2" "C v3" "D v1" "E v1" "F v1"
do echo $label | while read -r era version
    do
	mkdir -p 2017${era}/${version}
	path=/store/user/kmcdermo/${dataset}/${base}_Run2017${era}-PromptReco-${version}
	timestamp=$(eos ls ${path})
	xrdcp -r root://eoscms/${path}/${timestamp}/0000/ 2017${era}/${version}
    done
done

./hadd_DataCombined.sh
