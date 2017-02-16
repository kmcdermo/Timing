import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## processName
options.register (
	'processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'process name to be considered');

## GT to be used    
options.register (
	'globalTag','80X_mcRun2_asymptotic_2016_TrancheIV_v6',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'gloabl tag to be used');

## do a demo run over only 100k events
options.register (
	'demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to run over only 1k events as a demo');

## outputFile Name
options.register (
	'outputFileName','aodrhcheck.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'output file name created by cmsRun');

## parsing command line arguments
options.parseArguments()

print "################# Settings ##################"
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
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_1.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_2.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_3.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_4.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_5.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_6.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_7.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_8.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_9.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_10.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_11.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_12.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_13.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_14.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_15.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_16.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_17.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_18.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_19.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_AOD-v3/170201_135958/0000/GMSB_L180_Ctau6000_AOD_20.root'
		) 
)

process.options = cms.untracked.PSet( 
	allowUnscheduled = cms.untracked.bool(True),
	wantSummary = cms.untracked.bool(False)
)

# Set the global tag depending on the sample type
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag.globaltag = options.globalTag  

## How many events to process
if not options.demoMode:
	process.maxEvents = cms.untracked.PSet( 
		input = cms.untracked.int32(options.maxEvents))
else:
	process.maxEvents = cms.untracked.PSet( 
		input = cms.untracked.int32(1000))

## Create output file
## Setup the service to make a ROOT TTree
process.TFileService = cms.Service("TFileService", 
		                   fileName = cms.string(options.outputFileName))

# Make the tree 
process.tree = cms.EDAnalyzer("AODRecHitsCheck",
   ##photons
   photons    = cms.InputTag("photons"),
   gedPhotons = cms.InputTag("gedPhotons"),
   ##
   clustersEB = cms.InputTag("hybridSuperClusters","uncleanOnlyHybridSuperClusters"),
   clustersEE = cms.InputTag("multi5x5SuperClusters","multi5x5EndcapBasicClusters"),
   ## ecal recHits			      
   recHitsEB = cms.InputTag("reducedEcalRecHitsEB"),
   recHitsEE = cms.InputTag("reducedEcalRecHitsEE"),
)

# Set up the path
process.treePath = cms.Path(process.tree)
