import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## data or MC options
options.register (
	'isMC',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate data or MC');

## processName
options.register (
	'processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'process name to be considered');

options.register (
	'miniAODProcess','PAT',VarParsing.multiplicity.singleton,VarParsing.varType.string, #why RECO>???
	'process name used for miniAOD production (target is miniAODv2)');

## outputFile Name
options.register (
	'outputFileName','tree.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'output file name created by cmsRun');

## GT to be used    
options.register (
	'globalTag','80X_dataRun2_Prompt_ICHEP16JEC_v0',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'gloabl tag to be used');

## Skim on events that pass hlt paths and tight electron kinematic requirements
options.register (
	'filterOnHLT',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate if to apply HLT requirements');

options.register (
	'filterOnKinematics',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate if to apply tight electron kinematic requirements');

## nThreads to run
options.register (
	'nThreads',1,VarParsing.multiplicity.singleton, VarParsing.varType.int,
	'default number of threads');

## do a demo run over only 100k events
options.register (
	'demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to run over only 100k events as a demo');

options.register (
	'doZmassSort',True,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to pick electrons based on dilepton mass');

## parsing command line arguments
options.parseArguments()

### check consistentcy of basic options
if options.isMC and 'dataRun2' in options.globalTag:
	options.globalTag = '80X_mcRun2_asymptotic_2016_miniAODv2_v1';

if options.isMC and options.miniAODProcess != 'PAT':
	options.miniAODProcess  = 'PAT'

print "##### Settings ######"
print "Running with isMC                = ",options.isMC	
print "Running with processName         = ",options.processName	
print "Running with miniAODProcess      = ",options.miniAODProcess	
print "Running with outputFileName      = ",options.outputFileName	
print "Running with globalTag           = ",options.globalTag	
print "Running with filterOnHLT         = ",options.filterOnHLT
print "Running with filterOnKinematics  = ",options.filterOnKinematics
print "Running with nThreads            = ",options.nThreads
print "Running with demoMode            = ",options.demoMode
print "Running with doZmassSort         = ",options.doZmassSort
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
if not options.isMC :
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/data/Run2016B/DoubleEG/MINIAOD/PromptReco-v2/000/273/158/00000/308C0A95-E819-E611-9413-02163E01386F.root', #2016B			
			'/store/data/Run2016C/DoubleEG/MINIAOD/PromptReco-v2/000/275/658/00000/36A8FE5F-813B-E611-82A0-02163E014365.root', #2016C
			'/store/data/Run2016D/DoubleEG/MINIAOD/PromptReco-v2/000/276/318/00000/349ADEDB-2145-E611-B8B6-02163E012598.root', #2016D
			'/store/data/Run2016E/DoubleEG/MINIAOD/PromptReco-v2/000/276/831/00000/56C5F0A6-034D-E611-A6F3-02163E0141F1.root', #2016E
			'/store/data/Run2016F/DoubleEG/MINIAOD/PromptReco-v1/000/278/018/00000/2AAB6E8C-885A-E611-829C-02163E0128B6.root', #2016F
			'/store/data/Run2016G/DoubleEG/MINIAOD/PromptReco-v1/000/278/820/00000/CC0E121C-7D64-E611-AA12-FA163ED4D6E2.root', #2016G
			'/store/data/Run2016H/DoubleEG/MINIAOD/PromptReco-v2/000/281/693/00000/04D532E5-7786-E611-B126-02163E014682.root'  #2016H
			) 
				    )
else:
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/mc/RunIISpring16MiniAODv2/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/MINIAODSIM/PUSpring16RAWAODSIM_reHLT_80X_mcRun2_asymptotic_v14-v1/40000/00A42394-FA5C-E611-9E01-0025905C96E8.root',
			'/store/mc/RunIISpring16MiniAODv2/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/MINIAODSIM/PUSpring16RAWAODSIM_reHLT_80X_mcRun2_asymptotic_v14-v1/40000/0C109D17-0D5D-E611-9D5B-A0000420FE80.root',
			'/store/mc/RunIISpring16MiniAODv2/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/MINIAODSIM/PUSpring16RAWAODSIM_reHLT_80X_mcRun2_asymptotic_v14-v1/40000/1686540B-055D-E611-AC79-0002C94CD0D8.root'
			)    	
)

## Set the process options -- Display summary at the end, enable unscheduled execution
if options.nThreads == 1 or options.nThreads == 0:
	process.options = cms.untracked.PSet( 
		allowUnscheduled = cms.untracked.bool(True),
		wantSummary = cms.untracked.bool(True))
else:
	process.options = cms.untracked.PSet( 
		allowUnscheduled = cms.untracked.bool(True),
		wantSummary = cms.untracked.bool(True),
		numberOfThreads = cms.untracked.uint32(options.nThreads),
		numberOfStreams = cms.untracked.uint32(options.nThreads))

## How many events to process
if not options.demoMode:
	process.maxEvents = cms.untracked.PSet( 
		input = cms.untracked.int32(options.maxEvents))
else:
	process.maxEvents = cms.untracked.PSet( 
		input = cms.untracked.int32(100000))

# Set the global tag depending on the sample type
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag.globaltag = options.globalTag  

# run cut-based electron ID https://twiki.cern.ch/twiki/bin/viewauth/CMS/CutBasedElectronIdentificationRun2
from Timing.TimingAnalyzer.ElectronTools_cff import ElectronTools
ElectronTools(process,options.isMC)

# Create a set of objects to read from
process.selectedObjects = cms.EDProducer("PFCleaner",
     electrons        = cms.InputTag("calibratedElectrons"), 
     electronidveto   = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-veto"),
     electronidloose  = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-loose"),
     electronidmedium = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-medium"),
     electronidtight  = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Summer16-80X-V1-tight"),
     electronidheep   = cms.InputTag("egmGsfElectronIDs:heepElectronID-HEEPV60"),
)

## Create output file
## Setup the service to make a ROOT TTree
process.TFileService = cms.Service("TFileService", 
		                   fileName = cms.string(options.outputFileName))

# Make the tree 
process.tree = cms.EDAnalyzer("TimingAnalyzer",
   ## gen info			     
   isMC       = cms.bool(options.isMC),
   pileup     = cms.InputTag("slimmedAddPileupInfo"),
   genevt     = cms.InputTag("generator"),
   gens       = cms.InputTag("prunedGenParticles"),
   ## trigger info + apply filter?
   triggerResults = cms.InputTag("TriggerResults", "", "HLT"),
   applyHLTFilter = cms.bool(options.filterOnHLT),
   ## vertexes			    	
   vertices  = cms.InputTag("offlineSlimmedPrimaryVertices"),
   ## electrons
   vetoelectrons   = cms.InputTag("selectedObjects", "vetoelectrons"),
   looseelectrons  = cms.InputTag("selectedObjects", "looseelectrons"),
   mediumelectrons = cms.InputTag("selectedObjects", "mediumelectrons"),
   tightelectrons  = cms.InputTag("selectedObjects", "tightelectrons"),
   heepelectrons   = cms.InputTag("selectedObjects", "heepelectrons"),
   applyKinematicsFilter = cms.bool(options.filterOnKinematics),
   ## ecal recHits			      
   recHitCollectionEB = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
   recHitCollectionEE = cms.InputTag("reducedEgamma", "reducedEERecHits"),
   ## turn on dilepton selection based on mass
   doZmassSort = cms.bool(options.doZmassSort)
)

# Set up the path
process.treePath = cms.Path(process.tree)
