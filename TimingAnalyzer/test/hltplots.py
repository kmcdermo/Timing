import os, re
import FWCore.ParameterSet.Config as cms
import FWCore.PythonUtilities.LumiList as LumiList  
import FWCore.ParameterSet.Types as CfgTypes    
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## pre-selection
options.register('applyTriggerPS',True,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to apply pre-selection trigger');
options.register('psPath','HLT_IsoMu27_v',VarParsing.multiplicity.singleton,VarParsing.varType.string,'name of pre-selection trigger');

## object selection
options.register('phpTmin',20.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'photon pT minimum cut');
options.register('jetpTmin',15.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'jet pT minimum cut');
options.register('jetIDmin',1,VarParsing.multiplicity.singleton,VarParsing.varType.int,'jet ID minimum cut');
options.register('jetEtamax',3.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'jet eta maximum cut');

## object extra pruning cuts
options.register('seedTimemin',-25.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'photon seed time minimum cut');
options.register('nPhosmax',2,VarParsing.multiplicity.singleton,VarParsing.varType.int,'number of photons to clean jets');

## object matching
options.register('dRmin',0.3,VarParsing.multiplicity.singleton,VarParsing.varType.float,'dR minimum cut');
options.register('pTres',0.5,VarParsing.multiplicity.singleton,VarParsing.varType.float,'pT resolution cut');
options.register('trackpTmin',5.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'track pT minimum cut');
options.register('trackdRmin',0.2,VarParsing.multiplicity.singleton,VarParsing.varType.float,'track dR minimum cut');

## trigger input
options.register('inputPaths','/afs/cern.ch/user/k/kmcdermo/public/input/HLTpaths.txt',VarParsing.multiplicity.singleton,VarParsing.varType.string,'text file list of input signal paths');
options.register('inputFilters','/afs/cern.ch/user/k/kmcdermo/public/input/HLTfilters.txt',VarParsing.multiplicity.singleton,VarParsing.varType.string,'text file list of input signal filters');

## GT to be used    
options.register('globalTag','94X_dataRun2_v11',VarParsing.multiplicity.singleton,VarParsing.varType.string,'global tag to be used');

## do a demo run over only 1k events
options.register('demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to run over only 1k events as a demo');

## processName
options.register('processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,'process name to be considered');

## outputFile Name
options.register('outputFileName','hltplots.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,'output file name created by cmsRun');

## parsing command line arguments
options.parseArguments()

## Set PSPath to empty string and to not apply PS if psPath='NONE' or applyTriggerPS=False
if not options.applyTriggerPS or options.psPath == 'NONE' or options.psPath == 'none' or options.psPath == '' : 
    options.applyTriggerPS = False
    options.psPath = ''

## Print configuration
print "     ##### Settings ######"
print "     -- Pre-Selection --"
print "applyTriggerPS : ",options.applyTriggerPS
print "psPath         : ",options.psPath
print "      -- Object Prep --"
print "phpTmin        : ",options.phpTmin
print "jetpTmin       : ",options.jetpTmin
print "jetIDmin       : ",options.jetIDmin
print "jetEtamax      : ",options.jetEtamax
print "     -- Extra Pruning --"
print "seedTimemin    : ",options.seedTimemin
print "nPhosmax       : ",options.nPhosmax
print "        -- Matching --"
print "dRmin          : ",options.dRmin
print "pTres          : ",options.pTres
print "trackdRmin     : ",options.trackdRmin
print "trackpTmin     : ",options.trackpTmin
print "        -- Trigger --"
print "inputPaths     : ",options.inputPaths
print "inputFilters   : ",options.inputFilters
print "           -- GT --"
print "globalTag      : ",options.globalTag	
print "         -- Output --"
print "demoMode       : ",options.demoMode
print "processName    : ",options.processName	
print "outputFileName : ",options.outputFileName	
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
        ## Data: SingleMu miniAODv2, GT: 94X_dataRun2_v11, PSPath: HLT_IsoMu27_v
        #'/store/user/kmcdermo/files/miniAOD/SMU_2017D_miniAODv2.root'

        ## Data: SinglePh miniAODv2, GT: 94X_dataRun2_v11, PSPath: HLT_Photon60_R9Id90_CaloIdL_IsoL_v, HLT_Photon60_R9Id90_CaloIdL_IsoL_DisplacedIdL_v
        #'/store/user/kmcdermo/files/miniAOD/SPH_2017E_miniAODv2.root'

        ## GSMB: L: 200TeV CTau: 200cm miniAODv2, GT: 94X_mc2017_realistic_v17, PSPath: NONE
        '/store/user/kmcdermo/files/miniAOD/GMSB_L200TeV_CTau200cm_miniAODv2.root'
        ))

## How many events to process
if   options.demoMode : process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1000))
else                  : process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

# Set the global tag depending on the sample type
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag.globaltag = options.globalTag  

## Create output file
## Setup the service to make a ROOT TTree
process.TFileService = cms.Service("TFileService", 
		                   fileName = cms.string(options.outputFileName))

## generate track collection at miniAOD
from PhysicsTools.PatAlgos.slimming.unpackedTracksAndVertices_cfi import unpackedTracksAndVertices
process.unpackedTracksAndVertices = unpackedTracksAndVertices.clone()

## MET corrections for 2017 data: https://twiki.cern.ch/twiki/bin/view/CMS/MissingETUncertaintyPrescription#Instructions_for_9_4_X_X_9_for_2
from PhysicsTools.PatUtils.tools.runMETCorrectionsAndUncertainties import runMetCorAndUncFromMiniAOD
runMetCorAndUncFromMiniAOD (
        process,
        isData = True,
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

## Apply Scale/Smearing + GED and OOT VID to ootPhotons
from RecoEgamma.EgammaTools.OOTPhotonPostRecoTools import setupOOTPhotonPostRecoSeq
setupOOTPhotonPostRecoSeq(process)

# Make the tree 
process.tree = cms.EDAnalyzer("HLTPlots",
   ## pre-selection         
   applyTriggerPS = cms.bool(options.applyTriggerPS),
   psPath         = cms.string(options.psPath),
   ## object selection
   phpTmin   = cms.double(options.phpTmin),
   jetpTmin  = cms.double(options.jetpTmin),
   jetIDmin  = cms.int32(options.jetIDmin),
   jetEtamax = cms.double(options.jetEtamax),
   ## extra object pruning
   seedTimemin = cms.double(options.seedTimemin),
   nPhosmax    = cms.int32(options.nPhosmax),
   ## object matching
   dRmin = cms.double(options.dRmin),
   pTres = cms.double(options.pTres),
   trackpTmin = cms.double(options.trackpTmin),
   trackdRmin = cms.double(options.trackdRmin),
   ## triggers
   inputPaths     = cms.string(options.inputPaths),
   inputFilters   = cms.string(options.inputFilters),
   triggerResults = cms.InputTag("TriggerResults", "", "HLT"),
   triggerObjects = cms.InputTag("slimmedPatTrigger"),
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
   ## photons
   gedPhotons = cms.InputTag("slimmedPhotons"),
   ootPhotons = cms.InputTag("slimmedOOTPhotons"),
   ## ecal recHits
   recHitsEB = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
   recHitsEE = cms.InputTag("reducedEgamma", "reducedEERecHits")
)

# Set up the path
process.treePath = cms.Path(
	process.patJetCorrFactorsUpdatedJEC +
	process.updatedPatJetsUpdatedJEC +
	process.fullPatMetSequenceModifiedMET +
	process.unpackedTracksAndVertices +
	process.ootPhotonPostRecoSeq +
	process.tree
)
