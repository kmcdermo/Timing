# Displaced Photon Analysis Framework

## HUGE N.B.
OOT and GED VIDs all messed up! Make sure to apply by hand (or with new VID producers)! If doing by hand, will need to compute Smajor/Sminor!

### Standard Setup
```
cmsrel CMSSW_9_4_13
cd CMSSW_9_4_13/src
cmsenv
git cms-init

git cms-merge-topic kmcdermo:post_reco_OOT_AND_add_OOT_VID
git cms-addpkg RecoMET/METFilters

git clone git@github.com:kmcdermo/Timing.git
pushd Timing
git remote rename origin kmcdermo
popd

scram b -j 8
```

Will also need to setup combine somewhere in a separate CMSSW release area (see links below). Change the relative path for the setup in ```scripts/common_variables.sh```. Currently using Higgs combine with CMSSW_8_1_0. 

N.B. The previous instructions included: ```git cms-merge-topic cms-met:METFixEE2017_949_v2``` has been dropped as this code is merged in 9_4_12.

Made ```kmcdermo:post_reco_OOT_AND_add_OOT_VID``` with the following:
- ```kmcdermo:post_reco_OOT```: adds scale/smear + GED cut-based VID for OOT photons
- combo of squashed commits from: ```lsoffi:CMSSW_10_1_1_OOT_PhotonID_wSMaj``` and ```kmcdermo:add_OOT_VID```
- add commits for OOT VID for GED and OOT Photons

### Additional useful links
 - Analysis recipes for 2017: https://twiki.cern.ch/twiki/bin/viewauth/CMS/PdmVAnalysisSummaryTable
   - Old 2017 page: https://twiki.cern.ch/twiki/bin/view/CMS/PdmV2017Analysis
 - Object recommendations: https://twiki.cern.ch/twiki/bin/view/CMS/ExoObjectRecommendationsRun2#2017_AN1
 - miniAOD 2017: https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookMiniAOD2017
 - Setup combine: https://cms-hcomb.gitbooks.io/combine/content/part1/#for-end-users-that-dont-need-to-commit-or-do-any-development
 - EGM cut-based VID: https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedPhotonIdentificationRun2
 - MET corrections+fixes: https://twiki.cern.ch/twiki/bin/view/CMS/MissingETUncertaintyPrescription#Instructions_for_9_4_X_X_9_for_2
 - JECs: https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections#CorrPatJets  
 - MET Filters: https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2#How_to_run_ecal_BadCalibReducedM
 - Computing EGM Isolations
   - https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedPhotonIdentificationRun2#Selection_implementation_details
   - https://twiki.cern.ch/twiki/bin/view/CMS/EgammaPFBasedIsolationRun2#Recipe_for_accessing_PF_isol_AN1
   - https://github.com/cms-sw/cmssw/blob/master/RecoEgamma/PhotonIdentification/plugins/PhotonIDValueMapProducer.cc#L338-L395
 - PFJetID 2017: https://twiki.cern.ch/twiki/bin/view/CMS/JetID13TeVRun2017
 - Accessors for handles
   - ECAL geometry : https://gitlab.cern.ch/shervin/ECALELF
   - Laser constants : http://cmslxr.fnal.gov/source/RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h
   - Intercalibration constants : http://cmslxr.fnal.gov/source/RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h
   - ADCToGeV : http://cmslxr.fnal.gov/source/RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h
   - Pedestals : https://github.com/ferriff/usercode/blob/master/DBDump/plugins/DBDump.cc
   - JERs : https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyResolution#Accessing_factors_from_Global_Ta
 - JER procedure explanation from https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetResolution#Smearing_procedures
 - JER implementation copied from https://github.com/cms-sw/cmssw/blob/master/PhysicsTools/PatUtils/interface/SmearedJetProducerT.h#L208-L215
 - Muon ID: https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideMuonIdRun2#Muon_selectors_Since_9_4_X
 - Computing adcToGeVInfo: http://cmslxr.fnal.gov/source/RecoEcal/EgammaCoreTools/src/EcalClusterLazyTools.cc#0204
 - Basic EGM kinematic info in miniAODv2: https://twiki.cern.ch/twiki/bin/viewauth/CMS/EgammaMiniAODV2#Applying_the_Energy_Scale_and_sm
 - Brilcalc: https://cms-service-lumi.web.cern.ch/cms-service-lumi/brilwsdoc.html
 - Lumi for 2017
   - https://hypernews.cern.ch/HyperNews/CMS/get/luminosity/761/1.html
   - https://hypernews.cern.ch/HyperNews/CMS/get/physics-validation/3067.html
   - JSON: https://cms-service-dqm.web.cern.ch/cms-service-dqm/CAF/certification/Collisions17/13TeV/ReReco/
 - Data PU distribution info
   - Instructions: https://twiki.cern.ch/twiki/bin/view/CMS/PileupJSONFileforData
   - JSON: https://cms-service-dqm.web.cern.ch/cms-service-dqm/CAF/certification/Collisions17/13TeV/PileUp/
 - GenXSecTool: https://twiki.cern.ch/twiki/bin/viewauth/CMS/HowToGenXSecAnalyzer#Automated_scripts_to_compute_the
 - Pub guidelines: https://twiki.cern.ch/twiki/bin/viewauth/CMS/Internal/PubGuidelines
 - TDR processing: https://twiki.cern.ch/twiki/bin/viewauth/CMS/Internal/TdrProcessing
 - HN Review of analysis: https://hypernews.cern.ch/HyperNews/CMS/get/EXO-19-005.html
 - CADI line: http://cms.cern.ch/iCMS/analysisadmin/cadilines?line=EXO-19-005
 - AN: http://cms.cern.ch/iCMS/jsp/db_notes/noteInfo.jsp?cmsnoteid=CMS%20AN-2018/306
 - CMS Glossary: https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookGlossary
 - LXR: http://cmslxr.fnal.gov/source
 - Working group info
  - Exotica Twiki: https://twiki.cern.ch/twiki/bin/viewauth/CMS/EXOTICA
  - EXO Meetings: https://indico.cern.ch/category/5796/
  - LLP Twiki: https://twiki.cern.ch/twiki/bin/view/CMS/ExoticaLongLived
  - LLP Meetings: https://indico.cern.ch/category/5791/

