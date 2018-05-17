#!/bin/bash

for ws in X Y 2D
do
    combine -M MaxLikelihoodFit -t -1 --expectSignal 0 datacard_test_displacedPhotons_${ws}.txt --verbose 2 --name maxll_exsig0_${ws} >& maxll_exsig0_${ws}.txt
    combine -M MaxLikelihoodFit -t -1 --expectSignal 1 datacard_test_displacedPhotons_${ws}.txt --verbose 2 --name maxll_exsig1_${ws} >& maxll_exsig1_${ws}.txt
    combine -M AsymptoticLimits datacard_test_displacedPhotons_${ws}.txt --run=expected --verbose 2 --name asym_${ws} >& asym_${ws}.txt
done
