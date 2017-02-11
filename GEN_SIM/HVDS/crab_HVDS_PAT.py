from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'HVDS_PAT-v1'
config.General.workArea    = 'crab_projects'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName   = 'step3_PAT.py'

config.Data.inputDataset = '/HVDS_13TeV_GEN_SIM/kmcdermo-HVDS_RAW2DIGI_L1Reco_RECO-v1-9162a384de1881d24894df196092366e/USER'
config.Data.inputDBS     = 'phys03'
config.Data.splitting    = 'FileBased'
config.Data.unitsPerJob  = 5
config.Data.publication  = True
config.Data.publishDBS   = 'phys03'
config.Data.outputDatasetTag = 'HVDS_PAT-v1'

config.Site.storageSite = 'T2_CH_CERN'

