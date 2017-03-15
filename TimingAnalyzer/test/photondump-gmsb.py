import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## dump trigger menu 
options.register (
	'dumpTriggerMenu',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to dump trigger menu');

## which trigger menu??
options.register (
	'isHLT2',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to use displaced photon menu');

options.register (
	'isHLT3',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to use displaced photon breakdown menu');

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
	'isMC',True,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate data or MC');

options.register (
	'isGMSB',True,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate GMSB');

options.register (
	'isHVDS',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate GMSB');

options.register (
	'ctau','6000',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'which GMSB sample to use');

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

## setup triggerPName from bools
if   options.isHLT2 : options.triggerPName = 'HLT2'
elif options.isHLT3 : options.triggerPName = 'HLT3'

## reset file name
options.outputFileName = 'photondump-gmsb-ctau'+options.ctau+'-'+options.triggerPName+'.root'

print "##### Settings ######"
print "Running with dumpTriggerMenu     = ",options.dumpTriggerMenu
print "Running with isHLT2              = ",options.isHLT2
print "Running with isHLT3              = ",options.isHLT3
print "Running with triggerPName        = ",options.triggerPName
print "Running with dumpRHs             = ",options.dumpRHs
print "Running with dumpIds             = ",options.dumpIds
print "Running with ctau                = ",options.ctau
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
if options.ctau == '6000' :
	if options.triggerPName == 'HLT':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_1.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_2.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_3.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_4.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_5.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_6.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_7.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_8.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_9.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_10.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_11.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_12.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_13.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_14.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_15.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_16.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_17.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_18.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_19.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_132410/0000/PAT-MINIAODSIM_20.root'
				))
	elif options.triggerPName == 'HLT2':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_1.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_10.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_11.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_12.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_14.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_15.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_16.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_17.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_18.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_19.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_2.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_20.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_21.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_22.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_23.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_24.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_25.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_26.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_27.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_28.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_29.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_3.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_30.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_31.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_32.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_33.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_34.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_35.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_36.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_37.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_38.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_39.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_4.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_40.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_41.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_43.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_44.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_45.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_46.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_47.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_48.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_49.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_5.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_50.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_51.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_52.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_53.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_54.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_55.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_56.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_57.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_58.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_59.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_6.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_60.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_61.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_62.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_63.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_64.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_65.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_66.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_67.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_68.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_69.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_7.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_70.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_71.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_72.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_73.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_74.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_75.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_76.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_77.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_78.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_79.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_8.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_80.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_81.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_82.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_83.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_84.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_85.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_86.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_87.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_88.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_89.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_9.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_90.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_91.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_92.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_93.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_94.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_95.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_96.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_97.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_98.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v5/170314_105038/0000/reHLT-PAT-MINIAODSIM_99.root'
				))
	elif options.triggerPName == 'HLT3':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_1.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_10.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_11.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_12.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_13.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_14.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_15.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_16.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_17.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_18.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_19.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_2.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_20.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_21.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_22.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_23.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_24.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_25.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_26.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_27.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_28.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_29.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_3.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_30.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_31.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_32.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_33.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_34.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_35.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_36.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_37.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_38.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_39.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_4.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_40.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_41.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_42.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_43.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_44.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_45.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_46.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_47.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_48.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_49.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_5.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_50.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_51.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_52.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_53.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_54.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_55.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_56.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_57.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_58.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_59.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_6.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_60.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_61.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_62.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_63.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_64.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_65.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_66.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_67.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_68.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_69.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_7.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_70.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_71.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_72.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_73.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_74.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_75.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_76.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_77.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_78.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_79.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_8.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_80.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_81.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_82.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_83.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_84.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_85.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_86.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_87.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_88.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_9.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_92.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_93.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_94.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_95.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_96.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_97.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_98.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_reHLT-DisplacedBreakDown_V3_customiseTiming_PAT-MINIAODSIM-v1/170314_111217/0000/reHLT-BreakDown-PAT-MINIAODSIM_99.root'
				))
elif options.ctau == '2000' :
	if options.triggerPName == 'HLT':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_1.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_2.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_3.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_4.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_5.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_6.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_7.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_8.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_9.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_10.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_11.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_12.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_13.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_14.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_15.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_16.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_17.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_18.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_19.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003359/0000/PAT-MINIAODSIM_20.root'
				))
	elif options.triggerPName == 'HLT2':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_1.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_10.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_11.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_12.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_13.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_14.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_15.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_16.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_17.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_18.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_19.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_2.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_20.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_21.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_22.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_23.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_24.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_25.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_26.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_27.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_28.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_29.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_3.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_30.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_31.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_32.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_33.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_34.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_35.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_36.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_37.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_38.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_39.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_4.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_40.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_41.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_42.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_43.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_44.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_45.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_46.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_47.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_48.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_49.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_5.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_50.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_51.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_52.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_53.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_54.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_55.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_56.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_57.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_58.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_59.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_6.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_60.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_61.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_62.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_63.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_64.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_65.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_66.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_67.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_68.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_7.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_70.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_71.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_72.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_73.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_74.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_75.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_76.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_77.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_78.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_79.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_8.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_80.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_81.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_82.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_83.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_84.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_86.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_87.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_88.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_89.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_9.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_90.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_91.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_92.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_93.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_94.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_95.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_96.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_97.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau2000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111113/0000/reHLT-PAT-MINIAODSIM_98.root'
				))
elif options.ctau == '100' :
	if options.triggerPName == 'HLT':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_1.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_2.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_3.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_4.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_5.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_6.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_7.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_8.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_9.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_10.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_11.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_12.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_13.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_14.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_15.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_16.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_17.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_18.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_19.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1/170305_003318/0000/PAT-MINIAODSIM_20.root'
				))
	elif options.triggerPName == 'HLT2':
		process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_1.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_10.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_11.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_12.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_13.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_14.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_15.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_16.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_17.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_18.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_19.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_2.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_20.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_21.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_22.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_23.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_24.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_25.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_26.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_27.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_28.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_29.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_3.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_30.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_31.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_32.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_33.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_34.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_35.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_36.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_37.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_38.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_39.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_4.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_40.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_41.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_42.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_43.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_44.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_45.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_46.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_47.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_48.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_49.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_5.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_50.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_51.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_52.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_53.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_54.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_55.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_56.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_57.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_58.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_59.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_6.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_60.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_61.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_62.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_63.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_65.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_66.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_67.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_68.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_69.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_7.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_70.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_71.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_72.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_73.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_74.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_75.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_76.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_77.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_78.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_79.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_8.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_80.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_81.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_82.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_83.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_85.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_86.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_87.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_88.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_89.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_9.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_90.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_91.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_92.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_93.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_94.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_95.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_96.root',
				'/store/user/kmcdermo/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/GMSB_L180_Ctau100_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v1/170314_111006/0000/reHLT-PAT-MINIAODSIM_98.root'
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
   ## triggers
   dumpTriggerMenu = cms.bool(options.dumpTriggerMenu),
   isHLT2          = cms.bool(options.isHLT2),
   isHLT3          = cms.bool(options.isHLT3),
   triggerResults  = cms.InputTag("TriggerResults", "", options.triggerPName),
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
