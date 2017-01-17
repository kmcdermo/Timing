import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## processName
options.register (
	'processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'process name to be considered');

## do a demo run over only 1k events
options.register (
	'demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to run over only 1k events as a demo');

## GT to be used    
options.register (
	'globalTag','80X_mcRun2_asymptotic_2016_TrancheIV_v6',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'gloabl tag to be used');

## outputFile Name
options.register (
	'outputFileName','photondump.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'output file name created by cmsRun');

## parsing command line arguments
options.parseArguments()

print "##### Settings ######"
print "Running with processName         = ",options.processName	
print "Running with demoMode            = ",options.demoMode
print "Running with globalTag           = ",options.globalTag	
print "Running with outputFileName      = ",options.outputFileName	
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
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_1.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_2.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_3.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_4.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_5.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_6.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_7.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_8.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_9.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_10.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_11.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_12.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_13.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_14.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_15.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_16.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_17.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_18.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_19.root',
'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_ReRECO_PAT-v1/161223_141459/0000/GMSB_L180_Ctau6000_ReRECO_PAT_MINIAOD_20.root'
))

## How many events to process
if not options.demoMode:
	process.maxEvents = cms.untracked.PSet( 
		input = cms.untracked.int32(options.maxEvents))
else:
	process.maxEvents = cms.untracked.PSet( 
		input = cms.untracked.int32(1000))

# Set the global tag depending on the sample type
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag.globaltag = options.globalTag  

# EGM Smearing and VID
from Timing.TimingAnalyzer.PhotonTools_cff import PhotonTools
PhotonTools(process,options.isMC)

## Create output file
## Setup the service to make a ROOT TTree
process.TFileService = cms.Service("TFileService", 
		                   fileName = cms.string(options.outputFileName))

# Make the tree 
process.tree = cms.EDAnalyzer("GenPhDump",
   ## gen info
   prunedGenParticles = cms.InputTag("prunedGenParticles"),
   packedGenParticles = cms.InputTag("packedGenParticles"),
   ## photons		
   loosePhotonID      = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring15-25ns-V1-standalone-loose"),
   mediumPhotonID     = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring15-25ns-V1-standalone-medium"),
   tightPhotonID      = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring15-25ns-V1-standalone-tight"),
   photons            = cms.InputTag("calibratedPhotons")
)

# Set up the path
process.treePath = cms.Path(process.selectedPhotons * process.calibratedPhotons * process.egmPhotonIDs * process.tree)
