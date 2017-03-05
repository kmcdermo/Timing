from CRABClient.UserUtilities import config
config = config()

config.General.requestName   = 'GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1'
config.General.workArea      = 'crab_projects'

config.JobType.pluginName    = 'Analysis'
config.JobType.psetName      = 'step3_PAT-MINIAODSIM.py'

config.Data.inputDataset     = '/GMSB_L180_Ctau100_Pythia8_13TeV_cff_py_GEN_SIM_1810/kmcdermo-GMSB_L180_Ctau100_userHLT_customiseTiming_RAW2DIGI_L1Reco_RECO-AODSIM-v1-3781de709ac205a731fcece5ee910a35/USER'
config.Data.inputDBS         = 'phys03'
config.Data.splitting        = 'FileBased'
config.Data.unitsPerJob      = 5
config.Data.publication      = True
config.Data.publishDBS       = 'phys03'
config.Data.outputDatasetTag = 'GMSB_L180_Ctau100_userHLT_customiseTiming_PAT-MINIAODSIM-v1'

config.Site.storageSite      = 'T2_CH_CERN'
