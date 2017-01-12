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
	'globalTag','80X_dataRun2_Prompt_ICHEP16JEC_v0',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'gloabl tag to be used');

## do a demo run over only 100k events
options.register (
	'demoMode',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to run over only 100k events as a demo');

## parsing command line arguments
options.parseArguments()

### check consistentcy of basic options
if options.isMC and 'dataRun2' in options.globalTag:
	options.globalTag = '80X_mcRun2_asymptotic_2016_TrancheIV_v6'; #otherwise this craps out claiming no calo geometry...

print "##### Settings ######"
print "Running with dumpRHs             = ",options.dumpRHs
print "Running with isMC                = ",options.isMC	
if options.isMC:
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
if not options.isMC :
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'/store/data/Run2016D/SinglePhoton/MINIAOD/PromptReco-v2/000/276/317/00000/DE9EC1AE-1745-E611-AF8F-02163E014413.root' #2016D (276317)
			))
else:
	process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_1.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_2.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_3.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_4.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_5.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_6.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_7.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_8.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_9.root'
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_10.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_11.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_12.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_13.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_14.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_15.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_16.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_17.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_18.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_19.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_20.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_21.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_22.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_23.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_24.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_25.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_26.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_27.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_28.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_29.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_30.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_31.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_32.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_33.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_34.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_35.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_36.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_37.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_38.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_39.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_40.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_41.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_42.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_43.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_44.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_45.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_46.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_47.root',
			'root://xrootd-cms.infn.it//store/user/soffi/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_2811_step3_wTIMING/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_MINIAOD_step3_wTIMING/161128_185505/0000/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_MINIAOD_48.root'
			))

## How many events to process
if not options.demoMode:
	process.maxEvents = cms.untracked.PSet( 
		input = cms.untracked.int32(options.maxEvents))
else:
	process.maxEvents = cms.untracked.PSet( 
		input = cms.untracked.int32(1000))

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
   isMC       = cms.bool(options.isMC),
   dumpIds    = cms.bool(options.dumpIds),
   genevt     = cms.InputTag("generator"),
   pileup     = cms.InputTag("slimmedAddPileupInfo"),
   genparts   = cms.InputTag("prunedGenParticles"),
   genjets    = cms.InputTag("slimmedGenJets"),
   ## vertices
   vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
   ## MET
   mets = cms.InputTag("slimmedMETs"),
   ## jets			    	
   jets = cms.InputTag("slimmedJets"),
   ## photons		
   loosePhotonID  = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring15-25ns-V1-standalone-loose"),
   mediumPhotonID = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring15-25ns-V1-standalone-medium"),
   tightPhotonID  = cms.InputTag("egmPhotonIDs:cutBasedPhotonID-Spring15-25ns-V1-standalone-tight"),
   photons        = cms.InputTag("calibratedPhotons"),
   ## ecal recHits			      
   dumpRHs            = cms.bool(options.dumpRHs),
   recHitCollectionEB = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
   recHitCollectionEE = cms.InputTag("reducedEgamma", "reducedEERecHits")
)

# Set up the path
process.treePath = cms.Path(process.selectedPhotons * process.calibratedPhotons * process.egmPhotonIDs * process.tree)
