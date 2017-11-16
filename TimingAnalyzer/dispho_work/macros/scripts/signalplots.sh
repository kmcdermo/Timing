#!/bin/bash

#####
# Need to add trigger inside .cpp
# Undo scaling?
# Remove data from setup, and continue in loop over samples
#####

## setup cuts
basecut="abs(phosceta_0)<1.4442&&((phoID_0>=3&&!phoisOOT_0)||(phoID_0>=2&&phoisOOT_0))&&phopt_0>70"

fullcut="signal ${basecut}&&njets>2&&jetHT>600"
nojetcut="signal ${basecut}"

## setup plots
logaxis="0 1" # !islogx && !islogy
ytitle="Events"

## blanks in titles need XXX
phopt_0="phopt_0 phopt_0 100 0 1000 ${logaxis} phopt_0 LeadingXXXPhotonXXXp_{T}XXX[GeV/c] ${ytitle}"
phoeta_0="phoeta_0 phoeta_0 30 -1.5 1.5 ${logaxis} phoeta_0 LeadingXXXPhotonXXX#eta ${ytitle}"
phoseedtime_0="rhtime[phoseed_0] phoseedtime_0 60 -5 25 ${logaxis} phoseedtime_0 LeadingXXXPhotonXXXSeedXXXTimeXXX[ns] ${ytitle}"
met="t1pfMETpt met 100 0 600 ${logaxis} met METXXX[GeV] ${ytitle}"
njets="njets njets_pt15 25 0 25 ${logaxis} njets nJetsXXX[p_{T}>15 GeV/c] ${ytitle}"
HT="jetHT HT 100 0 5000 ${logaxis} HT H_{T}XXX[GeV],XXXjetXXXp_{T}>15XXXGeV/c ${ytitle} "

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
