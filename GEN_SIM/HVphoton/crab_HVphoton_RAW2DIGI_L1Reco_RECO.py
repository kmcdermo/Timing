from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'HVphoton_RAW2DIGI_L1Reco_RECO-v1'
config.General.workArea    = 'crab_projects'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName   = 'step2_RAW2DIGI_L1Reco_RECO.py'

config.Data.inputDataset = '/HVphoton_13TeV_GEN_SIM/kmcdermo-HVphoton_DIGI_L1_DIGI2RAW_HLT_PU-v1-19898e58c9c00509372f15bcc801ecbe/USER'
config.Data.inputDBS     = 'phys03'
config.Data.splitting    = 'FileBased'
config.Data.unitsPerJob  = 5
config.Data.publication  = True
config.Data.publishDBS   = 'phys03'
config.Data.outputDatasetTag = 'HVphoton_RAW2DIGI_L1Reco_RECO-v1'

config.Site.storageSite = 'T2_CH_CERN'

