from CRABClient.UserUtilities import config
config = config()

config.General.requestName   = 'HVDS_ctau1000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v2'
config.General.workArea      = 'crab_projects'

config.JobType.pluginName    = 'Analysis'
config.JobType.psetName      = 'step1_reHLT-DisplacedPhoton_8_0_24_V11_DIGI_L1_DIGI2RAW_HLT_PU-MINIAODSIM.py'
config.JobType.maxMemoryMB   = 6000
config.JobType.maxJobRuntimeMin = 2630

config.Data.inputDataset          = '/HVDS_13TeV_ctau1000_GEN_SIM/kmcdermo-HVDS_ctau1000_userHLT_customiseTiming_PAT-MINIAODSIM-v1-77a214dfa1bce5cc5ac171402a5768dd/USER'
config.Data.secondaryInputDataset = '/HVDS_13TeV_ctau1000_GEN_SIM/kmcdermo-HVDS_13TeV_ctau1000_GEN_SIM-bbc864f2c738ac52d65933dafb010538/USER'

config.Data.inputDBS         = 'phys03'
config.Data.splitting        = 'FileBased'
config.Data.unitsPerJob      = 1
config.Data.publication      = True
config.Data.publishDBS       = 'phys03'
config.Data.outputDatasetTag = 'HVDS_ctau1000_reHLT-DisplacedPhoton_8_0_24_V11_customiseTiming_PAT-MINIAODSIM-v2'

config.Site.storageSite      = 'T2_CH_CERN'
