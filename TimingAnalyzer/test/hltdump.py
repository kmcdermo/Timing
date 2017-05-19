import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## general cuts
options.register (
	'jetpTmin',15.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,
	'jet pT minimum cut');

options.register (
	'dRmin',0.4,VarParsing.multiplicity.singleton,VarParsing.varType.float,
	'dR minimum cut');

options.register (
	'pTres',0.5,VarParsing.multiplicity.singleton,VarParsing.varType.float,
	'pT resolution cut');

## which trigger menu though??
options.register (
	'triggerPName','HLT',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'process name of trigger menu to consider');

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
	'globalTag','80X_dataRun2_Prompt_v14',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'gloabl tag to be used');

## do a demo run over only 100k events
options.register (
	'demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to run over only 1k events as a demo');

## parsing command line arguments
options.parseArguments()

print "##### Settings ######"
print "Running with jetpTmin            = ",options.jetpTmin
print "Running with dRmin               = ",options.dRmin
print "Running with pTres               = ",options.pTres
print "Running with triggerPName        = ",options.triggerPName
print "Running with processName         = ",options.processName	
print "Running with outputFileName      = ",options.outputFileName	
print "Running with globalTag           = ",options.globalTag	
print "Running with demoMode            = ",options.demoMode
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
if options.triggerPName == 'userHLT':
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_9.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_10.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_11.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_14.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_15.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_24.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_25.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_26.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_27.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_28.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_29.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_30.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_31.root'
#			'root://eoscms.cern.ch//eos/cms/store/group/phys_exotica/displacedPhotons/SinglePhoton/test/reHLT.root'
			))
else : exit

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
process.tree = cms.EDAnalyzer("HLTDump",
   ## general cuts
   jetpTmin = cms.double(options.jetpTmin),
   dRmin = cms.double(options.dRmin),
   pTres = cms.double(options.pTres),
   ## triggers
   inputPaths      = cms.string("test/"+options.triggerPName+"paths.txt"),
   inputFilters    = cms.string("test/"+options.triggerPName+"filters.txt"),
   triggerResults  = cms.InputTag("TriggerResults", "", options.triggerPName),
   triggerEvent    = cms.InputTag("hltTriggerSummaryAOD", "", options.triggerPName),
   ## rho
   rhos = cms.InputTag("fixedGridRhoFastjetAll"), #fixedGridRhoAll
   ## jets			    	
   jets = cms.InputTag("slimmedJets"),
   ## photons		
   photons        = cms.InputTag("slimmedPhotons"),
   ## ecal recHits			      
   recHitsEB = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
   recHitsEE = cms.InputTag("reducedEgamma", "reducedEERecHits"),
)

# Set up the path
process.treePath = cms.Path(process.tree)