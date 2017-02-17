from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'HVDS_DIGI_L1_DIGI2RAW_HLT_PU-v1'
config.General.workArea    = 'crab_projects'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName   = 'step1_DIGI_L1_DIGI2RAW_HLT_PU.py'

config.Data.inputDataset = '/HVDS_13TeV_GEN_SIM/kmcdermo-HVDS_13TeV_GEN_SIM-6da5a015ddbab982ec1ff08a905ba73c/USER'
config.Data.inputDBS     = 'phys03'
config.Data.splitting    = 'FileBased'
config.Data.unitsPerJob  = 1
config.Data.publication  = True
config.Data.publishDBS   = 'phys03'
config.Data.outputDatasetTag = 'HVDS_DIGI_L1_DIGI2RAW_HLT_PU-v1'

config.Site.storageSite = 'T2_CH_CERN'

