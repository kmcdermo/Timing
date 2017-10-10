import os, re
import FWCore.ParameterSet.Config as cms
import FWCore.PythonUtilities.LumiList as LumiList  
import FWCore.ParameterSet.Types as CfgTypes    
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## general cuts
options.register (
	'phpTmin',20.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,
	'photon pT minimum cut');

options.register (
	'jetpTmin',15.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,
	'jet pT minimum cut');

options.register (
	'dRmin',0.4,VarParsing.multiplicity.singleton,VarParsing.varType.float,
	'dR minimum cut');

options.register (
	'pTres',0.5,VarParsing.multiplicity.singleton,VarParsing.varType.float,
	'pT resolution cut');

options.register (
	'trackpTmin',5.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,
	'track pT minimum cut');

options.register (
	'trackdRmin',0.2,VarParsing.multiplicity.singleton,VarParsing.varType.float,
	'track dR minimum cut');

options.register (
	'saveTrigObjs',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to save trigger objects from associated filters');

## trigger input
options.register (
        'inputPaths','/afs/cern.ch/user/k/kmcdermo/public/input/HLTpaths.txt',VarParsing.multiplicity.singleton,VarParsing.varType.string,
        'text file list of input signal paths');

options.register (
        'inputFilters','/afs/cern.ch/user/k/kmcdermo/public/input/HLTfilters.txt',VarParsing.multiplicity.singleton,VarParsing.varType.string,
        'text file list of input signal filters');

## processName
options.register (
	'processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'process name to be considered');

## outputFile Name
options.register (
	'outputFileName','hltdump.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'output file name created by cmsRun');

## GT to be used    
options.register (
	'globalTag','92X_dataRun2_Prompt_v7',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'global tag to be used');

## do a demo run over only 100k events
options.register (
	'demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to run over only 1k events as a demo');

## flag to use ootPhoton collections (should be false if the collection is missing from the data!!
options.register (
	'useOOTPhotons',True,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to use ootPhoton collections in analyzer');

## parsing command line arguments
options.parseArguments()

print "##### Settings ######"
print "Running with phpTmin             = ",options.phpTmin
print "Running with jetpTmin            = ",options.jetpTmin
print "Running with dRmin               = ",options.dRmin
print "Running with pTres               = ",options.pTres
print "Running with trackpTmin          = ",options.trackpTmin
print "Running with trackdRmin          = ",options.trackdRmin
print "Running with saveTrigObjs        = ",options.saveTrigObjs
print "Running with inputPaths          = ",options.inputPaths
print "Running with inputFilters        = ",options.inputFilters
print "Running with processName         = ",options.processName	
print "Running with outputFileName      = ",options.outputFileName	
print "Running with globalTag           = ",options.globalTag	
print "Running with demoMode            = ",options.demoMode
print "Running with useOOTPhotons       = ",options.useOOTPhotons
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
process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
		'root://xrootd-cms.infn.it//store/data/Run2017C/SinglePhoton/MINIAOD/PromptReco-v2/000/300/087/00000/2A8DF124-0977-E711-9E94-02163E0145A7.root'
#		'root://xrootd-cms.infn.it//store/data/Run2017B/SinglePhoton/MINIAOD/PromptReco-v2/000/298/855/00000/56F16894-AA68-E711-8781-02163E01A456.root'
		))

# Set the json locally 
#process.source.lumisToProcess = CfgTypes.untracked(CfgTypes.VLuminosityBlockRange())  
#JSONfile = 'test/hcal_good_gte297467.txt'
#myLumis = LumiList.LumiList(filename = JSONfile).getCMSSWString().split(',')  
#process.source.lumisToProcess.extend(myLumis) 

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

if   options.useOOTPhotons : ootPhotonsTag = cms.InputTag("slimmedOOTPhotons")
else                       : ootPhotonsTag = cms.InputTag("")

# make track collection
#process.load('PhysicsTools.PatAlgos.slimming.unpackedTracksAndVertices_cfi')

from PhysicsTools.PatAlgos.slimming.unpackedTracksAndVertices_cfi import unpackedTracksAndVertices
process.unpackedTracksAndVertices = unpackedTracksAndVertices.clone()

# Make the tree 
process.tree = cms.EDAnalyzer("HLTDump",
   ## general cuts
   phpTmin = cms.double(options.phpTmin),
   jetpTmin = cms.double(options.jetpTmin),
   dRmin = cms.double(options.dRmin),
   pTres = cms.double(options.pTres),
   saveTrigObjs = cms.bool(options.saveTrigObjs),
   trackpTmin = cms.double(options.trackpTmin),
   trackdRmin = cms.double(options.trackdRmin),
   ## triggers
   inputPaths     = cms.string(options.inputPaths),
   inputFilters   = cms.string(options.inputFilters),
   triggerResults = cms.InputTag("TriggerResults", "", "HLT"),
   triggerObjects = cms.InputTag("slimmedPatTrigger"),
   ## vertices
   vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
   ## rho
   rhos = cms.InputTag("fixedGridRhoFastjetAll"), #fixedGridRhoAll
   ## MET
   mets = cms.InputTag("slimmedMETs"),
   ## jets			    	
   jets = cms.InputTag("slimmedJets"),
   ## photons		
   photons    = cms.InputTag("slimmedPhotons"),
   ootPhotons = ootPhotonsTag,
   ## ecal recHits			      
   recHitsEB = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
   recHitsEE = cms.InputTag("reducedEgamma", "reducedEERecHits"),
   ## tracks
   tracks = cms.InputTag("unpackedTracksAndVertices"),
)

# Set up the path
process.treePath = cms.Path(process.unpackedTracksAndVertices + process.tree)
