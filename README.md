### Timing
Displaced Photon Analysis Framework

Recipes below. Lots of analyzers, macros, many of the files/functions are duplicated. To run this code, we will need to keep up with the latest EGM smearing and VID:
* https://twiki.cern.ch/twiki/bin/viewauth/CMS/EGMSmearer
* https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedPhotonIdentificationRun2
* https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedElectronIdentificationRun2

HLTDisplacedPhoton Trigger documentation: 
* https://twiki.cern.ch/twiki/bin/view/CMS/DisplacedPhotonTriggerRun2

------------------------------------

```
### Recipe for GEN-SIM (step0) only ###
cmsrel CMSSW_7_1_25
cd CMSSW_7_1_25/src/
cmsenv
git cms-init

git clone ssh://git@github.com/kmcdermo/Timing.git
mv Timing/GEN_SIM/Configuration . 
```

Further Documentation:
 * cmsDriver.py for step0 must be launched from CMSSW_7_1_25/src/
 * https://github.com/lsoffi/GMSB-MC2016-PRODUCTION
 * https://twiki.cern.ch/twiki/bin/viewauth/CMS/PdmVMCcampaignRunIISummer16DR80
 * https://twiki.cern.ch/twiki/bin/view/CMS/PdmVMCcampaignRunIISummer16MiniAODv2

------------------------------------

```
### Full recipe for everything else ###
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

### Trigger tests --> Optional ###
git cms-addpkg HLTrigger/Configuration
git cms-addpkg GeneratorInterface/GenFilters
git cms-addpkg L1Trigger/L1TGlobal
git clone https://github.com/cms-data/L1Trigger-L1TGlobal.git L1Trigger/L1TGlobal/data
git clone git@github.com:cms-steam/RemovePileUpDominatedEvents.git RemovePileUpDominatedEvent
git cms-addpkg HLTrigger/HLTanalyzers
git clone git@github.com:cms-steam/HLTrigger temp
cp -r temp/* HLTrigger/
rm -fr temp/
cp /afs/cern.ch/user/n/ndaci/public/STEAM/L1Menus/L1Menu_Collisions2016_v9_m2.xml L1Trigger/L1TGlobal/data/Luminosity/startup/
cp /afs/cern.ch/user/n/ndaci/public/STEAM/Prescales/UGT_BASE_RS_FINOR_MASK_v91.xml L1Trigger/L1TGlobal/data/Luminosity/startup/
cp /afs/cern.ch/user/n/ndaci/public/STEAM/Prescales/UGT_BASE_RS_PRESCALES_v214.xml L1Trigger/L1TGlobal/data/Luminosity/startup/
cp /afs/cern.ch/user/n/ndaci/public/STEAM/Prescales/L1TGlobalPrescalesVetosESProducer.cc L1Trigger/L1TGlobal/plugins/

### Copy this directory for analyzers ###
git clone ssh://git@github.com/kmcdermo/Timing.git
cd Timing/
git remote rename origin kmcdermo
cd ../

### Copy this for custom timing reco ###
git cms-addpkg EgammaAnalysis/Configuration
cp Timing/TimingAnalyzer/python/RecoEcal_CustomiseTimeCleaning.py EgammaAnalysis/Configuration/python

scram b -j 16
```
