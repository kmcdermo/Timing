import os, re
import FWCore.ParameterSet.Config as cms
  
### CMSSW command line parameter parser
from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('python')

## input files
options.register (
	'input','legacyMC',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'input files to be run');

## processName
options.register (
	'processName','TREE',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'process name to be considered');

## isMC
options.register (
	'isMC',False,VarParsing.multiplicity.singleton,VarParsing.varType.bool,
	'flag to indicate data or MC');

## outputFile Name
options.register (
	'outputFileName','patphoton.root',VarParsing.multiplicity.singleton,VarParsing.varType.string,
	'output file name created by cmsRun');

## parsing command line arguments
options.parseArguments()

## reset file name
options.outputFileName = 'patphoton-'+options.input+'.root'

if options.input == 'legacyMC' :
    options.isMC = True
else :
    options.isMC = False

print "##### Settings ######"
print "Running with input           = ",options.input
print "Running with processName     = ",options.processName	
print "Running with isMC            = ",options.isMC
print "Running with outputFileName  = ",options.outputFileName	
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
if options.input == 'legacyMC' :
    process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_1.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_2.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_3.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_4.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_5.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_6.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_7.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_8.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_9.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_10.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_11.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_12.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_13.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_14.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_15.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_16.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_17.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_18.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_19.root',
            '/store/user/kmcdermo/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/GMSB_L180_Ctau6000_userHLT_legacy_PAT-MINIAODSIM-v1/170625_184255/0000/step3_mc_20.root'
            ))
elif options.input == 'legacyDATA' :
    process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_1.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_2.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_3.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_4.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_5.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_6.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_7.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_8.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_9.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_10.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_11.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_12.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_13.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_14.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_15.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_16.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_17.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_18.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_19.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_20.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_21.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_22.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_23.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_24.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_25.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_26.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_27.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_28.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_29.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_30.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_31.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_32.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_33.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_34.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_35.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_36.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_37.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_38.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_39.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_40.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_41.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_42.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_43.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_44.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_45.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_46.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2016G_legacy_PAT-MINIAODSIM-v1/170626_151735/0000/sph_data2016G_PAT_47.root'
            ))
elif options.input == '2017DATA' :
    process.source = cms.Source("PoolSource", fileNames = cms.untracked.vstring( 
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_1.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_2.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_3.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_4.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_5.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_6.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_7.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_8.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_9.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_10.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_11.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_12.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_13.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_14.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_15.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_16.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_17.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_18.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_19.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_20.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_21.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_22.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_23.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_24.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_25.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_26.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_27.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_28.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_29.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_30.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_31.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_32.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_33.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_34.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_35.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_36.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_37.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_38.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_39.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_40.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_41.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_42.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_43.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_44.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_45.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_46.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_47.root',
            '/store/user/kmcdermo/SinglePhoton/crab_SinglePhoton_2017B_PAT-MINIAODSIM-v2/170626_181239/0000/sph_data2017B_PAT_48.root'
            ))
else : exit

## How many events to process
process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(-1))

# Set the global tag depending on the sample type
from Configuration.AlCa.GlobalTag import GlobalTag
from Configuration.AlCa.autoCond import autoCond
if options.input == 'legacyMC' :
    process.GlobalTag.globaltag = autoCond['run2_mc']
elif options.input == 'legacyDATA' :
    process.GlobalTag.globaltag = autoCond['run2_data']
elif options.input == '2017DATA' :
    process.GlobalTag.globaltag = '92X_dataRun2_Prompt_v4'
else : exit

## Create output file
## Setup the service to make a ROOT TTree
process.TFileService = cms.Service("TFileService", 
		                   fileName = cms.string(options.outputFileName))

# Make the tree 
process.tree = cms.EDAnalyzer("SimplePATTree",
   ## gen info
   isMC   = cms.bool(options.isMC),
   pileup = cms.InputTag("slimmedAddPileupInfo"),
   ## rho
   rhos = cms.InputTag("fixedGridRhoFastjetAll"), #fixedGridRhoAll
   ## vertices
   vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
   ## MET
   mets = cms.InputTag("slimmedMETs"),
   ## jets			    	
   jets = cms.InputTag("slimmedJets"),
   ## photons		
   photons    = cms.InputTag("slimmedPhotons"   ,"","PAT"),
   ootphotons = cms.InputTag("slimmedOOTPhotons","","PAT"),
   ## ecal recHits			      
   recHitsEB = cms.InputTag("reducedEgamma", "reducedEBRecHits"),
   recHitsEE = cms.InputTag("reducedEgamma", "reducedEERecHits"),
)

# Set up the path
process.treePath = cms.Path(process.tree)
