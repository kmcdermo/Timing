import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('python')

## blinding
options.register('blindSF',1000,VarParsing.multiplicity.singleton,VarParsing.varType.int,'pick every nth SF event');
options.register('applyBlindSF',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to apply event SF blinding');
options.register('blindMET',100.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'blind events greater than MET cut');
options.register('applyBlindMET',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to apply event MET blinding');

## object prep cuts
options.register('jetpTmin',15.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'jet pT minimum cut');
options.register('jetEtamax',3.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'jet eta maximum cut');
options.register('jetIDmin',1,VarParsing.multiplicity.singleton,VarParsing.varType.int,'jet ID minimum cut');
options.register('rhEmin',1.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'recHit energy minimum cut');
options.register('phpTmin',20.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'photon pT minimum cut');
options.register('phIDmin','none',VarParsing.multiplicity.singleton,VarParsing.varType.string,'photon ID minimum cut');

## object extra pruning cuts
options.register('seedTimemin',-25.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'photon seed time minimum cut');
options.register('nPhosmax',2,VarParsing.multiplicity.singleton,VarParsing.varType.int,'number of photons to clean jets');

## photon storing options
options.register('splitPho',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'store leading top two photons, OOT and GED');
options.register('onlyGED',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'store only leading GED photons, at most four');
options.register('onlyOOT',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'store only leading OOT photons, at most four');

## lepton prep cuts
options.register('ellowpTmin',20.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'electron low pT minimum cut');
options.register('elhighpTmin',50.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'electron high pT minimum cut');
options.register('mulowpTmin',20.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'muon low pT minimum cut');
options.register('muhighpTmin',50.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'muon high pT minimum cut');

## rechit storing options
options.register('storeRecHits',True,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'store all rechits above rhEmin');

## pre-selection cuts
options.register('applyTrigger',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to apply trigger pre-selection');
options.register('minHT',400.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'jet HT minimum cut');
options.register('applyHT',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to apply HT pre-selection');
options.register('phgoodpTmin',70.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'good photon pT minimum cut');
options.register('phgoodIDmin','loose',VarParsing.multiplicity.singleton,VarParsing.varType.string,'good photon ID minimum cut');
options.register('applyPhGood',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to require at least one good photon in pre-selection');

## matching cuts
options.register('dRmin',0.3,VarParsing.multiplicity.singleton,VarParsing.varType.float,'dR minimum cut');
options.register('pTres',100.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'pT resolution cut');
options.register('gendRmin',0.1,VarParsing.multiplicity.singleton,VarParsing.varType.float,'gen dR minimum cut');
options.register('genpTres',0.5,VarParsing.multiplicity.singleton,VarParsing.varType.float,'gen pT resolution cut');
options.register('trackdRmin',0.2,VarParsing.multiplicity.singleton,VarParsing.varType.float,'track dR minimum cut');
options.register('trackpTmin',5.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'track pT minimum cut');
options.register('genjetdRmin',0.2,VarParsing.multiplicity.singleton,VarParsing.varType.float,'genjet dR minimum cut for smearing');
options.register('genjetpTfactor',3.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'genjet pT resolution factor for smearing');
options.register('leptondRmin',0.3,VarParsing.multiplicity.singleton,VarParsing.varType.float,'lepton dR minimum cut for veto');

## extra JER info
options.register('smearjetEmin',0.01,VarParsing.multiplicity.singleton,VarParsing.varType.float,'min jet E for smearing');

## trigger input
options.register('inputPaths','/afs/cern.ch/user/k/kmcdermo/public/input/HLTpathsWExtras.txt',VarParsing.multiplicity.singleton,VarParsing.varType.string,'text file list of input signal paths');
options.register('inputFilters','/afs/cern.ch/user/k/kmcdermo/public/input/HLTfilters.txt',VarParsing.multiplicity.singleton,VarParsing.varType.string,'text file list of input signal filters');

## met filter input
options.register('inputFlags','/afs/cern.ch/user/k/kmcdermo/public/input/METflags.txt',VarParsing.multiplicity.singleton,VarParsing.varType.string,'text file list of input MET filter flags');

## data or MC options
options.register('isMC',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to indicate data or MC');
options.register('isGMSB',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to indicate GMSB');
options.register('isHVDS',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to indicate HVDS');
options.register('isBkgd',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to indicate Background MC');
options.register('isToy',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to indicate Toy MC');
options.register('isADD',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to indicate ADD Monophoton');
options.register('xsec',1.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'cross section in pb');
options.register('filterEff',1.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'filter efficiency of MC');
options.register('BR',1.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'branching ratio of MC');

## GT to be used
options.register('globalTag','94X_mc2017_realistic_v17',VarParsing.multiplicity.singleton,VarParsing.varType.string,'gloabl tag to be used');

## do a demo run over only 1k events
options.register('demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to run over only 1k events');

## processName
options.register('processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,'process name to be considered');

## outputFile Name
options.register('outputFileName','dispho.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,'output file name created by cmsRun');

## etra bits
options.register('deleteEarly',True,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'delete temp products early if not needed');
options.register('runUnscheduled',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'run unscheduled for products');

## parsing command line arguments
options.parseArguments()

if options.isGMSB or options.isHVDS or options.isBkgd or options.isToy or options.isADD: options.isMC = True

print "     ##### Settings ######"
print "       -- Blinding --"
print "blindSF        : ",options.blindSF
print "applyBlindSF   : ",options.applyBlindSF
print "blindMET       : ",options.blindMET
print "applyBlindMET  : ",options.applyBlindMET
print "      -- Object Prep --"
print "jetpTmin       : ",options.jetpTmin
print "jetEtamax      : ",options.jetEtamax
print "jetIDmin       : ",options.jetIDmin
print "rhEmin         : ",options.rhEmin
print "phpTmin        : ",options.phpTmin
print "phIDmin        : ",options.phIDmin
print "     -- Extra Pruning --"
print "seedTimemin    : ",options.seedTimemin
print "nPhosmax       : ",options.nPhosmax
print "     -- Photon Storing --"
print "splitPho       : ",options.splitPho
print "onlyGED        : ",options.onlyGED
print "onlyOOT        : ",options.onlyOOT
print "      -- Lepton Prep --"
print "ellowpTmin     : ",options.ellowpTmin
print "elhighpTmin    : ",options.elhighpTmin
print "mulowpTmin     : ",options.mulowpTmin
print "muhighpTmin    : ",options.muhighpTmin
print "     -- RecHit Storing --"
print "storeRecHits   : ",options.storeRecHits
print "   -- Event Pre-Selection --"
print "applyTrigger   : ",options.applyTrigger
print "minHT          : ",options.minHT
print "applyHT        : ",options.applyHT
print "phgoodpTmin    : ",options.phgoodpTmin	
print "phgoodIDmin    : ",options.phgoodIDmin
print "applyPhGood    : ",options.applyPhGood	
print "        -- Matching --"
print "dRmin          : ",options.dRmin
print "pTres          : ",options.pTres
print "gendRmin       : ",options.gendRmin
print "genpTres       : ",options.genpTres
print "trackdRmin     : ",options.trackdRmin
print "trackpTmin     : ",options.trackpTmin
print "genjetdRmin    : ",options.genjetdRmin
print "genjetpTfactor : ",options.genjetpTfactor
print "leptondRmin    : ",options.leptondRmin
print "       -- Extra JER --"
print "smearjetEmin   : ",options.smearjetEmin
print "        -- Trigger --"
print "inputPaths     : ",options.inputPaths
print "inputFilters   : ",options.inputFilters
print "       -- MET Filters --"
print "inputFlags     : ",options.inputFlags
print "        -- MC Info --"
print "isMC           : ",options.isMC
if options.isMC:
	print "isGMSB         : ",options.isGMSB
	print "isHVDS         : ",options.isHVDS
	print "isBkgd         : ",options.isBkgd
	print "isToy          : ",options.isToy
	print "isADD          : ",options.isADD
	print "xsec           : ",options.xsec
	print "filterEff      : ",options.filterEff
	print "BR             : ",options.BR
print "           -- GT --"
print "globalTag      : ",options.globalTag	
print "         -- Output --"
print "demoMode       : ",options.demoMode
print "processName    : ",options.processName	
print "outputFileName : ",options.outputFileName	
print "        -- Extra bits --"
print "deleteEarly    : ",options.deleteEarly
print "runUnscheduled : ",options.runUnscheduled
print "     #####################"

## Define the CMSSW process
process = cms.Process(options.processName)

## Load the standard set of configuration modules
process.load("Configuration.Geometry.GeometryRecoDB_cff")
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
process.load("Geometry.CaloEventSetup.CaloTowerConstituents_cfi")

## Message Logger settings
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.destinations = ['cout', 'cerr']
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

## Define the input source
process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
		# reminiaod data: 94X_dataRun2_v11
		#'/store/user/kmcdermo/files/miniAOD/SPH_2017E_miniAODv2.root'
		# reminiaod GJets, GT: 94X_mc2017_realistic_v17
		#'/store/user/kmcdermo/files/miniAOD/GJets_600toInf_miniAODv2.root'
		# miniaodv2 GMSB, GT: 94X_mc2017_realistic_v17
		'/store/user/kmcdermo/files/miniAOD/GMSB_L200TeV_CTau200cm_miniAODv2.root'
		))

## How many events to process
if   options.demoMode : process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1000))
else                  : process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))

# Set the global tag depending on the sample type
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag.globaltag = options.globalTag  

## Create output file
## Setup the service to make a ROOT TTree
process.TFileService = cms.Service("TFileService", 
		                   fileName = cms.string(options.outputFileName))

## Decide which label to use for MET Flags
if   options.isMC : triggerFlagsProcess = "PAT"
else              : triggerFlagsProcess = "RECO"

## generate track collection at miniAOD
from PhysicsTools.PatAlgos.slimming.unpackedTracksAndVertices_cfi import unpackedTracksAndVertices
process.unpackedTracksAndVertices = unpackedTracksAndVertices.clone()

## MET corrections for 2017 data: https://twiki.cern.ch/twiki/bin/view/CMS/MissingETUncertaintyPrescription#Instructions_for_9_4_X_X_9_for_2
from PhysicsTools.PatUtils.tools.runMETCorrectionsAndUncertainties import runMetCorAndUncFromMiniAOD
runMetCorAndUncFromMiniAOD (
        process,
        isData = not options.isMC,
        fixEE2017 = True,
	fixEE2017Params = {'userawPt':True, 'ptThreshold':50.0, 'minEtaThreshold':2.65, 'maxEtaThreshold':3.139},
        postfix = "ModifiedMET"
)

## Apply JECs : https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections#CorrPatJets
from PhysicsTools.PatAlgos.tools.jetTools import updateJetCollection
updateJetCollection (
   process,
   jetSource = cms.InputTag('slimmedJets'),
   labelName = 'UpdatedJEC',
   jetCorrections = ('AK4PFchs', cms.vstring(['L1FastJet', 'L2Relative', 'L3Absolute', 'L2L3Residual']), 'None')
)

## Rerun one MET filter: https://twiki.cern.ch/twiki/bin/viewauth/CMS/MissingETOptionalFiltersRun2#How_to_run_ecal_BadCalibReducedM
baddetEcallist = cms.vuint32(
    [872439604,872422825,872420274,872423218,
     872423215,872416066,872435036,872439336,
     872420273,872436907,872420147,872439731,
     872436657,872420397,872439732,872439339,
     872439603,872422436,872439861,872437051,
     872437052,872420649,872422436,872421950,
     872437185,872422564,872421566,872421695,
     872421955,872421567,872437184,872421951,
     872421694,872437056,872437057,872437313])

process.ecalBadCalibReducedMINIAODFilter = cms.EDFilter("EcalBadCalibFilter",
    EcalRecHitSource = cms.InputTag("reducedEgamma:reducedEERecHits"),
    ecalMinEt        = cms.double(50.),
    baddetEcal       = baddetEcallist, 
    taggingMode      = cms.bool(True),
    debug            = cms.bool(False)
)

## Apply Scale/Smearing + GED and OOT VID to ootPhotons
from RecoEgamma.EgammaTools.OOTPhotonPostRecoTools import setupOOTPhotonPostRecoSeq
setupOOTPhotonPostRecoSeq(process)

# Make the tree 
process.tree = cms.EDAnalyzer("DisPho",
   ## blinding 
   blindSF = cms.int32(options.blindSF),
   applyBlindSF = cms.bool(options.applyBlindSF),
   blindMET = cms.double(options.blindMET),
   applyBlindMET = cms.bool(options.applyBlindMET),
   ## object prep cuts
   jetpTmin  = cms.double(options.jetpTmin),
   jetEtamax = cms.double(options.jetEtamax),
   jetIDmin  = cms.int32(options.jetIDmin),
   rhEmin    = cms.double(options.rhEmin),
   phpTmin   = cms.double(options.phpTmin),
   phIDmin   = cms.string(options.phIDmin),
   ## extra object pruning
   seedTimemin = cms.double(options.seedTimemin),
   nPhosmax    = cms.int32(options.nPhosmax),
   ## photon storing options
   splitPho = cms.bool(options.splitPho),
   onlyGED  = cms.bool(options.onlyGED),
   onlyOOT  = cms.bool(options.onlyOOT),
   ## lepton prep cuts 
   ellowpTmin  = cms.double(options.ellowpTmin),
   elhighpTmin = cms.double(options.elhighpTmin),
   mulowpTmin  = cms.double(options.mulowpTmin),
   muhighpTmin = cms.double(options.muhighpTmin),
   ## recHit storing options
   storeRecHits = cms.bool(options.storeRecHits),
   ## pre-selection
   applyTrigger = cms.bool(options.applyTrigger),
   minHT        = cms.double(options.minHT),
   applyHT      = cms.bool(options.applyHT),
   phgoodpTmin  = cms.double(options.phgoodpTmin),
   phgoodIDmin  = cms.string(options.phgoodIDmin),
   applyPhGood  = cms.bool(options.applyPhGood),
   ## matched criteria
   dRmin = cms.double(options.dRmin),
   pTres = cms.double(options.pTres),
   gendRmin = cms.double(options.gendRmin),
   genpTres = cms.double(options.genpTres),
   trackdRmin = cms.double(options.trackdRmin),
   trackpTmin = cms.double(options.trackpTmin),
   genjetdRmin = cms.double(options.genjetdRmin),
   genjetpTfactor = cms.double(options.genjetpTfactor),
   leptondRmin = cms.double(options.leptondRmin),
   ## extra JER info
   smearjetEmin = cms.double(options.smearjetEmin),
   ## triggers
   inputPaths     = cms.string(options.inputPaths),
   inputFilters   = cms.string(options.inputFilters),
   triggerResults = cms.InputTag("TriggerResults", "", "HLT"),
   triggerObjects = cms.InputTag("slimmedPatTrigger"),
   ## met filters
   inputFlags       = cms.string(options.inputFlags),
   triggerFlags     = cms.InputTag("TriggerResults", "", triggerFlagsProcess),
   ecalBadCalibFlag = cms.InputTag("ecalBadCalibReducedMINIAODFilter"),			      
   ## tracks
   tracks = cms.InputTag("unpackedTracksAndVertices"),
   ## vertices
   vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
   ## rho
   rho = cms.InputTag("fixedGridRhoFastjetAll"), #fixedGridRhoAll
   ## METs
   mets = cms.InputTag("slimmedMETsModifiedMET"),
   ## jets
   jets = cms.InputTag("updatedPatJetsUpdatedJEC"),
   ## electrons
   electrons = cms.InputTag("slimmedElectrons"),
   ## muons
   muons = cms.InputTag("slimmedMuons"),
   ## photons
   gedPhotons = cms.InputTag("slimmedPhotons"),
   ootPhotons = cms.InputTag("slimmedOOTPhotons"),
   ## ecal recHits
   recHitsEB = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
   recHitsEE = cms.InputTag("reducedEgamma", "reducedEERecHits"),
   ## gen info
   isGMSB       = cms.bool(options.isGMSB),
   isHVDS       = cms.bool(options.isHVDS),
   isBkgd       = cms.bool(options.isBkgd),
   isToy        = cms.bool(options.isToy),
   isADD        = cms.bool(options.isADD),
   xsec         = cms.double(options.xsec),
   filterEff    = cms.double(options.filterEff),
   BR           = cms.double(options.BR),
   genEvt       = cms.InputTag("generator"),
   gent0        = cms.InputTag("genParticles", "t0"),
   genxyz0      = cms.InputTag("genParticles", "xyz0"),
   pileups      = cms.InputTag("slimmedAddPileupInfo"),
   genParticles = cms.InputTag("prunedGenParticles"),
   genJets      = cms.InputTag("slimmedGenJets"),
)

# Set up the path
process.treePath = cms.Path(
	process.patJetCorrFactorsUpdatedJEC +
	process.updatedPatJetsUpdatedJEC +
	process.fullPatMetSequenceModifiedMET +
	process.ecalBadCalibReducedMINIAODFilter +
	process.unpackedTracksAndVertices +
	process.ootPhotonPostRecoSeq +
	process.tree
)

### Extra bits from other configs
process.options = cms.untracked.PSet()

from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
if options.deleteEarly:
	process = customiseEarlyDelete(process)

from FWCore.ParameterSet.Utilities import convertToUnscheduled
if options.runUnscheduled : 
	process = convertToUnscheduled(process)