-----------------------

### Notation on variable names in nTuples

- rh = recHit
- sc = SuperCluster
- ph = photon
- pho = photon (used in branch names)
- is* = "bool" for is*
  - isHLT = is matched to HLT object
  - isGen = is matched to gen particle
- genVpion = generator V-pion (from HVDS)
- gengmsb = generator GMSB matched neutralino

- object*
  - E = energy
  - pt = transverse energy

- jet* (vector up to 10)
  - ID = jet cut based ID
  - NHF = neutral hadron energy fraction
  - NEMF = neutral EM energy fraction
  - CHF = charged hadron energy fraction
  - CEMF = charged EM energy fraction
  - MUF = muon energy fraction
  - NHM = neutral multiplicity
  - CHM = charged multiplicity

- rh* = vector of rechit values
  - ID = raw detector ID
  - TOF = time of flight corrected to PV
  - isOOT = if true, recHit is flagged OOT collection
  - isGS6,isGS1 = if true, photon has Gain Swith 6, 1 (reminder, base gain is 12, then 6, then 1)
  - ped* = pedestal noise (Gain 12, 6, 1)
  - pedrms* = pedestal noise error (Gain 12, 6, 1)

- pho* [photons 0-3] 
  - passEleVeto = photon passes electron veto
  - hasPixSeed = photon has pixel seed
  - HoE = hadTowerOverEM (single HCAL tower over EM energy)
  - r9 = seed E / 3x3 E
  - sieie = sigma_ieta,ieta
  - sipip = sigma_iphi,iphi
  - sieip = sigma_ieta,iphi
  - smaj = S_major (semi-major axis)
  - smin = S_minor (semi-minor axis)
  - alpha = angle of smaj to origin
  - suisseX = swisscross, seed energy / four surrounding energy
  - gedID = GED cut-based ID
  - ootID = OOT cut-based ID
  - Iso = Isolation
  - IsoC = Isolation corrected for (rho, pt)
    - ChgHadIso = Charged Hadron Iso
    - NeuHadIso = Neutral Hadron Iso
    - PhoIso = Photon Iso
    - EcalPFClIso = ECAL PF Cluster Iso
    - HcalPFClIso = HCAL PF Cluster Iso
    - TrkIso = Tracker Iso (hollow cone dR < 0.3)
  - recHits = vector of indices inside rh* branches for each photon
  - seed = index of seed rec hit for photon
  - seed* = value of seed rec hit branch
  - weighttime* = cluster weighted time, computed at first level skim
  - is (bools)
    - isEB = seed rechit is from ECAL barrel, else assumed endcap
    - isSignal = photon is matched to gen photon from signal particle
    - isOOT = if true, photon is from OOT collection, else from GED collection
    - isTrk = if true, object is matched to a single track pT > 5 within dR < 0.2

