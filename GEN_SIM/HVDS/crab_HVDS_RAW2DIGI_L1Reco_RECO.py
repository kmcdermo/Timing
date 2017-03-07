from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'HVDS_userHLT_customiseTiming_RAW2DIGI_L1Reco_RECO-AODSIM-v1'
config.General.workArea    = 'crab_projects'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName   = 'step2_customiseTiming_RAW2DIGI_L1Reco_RECO-AODSIM.py'

config.Data.inputDataset = '/HVDS_13TeV_ctau1000_GEN_SIM/kmcdermo-HVDS_userHLT_DIGI_L1_DIGI2RAW_HLT_PU-RAWSIM-v1-4763f36c0c6ad7117eaff44776e31715/USER'
config.Data.inputDBS     = 'phys03'
config.Data.splitting    = 'FileBased'
config.Data.unitsPerJob  = 5
config.Data.publication  = True
config.Data.publishDBS   = 'phys03'
config.Data.outputDatasetTag = 'HVDS_userHLT_customiseTiming_RAW2DIGI_L1Reco_RECO-AODSIM-v1'

config.Site.storageSite = 'T2_CH_CERN'

