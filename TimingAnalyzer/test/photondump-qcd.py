import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

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
	'isBkg',True,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate Background MC');

options.register (
	'HTbin','40To100',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'which GMSB sample to use');

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
options.outputFileName = 'photondump-QCD_HT'+options.HTbin+'.root'

print "##### Settings ######"
print "Running with dumpRHs             = ",options.dumpRHs
print "Running with dumpIds             = ",options.dumpIds
print "Running with HTbin               = ",options.HTbin
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
if options.HTbin == '50To100' :
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT50to100_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/064038FF-B0B6-E611-B114-D48564593FAC.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT50to100_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/128D78DF-AFB6-E611-BB25-14187764197C.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT50to100_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/1A791B49-A5B6-E611-9984-00259073E36C.root'
			))
elif options.HTbin == '100To200' :
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT100to200_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/120000/00DD3241-3ABE-E611-85DE-0CC47A78A360.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT100to200_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/120000/00FF03B2-EFBD-E611-878D-C4346BC7EE18.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT100to200_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/120000/025CD9F6-2FBE-E611-ADE4-0025905B8562.root'
			))
elif options.HTbin == '200To300' :
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT200to300_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6_ext1-v1/60000/004F259D-BBBD-E611-9206-0025905B85D6.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT200to300_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6_ext1-v1/60000/0217C608-AEBD-E611-A355-0025905A6138.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT200to300_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6_ext1-v1/60000/021D9B9D-BBBD-E611-ADE6-0025905A608C.root'
			))
elif options.HTbin == '300To500' :
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT300to500_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/04B528C6-7FBD-E611-AD61-3417EBE6446E.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT300to500_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/08C83E28-82BD-E611-8F06-0CC47A5FA215.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT300to500_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/0C27A786-99BD-E611-BA36-0CC47A537688.root'
			))
elif options.HTbin == '500To700' :
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring(
 			'/store/mc/RunIISummer16MiniAODv2/QCD_HT500to700_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/06A28FC9-A2BE-E611-BA9F-0CC47A4C8E82.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT500to700_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/000316AF-9FBE-E611-9761-0CC47A7C35F8.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT500to700_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/70000/02DF45E6-8CBE-E611-9636-0CC47A4C8E26.root'
			))
elif options.HTbin == '700To1000' :
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT700to1000_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/50000/00D17FD4-8EBD-E611-B17D-002590D0AFC2.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT700to1000_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/50000/0258B60A-8FBD-E611-8B0B-24BE05C6C7F1.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT700to1000_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/50000/0413591E-99BD-E611-B281-0CC47A4C8ECA.root'
			))
elif options.HTbin == '1000To1500' :
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT1000to1500_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6_ext1-v1/60000/065FD658-03B7-E611-B1C3-A0000420FE80.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT1000to1500_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6_ext1-v1/60000/085B2737-0DB7-E611-A854-BC305B390AA7.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT1000to1500_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6_ext1-v1/60000/10233A22-08B7-E611-B585-A0369F5BD91C.root'
			))
elif options.HTbin == '1500To2000' :
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT1500to2000_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/80000/1C1FD02A-21BB-E611-8071-6C3BE5B50180.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT1500to2000_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/80000/0809E9C0-23BB-E611-A10E-6C3BE5B5B078.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT1500to2000_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/80000/1CCA66D8-2CBB-E611-B0BE-0CC47A4D76C8.root'
			))
elif options.HTbin == '2000ToInf' :
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT2000toInf_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/120000/02FBE5E0-1AB6-E611-85B1-0CC47A7DFF82.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT2000toInf_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/120000/1E2D6481-0DB6-E611-A45C-0CC47A7E6BDE.root',
			'/store/mc/RunIISummer16MiniAODv2/QCD_HT2000toInf_TuneCUETP8M1_13TeV-madgraphMLM-pythia8/MINIAODSIM/PUMoriond17_80X_mcRun2_asymptotic_2016_TrancheIV_v6-v1/120000/3CE3DFA3-0EB6-E611-B917-0CC47A7FC44E.root'
			))
else : exit

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
   isBkg      = cms.bool(options.isBkg),
   dumpIds    = cms.bool(options.dumpIds),
   genevt     = cms.InputTag("generator"),
   pileup     = cms.InputTag("slimmedAddPileupInfo"),
   genparts   = cms.InputTag("prunedGenParticles"),
   genjets    = cms.InputTag("slimmedGenJets"),
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
