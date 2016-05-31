#!/bin/sh

directory=$1/important

mkdir -p ${directory} 
mkdir -p ${directory}/norm 
mkdir -p ${directory}/fits

# validation
cp output/stackedplots/standard/log/eff_dielpt.png ${directory}
cp output/stackedplots/standard/log/zmass.png ${directory}
cp output/stackedplots/standard/log/nvtx.png ${directory}

#inclusive timing
cp output/stackedplots/standard/log/el1time.png ${directory}
cp output/stackedplots/standard/log/el1time_EB.png ${directory}
cp output/stackedplots/standard/log/el1time_EE.png ${directory}

cp output/stackedplots/standard/log/el2time.png ${directory}
cp output/stackedplots/standard/log/el2time_EB.png ${directory}
cp output/stackedplots/standard/log/el2time_EE.png ${directory}

cp output/stackedplots/standard/log/timediff.png ${directory}
cp output/stackedplots/timing/effpt/EBEB/log/tdEBEB_inclusive.png ${directory}
cp output/stackedplots/timing/effpt/EBEE/log/tdEBEE_inclusive.png ${directory}
cp output/stackedplots/timing/effpt/EEEE/log/tdEEEE_inclusive.png ${directory}

#timing resolution + bias of electron pair as a function of z parameters (pt,eta,phi)
cp output/stackedplots/timing/z/pt/lin/td_zpt_mean.png ${directory}
cp output/stackedplots/timing/z/pt/lin/td_zpt_sigma.png ${directory}

cp output/stackedplots/timing/z/abseta/lin/td_abszeta_mean.png ${directory}
cp output/stackedplots/timing/z/abseta/lin/td_abszeta_sigma.png ${directory}

cp output/stackedplots/timing/z/phi/lin/td_zphi_mean.png ${directory}
cp output/stackedplots/timing/z/phi/lin/td_zphi_sigma.png ${directory}

#timing resolution + bias of electron pair as a function of effective pT, in three eta categories
cp output/stackedplots/timing/effpt/EBEB/lin/tdEBEB_effpt_mean.png ${directory}
cp output/stackedplots/timing/effpt/EBEB/lin/tdEBEB_effpt_sigma.png ${directory}

cp output/stackedplots/timing/effpt/EBEE/lin/tdEBEE_effpt_mean.png ${directory}
cp output/stackedplots/timing/effpt/EBEE/lin/tdEBEE_effpt_sigma.png ${directory}

cp output/stackedplots/timing/effpt/EEEE/lin/tdEEEE_effpt_mean.png ${directory}
cp output/stackedplots/timing/effpt/EEEE/lin/tdEEEE_effpt_sigma.png ${directory}

#single electron resolution + bias, function of single electron pT in terms of two eta categories
cp output/stackedplots/timing/el1/pt/EB/lin/el1pt_EB_mean.png ${directory}
cp output/stackedplots/timing/el1/pt/EB/lin/el1pt_EB_sigma.png ${directory}
cp output/stackedplots/timing/el1/pt/EE/lin/el1pt_EE_mean.png ${directory}
cp output/stackedplots/timing/el1/pt/EE/lin/el1pt_EE_sigma.png ${directory}

cp output/stackedplots/timing/el2/pt/EB/lin/el2pt_EB_mean.png ${directory}
cp output/stackedplots/timing/el2/pt/EB/lin/el2pt_EB_sigma.png ${directory}
cp output/stackedplots/timing/el2/pt/EE/lin/el2pt_EE_mean.png ${directory}
cp output/stackedplots/timing/el2/pt/EE/lin/el2pt_EE_sigma.png ${directory}

#single electron resolution + bias, function of single electron eta
cp output/stackedplots/timing/el1/eta/lin/el1eta_time_mean.png ${directory}
cp output/stackedplots/timing/el1/eta/lin/el1eta_time_sigma.png ${directory}

cp output/stackedplots/timing/el2/eta/lin/el2eta_time_mean.png ${directory}
cp output/stackedplots/timing/el2/eta/lin/el2eta_time_sigma.png ${directory}

#copy data only time dielectron res + bias vs run number, in three different eta categories
cp output/DATA/doubleeg/timing/runs/EBEB/tdEBEB_runs_mean.png ${directory}
cp output/DATA/doubleeg/timing/runs/EBEB/tdEBEB_runs_sigma.png ${directory}

