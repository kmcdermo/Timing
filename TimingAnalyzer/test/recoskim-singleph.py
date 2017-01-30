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

## run era
options.register (
	'era','2016B',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'data run era');

## use which block of events
options.register (
	'blockNo',1,VarParsing.multiplicity.singleton,VarParsing.varType.int,
	'flag to use which block of files');

## processName
options.register (
	'processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'process name to be considered');

## outputFile Name
options.register (
	'outputFileName','recoskim-singleph.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'output file name created by cmsRun');

## GT to be used    
options.register (
	'globalTag','80X_dataRun2_2016SeptRepro_v4',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'gloabl tag to be used');

## parsing command line arguments
options.parseArguments()

if   options.blockNo == 1: options.outputFileName = 'recoskim-singleph-'+era+'-1.root'
elif options.blockNo == 2: options.outputFileName = 'recoskim-singleph-'+era+'-2.root'
else: exit

print "##### Settings ######"
print "Running with demoMode            = ",options.demoMode
print "Running with era                 = ",options.era
print "Running with blockNo             = ",options.blockNo
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
if era == "2016B" :
	if options.blockNo == 1:
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/14EE3AEC-471A-E611-A84F-02163E014380.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/16F71DEB-1E1A-E611-B34A-02163E011EE1.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/1879CF4C-5A1A-E611-BF7F-02163E0142F8.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/1A30D6F9-2E1A-E611-8654-02163E014167.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/1A70CC9C-1E1A-E611-BA9B-02163E01190F.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/1C28DBD0-491A-E611-9195-02163E01438C.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/1C3D7A90-671A-E611-9062-02163E014150.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/20DBD299-221A-E611-998B-02163E014251.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/20E5C0C3-281A-E611-93D2-02163E01426D.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/2656477F-291A-E611-AA71-02163E011C18.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/26FD0C51-271A-E611-8323-02163E0129CF.root'
				))
	elif options.blockNo == 2:
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/006806FF-211A-E611-A850-02163E014111.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/02542059-2E1A-E611-9602-02163E0136B1.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/029E986F-1E1A-E611-A489-02163E0122B1.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/04D17866-261A-E611-B00F-02163E014259.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/068244C0-481A-E611-B513-02163E01380C.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/08A87850-201A-E611-AAA5-02163E01271B.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/08FBFF90-3C1A-E611-BB7F-02163E01462D.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/0E00A648-381A-E611-AC04-02163E0141E1.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/1061E38E-291A-E611-9C28-02163E011857.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/1409FF3B-2D1A-E611-8A0C-02163E0143A7.root',
				'/store/data/Run2016B/SinglePhoton/RECO/PromptReco-v2/000/273/158/00000/14B7A527-1A1A-E611-AE84-02163E01475F.root'
				))
	else: exit
elif era == "2016D" :
	if options.blockNo == 1:
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/00C8D27D-0545-E611-8A03-02163E01191E.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/024EBF60-0845-E611-A3DC-02163E011BEC.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/04B0F29F-0945-E611-8F3F-02163E0144AB.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/06D3A4C0-0745-E611-ADFD-02163E014769.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/080D3F0F-0B45-E611-B26F-02163E01456D.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/0C249CF1-0545-E611-87FE-02163E011BB4.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/0C7843ED-0445-E611-A15C-02163E014536.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/10735A75-0745-E611-89EF-02163E013520.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/1E274E44-0545-E611-8440-02163E01438B.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/222A5144-0545-E611-8082-02163E013469.root'
				))
	elif options.blockNo == 2:
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/28D71DA5-0645-E611-9B6D-02163E0126E9.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/28FBD075-0945-E611-99BB-02163E0140D9.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/2C06FC3C-0845-E611-80C5-02163E014607.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/2C304BED-0645-E611-A4A3-02163E01372E.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/2C84FBA2-0645-E611-8BB3-02163E01261C.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/34515BB5-0545-E611-B69F-02163E01346E.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/38790BA7-0A45-E611-B887-02163E0137AF.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/3A2A5E94-0345-E611-9317-02163E011D90.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/3AF0C7FE-0445-E611-BC2A-02163E013575.root',
				'/store/data/Run2016D/DoubleEG/RECO/PromptReco-v2/000/276/315/00000/3C50024E-0545-E611-86CD-02163E013499.root'
				))
	else: exit
