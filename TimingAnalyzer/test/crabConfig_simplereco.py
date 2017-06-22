from CRABClient.UserUtilities import config
config = config()

## Name of output directory ##
config.General.requestName = 'SimpleRECOPhotons_2017B'
config.General.workArea    = 'crab_projects'

## Input analyzer pset ## 
config.JobType.pluginName  = 'Analysis'
config.JobType.psetName    = 'simplerecotree.py'

## Input Data ##
config.Data.inputDataset   = '/SinglePhoton/Run2017B-PromptReco-v1/AOD'
config.Data.lumiMask       = 'dcsonly_json.txt'
config.Data.unitsPerJob    = 100000
config.Data.splitting      = 'EventAwareLumiBased' 

## Output Data ##
config.Data.publication   = False
config.Site.storageSite   = 'T2_CH_CERN'