cp output/DATA/doubleeg/timing/runs/EBEE/tdEBEE_runs_mean.png ${directory}
cp output/DATA/doubleeg/timing/runs/EBEE/tdEBEE_runs_sigma.png ${directory}

cp output/DATA/doubleeg/timing/runs/EEEE/tdEEEE_runs_mean.png ${directory}
cp output/DATA/doubleeg/timing/runs/EEEE/tdEEEE_runs_sigma.png ${directory}

#copy inclusive normalized plots here
cp output/stackedplots/standard/log/el1time_norm.png ${directory}/norm
cp output/stackedplots/standard/log/el1time_EB_norm.png ${directory}/norm
cp output/stackedplots/standard/log/el1time_EE_norm.png ${directory}/norm

cp output/stackedplots/standard/log/el2time_norm.png ${directory}/norm
cp output/stackedplots/standard/log/el2time_EB_norm.png ${directory}/norm
cp output/stackedplots/standard/log/el2time_EE_norm.png ${directory}/norm

cp output/stackedplots/standard/log/timediff_norm.png ${directory}/norm
cp output/stackedplots/timing/effpt/EBEB/log/tdEBEB_inclusive_norm.png ${directory}/norm
cp output/stackedplots/timing/effpt/EBEE/log/tdEBEE_inclusive_norm.png ${directory}/norm
cp output/stackedplots/timing/effpt/EEEE/log/tdEEEE_inclusive_norm.png ${directory}/norm

#copy inclusive fit plots here, unfortunately not stacked...
cp output/DATA/doubleeg/standard/log/el1time_fit.png ${directory}/fits/el1time_fit_data.png
cp output/DATA/doubleeg/standard/log/el1time_EB_fit.png ${directory}/fits/el1time_EB_fit_data.png
cp output/DATA/doubleeg/standard/log/el1time_EE_fit.png ${directory}/fits/el1time_EE_fit_data.png

cp output/DATA/doubleeg/standard/log/el2time_fit.png ${directory}/fits/el2time_fit_data.png
cp output/DATA/doubleeg/standard/log/el2time_EB_fit.png ${directory}/fits/el2time_EB_fit_data.png
cp output/DATA/doubleeg/standard/log/el2time_EE_fit.png ${directory}/fits/el2time_EE_fit_data.png

cp output/DATA/doubleeg/standard/log/timediff_fit.png ${directory}/fits/timediff_fit_data.png
cp output/DATA/doubleeg/timing/effpt/EBEB/log/tdEBEB_inclusive_fit.png ${directory}/fits/tdEBEB_inclusive_fit_data.png
cp output/DATA/doubleeg/timing/effpt/EBEE/log/tdEBEE_inclusive_fit.png ${directory}/fits/tdEBEE_inclusive_fit_data.png
cp output/DATA/doubleeg/timing/effpt/EEEE/log/tdEEEE_inclusive_fit.png ${directory}/fits/tdEEEE_inclusive_fit_data.png

cp output/MC/dyll/standard/log/el1time_fit.png ${directory}/fits/el1time_fit_mc.png
cp output/MC/dyll/standard/log/el1time_EB_fit.png ${directory}/fits/el1time_EB_fit_mc.png
cp output/MC/dyll/standard/log/el1time_EE_fit.png ${directory}/fits/el1time_EE_fit_mc.png

cp output/MC/dyll/standard/log/el2time_fit.png ${directory}/fits/el2time_fit_mc.png
cp output/MC/dyll/standard/log/el2time_EB_fit.png ${directory}/fits/el2time_EB_fit_mc.png
cp output/MC/dyll/standard/log/el2time_EE_fit.png ${directory}/fits/el2time_EE_fit_mc.png

cp output/MC/dyll/standard/log/timediff_fit.png ${directory}/fits/timediff_fit_mc.png
cp output/MC/dyll/timing/effpt/EBEB/log/tdEBEB_inclusive_fit.png ${directory}/fits/tdEBEB_inclusive_fit_mc.png
cp output/MC/dyll/timing/effpt/EBEE/log/tdEBEE_inclusive_fit.png ${directory}/fits/tdEBEE_inclusive_fit_mc.png
cp output/MC/dyll/timing/effpt/EEEE/log/tdEEEE_inclusive_fit.png ${directory}/fits/tdEEEE_inclusive_fit_mc.png

