from CRABClient.UserUtilities import config
config = config()

## Name of output directory ##
config.General.requestName = 'HLTPhysics2-DisPhoMenu-PS3-v1'
config.General.workArea    = 'crab_projects'

## Input analyzer pset ## 
config.JobType.pluginName  = 'Analysis'
config.JobType.psetName    = 'grun_L1uGT_PS_1p35e34.py'
config.JobType.numCores    = 4
config.JobType.outputFiles = ['hltbits.root']

## Input Data ##
config.Data.inputDataset   = '/HLTPhysics2/Run2016H-v1/RAW'
config.Data.lumiMask       = 'columns_1p35e34.txt'
config.Data.unitsPerJob    = 10
config.Data.splitting      = 'LumiBased' 

## Output Data ##
config.Data.publication   = False
config.Data.outLFNDirBase = '/store/group/phys_exotica/displacedPhotons/'
config.Site.storageSite   = 'T2_CH_CERN'
