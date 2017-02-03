from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'GMSB_L180_Ctau6000_DIGI_L1_DIGI2RAW_HLT_PU-v1'
config.General.workArea = 'crab_projects'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'step1_DIGI_L1_DIGI2RAW_HLT_PU.py'

config.Data.inputDataset = '/GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM/kmcdermo-GMSB_L180_Ctau6000_Pythia8_13TeV_cff_py_GEN_SIM-bd66f258b1e0722589af2e4f7e9bac46/USER'
config.Data.inputDBS = 'phys03'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1
config.Data.publication = True
config.Data.publishDBS = 'phys03'
config.Data.outputDatasetTag = 'GMSB_L180_Ctau6000_DIGI_L1_DIGI2RAW_HLT_PU-v1'

config.Site.storageSite = 'T2_CH_CERN'

