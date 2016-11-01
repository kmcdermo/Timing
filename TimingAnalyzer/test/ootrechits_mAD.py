import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## apply deltaR cut in photon rechit analysis
options.register (
	'addrhsInDelR',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to add potentially more recHits to photon within deltaR cone');

options.register (
	'delRcut',0.3,VarParsing.multiplicity.singleton,VarParsing.varType.float,
	'value of deltaR cut');

## processName
options.register (
	'processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'process name to be considered');

## GT to be used    
options.register (
	'globalTag','80X_dataRun2_Prompt_ICHEP16JEC_v0',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'gloabl tag to be used');

## do a demo run over only 100k events
options.register (
	'demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to run over only 100k events as a demo');

## outputFile Name
options.register (
	'outputFileName','rechits.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'output file name created by cmsRun');

## parsing command line arguments
options.parseArguments()

print "################# Settings ##################"
print "Running with addrhsInDelR (delRcut) = ",options.addrhsInDelR," (",options.delRcut,")"
print "Running with processName            = ",options.processName	
print "Running with globalTag              = ",options.globalTag	
print "Running with demoMode               = ",options.demoMode
print "Running with outputFileName         = ",options.outputFileName	
print "#############################################"

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
		) 
)

process.options = cms.untracked.PSet( 
	allowUnscheduled = cms.untracked.bool(True),
	wantSummary = cms.untracked.bool(True))

# Set the global tag depending on the sample type
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag.globaltag = options.globalTag  

## How many events to process
if not options.demoMode:
	process.maxEvents = cms.untracked.PSet( 
		input = cms.untracked.int32(options.maxEvents))
else:
	process.maxEvents = cms.untracked.PSet( 
		input = cms.untracked.int32(100))

# EGM Smearing and VID
from Timing.TimingAnalyzer.PhotonTools_cff import PhotonTools
PhotonTools(process,True)

## Create output file
## Setup the service to make a ROOT TTree
process.TFileService = cms.Service("TFileService", 
		                   fileName = cms.string(options.outputFileName))

# Make the tree 
process.tree = cms.EDAnalyzer("OOTRecHits_mAD",
   ## delta R cut
   addrhsInDelR = cms.bool(options.addrhsInDelR),
   delRcut      = cms.double(options.delRcut),
   ## trigger
   triggerResults = cms.InputTag("TriggerResults", "", "HLT"),
   ## photons
   photons      = cms.InputTag("calibratedPhotons"),
   ## ecal recHits			      
   recHitsReducedEB = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
   recHitsReducedEE = cms.InputTag("reducedEgamma", "reducedEERecHits"),
)

# Set up the path
process.treePath = cms.Path(process.tree)
