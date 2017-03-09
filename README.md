# Displaced Photon Analysis Framework

Recipes below. EGM ID doesn't play nice with MC production, so I have to separate those recipes. Trigger development is also outside of the main production as well, as this is meant for on-the-fly edits of confDB autogenerated configs and rate measurement tests. 

For the analysis, we have lots of analyzers, macros, scripts, etc. Many of the files/functions are duplicated. To run the analysis plugins, we will need to keep up with the latest EGM smearing and VID:
* https://twiki.cern.ch/twiki/bin/viewauth/CMS/EGMSmearer
* https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedPhotonIdentificationRun2
* https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedElectronIdentificationRun2

------------------------------------

## Recipe for Analyzers 

Recipe to install of all the code necessary to run all the final analysis analyzers. Tested on LXPLUS. 

```
mkdir Analysis
cd Analysis
cmsrel CMSSW_8_0_26_patch2
cd CMSSW_8_0_26_patch2/src/
cmsenv
git cms-init

### EGM Smearing ###
git cms-merge-topic shervin86:Moriond17_23Jan
cd EgammaAnalysis/ElectronTools/data
git clone https://github.com/ECALELFS/ScalesSmearings.git
cd ../../../

### Electron VID modules ###
git cms-merge-topic ikrav:egm_id_80X_v2

### Photon VID modules ###
git cms-merge-topic ikrav:egm_id_80X_v3_photons

### Copy this directory for analyzers ###
git clone ssh://git@github.com/kmcdermo/Timing.git
cd Timing/
git remote rename origin kmcdermo
cd ../

scram b -j 16
```

#### Running the ECAL Timing Resolution Study

The analyzer used for producing the ECAL timing resolution ntuples is Timing/TimingAnalyzer/plugins/ZeeTree.cc[h].  The analyzer first forms pairs of oppositely signed tightly indentified electrons.  If more than one pair is formed in event, then it selects only the pair with an invariant mass closest to the Z-mass. It then saves the timing info (as well as other kinematic info) for each recHit in each electron's SC (accounting for the double counting of recHits in the SC).

The python config for this analyzer is Timing/TimingAnalyzer/test/zeetree.py.  This config can be configured to run over CRAB: crabConfig_zee.py.  To save on space, you can pass a boolean (filterOnKinematics, and also filterOnHLT) to only write out events that pass the full kinematic selection.  Otherwise, most of the branches will be left with their intiailized values.

After the analyzer finishes producing ntuples, the main analysis is in Timing/TimingAnalyzer/work.  You will need to put the output root files in "input/DATA/2016/doubleeg" for data, and "input/MC/2016/dyll" for MC.  For now, the analysis expects the name of root file to be skimmedtree.root, but this can of course be changed.  The main analysis code is inside /work/src/Analysis.cc (header: /work/interface/Analysis.hh), while the executable is work/src/main.cc.  The executable needs to be compiled with a Makefile (so simply do ```make -j 4``` inside work/).  It requires a ROOT6 build to compile.  This will produce an executable named "main".

The exectuable is fairly flexible.  To see the command line options, simply do ```./main --help```. There are a number of other configurable parameters that are currently not set at the command line, like the lumi, that are set in work/src/Config.cc or work/interface/Config.hh.

To do the basic timing resolution study versus effective energy for data, simply do:

```
### uses just the seed crystal time as the electron time
./main --use-DEG --do-effE --use-sigman

### use weighted average of rechits to evaluate electron time --> automatically applies sigma_n
./main --use-DEG --do-effE --wgt-time

#### apply Time of Flight correction
./main --use-DEG --do-effE --wgt-time --apply-TOF
``` 

If you have produced both data and MC, then you can run some basic comparison plots, like:
```
./main --use-DEG --use-DYll --do-purw --do-standard --do-stacks
``` 

There are a number of other analyses one can do, like timing versus single electron E, nvertices, eta, etc. See ```./main --help``` for guidance.

There are a number of other helper macros to do refitting, skims, etc. in work/macros.  Last, but certainly not least, there is the "final" macro for producing the fit over the time resolution vs effective energy plot to extract the intrinsic resolution: work/macros/timefit.C.  Simply run like an macro with ```root -l -b -q timefit.C```.  Inside the macro, you can specify which plot exactly you want fit (EB vs EE, data vs MC, etc).

