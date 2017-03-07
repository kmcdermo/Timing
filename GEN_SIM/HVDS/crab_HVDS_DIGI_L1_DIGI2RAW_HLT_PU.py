from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'HVDS_ctau1000_userHLT_DIGI_L1_DIGI2RAW_HLT_PU-RASWIM-v1'
config.General.workArea    = 'crab_projects'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName   = 'step1_userHLT_DIGI_L1_DIGI2RAW_HLT_PU-RAWSIM.py'

config.Data.inputDataset = '/HVDS_13TeV_ctau1000_GEN_SIM/kmcdermo-HVDS_13TeV_ctau1000_GEN_SIM-bbc864f2c738ac52d65933dafb010538/USER'
                          #'/HVDS_13TeV_GEN_SIM/kmcdermo-HVDS_13TeV_GEN_SIM-6da5a015ddbab982ec1ff08a905ba73c/USER'
config.Data.inputDBS     = 'phys03'
config.Data.splitting    = 'FileBased'
config.Data.unitsPerJob  = 1
config.Data.publication  = True
config.Data.publishDBS   = 'phys03'
config.Data.outputDatasetTag = 'HVDS_ctau1000_userHLT_DIGI_L1_DIGI2RAW_HLT_PU-RAWSIM-v1'

config.Site.storageSite = 'T2_CH_CERN'
