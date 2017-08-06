import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## general cuts
options.register (
	'dRmin',0.4,VarParsing.multiplicity.singleton,VarParsing.varType.float,
	'dR minimum cut');

options.register (
	'pTres',0.5,VarParsing.multiplicity.singleton,VarParsing.varType.float,
	'pT resolution cut');

## dump trigger menu 
options.register (
	'dumpTriggerMenu',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to dump trigger menu');

## which trigger menu though??
options.register (
	'triggerPName','HLT',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'process name of trigger menu to consider');

## dump rec hit info
options.register (
	'dumpRHs',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to dump rec hits info');

## data or MC options
options.register (
	'isMC',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate data or MC');

options.register (
	'isGMSB',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate GMSB');

options.register (
	'isHVDS',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate HVDS');

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
	'outputFileName','photondump-2016H.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'output file name created by cmsRun');

## GT to be used    
options.register (
	'globalTag','80X_dataRun2_Prompt_v14',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'gloabl tag to be used');

## do a demo run over only 100k events
options.register (
	'demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to run over only 1k events as a demo');

## parsing command line arguments
options.parseArguments()

## reset file name

print "##### Settings ######"
print "Running with dRmin               = ",options.dRmin
print "Running with pTres               = ",options.pTres
print "Running with dumpTriggerMenu     = ",options.dumpTriggerMenu
print "Running with triggerPName        = ",options.triggerPName
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
if options.triggerPName == 'userHLT':
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_9.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_10.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_11.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_14.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_15.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_24.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_25.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_26.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_27.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_28.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_29.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_30.root',
			'/store/group/phys_exotica/displacedPhotons/SinglePhoton/crab_SinglePhoton_2016H_reHLT-v3/170508_140412/0000/reHLT_31.root'
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
   ## general cuts
   dRmin = cms.double(options.dRmin),
   pTres = cms.double(options.pTres),
   ## triggers
   dumpTriggerMenu = cms.bool(options.dumpTriggerMenu),
   inputPaths      = cms.string("test/"+options.triggerPName+"paths.txt"),
   inputFilters    = cms.string("test/"+options.triggerPName+"filters.txt"),
   triggerResults  = cms.InputTag("TriggerResults", "", options.triggerPName),
   triggerEvent    = cms.InputTag("hltTriggerSummaryAOD", "", options.triggerPName),
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
   dumpRHs   = cms.bool(options.dumpRHs),
   recHitsEB = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
   recHitsEE = cms.InputTag("reducedEgamma", "reducedEERecHits"),
   ## gen info			     
   isGMSB   = cms.bool(options.isGMSB),
   isHVDS   = cms.bool(options.isHVDS),
   isBkg    = cms.bool(options.isBkg),
   dumpIds  = cms.bool(options.dumpIds),
   genevt   = cms.InputTag("generator"),
   pileup   = cms.InputTag("slimmedAddPileupInfo"),
   genparts = cms.InputTag("prunedGenParticles"),
   genjets  = cms.InputTag("slimmedGenJets")
)

# Set up the path
process.treePath = cms.Path(process.selectedPhotons * process.calibratedPhotons * process.egmPhotonIDs * process.tree)
