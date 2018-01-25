#!/bin/bash

#### setup cuts ####

## empty cut
none="NONE"

## hlt cuts
phohltcut="((phoisEB_0&&phopt_0>70&&phor9_0>0.95&&phosieie_0<0.012&&phoHoE_0<0.1&&phosmaj_0<1.&&phosmin_0<0.25&&phoisTrk_0==0&&phoEcalPFClIso_0<(2+0.01*phopt_0)&&phoHcalPFClIso_0<(6+0.03*phopt_0)&&phoTrkIso_0<3.)||(phoisEB_1&&phopt_1>70&&phor9_1>0.95&&phosieie_1<0.012&&phoHoE_1<0.1&&phosmaj_1<1.&&phosmin_1<0.25&&phoisTrk_1==0&&phoEcalPFClIso_1<(2+0.01*phopt_1)&&phoHcalPFClIso_1<(6+0.03*phopt_1)&&phoTrkIso_1<3.)||(phoisEB_2&&phopt_2>70&&phor9_2>0.95&&phosieie_2<0.012&&phoHoE_2<0.1&&phosmaj_2<1.&&phosmin_2<0.25&&phoisTrk_2==0&&phoEcalPFClIso_2<(2+0.01*phopt_2)&&phoHcalPFClIso_2<(6+0.03*phopt_2)&&phoTrkIso_2<3.)||(phoisEB_3&&phopt_3>70&&phor9_3>0.95&&phosieie_3<0.012&&phoHoE_3<0.1&&phosmaj_3<1.&&phosmin_3<0.25&&phoisTrk_3==0&&phoEcalPFClIso_3<(2+0.01*phopt_3)&&phoHcalPFClIso_3<(6+0.03*phopt_3)&&phoTrkIso_3<3.))"
hltcut="hltSignal"

## offline photon cuts
pho0cut="(((phogedID_0==3&&!phoisOOT_0)||(phoootID_0==3&&phoisOOT_0))&&phopt_0>70&&phoisEB_0)"
pho1cut="(((phogedID_1>=1&&!phoisOOT_1)||(phoootID_1>=1&&phoisOOT_1))&&phopt_1>50&&phoisEB_1)"

## jets cuts
jetscut="(jetHTeta3>600&&njets>2)"

## union of event cuts
eventcut="signal (${phohltcut}&&${pho0cut}&&${pho1cut}&&${jetscut})"

## setup plots
logaxis="0 1" # !islogx && !islogy
ytitle="Events"
delim="XXX"

## blanks in titles need XXX
phopt_0="phopt_0 phopt_0 100 0 2000 ${logaxis} phopt_0 LeadingXXXPhotonXXXp_{T}XXX[GeV/c] ${ytitle} ${delim}"
phoeta_0="phoeta_0 phoeta_0 30 -2 3 ${logaxis} phoeta_0 LeadingXXXPhotonXXX#eta ${ytitle} ${delim}"
phophi_0="phophi_0 phophi_0 32 -3.2 3.2 ${logaxis} phophi_0 LeadingXXXPhotonXXX#phi ${ytitle} ${delim}"
phoseedtime_0="phoseedtime_0 phoseedtime_0 80 -10 10 ${logaxis} phoseedtime_0 LeadingXXXPhotonXXXSeedXXXTimeXXX[ns] ${ytitle} ${delim}"
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

