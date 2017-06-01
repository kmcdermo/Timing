import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## general cuts
options.register (
	'dataset','sph_2016H',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'dataset to be used');

options.register (
	'reco','OOT',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'dataset to be used');

## processName
options.register (
	'processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'process name to be considered');

## outputFile Name
options.register (
	'outputFileName','patphoton.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'output file name created by cmsRun');

## GT to be used    
options.register (
	'globalTag','91X_dataRun2_PromptLike_v4',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'gloabl tag to be used');

## parsing command line arguments
options.parseArguments()

## reset file name
options.outputFileName = 'patphoton-'+options.dataset+'-'+options.reco+'.root'

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
if options.dataset == 'sph_2016H' :
	if options.reco == 'OOT':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/OOT/oot_miniaod.root'
				))
	elif options.reco == 'OOT_NoHoE':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/OOT_NoHoE/oot_nohoe_miniaod.root'
				))
	elif options.reco == 'vanilla':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/vanilla/prompt_miniaod.root'
				))
	elif options.reco == 'gedPho':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/OOT/oot_miniaod.root'
				))
	elif options.reco == 'OOT_sep':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/OOT/oot_miniaod_1.root',
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/OOT/oot_miniaod_2.root',
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/OOT/oot_miniaod_3.root',
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/OOT/oot_miniaod_4.root',
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/OOT/oot_miniaod_5.root',
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/OOT/oot_miniaod_6.root',
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/OOT/oot_miniaod_7.root',
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/OOT/oot_miniaod_8.root',
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/OOT/oot_miniaod_9.root',
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/OOT/oot_miniaod_10.root',
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/OOT/oot_miniaod_11.root',
				'file:/afs/cern.ch/work/k/kmcdermo/public/files/MINIAOD/OOT/oot_miniaod_12.root',
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

if 'OOT' in options.reco :
	photonsTag = cms.InputTag("slimmedOOTPhotons","","PAT")
else :
	photonsTag = cms.InputTag("slimmedPhotons","","PAT")

# Make the tree 
process.tree = cms.EDAnalyzer("SimplePATTree",
   ## rho
   rhos = cms.InputTag("fixedGridRhoFastjetAll"), #fixedGridRhoAll
   ## photons		
   photons = photonsTag,
   ## ecal recHits			      
   recHitsEB = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
   recHitsEE = cms.InputTag("reducedEgamma", "reducedEERecHits"),
)

# Set up the path
process.treePath = cms.Path(process.tree)
