from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'HVDS_ctau1000_userHLT_customiseTiming_PAT-MINIAODSIM-v1'
config.General.workArea    = 'crab_projects'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName   = 'step3_PAT-MINIAODSIM.py'

config.Data.inputDataset = '/HVDS_13TeV_ctau1000_GEN_SIM/kmcdermo-HVDS_userHLT_customiseTiming_RAW2DIGI_L1Reco_RECO-AODSIM-v1-3781de709ac205a731fcece5ee910a35/USER'
config.Data.inputDBS     = 'phys03'
config.Data.splitting    = 'FileBased'
config.Data.unitsPerJob  = 5
config.Data.publication  = True
config.Data.publishDBS   = 'phys03'
config.Data.outputDatasetTag = 'HVDS_ctau1000_userHLT_customiseTiming_PAT-MINIAODSIM-v1'

config.Site.storageSite = 'T2_CH_CERN'

