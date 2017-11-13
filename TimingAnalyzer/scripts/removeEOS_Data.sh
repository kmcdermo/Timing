#!/bin/bash

base=${1}

for label in "B v1" "B v2" "C v1" "C v2" "C v3" "D v1" "E v1" "F v1"
do echo $label | while read -r era version
    do
	eos rm -r /store/user/kmcdermo/${base}_Run2017${era}-PromptReco-${version}
    done
done