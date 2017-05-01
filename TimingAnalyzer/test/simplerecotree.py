import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## general cuts
options.register (
	'dataset','deg_2016B',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'dataset to be used');

options.register (
	'reco','rereco',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'dataset to be used');

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
	'globalTag','80X_dataRun2_2016SeptRepro_v4',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'gloabl tag to be used');

## parsing command line arguments
options.parseArguments()

## reset file name
options.outputFileName = 'recophoton-'+options.dataset+'-'+options.reco+'.root'

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
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

## Define the input source
if options.dataset == 'deg_2016B' :
	if options.reco == 'reco':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'file:/afs/cern.ch/work/k/kmcdermo/files/RECO/regular_reco_deg_2016B.root'
				))
	elif options.reco == 'rereco':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'file:/afs/cern.ch/work/k/kmcdermo/files/RECO/rereco_deg_2016B.root'
				))
elif options.dataset == 'sph_2016C' :
	if options.reco == 'reco':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'file:/afs/cern.ch/work/k/kmcdermo/files/RECO/regular_reco_sph_2016C.root'
				))
	elif options.reco == 'rereco':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'file:/afs/cern.ch/work/k/kmcdermo/files/RECO/rereco_sph_2016C.root'
				))
else : exit

## How many events to process
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

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
   ## photons		
   photons = cms.InputTag("gedPhotons"),
   ## ecal recHits			      
   recHitsEB = cms.InputTag("ecalRecHit","EcalRecHitsEB"),
   recHitsEE = cms.InputTag("ecalRecHit","EcalRecHitsEE"),
)

# Set up the path
process.treePath = cms.Path(process.tree)
