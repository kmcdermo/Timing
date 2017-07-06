from CRABClient.UserUtilities import config
config = config()

## Name of output directory ##
config.General.requestName = 'HLT_DCS_SP'
config.General.workArea    = 'crab_projects'

## Input analyzer pset ## 
config.JobType.pluginName  = 'Analysis'
config.JobType.psetName    = 'hltdump.py'
config.JobType.inputFiles  = ['HLTpaths.txt','HLTfilters.txt']

## Input Data ##
config.Data.inputDataset   = '/SinglePhoton/Run2017B-PromptReco-v1/MINIAOD'
config.Data.lumiMask       = 'dcs2017.json'
config.Data.unitsPerJob    = 1000000
config.Data.splitting      = 'EventAwareLumiBased' 
#config.Data.totalUnits     = 1000

## Output Data ##
config.Data.publication   = False
config.Site.storageSite   = 'T2_CH_CERN'
config.Data.outLFNDirBase = '/store/group/phys_exotica/displacedPhotons/'