elif era == "2016F" :
	if options.blockNo == 1:
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/06F29843-9959-E611-AA51-02163E011DDE.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/0A2196BD-9959-E611-BB7D-02163E012797.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/0A711C15-9959-E611-BAD9-FA163EDB8B04.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/0E966A3E-9959-E611-8A25-02163E0119B0.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/1030481F-9959-E611-BF7F-FA163ED57BE5.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/10A96A38-9959-E611-95DB-02163E01412A.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/12B5CA47-9959-E611-B2E2-02163E013498.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/14ED9B32-9959-E611-8867-02163E0146DD.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/180B141B-9959-E611-80F0-FA163ED57BE5.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/182FAB23-9959-E611-85FF-FA163EABCA0F.root'
				))
	elif options.blockNo == 2:
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/1A337134-9959-E611-8B72-02163E01430D.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/1EFC3F1B-9959-E611-A9CC-FA163E110010.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/2017AA5C-9959-E611-9AFD-02163E0142F3.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/201E3E21-9959-E611-BF76-FA163E8586D4.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/2226825A-9959-E611-9B45-02163E0135EA.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/2237A224-9959-E611-B1B0-FA163EA628DC.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/22CAED2C-9959-E611-8C18-02163E011DF1.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/2C6B3F17-9959-E611-964D-FA163E736D17.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/2EA8CC36-9959-E611-9335-02163E01412A.root',
				'/store/data/Run2016F/DoubleEG/RECO/PromptReco-v1/000/277/992/00000/306FA91D-9959-E611-9BB6-FA163E6F017D.root'
				))
	else: exit
elif era == "2016H" :
	if options.blockNo == 1:
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/0A486505-5885-E611-AB10-02163E01415E.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/0C3EFCF3-5785-E611-A14C-FA163E6FB889.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/107A4A29-5885-E611-ACDE-02163E01338A.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/126CB6F1-5785-E611-89F7-FA163E6C447B.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/1651C021-5885-E611-BF5C-02163E01340B.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/18C97029-5885-E611-A507-02163E012482.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/1AAF2DF8-5785-E611-ACB7-02163E014597.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/1E95DCE6-5785-E611-8084-FA163E776DCA.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/1EB3E9FC-5785-E611-8A1B-02163E0144D3.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/22D94C06-5885-E611-BF9E-02163E01225E.root'
				))
	elif options.blockNo == 2:
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/2428A5F8-5785-E611-B3AC-02163E0144B4.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/26F87B1D-5885-E611-9F31-02163E011AE9.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/28FF89F7-5785-E611-9B2C-02163E01464C.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/2EA84A00-5885-E611-9F0C-02163E013816.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/303C4EE4-5785-E611-81E5-FA163E6C51B3.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/36C16901-5885-E611-84F1-02163E0122F0.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/3C2E1AF7-5785-E611-845E-FA163E19056B.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/3CE6ADEC-5785-E611-9E25-02163E0141A7.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/402378F0-5785-E611-A291-FA163E51BCFD.root',
				'/store/data/Run2016H/DoubleEG/RECO/PromptReco-v2/000/281/639/00000/4249BFEA-5785-E611-A4FE-FA163E70A7CF.root'
				))
	else: exit
else: exit

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
   rhos = cms.InputTag("fixedGridRhoFastjetAll"), #fixedGridRhoAll
   ## trigger
   triggers = cms.InputTag("TriggerResults", "", "HLT"),
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
