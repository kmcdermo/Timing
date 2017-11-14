#!/bin/bash

basecut="hltPho50&&abs(phosceta_0)<1.4442"

gjetscut="gjets_ctrl ${basecut}&&njets<=2&&phoID_0==3&&abs(TVector2::Phi_mpi_pi(jetphi_0-phophi_0))>2.1&&(jetpt_0/phopt_0>0.6)&&(jetpt_0/phopt_0<1.4)"
qcdcut="qcd_ctrl ${basecut}&&njets>2&&phoID_0==1&&jetHT>600"

phopt_0="phopt_0 phopt_0 100 0 1000"
phoeta_0="phoeta_0 phoeta_0 30 -1.5 1.5"
phoseedtime_0="rhtime[phoseed_0] phoseedtime_0 60 -5 25"
met="t1pfMETpt met 100 0 300"

## loop over cuts
for cut in gjetscut qcdcut
do echo ${!cut} | while read -r text commoncut
    do 
	for plot in phopt_0 phoeta_0 phoseedtime_0 met
	do echo ${!plot} | while read -r var plotinfo
	    do
		./scripts/runTreePlotter.sh ${var} ${commoncut} ${text}_${var} ${plotinfo}
	    done
	done
    done
done