One last detail: the time resolution is related to the intrinsic pedestal noise of each crystal. So, to factorize this, when calculating the effective energy, each leg of the effective energy (either just the seed crystal energies or both electron's full recHit energies) is weighted by the pedestal noise of the corresponding crystal.  This value changes over a run era, so you have to make sure you have the IOV for each crystal's pedestal noise value.  Each IOV is read into the main executable from the following text files: /work/config/pedestals/*.txt. 

The following github repo has the tools to download the pedestals and convert them into the text format the executbale expects: https://github.com/kmcdermo/CondDBDumperECAL/tree/ecalpeds/DBDump.  Use confDB and the GT you ran the analyzer over to dump out the appropriate IOVs. I have not updated this for the re-miniAOD, so the pedestals need to be dumped and saved again.

#### Running the general displaced photon analyzers and macros

The general displaced photon analyzer for now is Timing/TimingAnalyzer/plugins/PhotonDump.cc[h]. It has a number of unique branches for each MC samples (GMSB, HVDS, and background).  The gen info stored is specific to each sample.  The python config to run this plugin is Timing/TimingAnalyzer/test/photondump-*.py. Make sure to checkout each one and modify it accordingly for new samples.  One thing to be aware of is the fact that there is a moderate level of configuration via the command line, so make sure to check out the available options.  

```
cd CMSSW_8_0_26_patch2/src/Timing/TimingAnalyzer
cmsenv
cmsRun test/photondump-gmsb.py

### We now need ROOT5 for this next part... non-ideal, I realize as the default is ROOT6 with cmsenv for CMSSW_8_0_X+. ###
### ROOT5 is actually the default of lxplus without setting the CMS environment. ###
### Or one can copy the output of the analyzer to a laptop with ROOT5 (my personal favorite). ###
### I will soon make this macro into a compiled code, like the Zee timing resolution tools (i.e. plugins/ZeeTree.cc[h] and work/) ###

cd macros/
root -l -b -j runPhotonPlots.C

### Few things worth mentioning... there is again another huge set of configurables, so make sure to set them properly! ###
### This code compiles PlotPhotons.cc[hh]. ### 
### This code is in flux, as some sections are commented out, so be sure to comment them in if you want certain plots to be made. ###  

### Also, pay attention to the line containing: ###
### if (((fIsGMSB || fIsHVDS) && (*phmatch)[iph] <= 0) || (fIsBkg && (*phisMatched)[iph] != 0) ) continue; (at L632 at the moment) ###
### Comment this out if you do NOT want gen-level matching (for signals and gamma+jets) or gen-level matching veto (for QCD). ###
```

------------------------------------

## Recipe for MC Production

First make a global GEN-SIM directory which will launch step 0 in 71X and steps 1-3 in 80X.

```
mkdir GEN_SIM_ONLY
```

#### Recipe for GEN-SIM ONLY (step0)  

```
cd GEN_SIM_ONLY
cmsrel CMSSW_7_1_25
cd CMSSW_7_1_25/src/
cmsenv
git cms-init

### Contains Timing/GEN_SIM directory ###
git clone ssh://git@github.com/kmcdermo/Timing.git
cd Timing/
git remote rename origin kmcdermo
cd ../
mv Timing/GEN_SIM/Configuration . 
```

cmsDriver.py for step0 must be launched from CMSSW_7_1_25/src/ in order to properly pickup Configuration/.  After that move the autogenerated config into the respective sample subdirectory and edit the CRAB config accordingly (i.e. Timing/GEN_SIM).

For an example of the cmsDriver.py command for generating MC with SLHA files, see: https://github.com/kmcdermo/Timing/blob/80X/GEN_SIM/GMSB/step0_GMSB_L180_Ctau6000_Pythia8_13TeV_GEN_SIM-RAWSIM.py#L5 

Further Documentation:
 * https://github.com/lsoffi/GMSB-MC2016-PRODUCTION

#### Recipe for Detector Simulation (Steps 1-3) 

```
cd GEN_SIM_ONLY
cmsrel CMSSW_8_0_26_patch2
cd CMSSW_8_0_26_patch2/src/
cmsenv
git cms-init

### Contains Timing/GEN_SIM directory ###
git clone ssh://git@github.com/kmcdermo/Timing.git
cd Timing/
git remote rename origin kmcdermo
cd ../

### Copy this for custom timing reco ###
git cms-addpkg RecoEcal/Configuration
cp Timing/GEN_SIM/python/RecoEcal_CustomiseTimeCleaning.py RecoEcal/Configuration/python

scram b -j 16

cd Timing/GEN_SIM
```

From here, we can issue a set of cmsDriver commands to autogenerate python configs. You may need to get a proxy to run this, as it requires pinging confDB.  To run locally, we will need a GEN_SIM ROOT file from step0 as input into step1.  Make sure to test each autogenerated config locally with cmsRun before submitting to CRAB!

```
cd GEN_SIM_ONLY/CMSSW_8_0_26_patch2/src/
cmsenv
voms-proxy-init --voms cms --valid 168:00

### Step1: L1 and HLT simulation. ### 
### Since we are developing our own trigger, we have a bit of customization in the HLT step ###
### Exact config found in Timing/GEN_SIM/GMSB/step1_userHLT_DIGI_L1_DIGI2RAW_HLT_PU-RAWSIM.py ###

cmsDriver.py step1_userHLT --mc --era Run2_2016 --conditions 80X_mcRun2_asymptotic_2016_TrancheIV_v6 --eventcontent RAWSIM --datatier GEN-SIM-RAW --step DIGI,L1,DIGI2RAW,HLT:hltdev:/users/kmcdermo/TestDev/DisplacedPhoton_8_0_24/V1 --pileup 2016_25ns_Moriond17MC_PoissonOOTPU --pileup_input dbs:/MinBias_TuneCUETP8M1_13TeV-pythia8/RunIISummer15GS-MCRUN2_71_V1_ext1-v1/GEN-SIM --filein file:GEN_SIM-RAWSIM.root --fileout file:DIGI_L1_DIGI2RAW_HLT_PU-RAWSIM.root --no_exec

mv step1_userHLT_DIGI_L1_DIGI2RAW_HLT_PU.py step1_userHLT_DIGI_L1_DIGI2RAW_HLT_PU-RAWSIM.py 
cmsRun step1_userHLT_DIGI_L1_DIGI2RAW_HLT_PU-RAWSIM.py 

### Step2: Full RECO simulation, store AODSIM. ###
### Again, since we are interested in OOT recHits as seeds, need some more customization compared to the standard workflow ###
### Exact config found in Timing/GEN_SIM/GMSB/step2_customiseTiming_RAW2DIGI_L1Reco_RECO-AODSIM.py ###

cmsDriver.py step2_customise --customise RecoEcal/Configuration/RecoEcal_CustomiseTimeCleaning.customiseTimeCleaning --mc --eventcontent AODSIM --runUnscheduled --datatier AODSIM --conditions 80X_mcRun2_asymptotic_2016_TrancheIV_v6 --step RAW2DIGI,L1Reco,RECO --era Run2_2016 --filein file:DIGI_L1_DIGI2RAW_HLT_PU-RAWSIM.root --fileout file:RAW2DIGI_L1Reco_RECO-AODSIM.root --no_exec

mv step2_customiseTiming_RAW2DIGI_L1Reco_RECO.py step2_customiseTiming_RAW2DIGI_L1Reco_RECO-AODSIM.py
cmsRun step2_customiseTiming_RAW2DIGI_L1Reco_RECO-AODSIM.py

### Step3: PAT, i.e. MINIAOD format. ###
### Exact config found in Timing/GEN_SIM/GMSB/step3_PAT-MINIAODSIM.root ###

cmsDriver.py step3 --mc --eventcontent MINIAODSIM --runUnscheduled --datatier MINIAODSIM --conditions 80X_mcRun2_asymptotic_2016_TrancheIV_v6 --step PAT --era Run2_2016 --filein file:RAW2DIGI_L1Reco_RECO-AODSIM.root --fileout file:PAT-MINIAODSIM.root --no_exec

mv step3_PAT.py step3_PAT-MINIAODSIM.py
cmsRun step3_PAT-MINIAODSIM.py
```

When you have finished the PAT step, save the output directory from ```crab status``` or just search for the files with ```eos ls``` to be used in the analysis plugins and macros.

Further Documentation:
 * https://twiki.cern.ch/twiki/bin/viewauth/CMS/PdmVMCcampaignRunIISummer16DR80
 * https://twiki.cern.ch/twiki/bin/view/CMS/PdmVMCcampaignRunIISummer16MiniAODv2

------------------------------------

## HLT Trigger Development 

This is only for tinkering with the HLTDisplacedPhoton path on the fly and for rate/time measurements.  Efficiencies can be measured with the analysis plugin PhotonDump.cc, after following the new rules for MC production in step1.

```
mkdir HLT_Dev
cd HLT_Dev
cmsrel CMSSW_8_0_26_patch2
cd CMSSW_8_0_26_patch2/src/
cmsenv
git cms-init

### Trigger tests --> Optional ###
git cms-addpkg HLTrigger/Configuration
git cms-addpkg GeneratorInterface/GenFilters
git cms-addpkg L1Trigger/L1TGlobal
git clone https://github.com/cms-data/L1Trigger-L1TGlobal.git L1Trigger/L1TGlobal/data
git clone git@github.com:cms-steam/RemovePileUpDominatedEvents.git RemovePileUpDominatedEvent
git cms-addpkg HLTrigger/HLTanalyzers
git clone git@github.com:cms-steam/HLTrigger temp
cp -r temp/* HLTrigger/
rm -rf temp/
cp /afs/cern.ch/user/n/ndaci/public/STEAM/L1Menus/L1Menu_Collisions2016_v9_m2.xml L1Trigger/L1TGlobal/data/Luminosity/startup/
cp /afs/cern.ch/user/n/ndaci/public/STEAM/Prescales/UGT_BASE_RS_FINOR_MASK_v91.xml L1Trigger/L1TGlobal/data/Luminosity/startup/
cp /afs/cern.ch/user/n/ndaci/public/STEAM/Prescales/UGT_BASE_RS_PRESCALES_v214.xml L1Trigger/L1TGlobal/data/Luminosity/startup/
cp /afs/cern.ch/user/n/ndaci/public/STEAM/Prescales/L1TGlobalPrescalesVetosESProducer.cc L1Trigger/L1TGlobal/plugins/

git clone git@github.com:cms-steam/RateEstimate.git RateEstimate

scram b -j 16

### Presumably STEAM needs this particular version of ROOT6 
source /cvmfs/cms.cern.ch/slc6_amd64_gcc491/lcg/root/6.02.10/bin/thisroot.sh
```

Further Documentation:
* https://twiki.cern.ch/twiki/bin/view/CMS/DisplacedPhotonTriggerRun2
