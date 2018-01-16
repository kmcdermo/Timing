#!/bin/bash

basecut="phoisEB_0&&phopt_0>70"
gjetscut="${basecut}&&njets<=2&&((phogedID_0==3&&!phoisOOT_0)||(phoootID_0==3&&phoisOOT_0))&&abs(TVector2::Phi_mpi_pi(jetphi_0-phophi_0))>2.1&&(jetpt_0/phopt_0>0.6)&&(jetpt_0/phopt_0<1.4)&&phor9_0>0.95&&hltPho50"
qcdcut="${basecut}&&njets>2&&((phogedID_0==1&&!phoisOOT_0)||(phoootID_0==1&&phoisOOT_0))&&hltPho50"

commoncut=${1:-"${qcdcut}"}
text=${2:-"met_vs_time"}
inconfig=${3:-"plot_config/met_vs_time.txt"}

root -l -b -q runTreePlotter2D.C\(\"${commoncut}\",\"${text}\",\"${inconfig}\"\)

cp ${text}.root ~/www/quick/
