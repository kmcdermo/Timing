import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## processName
options.register (
	'processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'process name to be considered');

## outputFile Name
options.register (
	'outputFileName','recophoton.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'output file name created by cmsRun');

## GT to be used    
options.register (
	'globalTag','92X_dataRun2_Prompt_v4',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'gloabl tag to be used');

## parsing command line arguments
options.parseArguments()

print "##### Settings ######"
print "Running with processName     = ",options.processName	
print "Running with outputFileName  = ",options.outputFileName	
print "Running with globalTag       = ",options.globalTag	
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
process.MessageLogger.cerr.FwkReport.reportEvery = 100

## Define the input source
process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
		'/store/data/Run2017B/SinglePhoton/AOD/PromptReco-v1/000/297/046/00000/1CAEB625-3356-E711-AA79-02163E019DB0.root'
		))

## How many events to process
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1000))

# Set the global tag depending on the sample type
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag.globaltag = options.globalTag  

## Create output file
## Setup the service to make a ROOT TTree
process.TFileService = cms.Service("TFileService", 
		                   fileName = cms.string(options.outputFileName))

# Make the tree 
process.tree = cms.EDAnalyzer("SimpleRECOTree",
   ## rho
   rhos = cms.InputTag("fixedGridRhoFastjetAll"), #fixedGridRhoAll
   ## prompt photons		
   photons = cms.InputTag("gedPhotons","","RECO"),
   ecalIso = cms.InputTag("photonEcalPFClusterIsolationProducer","","RECO"),
   hcalIso = cms.InputTag("photonHcalPFClusterIsolationProducer","","RECO"),
   ## out-of-time photons		
   ootphotons = cms.InputTag("ootPhotons","","RECO"),
   ootecalIso = cms.InputTag("ootPhotonEcalPFClusterIsolationProducer","","RECO"),
   oothcalIso = cms.InputTag("ootPhotonHcalPFClusterIsolationProducer","","RECO"),
   ## ecal recHits			      
   recHitsEB = cms.InputTag("reducedEcalRecHitsEB","","RECO"),
   recHitsEE = cms.InputTag("reducedEcalRecHitsEE","","RECO"),
)

# Set up the path
process.treePath = cms.Path(process.tree)
