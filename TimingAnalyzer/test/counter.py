import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('python')

## matching cuts
options.register('dRmin',0.3,VarParsing.multiplicity.singleton,VarParsing.varType.float,'dR minimum cut');
options.register('pTmin',20.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'pT minimum cut');
options.register('pTres',0.5,VarParsing.multiplicity.singleton,VarParsing.varType.float,'pT resolution cut');

## useGEDVID
options.register('useGEDVID',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to use GED VID in cuts');

## debug it all?
options.register('debug',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to dump info');

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
options.register('outputFileName','counter.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,'output file name created by cmsRun');

## etra bits
options.register('nThreads',8,VarParsing.multiplicity.singleton,VarParsing.varType.int,'number of threads per job');
options.register('deleteEarly',True,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'delete temp products early if not needed');
options.register('runUnscheduled',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'run unscheduled for products');

## parsing command line arguments
options.parseArguments()

print "     ##### Settings ######"
print "        -- Matching --"
print "dRmin          : ",options.dRmin
print "pTmin          : ",options.pTmin
print "pTres          : ",options.pTres
print "        -- GED VID --"
print "useGEDVID      : ",options.useGEDVID
print "         -- Debug --"
print "debug          : ",options.debug
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
		#'/store/user/kmcdermo/files/GMSB_L600TeV_Ctau200cm_miniAODv2.root'

		'root://xrootd-cms.infn.it//store/mc/RunIIFall17MiniAODv2/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/100000/A0C1FB84-F0D8-E811-829F-484D7E8DF0FA.root',
		'root://xrootd-cms.infn.it//store/mc/RunIIFall17MiniAODv2/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/100000/9273BB85-F0D8-E811-9A57-0CC47A706CDE.root',
		'root://xrootd-cms.infn.it//store/mc/RunIIFall17MiniAODv2/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/00000/FA78EC1A-E0BA-E811-9524-F02FA768CCD8.root',
		'root://xrootd-cms.infn.it//store/mc/RunIIFall17MiniAODv2/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/00000/C4CB870D-E0BA-E811-84A3-A0369FC51A44.root',
		'root://xrootd-cms.infn.it//store/mc/RunIIFall17MiniAODv2/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/00000/C0DEB1AC-DFBA-E811-923B-E0071B6C9DF0.root',
		'root://xrootd-cms.infn.it//store/mc/RunIIFall17MiniAODv2/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/00000/A201B427-E0BA-E811-96F6-A0369FD20DA0.root',
		'root://xrootd-cms.infn.it//store/mc/RunIIFall17MiniAODv2/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/00000/9ECFB21D-E0BA-E811-86F0-0242AC1C0500.root',
		'root://xrootd-cms.infn.it//store/mc/RunIIFall17MiniAODv2/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/00000/8CDA606C-E0BA-E811-AE5C-20CF3027A607.root',
		'root://xrootd-cms.infn.it//store/mc/RunIIFall17MiniAODv2/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/00000/82E04C6B-E0BA-E811-8C51-7CD30AD09316.root',
		'root://xrootd-cms.infn.it//store/mc/RunIIFall17MiniAODv2/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/00000/5C24B427-E0BA-E811-832B-A0369FD20DA0.root',
		'root://xrootd-cms.infn.it//store/mc/RunIIFall17MiniAODv2/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/00000/48885419-E0BA-E811-896D-A4BF0108B77A.root',
		'root://xrootd-cms.infn.it//store/mc/RunIIFall17MiniAODv2/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/00000/34374E75-E0BA-E811-8125-002590E7D5AE.root',
		'root://xrootd-cms.infn.it//store/mc/RunIIFall17MiniAODv2/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/00000/0EB82B7E-E0BA-E811-8289-0026B9278610.root',
		'root://xrootd-cms.infn.it//store/mc/RunIIFall17MiniAODv2/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/00000/0CA2A332-E0BA-E811-85BA-002590785950.root',
		'root://xrootd-cms.infn.it//store/mc/RunIIFall17MiniAODv2/GMSB_L-600TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/MINIAODSIM/PU2017_12Apr2018_94X_mc2017_realistic_v14-v1/00000/0068A170-E0BA-E811-952E-0017A4771074.root'
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
	fixEE2017Params = {'userawPt':True, 'ptThreshold':50.0, 'minEtaThreshold':2.65, 'maxEtaThreshold':3.139},
        postfix = "ModifiedMET"
)

## Apply Scale/Smearing + GED and OOT VID to ootPhotons
from RecoEgamma.EgammaTools.OOTPhotonPostRecoTools import setupOOTPhotonPostRecoSeq
setupOOTPhotonPostRecoSeq(process)

# Make the tree 
process.tree = cms.EDAnalyzer("Counter",
   ## matched criteria
   dRmin = cms.double(options.dRmin),
   pTmin = cms.double(options.pTmin),
   pTres = cms.double(options.pTres),
   ## ged vid
   useGEDVID = cms.bool(options.useGEDVID),
   ## debug config
   debug = cms.bool(options.debug),		      
   ## cands
   cands = cms.InputTag("packedPFCandidates"),
   ## MET
   mets = cms.InputTag("slimmedMETsModifiedMET"),
   ## photons
   gedPhotons = cms.InputTag("slimmedPhotons"),
   ootPhotons = cms.InputTag("slimmedOOTPhotons"),
   ## gen info
   isMC         = cms.bool(options.isMC),
   xsec         = cms.double(options.xsec),
   BR           = cms.double(options.BR),
   genEvt       = cms.InputTag("generator"),
   pileups      = cms.InputTag("slimmedAddPileupInfo"),
   genParticles = cms.InputTag("prunedGenParticles"),
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
