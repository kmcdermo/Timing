#!/bin/bash

dataset=${1}
base=${2}
era=${3}
version=${4}

mkdir -p 2017${era}/${version}
path=/store/user/kmcdermo/${dataset}/${base}_Run2017${era}-PromptReco-${version}
timestamp=$(eos ls ${path})
xrdcp -r root://eoscms/${path}/${timestamp}/0000/ 2017${era}/${version}
