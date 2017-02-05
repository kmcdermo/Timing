from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'HVphoton_DIGI_L1_DIGI2RAW_HLT_PU-v1'
config.General.workArea    = 'crab_projects'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName   = 'step1_DIGI_L1_DIGI2RAW_HLT_PU.py'

config.Data.inputDataset = '/HVphoton_13TeV_GEN_SIM/kmcdermo-HVphoton_13TeV_GEN_SIM-84087116fcd473a17911a03fe4d56595/USER'
config.Data.inputDBS     = 'phys03'
config.Data.splitting    = 'FileBased'
config.Data.unitsPerJob  = 1
config.Data.publication  = True
config.Data.publishDBS   = 'phys03'
config.Data.outputDatasetTag = 'HVphoton_DIGI_L1_DIGI2RAW_HLT_PU-v1'

config.Site.storageSite = 'T2_CH_CERN'

