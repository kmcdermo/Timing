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

if   options.blockNo == 1: options.outputFileName = 'recoskim-singleph-'+options.era+'-1.root'
elif options.blockNo == 2: options.outputFileName = 'recoskim-singleph-'+options.era+'-2.root'
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
if options.era == "2016B" :
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
elif options.era == "2016D" :
	if options.blockNo == 1:
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/0074ECE9-FB44-E611-B9F2-02163E0133CF.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/08E95BEE-FB44-E611-8791-02163E0135CC.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/0AF5B472-FC44-E611-9B0C-02163E011F8A.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/0C36AD86-FD44-E611-9DF0-02163E0138FC.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/0CB0372F-FB44-E611-9252-02163E013549.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/105C9708-FE44-E611-992F-02163E011C1F.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/124A5DDD-FA44-E611-AECD-02163E0146E7.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/126D8115-FE44-E611-90D0-02163E014516.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/1463B687-FF44-E611-89B7-02163E0127E8.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/1879ED27-FA44-E611-B963-02163E011B8C.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/1E2B474D-FF44-E611-A97D-02163E0144FE.root'
))
	elif options.blockNo == 2:
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/1E5A4E8F-FA44-E611-9F49-02163E011DC1.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/22D08927-FD44-E611-B981-02163E01358F.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/2654ED6E-FC44-E611-BC7E-02163E01428F.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/28C5C345-FA44-E611-814C-02163E011B5A.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/303E099F-FD44-E611-BF5F-02163E0145F2.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/3475848F-FC44-E611-85AC-02163E014151.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/38CBDFB1-FC44-E611-82FF-02163E012586.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/3A0F9DE4-FD44-E611-8C3F-02163E0144DD.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/3CD49A94-FC44-E611-BE13-02163E013744.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/3E2E75FB-FA44-E611-B391-02163E014222.root',
				'/store/data/Run2016D/SinglePhoton/RECO/PromptReco-v2/000/276/315/00000/40E63C2A-FA44-E611-BD22-02163E01348F.root'
				))
	else: exit
elif options.era == "2016F" :
	if options.blockNo == 1:
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/0A3D4A1F-5D5A-E611-9206-02163E013762.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/0C73CC1E-5D5A-E611-84A8-FA163EEE3D44.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/0E36BE2B-5D5A-E611-BBBD-02163E014629.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/0E373009-5D5A-E611-967C-02163E011B99.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/1081D5FF-5B5A-E611-ADEF-02163E0143D6.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/124F333A-5D5A-E611-811D-02163E0142F0.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/12BDC305-5C5A-E611-AF01-02163E014284.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/12EABB1C-5C5A-E611-B8AC-02163E012736.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/14796A0A-5C5A-E611-9DB4-02163E0141BD.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/167391F1-5B5A-E611-A0E7-FA163ED5641F.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/1805EC5B-5D5A-E611-A3BA-02163E01397E.root'
				))
	elif options.blockNo == 2:
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/1808E902-5C5A-E611-B54A-FA163E37255B.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/1813B215-5C5A-E611-8940-02163E0129DF.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/18F7162C-5C5A-E611-82ED-02163E0134F4.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/1A3EA202-5C5A-E611-9173-FA163ED18647.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/1A82E69B-5D5A-E611-954F-02163E01232B.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/1C4F6C11-5C5A-E611-B644-02163E0143C1.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/1E60F707-5E5A-E611-8B59-02163E0134C3.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/1E9CF510-5C5A-E611-A955-FA163E5751CB.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/205E65C2-5D5A-E611-BBE9-02163E012135.root',
				'/store/data/Run2016F/SinglePhoton/RECO/PromptReco-v1/000/278/018/00000/24BE0835-5C5A-E611-9B5F-02163E014376.root'
				))
	else: exit
elif options.era == "2016H" :
	if options.blockNo == 1:
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/4C027FFC-A584-E611-8172-02163E0144EC.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/526D1AFA-A584-E611-9FEC-FA163EE0D423.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/5AA900E9-A584-E611-B6C9-FA163E5A1011.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/5C05D4EE-A584-E611-B453-02163E014550.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/5C7F80EB-A584-E611-9239-FA163E84F3D6.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/5C92C5F7-A584-E611-B1C2-02163E014104.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/6247CBF0-A584-E611-AE3F-FA163EACE1FA.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/62AA8BFA-A584-E611-B179-FA163ECEA2E8.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/6613E8EA-A584-E611-BBAF-FA163EB9CF24.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/6E134701-A684-E611-A71D-FA163E5B1EE0.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/783773FC-A584-E611-A9C1-02163E0144EC.root'
				))
	elif options.blockNo == 2:
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/78796937-A684-E611-A53C-02163E0146B3.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/8007D30F-A684-E611-9833-02163E0144C2.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/807F6FEC-A584-E611-A02D-FA163E3EB09A.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/82621115-A684-E611-9A28-02163E0118AA.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/82FBAAEC-A584-E611-8360-FA163E72225D.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/90540EEB-A584-E611-A501-FA163E61FE92.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/942BD71E-A684-E611-801D-02163E0142E3.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/9ADEA500-A684-E611-845A-02163E0145CD.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/A26B6150-A684-E611-A9EA-02163E011D0B.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/BAB722F4-A584-E611-AA86-FA163E6696AB.root',
				'/store/data/Run2016H/SinglePhoton/RECO/PromptReco-v2/000/281/613/00000/BC2D16FC-A584-E611-9598-02163E0138E9.root'
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
