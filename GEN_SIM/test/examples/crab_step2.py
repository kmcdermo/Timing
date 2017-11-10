from CRABClient.UserUtilities import config
config = config()

config.General.requestName   = 'GMSB_L200TeV_CTau400cm_933_step2'
config.General.workArea      = 'crab_projects'

config.JobType.pluginName    = 'Analysis'
config.JobType.psetName      = 'step2_RAW2DIGI_RECO_EI.py'
config.JobType.numCores      = 1

config.Data.inputDataset     = '/GMSB_L200TeV_CTau400cm_930/kmcdermo-GMSB_L200TeV_CTau400cm_930_step1-4da9d00944ce71116536925e09c78c8e/USER'
config.Data.inputDBS         = 'phys03'
config.Data.splitting        = 'FileBased'
config.Data.unitsPerJob      = 1

config.Data.publication      = True
config.Data.publishDBS       = 'phys03'
config.Data.outputDatasetTag = config.General.requestName

config.Site.storageSite      = 'T2_CH_CERN'
config.Data.outLFNDirBase    = '/store/user/kmcdermo
