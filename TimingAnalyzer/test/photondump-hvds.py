import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## dump trigger menu 
options.register (
	'dumpTriggerMenu',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to dump trigger menu');

## dump rec hit info
options.register (
	'dumpRHs',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to dump rec hits info');

## data or MC options
options.register (
	'isMC',True,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate data or MC');

options.register (
	'isGMSB',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate GMSB');

options.register (
	'isHVDS',True,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate Hidden Valley Dark Shower');

options.register (
	'isBkg',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate Background MC');

options.register (
	'dumpIds',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to dump gen particles IDs');

## processName
options.register (
	'processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'process name to be considered');

## outputFile Name
options.register (
	'outputFileName','photondump.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'output file name created by cmsRun');

## GT to be used    
options.register (
	'globalTag','80X_mcRun2_asymptotic_2016_TrancheIV_v6',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'gloabl tag to be used');

## do a demo run over only 100k events
options.register (
	'demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to run over only 100k events as a demo');

## parsing command line arguments
options.parseArguments()

## reset file name
options.outputFileName = 'photondump-hvds.root'

print "##### Settings ######"
print "Running with dumpTriggerMenu     = ",options.dumpTriggerMenu
print "Running with dumpRHs             = ",options.dumpRHs
print "Running with dumpIds             = ",options.dumpIds
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
process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_1.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_2.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_3.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_4.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_5.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_6.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_7.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_8.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_9.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_10.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_11.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_12.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_13.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_14.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_15.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_16.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_17.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_18.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_19.root',
		'/store/user/kmcdermo/HVDS_13TeV_GEN_SIM/HVDS_PAT-v1/170217_125813/0000/HVDS_PAT_20.root'
		))

## How many events to process
if   options.demoMode : process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1000))
else                  : process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))

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
process.tree = cms.EDAnalyzer("PhotonDump",
   ## gen info			     
   isGMSB     = cms.bool(options.isGMSB),
   isHVDS     = cms.bool(options.isHVDS),
   isBkg      = cms.bool(options.isBkg),
   dumpIds    = cms.bool(options.dumpIds),
   genevt     = cms.InputTag("generator"),
   pileup     = cms.InputTag("slimmedAddPileupInfo"),
   genparts   = cms.InputTag("prunedGenParticles"),
   genjets    = cms.InputTag("slimmedGenJets"),
   ## triggers
   dumpTriggerMenu = cms.bool(options.dumpTriggerMenu),
   triggerResults  = cms.InputTag("TriggerResults", "", "HLT"),
   ## vertices
   vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
   ## rho
   rhos = cms.InputTag("fixedGridRhoFastjetAll"), #fixedGridRhoAll
   ## MET
   mets = cms.InputTag("slimmedMETs"),
   ## jets			    	
   jets = cms.InputTag("slimmedJets"),
   ## photons		
   loosePhotonID  = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring16-V2p2-loose"),
   mediumPhotonID = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring16-V2p2-medium"),
   tightPhotonID  = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring16-V2p2-tight"),
   photons        = cms.InputTag("calibratedPhotons"),
   ## ecal recHits			      
   dumpRHs            = cms.bool(options.dumpRHs),
   recHitCollectionEB = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
   recHitCollectionEE = cms.InputTag("reducedEgamma", "reducedEERecHits")
)

# Set up the path
process.treePath = cms.Path(process.selectedPhotons * process.calibratedPhotons * process.egmPhotonIDs * process.tree)
