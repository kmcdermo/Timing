import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('python')

## matching cuts
options.register('dRmin',0.3,VarParsing.multiplicity.singleton,VarParsing.varType.float,'dR minimum cut');
options.register('pTmin',20.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'pT minimum cut');
options.register('pTres',1.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'pT resolution cut');

## data or MC options
options.register('isMC',True,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to indicate data or MC');
options.register('xsec',1.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'cross section in pb');
options.register('BR',1.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'branching ratio of MC');

## GT to be used
options.register('globalTag','94X_mc2017_realistic_v14',VarParsing.multiplicity.singleton,VarParsing.varType.string,'gloabl tag to be used');

## do a demo run over only 1k events
options.register('demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to run over only 1k events');

## processName
options.register('processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,'process name to be considered');

## outputFile Name
options.register('outputFileName','dispho.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,'output file name created by cmsRun');

## etra bits
options.register('nThreads',8,VarParsing.multiplicity.singleton,VarParsing.varType.int,'number of threads per job');
options.register('deleteEarly',True,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'delete temp products early if not needed');
options.register('runUnscheduled',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'run unscheduled for products');

## parsing command line arguments
options.parseArguments()

if options.isGMSB or options.isHVDS or options.isBkgd or options.isToy or options.isADD: options.isMC = True

print "     ##### Settings ######"
print "        -- Matching --"
print "dRmin          : ",options.dRmin
print "pTmin          : ",options.pTmin
print "pTres          : ",options.pTres
print "        -- MC Info --"
print "isMC           : ",options.isMC
if options.isMC:
	print "xsec           : ",options.xsec
	print "BR             : ",options.BR
print "           -- GT --"
print "globalTag      : ",options.globalTag	
print "         -- Output --"
print "demoMode       : ",options.demoMode
print "processName    : ",options.processName	
print "outputFileName : ",options.outputFileName	
print "        -- Extra bits --"
print "nThreads       : ",options.nThreads
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
process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
		# reminiaod data: 94X_dataRun2_v6
		#'/store/user/kmcdermo/files/SPH_2017E_miniAODv2.root'
		# miniaodv2 GMSB, GT: 94X_mc2017_realistic_v14
		'/store/user/kmcdermo/files/GMSB_L600TeV_Ctau400cm_miniAODv2.root'
		))

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

## MET corrections for 2017 data: https://twiki.cern.ch/twiki/bin/view/CMS/MissingETUncertaintyPrescription#Instructions_for_9_4_X_X_9_for_2
from PhysicsTools.PatUtils.tools.runMETCorrectionsAndUncertainties import runMetCorAndUncFromMiniAOD
runMetCorAndUncFromMiniAOD (
        process,
        isData = not options.isMC,
        fixEE2017 = True,
	fixEE2017Params = {'userawPt':True, 'PtThreshold':50.0, 'MinEtaThreshold':2.65, 'MaxEtaThreshold':3.139},
        postfix = "ModifiedMET"
)

## Apply Scale/Smearing + GED and OOT VID to ootPhotons
from RecoEgamma.EgammaTools.OOTPhotonPostRecoTools import setupOOTPhotonPostRecoSeq
setupOOTPhotonPostRecoSeq(process)

# Make the tree 
process.tree = cms.EDAnalyzer("DisPho",
   ## matched criteria
   dRmin = cms.double(options.dRmin),
   pTmin = cms.double(options.pTmin),
   pTres = cms.double(options.pTres),
   ## cands
   cands = cms.InputTag("packedPFCandidates"),
   ## MET
   mets = cms.InputTag("slimmedMETsModifiedMET"),
   ## photons
   gedPhotons = cms.InputTag("slimmedPhotons"),
   ootPhotons = cms.InputTag("slimmedOOTPhotons"),
   ## gen info
   isMC   = cms.bool(options.isMC),
   xsec     = cms.double(options.xsec),
   BR       = cms.double(options.BR),
   genevt   = cms.InputTag("generator"),
   pileup   = cms.InputTag("slimmedAddPileupInfo"),
   genparts = cms.InputTag("prunedGenParticles"),
)

# Set up the path
process.treePath = cms.Path(
	process.fullPatMetSequenceModifiedMET +
	process.ootPhotonPostRecoSeq +
	process.tree
)

### Extra bits from other configs
process.options = cms.untracked.PSet(
	numberOfThreads=cms.untracked.uint32(options.nThreads),
	numberOfStreams=cms.untracked.uint32(options.nThreads/2)
)

from FWCore.ParameterSet.Utilities import convertToUnscheduled
if options.runUnscheduled : 
	process = convertToUnscheduled(process)

from Configuration.StandardSequences.earlyDeleteSettings_cff import customiseEarlyDelete
if options.deleteEarly:
	process = customiseEarlyDelete(process)