uncertainties:
- *Abs = (Absolute) Uncertainty = energy +/- *Abs
- *Rel = (Relative) Uncertainty = energy * (1 +/- *Rel)
- *SF  = (Scale Factor) = energy * SF

-----------------------

### Core chain of analysis

Produce ntuples with multicrab:
- test/multicrab_dispho_Data.py
- test/multicrab_dispho_Bkgd.py
- test/multicrab_dispho_GMSB.py
- test/multicrab_dispho_HVDS.py

N.B. XSecs taken from GenXSecAnalyzerTool (see fork of genproductions)
--> Currently sitting here: /afs/cern.ch/work/k/kmcdermo/private/dispho/XSEC/CMSSW_9_4_10/src/genproductions/test/calculateXSectionAndFilterEfficiency

Each script calls config: ```test/dispho.py```. Make sure to configure correctly! Plugins used:
- NTuplizer  : plugins/Dispho.cc/hh and plugins/DisPhoTypes.hh
- HelperUtils: plugins/CommonUtils.cc/hh

Output of ntuples end up in EOS group space: currently configured for /eos/cms/group/phys_exotica/displacedPhotons/nTuples/2017. Now begin macro chain of analysis: 

- cd to ```dispho_work/macros```.
- Produce skims with new time branches, event weights, basic core skim + met flags: scripts/skimAndMerge/*.sh
  - Also makes PU weights...
  - Can launch skims in parallel with loops in background
- Produce data+bkgd mc skims into a single file: scripts/runFastSkimmer.sh
- Produce signal skims into single file: scripts/runSignalSkimmer.sh
- Make sure to do both for GJets CR, QCD CR, and SR selection

- Then run the scripts for making the variable weight trees in CR to SR: scripts/makeWgtsAndPlots.sh
- Then check 1D final plots: scripts/makePlotsForSR.sh
- Finally, run 2D plots for fis: scripts/makeLimits.sh

Plots outputted to lxplus eos user space: /eos/user/k/kmcdermo/www/dispho/plots

-----------------------

### Extracting lumi info

Compute lumi for 2017: 

```
brilcalc lumi --normtag /cvmfs/cms-bril.cern.ch/cms-lumi-pog/Normtags/normtag_PHYSICS.json -u /fb -i /afs/cern.ch/cms/CAF/CMSCOMM/COMM_DQM/certification/Collisions17/13TeV/ReReco/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON_v1.txt
```

Compute PU info for 2017 with 94X environment:

```
pileupCalc.py -i ~/public/input/golden2017.json --inputLumiJSON ~/private/bril/pu2017json.txt --calcMode true --minBiasXsec 69200 --maxPileupBin 200 --numPileupBins 200 MyDataPileupHistogram.root
```

--------------------------------

### Old files of interest stored in cms user kmcdermo space

- Backup of older fast skims: /eos/cms/store/user/kmcdermo/nTuples/BACKUP/afs/cern.ch/work/k/kmcdermo/private/dispho/Analysis
  - production directory is older mirror of development directory
  - CMSSW_9_4_10 is copy of since deprecated skims
- Backup of since deprecated skims (original output from CRAB deleted): /eos/cms/store/user/kmcdermo/nTuples/skims
- Backup of since deprecated unskimmed CRAB output for OOT VID studies: /eos/cms/store/user/kmcdermo/nTuples/unskimmed

--------------------------------

### To do:
- switch ctau to tau [ns]
- make sure to use REAL ctau in limit plots... update scripts to read in this value
- make exclusion in 2D vs neutralino mass
- make exclusion in 1D vs REAL ctau (slices of neutralino mass)
