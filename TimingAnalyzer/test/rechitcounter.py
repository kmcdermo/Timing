import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('python')

## matching cuts
options.register('dRmin',0.3,VarParsing.multiplicity.singleton,VarParsing.varType.float,'dR minimum cut');
options.register('phpTmin',10.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'photon pT minimum cut');
options.register('phIDmin','none',VarParsing.multiplicity.singleton,VarParsing.varType.string,'photon ID minimum cut');

## data or MC options
options.register('isMC',True,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to indicate data or MC');

## GT to be used
options.register('globalTag','94X_mc2017_realistic_v17',VarParsing.multiplicity.singleton,VarParsing.varType.string,'gloabl tag to be used');

## do a demo run over only 1k events
options.register('demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to run over only 1k events');

## input list of files
options.register('inputFileNames','',VarParsing.multiplicity.singleton,VarParsing.varType.string,'input list of files to process');

## processName
options.register('processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,'process name to be considered');

## outputFile Name
options.register('outputFileName','recHitCounter.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,'output file name created by cmsRun');

## etra bits
options.register('deleteEarly',True,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'delete temp products early if not needed');
options.register('runUnscheduled',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'run unscheduled for products');

## parsing command line arguments
options.parseArguments()

## set globalTag
if options.isMC : options.globalTag = "94X_mc2017_realistic_v17"
else            : options.globalTag = "94X_dataRun2_v11"

print "     ##### Settings ######"
print "        -- Matching --"
print "dRmin          : ",options.dRmin
print "phpTmin        : ",options.phpTmin
print "phIDmin        : ",options.phIDmin
print "        -- MC Info --"
print "isMC           : ",options.isMC
print "           -- GT --"
print "globalTag      : ",options.globalTag	
print "         -- Input --"
print "inputFileNames : ",options.inputFileNames
print "         -- Output --"
print "demoMode       : ",options.demoMode
print "processName    : ",options.processName	
print "outputFileName : ",options.outputFileName	
print "        -- Extra bits --"
print "runUnscheduled : ",options.runUnscheduled
print "deleteEarly    : ",options.deleteEarly
print "     #####################"

## Define the CMSSW process
process = cms.Process(options.processName)

## Load the standard set of configuration modules
process.load("Configuration.Geometry.GeometryRecoDB_cff")
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
process.load("Geometry.CaloEventSetup.CaloTowerConstituents_cfi")

## Message Logger settings
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.destinations = ['cout', 'cerr']
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

## Define the input source
import FWCore.Utilities.FileUtils as FileUtils
if options.inputFileNames == "" :
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			# reminiaod data: 94X_dataRun2_v11
			#'/store/user/kmcdermo/files/miniAOD/SPH_2017E_miniAODv2.root'
			# reminiaod GJets, GT: 94X_mc2017_realistic_v17
			#'/store/user/kmcdermo/files/miniAOD/GJets_600toInf_miniAODv2.root'
			# miniaodv2 GMSB, GT: 94X_mc2017_realistic_v17
			'/store/user/kmcdermo/files/miniAOD/GMSB_L200TeV_CTau200cm_miniAODv2.root'
			))
else :
	fileList = FileUtils.loadListFromFile(options.inputFileNames)
	readFiles = cms.untracked.vstring( *fileList)
	process.source = cms.Source('PoolSource', fileNames = readFiles)

## How many events to process
if   options.demoMode : process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(500000))
else                  : process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(options.maxEvents))

# Set the global tag depending on the sample type
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag.globaltag = options.globalTag  

## Create output file
## Setup the service to make a ROOT TTree
process.TFileService = cms.Service("TFileService", 
		                   fileName = cms.string(options.outputFileName))

## Apply Scale/Smearing + GED and OOT VID to ootPhotons
from RecoEgamma.EgammaTools.OOTPhotonPostRecoTools import setupOOTPhotonPostRecoSeq
setupOOTPhotonPostRecoSeq(process)

# Make the tree 
process.tree = cms.EDAnalyzer("RecHitCounter",
   ## matched criteria
   dRmin = cms.double(options.dRmin),
   phpTmin = cms.double(options.phpTmin),
   phIDmin = cms.string(options.phIDmin),
   ## rho
   rho = cms.InputTag("fixedGridRhoFastjetAll"),
   ## MET
   mets = cms.InputTag("slimmedMETs"),
   ## photons
   gedPhotons = cms.InputTag("slimmedPhotons"),
   ootPhotons = cms.InputTag("slimmedOOTPhotons"),
   ## ecal recHits
   recHitsEB = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
   recHitsEE = cms.InputTag("reducedEgamma", "reducedEERecHits"),
)

# Set up the path
process.treePath = cms.Path(
	process.ootPhotonPostRecoSeq +
	process.tree
)

### Extra bits from other configs
process.options = cms.untracked.PSet()

from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
if options.deleteEarly:
	process = customiseEarlyDelete(process)

from FWCore.ParameterSet.Utilities import convertToUnscheduled
if options.runUnscheduled : 
	process = convertToUnscheduled(process)
