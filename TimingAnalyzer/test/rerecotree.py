import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## which analysis
options.register (
	'doPhRhs',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to run photon rec hits analysis');

## apply deltaR cut in photon rechit analysis
options.register (
	'addrhsInDelR',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to add potentially more recHits to photon within deltaR cone'

options.register (
	'delRcut',0.3,VarParsing.multiplicity.singleton,VarParsing.varType.float,
	'value of deltaR cut');

options.register (
	'doCount',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to run rec hit counting between different collections analysis');

## apply rhE cut in couting analysis
options.register (
	'applyrhEcut',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to apply energy cut on outer loop over rechits');

options.register (
	'rhEcut',1.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,
	'value of recHit energy cut [GeV]');

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
	'outputFileName','rerecotree.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'output file name created by cmsRun');

## parsing command line arguments
options.parseArguments()

print "################# Settings ##################"
print "Running with doPhRhs                = ",options.doPhRhs	
if options.doPhRhs:
	print "Running with addrhsInDelR (delRcut) = ",options.addrhsInDelR," (",options.delRcut,")"
print "Running with doCount                = ",options.doCount
if options.doCount:
	print "Running with applyrhEcut (rhEcut)   = ",options.applyrhEcut," (",options.rhEcut,")"
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
		'file:/afs/cern.ch/work/k/kmcdermo/private/dispho/CMSSW_8_0_20/src/Timing/TimingAnalyzer/tmp_rereco/rerecoOOT0.root',
		'file:/afs/cern.ch/work/k/kmcdermo/private/dispho/CMSSW_8_0_20/src/Timing/TimingAnalyzer/tmp_rereco/rerecoOOT1.root'
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

## Create output file
## Setup the service to make a ROOT TTree
process.TFileService = cms.Service("TFileService", 
		                   fileName = cms.string(options.outputFileName))

# Make the tree 
process.tree = cms.EDAnalyzer("OOTRecHits",
   ## analysis bools
   doPhRhs      = cms.bool(options.doPhRhs),
   doCount      = cms.bool(options.doCount),
   ## delta R cut
   addrhsInDelR = cms.bool(options.addrhsInDelR),
   delRcut      = cms.double(options.delRcut),
   ## rh energy cut
   applyrhEcut  = cms.bool(options.applyrhEcut),
   rhEcut       = cms.double(options.rhEcut),
   ## photons
   photons      = cms.InputTag("gedPhotons"),
   ## ecal recHits			      
   recHitsReducedEB = cms.InputTag("reducedEcalRecHitsEB"),
   recHitsReducedEE = cms.InputTag("reducedEcalRecHitsEE"),
   recHitsFullEB = cms.InputTag("ecalRecHit","EcalRecHitsEB"),
   recHitsFullEE = cms.InputTag("ecalRecHit","EcalRecHitsEE")
)

# Set up the path
process.treePath = cms.Path(process.tree)
