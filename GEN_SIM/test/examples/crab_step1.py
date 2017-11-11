from CRABClient.UserUtilities import config
config = config()

config.General.requestName   = 'GMSB_L200TeV_CTau400cm_933_step1'
config.General.workArea      = 'crab_projects'

config.JobType.pluginName    = 'Analysis'
config.JobType.psetName      = 'step1_DIGIPREMIX_S2_DATAMIX_L1_DIGI2RAW_HLT.py'
config.JobType.numCores      = 4

config.Data.inputDataset     = '/GMSB_L200TeV_CTau400cm_930/kmcdermo-GMSB_L200TeV_CTau400cm_930_step0-e119d546c9be4be42fa727e07fd85d93/USER'
config.Data.inputDBS         = 'phys03'
config.Data.splitting        = 'FileBased'
config.Data.unitsPerJob      = 2

config.Data.publication      = True
config.Data.publishDBS       = 'phys03'
config.Data.outputDatasetTag = config.General.requestName

config.Site.storageSite      = 'T2_CH_CERN'
config.Data.outLFNDirBase    = '/store/user/kmcdermo'
