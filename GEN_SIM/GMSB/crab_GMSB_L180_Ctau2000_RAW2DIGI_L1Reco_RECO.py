from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'GMSB_L180_Ctau6000_newHLT_RAW2DIGI_L1Reco_RECO-v1'
config.General.workArea = 'crab_projects'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'step2_customise_RAW2DIGI_L1Reco_RECO.py'

config.Data.inputDataset = '/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/kmcdermo-GMSB_L180_Ctau2000_newHLT_DIGI_L1_DIGI2RAW_HLT_PU-v1-ba0862a5ff778aabb2ae124a2b73a560/USER'
config.Data.inputDBS = 'phys03'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 5
config.Data.publication = True
config.Data.publishDBS = 'phys03'
config.Data.outputDatasetTag = 'GMSB_L180_Ctau6000_newHLT_RAW2DIGI_L1Reco_RECO-v1'

config.Site.storageSite = 'T2_CH_CERN'

