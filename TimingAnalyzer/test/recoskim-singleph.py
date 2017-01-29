import os, re
import FWCore.ParameterSet.Config as cms
import FWCore.PythonUtilities.LumiList as LumiList  
import FWCore.ParameterSet.Types as CfgTypes    

### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## do a demo run over only 1k events
options.register (
	'demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to run over only 1k events as a demo');

## use which block of events
options.register (
	'blockNo',1,VarParsing.multiplicity.singleton,VarParsing.varType.int,
	'flag to use which block of files');

## processName
options.register (
	'processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'process name to be considered');

## outputFile Name
options.register (
	'outputFileName','recoskim-singleph.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'output file name created by cmsRun');

## GT to be used    
options.register (
	'globalTag','80X_dataRun2_2016SeptRepro_v4',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'gloabl tag to be used');

## parsing command line arguments
options.parseArguments()

if options.blockNo == 1:
	options.outputFileName = 'recoskim-singleph1.root'
elif options.blockNo == 2:
	options.outputFileName = 'recoskim-singleph2.root'
else: exit

print "##### Settings ######"
print "Running with demoMode            = ",options.demoMode
print "Running with blockNo             = ",options.blockNo
print "Running with processName         = ",options.processName	
print "Running with outputFileName      = ",options.outputFileName	
print "Running with globalTag           = ",options.globalTag	
print "#####################"

## Define the CMSSW process
process = cms.Process(options.processName)

## Load the standard set of configuration modules
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_38T_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')

## Message Logger settings
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.destinations = ['cout', 'cerr']
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

## Define the input source
if options.blockNo == 1:
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/14EE3AEC-471A-E611-A84F-02163E014380.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/16F71DEB-1E1A-E611-B34A-02163E011EE1.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/1879CF4C-5A1A-E611-BF7F-02163E0142F8.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/1A30D6F9-2E1A-E611-8654-02163E014167.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/1A70CC9C-1E1A-E611-BA9B-02163E01190F.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/1C28DBD0-491A-E611-9195-02163E01438C.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/1C3D7A90-671A-E611-9062-02163E014150.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/20DBD299-221A-E611-998B-02163E014251.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/20E5C0C3-281A-E611-93D2-02163E01426D.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/2656477F-291A-E611-AA71-02163E011C18.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/26FD0C51-271A-E611-8323-02163E0129CF.root'
			))
elif options.blockNo == 2:
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/006806FF-211A-E611-A850-02163E014111.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/02542059-2E1A-E611-9602-02163E0136B1.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/029E986F-1E1A-E611-A489-02163E0122B1.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/04D17866-261A-E611-B00F-02163E014259.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/068244C0-481A-E611-B513-02163E01380C.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/08A87850-201A-E611-AAA5-02163E01271B.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/08FBFF90-3C1A-E611-BB7F-02163E01462D.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/0E00A648-381A-E611-AC04-02163E0141E1.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/1061E38E-291A-E611-9C28-02163E011857.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/1409FF3B-2D1A-E611-8A0C-02163E0143A7.root',
			'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/14B7A527-1A1A-E611-AE84-02163E01475F.root'
			))
else: exit

# Set the json locally because the ReRECO gets f'ed
# process.source.lumisToProcess = CfgTypes.untracked(CfgTypes.VLuminosityBlockRange())  
# JSONfile = '/afs/cern.ch/user/k/kmcdermo/private/dispho/CMSSW_8_0_21/src/Timing/TimingAnalyzer/test/data_rereco.json'
# myLumis = LumiList.LumiList(filename = JSONfile).getCMSSWString().split(',')  
# process.source.lumisToProcess.extend(myLumis)                              

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

# Make the tree 
process.tree = cms.EDAnalyzer("RECOSkim",
   ## vertices
   vertices = cms.InputTag("offlinePrimaryVertices"),
   ## rho
   rhos = cms.InputTag("fixedGridRhoFastjetAll"), #fixedGridRhoAll
   ## trigger
   triggers = cms.InputTag("TriggerResults", "", "HLT"),
   ## MET
   mets = cms.InputTag("pfMet"),
   ## jets			    	
   jets = cms.InputTag("ak4PFJetsCHS"),
   ## photons		
   photons        = cms.InputTag("gedPhotons"),
   ## ecal recHits			      
   recHitCollectionEB = cms.InputTag("ecalRecHit","EcalRecHitsEB"),
   recHitCollectionEE = cms.InputTag("ecalRecHit","EcalRecHitsEE")
)

# Set up the path
process.treePath = cms.Path(process.tree)
