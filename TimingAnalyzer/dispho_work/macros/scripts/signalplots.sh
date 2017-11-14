#!/bin/bash

#####
# Need to add trigger inside .cpp
# Undo scaling?
# Remove data from plot
# Add handling of 2D plots... may need to copy macro... UGH
#####

basecut="abs(phosceta_0)<1.4442&&phoID_0>=3"

fullcut="signal ${basecut}&&njets>2&&jetHT>600"
nojetcut="signal ${basecut}"

phopt_0="phopt_0 phopt_0 100 0 1000"
phoeta_0="phoeta_0 phoeta_0 30 -1.5 1.5"
phoseedtime_0="rhtime[phoseed_0] phoseedtime_0 60 -5 25"
met="t1pfMETpt met 100 0 300"
njets="njets njets_pt15 20 0 20"
HT="jetHT HT 100 0 5000"

## do full cuts first
echo ${fullcut} | while read -r text commoncut
do 
    for plot in phopt_0 phoeta_0 phoseedtime_0 met
    do echo ${!plot} | while read -r var vartext plotinfo
	do
	    ./scripts/runTreePlotter.sh ${var} ${commoncut} ${text}_${vartext} ${plotinfo}
	done
    done
done

## do njets selection last
echo ${nojetcut} | while read -r text commoncut
do 
    for plot in njets HT
    do echo ${!plot} | while read -r var vartext plotinfo
	do
	    ./scripts/runTreePlotter.sh ${var} ${commoncut} ${text}_${vartext} ${plotinfo}
	done
    done
done
