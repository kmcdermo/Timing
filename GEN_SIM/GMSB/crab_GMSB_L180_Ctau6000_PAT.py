from CRABClient.UserUtilities import config
config = config()

config.General.requestName   = 'GMSB_L180_Ctau6000_PAT-v1'
config.General.workArea      = 'crab_projects'

config.JobType.pluginName    = 'Analysis'
config.JobType.psetName      = 'step3_PAT.py'

config.Data.inputDataset     = '/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/kmcdermo-GMSB_L180_Ctau6000_RAW2DIGI_L1Reco_RECO-v1-66352319364a24b789f63792a51d31b1/USER'
config.Data.inputDBS         = 'phys03'
config.Data.splitting        = 'FileBased'
config.Data.unitsPerJob      = 5
config.Data.publication      = True
config.Data.publishDBS       = 'phys03'
config.Data.outputDatasetTag = 'GMSB_L180_Ctau6000_PAT-v1'

config.Site.storageSite      = 'T2_CH_CERN'
