import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('python')

## object prep cuts
options.register('jetpTmin',15.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'jet pT minimum cut');
options.register('jetIDmin',1,VarParsing.multiplicity.singleton,VarParsing.varType.int,'jet ID minimum cut');
options.register('rhEmin',1.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'recHit energy minimum cut');
options.register('phpTmin',20.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'photon pT minimum cut');
options.register('phIDmin','loose',VarParsing.multiplicity.singleton,VarParsing.varType.string,'photon ID minimum cut');

#pre-selection cuts
options.register('applyTrigger',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to apply trigger pre-selection');
options.register('minHT',400.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'jet HT minimum cut');
options.register('applyHT',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to apply HT pre-selection');
options.register('phgoodpTmin',70.0,VarParsing.multiplicity.singleton,VarParsing.varType.float,'good photon pT minimum cut');
options.register('phgoodIDmin','medium',VarParsing.multiplicity.singleton,VarParsing.varType.string,'good photon ID minimum cut');
options.register('applyPhGood',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to require at least one good photon in pre-selection');

## matching cuts
options.register('dRmin',0.4,VarParsing.multiplicity.singleton,VarParsing.varType.float,'dR minimum cut');
options.register('pTres',0.5,VarParsing.multiplicity.singleton,VarParsing.varType.float,'pT resolution cut');

## trigger input
options.register('inputPaths','test/HLTpaths.txt',VarParsing.multiplicity.singleton,VarParsing.varType.string,'text file list of input signal paths');
options.register('inputFilters','test/HLTfilters.txt',VarParsing.multiplicity.singleton,VarParsing.varType.string,'text file list of input signal filters');

## data or MC options
options.register('isMC',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to indicate data or MC');
options.register('isGMSB',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to indicate GMSB');
options.register('isHVDS',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to indicate HVDS');
options.register('isBkg',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to indicate Background MC');

## GT to be used
options.register('globalTag','92X_dataRun2_Prompt_v8',VarParsing.multiplicity.singleton,VarParsing.varType.string,'gloabl tag to be used');

## do a demo run over only 1k events
options.register('demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,'flag to run over only 1k events');

## processName
options.register('processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,'process name to be considered');

## outputFile Name
options.register('outputFileName','dispho.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,'output file name created by cmsRun');

## parsing command line arguments
options.parseArguments()

if options.isGMSB or options.isHVDS or options.isBkg: options.isMC

print "     ##### Settings ######"
print "      -- Object Prep --"
print "jetpTmin       : ",options.jetpTmin
print "jetIDmin       : ",options.jetIDmin
print "rhEmin         : ",options.rhEmin
print "phpTmin        : ",options.phpTmin
print "phIDmin        : ",options.phIDmin
print "     -- Pre-Selection --"
print "applyTrigger   : ",options.applyTrigger
print "minHT          : ",options.minHT
print "applyHT        : ",options.applyHT
print "phgoodpTmin    : ",options.phgoodpTmin	
print "phgoodIDmin    : ",options.phgoodIDmin
print "applyPhGood    : ",options.applyPhGood	
print "        -- Matching --"
print "dRmin          : ",options.dRmin
print "pTres          : ",options.pTres
print "        -- Trigger --"
print "inputPaths     : ",options.inputPaths
print "inputFilters   : ",options.inputFilters
if options.isMC:
	print "        -- MC Info --"
	print "isMC           : ",options.isMC
	print "isGMSB         : ",options.isGMSB
	print "isHVDS         : ",options.isHVDS
	print "isBkg          : ",options.isBkg
print "           -- GT --"
print "globalTag      : ",options.globalTag	
print "         -- Output --"
print "demoMode       : ",options.demoMode
print "processName    : ",options.processName	
print "outputFileName : ",options.outputFileName	
print "     #####################"

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
		# 2017A-v1, GT: 92X_dataRun2_Prompt_v4
		#'/store/data/Run2017A/SinglePhoton/MINIAOD/PromptReco-v1/000/295/977/00000/9CAC61AF-094A-E711-BA62-02163E0138FA.root',
		# 2017A-v2, GT: 92X_dataRun2_Prompt_v4
		#'/store/data/Run2017A/SinglePhoton/MINIAOD/PromptReco-v2/000/296/173/00000/D8A4F64B-6A4C-E711-9068-02163E01A6DE.root',
		# 2017A-v3, GT: 92X_dataRun2_Prompt_v4
		#'/store/data/Run2017A/SinglePhoton/MINIAOD/PromptReco-v3/000/296/888/00000/707282D6-8F55-E711-8BB3-02163E0146D5.root',
		# 2017B-v1, GT: 92X_dataRun2_Prompt_v4 
		#'/store/data/Run2017B/SinglePhoton/MINIAOD/PromptReco-v1/000/297/050/00000/1EFAAE6B-3D56-E711-B66E-02163E013854.root',
		# 2017B-v2, GT: 92X_dataRun2_Prompt_v5
		#'/store/data/Run2017B/SinglePhoton/MINIAOD/PromptReco-v2/000/299/065/00000/EAF9EEDA-E96A-E711-9370-02163E011DD8.root',
		# 2017C-v1, GT: 92X_dataRun2_Prompt_v6
		#'/store/data/Run2017C/SinglePhoton/MINIAOD/PromptReco-v1/000/299/370/00000/20A7ED55-C66D-E711-B7D5-02163E019C2A.root',
		# 2017C-v2, GT: 92X_dataRun2_Prompt_v7 
		#'/store/data/Run2017C/SinglePhoton/MINIAOD/PromptReco-v2/000/300/087/00000/DC73481C-0477-E711-BE6F-02163E012551.root',
		# 2017C-v3, GT: 92X_dataRun2_Prompt_v8
		#'/store/data/Run2017C/SinglePhoton/MINIAOD/PromptReco-v3/000/300/777/00000/18694619-C67E-E711-9CBF-02163E01A6D1.root',
		# 2017D-v1, GT: 92X_dataRun2_Prompt_v8 
		'/store/data/Run2017D/SinglePhoton/MINIAOD/PromptReco-v1/000/302/042/00000/18838DB3-698F-E711-9D1B-02163E01192A.root',
		# 2017E-v1, GT: 92X_dataRun2_Prompt_v9
		#'/store/data/Run2017E/SinglePhoton/MINIAOD/PromptReco-v1/000/303/819/00000/F8E8E7B5-68A2-E711-9655-02163E0138E0.root',
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

# Make the tree 
process.tree = cms.EDAnalyzer("DisPho",
   ## object prep cuts
   jetpTmin = cms.double(options.jetpTmin),
   jetIDmin = cms.int32(options.jetIDmin),
   rhEmin   = cms.double(options.rhEmin),
   phpTmin  = cms.double(options.phpTmin),
   phIDmin  = cms.string(options.phIDmin),
   ## pre-selection
   applyTrigger = cms.bool(options.applyTrigger),
   minHT        = cms.double(options.minHT),
   applyHT      = cms.bool(options.applyHT),
   phgoodpTmin  = cms.double(options.phgoodpTmin),
   phgoodIDmin  = cms.string(options.phgoodIDmin),
   applyPhGood  = cms.bool(options.applyPhGood),
   ## matched criteria
   dRmin = cms.double(options.dRmin),
   pTres = cms.double(options.pTres),
   ## triggers
   inputPaths     = cms.string(options.inputPaths),
   inputFilters   = cms.string(options.inputFilters),
   triggerResults = cms.InputTag("TriggerResults", "", "HLT"),
   triggerObjects = cms.InputTag("slimmedPatTrigger"),
   ## vertices
   vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
   ## rho
   rhos = cms.InputTag("fixedGridRhoFastjetAll"), #fixedGridRhoAll
   ## MET
   mets = cms.InputTag("slimmedMETs"),
   ## jets			    	
   jets = cms.InputTag("slimmedJets"),
   ## photons		
   photons    = cms.InputTag("slimmedPhotons"),
   ootPhotons = cms.InputTag("slimmedOOTPhotons"),
   ## ecal recHits			      
   recHitsEB = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
   recHitsEE = cms.InputTag("reducedEgamma", "reducedEERecHits"),
   ## gen info			     
   isGMSB   = cms.bool(options.isGMSB),
   isHVDS   = cms.bool(options.isHVDS),
   isBkg    = cms.bool(options.isBkg),
   genevt   = cms.InputTag("generator"),
   pileup   = cms.InputTag("slimmedAddPileupInfo"),
   genparts = cms.InputTag("prunedGenParticles")
)

# Set up the path
process.treePath = cms.Path(process.tree)
