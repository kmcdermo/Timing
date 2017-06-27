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
	'outputFileName','patphoton.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'output file name created by cmsRun');

## parsing command line arguments
options.parseArguments()

## reset file name
options.outputFileName = 'patphoton-legacy.root'

print "##### Settings ######"
print "Running with processName     = ",options.processName	
print "Running with outputFileName  = ",options.outputFileName	
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
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_1.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_2.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_3.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_4.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_5.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_6.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_7.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_8.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_9.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_10.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_11.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_12.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_13.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_14.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_15.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_16.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_17.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_18.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_19.root',
		'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_20.root'
		))

## How many events to process
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

# Set the global tag depending on the sample type
from Configuration.AlCa.GlobalTag import GlobalTag
from Configuration.AlCa.autoCond import autoCond
process.GlobalTag.globaltag = autoCond['run2_mc']

## Create output file
## Setup the service to make a ROOT TTree
process.TFileService = cms.Service("TFileService", 
		                   fileName = cms.string(options.outputFileName))

# Make the tree 
process.tree = cms.EDAnalyzer("SimplePATTree",
   ## photons		
   photons    = cms.InputTag("slimmedPhotons"   ,"","PAT"),
   ootphotons = cms.InputTag("slimmedOOTPhotons","","PAT"),
   ## ecal recHits			      
   recHitsEB = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
   recHitsEE = cms.InputTag("reducedEgamma", "reducedEERecHits"),
)

# Set up the path
process.treePath = cms.Path(process.tree)
