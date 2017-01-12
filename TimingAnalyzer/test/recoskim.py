import os, re
import FWCore.ParameterSet.Config as cms
import FWCore.PythonUtilities.LumiList as LumiList  
import FWCore.ParameterSet.Types as CfgTypes    

### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## do a demo run over only 1k events
options.register (
	'demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to run over only 1k events as a demo');

## use ReRECO files flag
options.register (
	'isReRECO',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to use ReRECO files');

## processName
options.register (
	'processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'process name to be considered');

## outputFile Name
options.register (
	'outputFileName','recoskim.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'output file name created by cmsRun');

## GT to be used    
options.register (
	'globalTag','80X_dataRun2_2016SeptRepro_v4',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'gloabl tag to be used');

## parsing command line arguments
options.parseArguments()

if options.isReRECO: 
	options.outputFileName = 'recoskim-rereco.root'

print "##### Settings ######"
print "Running with demoMode            = ",options.demoMode
print "Running with isReRECO            = ",options.isReRECO
print "Running with processName         = ",options.processName	
print "Running with outputFileName      = ",options.outputFileName	
print "Running with globalTag           = ",options.globalTag	
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
if options.isReRECO:
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_1.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_2.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_3.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_4.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_5.root',
#			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_6.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_7.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_8.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_9.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_10.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_11.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_12.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_13.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_14.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_15.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_16.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_17.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_18.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_19.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_20.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_21.root',
			'/store/user/kmcdermo/DoubleEG/ReRECO_UncleanedECALRecHits-v1/170105_210848/0000/rereco_test_22.root'
			))
else: 
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/409/00000/12FC6874-5C1B-E611-8C9C-02163E01436C.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/409/00000/26A716AE-561B-E611-B478-02163E014614.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/409/00000/489C74EB-581B-E611-BB7D-02163E01447B.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/409/00000/5877B6CA-561B-E611-B4C4-02163E01447B.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/409/00000/B2FE1A5F-591B-E611-9266-02163E01447B.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/409/00000/E6A20FAE-561B-E611-BDB0-02163E014614.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/410/00000/08143BFA-641B-E611-97CE-02163E0134FC.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/410/00000/243D9824-661B-E611-8BE7-02163E011A08.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/410/00000/465B0E04-621B-E611-AC90-02163E011A83.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/410/00000/AC47F4A6-681B-E611-8D19-02163E01396B.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/411/00000/1C885B23-6B1B-E611-B74B-02163E014647.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/411/00000/28A61BF4-691B-E611-88ED-02163E01414B.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/411/00000/3886E4BB-6B1B-E611-BE64-02163E014591.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/411/00000/A6078AC7-661B-E611-B6F9-02163E011D0A.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/411/00000/AEC55DFE-681B-E611-A644-02163E01342A.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/411/00000/B02FBE98-6B1B-E611-BEC8-02163E0134A9.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/273/411/00000/B0BC5A85-691B-E611-81E9-02163E014797.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/275/371/00000/56645C1C-1F39-E611-982E-02163E0119CE.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/275/371/00000/9AEFFDED-1D39-E611-B149-02163E014545.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/275/371/00000/B6F051E8-2039-E611-A5BD-02163E01367A.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/275/371/00000/B88F3F6A-2439-E611-8BBD-02163E0137B2.root',
			'/store/data/Run2016B/DoubleEG/RECO/PromptReco-v2/000/275/371/00000/C6E4A786-0F39-E611-AE8F-02163E0140DE.root'
			))

# Set the json locally because the ReRECO gets f'ed
process.source.lumisToProcess = CfgTypes.untracked(CfgTypes.VLuminosityBlockRange())  
JSONfile = '/afs/cern.ch/user/k/kmcdermo/private/dispho/CMSSW_8_0_21/src/Timing/TimingAnalyzer/test/data_rereco.json'
myLumis = LumiList.LumiList(filename = JSONfile).getCMSSWString().split(',')  
process.source.lumisToProcess.extend(myLumis)                              

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
process.tree = cms.EDAnalyzer("RECOSkim",
   ## vertices
   vertices = cms.InputTag("offlinePrimaryVertices"),
   ## rho
   rhos = cms.InputTag("fixedGridRhoAll"),
   ## MET
   mets = cms.InputTag("pfMet"),
   ## jets			    	
   jets = cms.InputTag("ak4PFJetsCHS"),
   ## photons		
   photons        = cms.InputTag("gedPhotons"),
   ## ecal recHits			      
   recHitCollectionEB = cms.InputTag("ecalRecHit","EcalRecHitsEB"),
   recHitCollectionEE = cms.InputTag("ecalRecHit","EcalRecHitsEE")
)

# Set up the path
process.treePath = cms.Path(process.tree)
