#!/bin/bash

era=$1 #"2017B"
version=$2 #"v1"

timestamp=$(eos ls /store/group/phys_exotica/displacedPhotons/SinglePhoton/multicrab_hltdump_Run${era}-PromptReco-${version})
xrdcp -r root://eoscms//store/group/phys_exotica/displacedPhotons/SinglePhoton/multicrab_hltdump_Run${era}-PromptReco-${version}/${timestamp}/0000/ ${era}/${version}

