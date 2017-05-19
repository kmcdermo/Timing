from CRABClient.UserUtilities import config
config = config()

## Name of output directory ##
config.General.requestName = 'SinglePhoton_2016H_reHLT-v4'
config.General.workArea    = 'crab_projects'

## Analysis config ##
config.JobType.pluginName = 'Analysis'
config.JobType.psetName   = 'reHLT_HLTDEBUG_HLT.py'

## Input Files ##
config.Data.inputDataset          = '/SinglePhoton/Run2016H-PromptReco-v2/MINIAOD'
config.Data.secondaryInputDataset = '/SinglePhoton/Run2016H-v1/RAW'
config.Data.lumiMask              = 'block_3e69b694-9ee5-11e6-9951-001e67abf228.json'
config.Data.splitting             = 'LumiBased'
config.Data.unitsPerJob           = 10
config.Site.whitelist             = ['T2_US_UCSD']

## Output Files ##
config.Data.publication   = False
config.Site.storageSite   = 'T2_CH_CERN'
config.Data.outLFNDirBase = '/store/group/phys_exotica/displacedPhotons/'
