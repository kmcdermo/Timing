from CRABClient.UserUtilities import config
config = config()

config.General.requestName   = 'GMSB_L200TeV_CTau400cm_933_step3'
config.General.workArea      = 'crab_projects'

config.JobType.pluginName    = 'Analysis'
config.JobType.psetName      = 'step3_PAT.py'
config.JobType.numCores      = 1

config.Data.inputDataset     = '/GMSB_L200TeV_CTau400cm_930/kmcdermo-GMSB_L200TeV_CTau400cm_930_step2-v3-1785bb8be6635fde1716906ff042c57d/USER'
config.Data.inputDBS         = 'phys03'
config.Data.splitting        = 'FileBased'
config.Data.unitsPerJob      = 10

config.Data.publication      = True
config.Data.publishDBS       = 'phys03'
config.Data.outputDatasetTag = config.General.requestName

config.Site.storageSite      = 'T2_CH_CERN'
config.Data.outLFNDirBase    = '/store/user/kmcdermo'
