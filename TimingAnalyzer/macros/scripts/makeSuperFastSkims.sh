#!/bin/bash

## source first 
source scripts/common_variables.sh

## config
cutflowconfig=${1:-"${cutconfigdir}/cuts_v1/one_at_a_time/orig_2phosCR/signal.${inTextExt}"}
region=${2:-"SR"}
outfiletext=${3:-"sr"}

## derived config
outfiletext_signal="signals_${outfiletext}"

## compile first!
./scripts/compile.sh

## do only specified region!
echo "${!region}" | while read -r label infile insigfile sel
do    
    ## produce slimmed skim (Data + Bkgd)
    nohup ./scripts/runSuperFastSkimmer.sh "${cutflowconfig}" "${skimdir}/${infile}.root" 0 "${outfiletext}" >& "${outfiletext}.${outTextExt}" &

    ## produce slimmed skims (Signals)    
    nohup ./scripts/runSuperFastSkimmer.sh "${cutflowconfig}" "${skimdir}/${insigfile}.root" 1 "${outfiletext_signal}" >& "${outfiletext_signal}.${outTextExt}" &
done

## Final message
echo "Finished Combo SuperFastSkimming"
