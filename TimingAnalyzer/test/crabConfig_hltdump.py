from CRABClient.UserUtilities import config
config = config()

## Name of output directory ##
config.General.requestName = 'HLTCheck_063017-v1'
config.General.workArea    = 'crab_projects'

## Input analyzer pset ## 
config.JobType.pluginName  = 'Analysis'
config.JobType.psetName    = 'hltdump.py'

## Input Data ##
config.Data.inputDataset   = '/SinglePhoton/Run2017B-PromptReco-v1/MINIAOD'
config.Data.lumiMask       = 'hcal_good_gte297467.txt'
config.Data.unitsPerJob    = 100000
config.Data.splitting      = 'EventAwareLumiBased' 

## Output Data ##
config.Data.publication   = False
config.Site.storageSite   = 'T2_CH_CERN'
config.Data.outLFNDirBase = '/store/group/phys_exotica/displacedPhotons/'
