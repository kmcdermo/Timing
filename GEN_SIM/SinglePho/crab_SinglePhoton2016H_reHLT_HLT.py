from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'SinglePhoton_2016H_reHLT-v2'
config.General.workArea    = 'crab_projects'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName   = 'reHLT_HLTDEBUG_HLT.py'

config.Data.inputDataset          = '/SinglePhoton/Run2016H-PromptReco-v2/MINIAOD'
config.Data.secondaryInputDataset = '/SinglePhoton/Run2016H-v1/RAW'
config.Data.lumiMask              = 'block_3e69b694-9ee5-11e6-9951-001e67abf228.json'

config.Data.splitting   = 'EventAwareLumiBased'
config.Data.unitsPerJob = 5000

config.Site.storageSite   = 'T2_CH_CERN'
config.Data.outLFNDirBase = '/store/group/phys_exotica/displacedPhotons/'
config.Data.publication   = False

