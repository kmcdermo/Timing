# Displaced Photon Analysis Framework

Update to README: old 80X code removed, but still have GEN_SIM...

setup:
```
cmsrel CMSSW_9_4_10
cd CMSSW_9_4_10/src
cmsenv
git cms-init

git cms-merge-topic cms-met:METFixEE2017_949_v2
git cms-merge-topic kmcdermo:scale_and_smear_OOT

scram b -j 16
```

-----------------------

notation on variable names in nTuples

rh = recHit
sc = SuperCluster
ph = photon
pho = photon (used in branch names)
is* = "bool" for is*
  isHLT = is matched to HLT object
  isGen = is matched to gen particle
genVpion = generator V-pion (from HVDS)
gengmsb = generator GMSB matched neutralino

object*
  E = energy
  pt = transverse energy

jet* (vector up to 10)
  ID = jet cut based ID
  NHF = neutral hadron energy fraction
  NEMF = neutral EM energy fraction
  CHF = charged hadron energy fraction
  CEMF = charged EM energy fraction
  MUF = muon energy fraction
  NHM = neutral multiplicity
  CHM = charged multiplicity

rh* = vector of rechit values
  ID = raw detector ID
  TOF = time of flight corrected to PV
  isOOT = if true, recHit is flagged OOT collection
  isGS6,isGS1 = if true, photon has Gain Swith 6, 1 (reminder, base gain is 12, then 6, then 1)
  ped* = pedestal noise (Gain 12, 6, 1)
  pedrms* = pedestal noise error (Gain 12, 6, 1)

pho* [photons 0-3] 
  passEleVeto = photon passes electron veto
  hasPixSeed = photon has pixel seed
  HoE = hadTowerOverEM (single HCAL tower over EM energy)
  r9 = seed E / 3x3 E
  sieie = sigma_ieta,ieta
  sipip = sigma_iphi,iphi
  sieip = sigma_ieta,iphi
  smaj = S_major (semi-major axis)
  smin = S_minor (semi-minor axis)
  alpha = angle of smaj to origin
  suisseX = swisscross, seed energy / four surrounding energy
  gedID = GED cut-based ID
  ootID = OOT cut-based ID
  Iso = Isolation
  IsoC = Isolation corrected for (rho, pt)
    ChgHadIso = Charged Hadron Iso
    NeuHadIso = Neutral Hadron Iso
    PhoIso = Photon Iso
    EcalPFClIso = ECAL PF Cluster Iso
    HcalPFClIso = HCAL PF Cluster Iso
    TrkIso = Tracker Iso (hollow cone dR < 0.3)
  recHits = vector of indices inside rh* branches for each photon
  seed = index of seed rec hit for photon
  seed* = value of seed rec hit branch
  is (bools)
    isEB = seed rechit is from ECAL barrel, else assumed endcap
    isSignal = photon is matched to gen photon from signal particle
    isOOT = if true, photon is from OOT collection, else from GED collection
    isTrk = if true, object is matched to a single track pT > 5 within dR < 0.2

uncertainties:
*Abs = (Absolute) Uncertainty = energy +/- *Abs
*Rel = (Relative) Uncertainty = energy * (1 +/- *Rel)
*SF  = (Scale Factor) = energy * SF

-----------------------

Core chain of analysis:

NTuplizer  : plugins/Dispho.cc/hh
HelperUtils: plugins/CommonUtils.cc/hh

Produce ntuples with multicrab:
test/multicrab_dispho_Data.py
test/multicrab_dispho_Bkgd.py
test/multicrab_dispho_Signal.py

Which calls config: test/dispho.py
Make sure to configure correctly!

XSecs from signals: see GEN_SIM examples
XSecs from bkgds: see genproductions GH (local fork branch: xsec)

Output of ntuples end up in EOS space. Now begin macro chain of analysis. cd to dispho_work/macros

Produce skims with new time branches, event weights, basic core skim + met flags: scripts/skimAndMerge/*.sh
Also makes PU weights...
Will have to launch each process... annoying. loops are serial and long... looking to parallelize

Produce data+bkgd mc skims: scripts/runFastSkimmer.sh
Produce signal skims: scripts/runSignalSkimmer.sh
Make sure to do both for GJets CR, QCD CR, and SR selection

Then run the scripts for making the variable weight trees in CR to SR: scripts/makeWgtsAndPlots.sh
Then check 1D final plots: scripts/makePlotsForSR.sh
Finally, run 2D plots for fis: scripts/makeLimits.sh

--------------------------------
to do:

- switch ctau to tau [ns]
- make sure to use REAL ctau in limit plots... update scripts to read in this value
- make exclusion in 2D vs neutralino mass
- make exclusion in 1D vs REAL ctau (slices of neutralino mass)
