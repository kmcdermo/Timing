#!/bin/bash


## blanks in titles need XXX
met="t1pfMETpt met 100 0 1000 ${logaxis} met METXXX[GeV/c] ${ytitle} ${delim}"

## do full cuts first w/o hlt
echo ${eventcut} | while read -r text commoncut
do 
    for plot in phopt_0 phoeta_0 phophi_0
    do echo ${!plot} | while read -r var vartext plotinfo
	do
	    ./scripts/runTreePlotter.sh ${var} ${commoncut} ${none} ${none} ${none} ${text}_${vartext} ${plotinfo} "dispho/no_hltSignal"
	done
    done
done

## do full cuts first w hlt
echo ${eventcut} | while read -r text commoncut
do 
    for plot in phopt_0 phoeta_0 phophi_0
    do echo ${!plot} | while read -r var vartext plotinfo
	do
	    ./scripts/runTreePlotter.sh ${var} ${commoncut} ${hltcut} ${none} ${hltcut} ${text}_${vartext} ${plotinfo} "dispho/hltSignal"
	done
    done
done

## do seedtime (blinded) w/o HLT
blindtime="abs(phoseedtime_0<3)"

echo ${eventcut} | while read -r text commoncut
do 
    for plot in phoseedtime_0
    do echo ${!plot} | while read -r var vartext plotinfo
	do
	    ./scripts/runTreePlotter.sh ${var} ${commoncut} ${none} ${none} ${blindtime} ${text}_${vartext} ${plotinfo} "dispho/no_hltSignal"
	done
    done
done

## do seedtime (blinded) w HLT
echo ${eventcut} | while read -r text commoncut
do 
    for plot in phoseedtime_0
    do echo ${!plot} | while read -r var vartext plotinfo
	do
	    ./scripts/runTreePlotter.sh ${var} ${commoncut} ${hltcut} ${none} ${hltcut}"&&"${blindtime} ${text}_${vartext} ${plotinfo} "dispho/hltSignal"
	done
    done
done

## do MET (blinded) w/o HLT
blindmet="t1pfMETpt<200"

echo ${eventcut} | while read -r text commoncut
do 
    for plot in met
    do echo ${!plot} | while read -r var vartext plotinfo
	do
	    ./scripts/runTreePlotter.sh ${var} ${commoncut} ${none} ${none} ${blindmet} ${text}_${vartext} ${plotinfo} "dispho/no_hltSignal"
	done
    done
done

## do seedtime (blinded) w HLT
echo ${eventcut} | while read -r text commoncut
do 
    for plot in met
    do echo ${!plot} | while read -r var vartext plotinfo
	do
	    ./scripts/runTreePlotter.sh ${var} ${commoncut} ${hltcut} ${none} ${hltcut}"&&"${blindmet} ${text}_${vartext} ${plotinfo} "dispho/hltSignal"
	done
    done
done









#HT="jetHTeta3 HT 100 0 5000 ${logaxis} HT H_{T}XXX[GeV],XXXjetXXXp_{T}>15XXXGeV/c ${ytitle} ${delim}"
#njets="njets njets_pt15 25 0 25 ${logaxis} njets nJetsXXX[p_{T}>15XXXGeV/c] ${ytitle} ${delim}"

