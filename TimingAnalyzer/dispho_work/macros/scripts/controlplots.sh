#!/bin/bash

## define cuts
basecut="hltPho50&&abs(phosceta_0)<1.4442&&phopt_0>70"

gjetscut="gjets_ctrl ${basecut}&&njets<=2&&phoID_0==3&&abs(TVector2::Phi_mpi_pi(jetphi_0-phophi_0))>2.1&&(jetpt_0/phopt_0>0.6)&&(jetpt_0/phopt_0<1.4)"
qcdcut="qcd_ctrl ${basecut}&&njets>2&&phoID_0==1"

## define plots
logaxis="0 1" # !islogx && !islogy
ytitle="Events"

## blank spaces in titles use XXX
phopt_0="phopt_0 phopt_0 100 0 1000 ${logaxis} phopt_0 LeadingXXXPhotonXXXp_{T} ${ytitle}"
phoeta_0="phoeta_0 phoeta_0 30 -1.5 1.5 ${logaxis} phoeta_0 LeadingXXXPhotonXXX#eta ${ytitle}"
phoseedtime_0="rhtime[phoseed_0] phoseedtime_0 60 -5 25 ${logaxis} phoseedtime_0 LeadingXXXPhotonXXXSeedXXXTimeXXX[ns] ${ytitle}"
met="t1pfMETpt met 100 0 300 ${logaxis} met METXXX[GeV] ${ytitle}"

## loop over cuts
for cut in gjetscut qcdcut
do echo ${!cut} | while read -r text commoncut
    do 
	for plot in phopt_0 phoeta_0 phoseedtime_0 met
	do echo ${!plot} | while read -r var vartext plotinfo
	    do
		if [ "${vartext}" == "phoseedtime_0" ] ; 
		then 
		    echo ./scripts/runTreePlotter.sh ${var} "${commoncut}&&phoseed_0>=0" ${text}_${vartext} ${plotinfo}
		else
		    echo ./scripts/runTreePlotter.sh ${var} ${commoncut} ${text}_${vartext} ${plotinfo}
		fi
	    done
	done
    done
done

