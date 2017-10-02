#!/bin/bash

for label in "B v1" "B v2" "C v1" "C v2" "C v3" "D v1"
do echo $label | while read -r era version
    do
	path=/store/group/phys_exotica/displacedPhotons/SinglePhoton/multicrab_dispho_Run2017${era}-PromptReco-${version}
	timestamp=$(eos ls ${path})
	xrdcp -r root://eoscms/${path}/${timestamp}/0000/ 2017${era}/${version}
    done
done

hadd -O tree.root 2017?/v?/*.root
rm -rf 2017?/v?/*
