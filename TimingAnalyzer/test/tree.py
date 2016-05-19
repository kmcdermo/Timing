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
	'globalTag','76X_dataRun2_16Dec2015_v0',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'gloabl tag to be used');

## Skim on events that pass hlt paths
options.register (
	'filterOnHLT',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate if apply or not trigger requirements');

## Dump Gen Level info
options.register(
	'addGenParticles',False,VarParsing.multiplicity.singleton, VarParsing.varType.bool,
	'dump gen info for Z,leptons');

options.register(
	'isSignalSample',False,VarParsing.multiplicity.singleton, VarParsing.varType.bool,
	'dump signal sample info');

## input cross section in case you want to store a different value wrt to the LHE file
options.register(
	'crossSection',-1.,VarParsing.multiplicity.singleton, VarParsing.varType.float,
	'external value for sample cross section, in case of data it is fixed to 0.001');

## nThreads to run
options.register (
	'nThreads',4,VarParsing.multiplicity.singleton, VarParsing.varType.int,
	'default number of threads');

## to be used when running crab jobs with local files                                                                                                                           
options.register ('isCrab',False,VarParsing.multiplicity.singleton, VarParsing.varType.bool,
		  'to be used to handle local files with crab');

## parsing command line arguments
options.parseArguments()

### check consistentcy of basic options
if options.isMC and 'dataRun2' in options.globalTag:
	options.globalTag = '76X_mcRun2_asymptotic_RunIIFall15DR76_v1';

if not options.isMC:
	options.crossSection = -1.;

if options.isMC and options.miniAODProcess != 'PAT':
	options.miniAODProcess  = 'PAT'

print "##### Settings ######"
print "Running with isMC                = ",options.isMC	
print "Running with processName         = ",options.processName	
print "Running with miniAODProcess      = ",options.miniAODProcess	
print "Running with outputFileName      = ",options.outputFileName	
print "Running with globalTag           = ",options.globalTag	
print "Running with filterOnHLT         = ",options.filterOnHLT
print "Running with isSignalSample      = ",options.isSignalSample
print "Running with addGenParticles     = ",options.addGenParticles
print "Running with nThreads            = ",options.nThreads
print "Running with isCrab              = ",options.isCrab
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
if options.inputFiles == []:

	process.source = cms.Source("PoolSource", 
   		 fileNames = cms.untracked.vstring())

	if not options.isMC :
		process.source.fileNames.append(
			'/store/data/Run2015D/DoubleEG/MINIAOD/16Dec2015-v2/00000/00A55FDF-74A6-E511-AD74-0CC47A4D7658.root'
			)
	else:
		process.source.fileNames.append(
			'file:pickevents.root'
			)    	
else:
   process.source = cms.Source("PoolSource",
   	  fileNames = cms.untracked.vstring(options.inputFiles))

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
process.maxEvents = cms.untracked.PSet( 
    input = cms.untracked.int32(options.maxEvents))

# Set the global tag depending on the sample type
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag.globaltag = options.globalTag  

# run cut-based electron ID https://twiki.cern.ch/twiki/bin/viewauth/CMS/CutBasedElectronIdentificationRun2
from Timing.TimingAnalyzer.ElectronTools_cff import ElectronTools
ElectronTools(process)

# run cut-based photon ID 
from Timing.TimingAnalyzer.PhotonTools_cff import PhotonTools
PhotonTools(process)

# Create a set of objects to read from
process.selectedObjects = cms.EDProducer("PFCleaner",
     electrons = cms.InputTag("slimmedElectrons"),
     electronidveto   = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Spring15-25ns-V1-standalone-veto"),
     electronidloose  = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Spring15-25ns-V1-standalone-loose"),
     electronidmedium = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Spring15-25ns-V1-standalone-medium"),
     electronidtight  = cms.InputTag("egmGsfElectronIDs:cutBasedElectronID-Spring15-25ns-V1-standalone-tight"),
     electronidheep   = cms.InputTag("egmGsfElectronIDs:heepElectronID-HEEPV60"),
)

## Create output file
## Setup the service to make a ROOT TTree
process.TFileService = cms.Service("TFileService", 
		                   fileName = cms.string(options.outputFileName))

# Make the tree 
process.tree = cms.EDAnalyzer("TimingAnalyzer",
   ## gen info			     
   isMC                   = cms.bool(options.isMC),
   isSignalSample         = cms.bool(options.isSignalSample),			      
   addGenParticles        = cms.bool(options.addGenParticles),			      
   pileup     = cms.InputTag("slimmedAddPileupInfo"),
   genevt     = cms.InputTag("generator"),
   gens       = cms.InputTag("prunedGenParticles"),
   xsec       = cms.double(options.crossSection),   
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
   ## ecal recHits			      
   recHitCollectionEB = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
   recHitCollectionEE = cms.InputTag("reducedEgamma", "reducedEERecHits"),
)

# Set up the path
if (options.isMC):
	process.treePath = cms.Path(process.gentree + 					    
				    process.tree)
else :
	process.treePath = cms.Path(
		process.tree)
