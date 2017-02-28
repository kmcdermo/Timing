from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'GMSB_L180_Ctau2000_newHLT_DIGI_L1_DIGI2RAW_HLT_PU-v1'
config.General.workArea = 'crab_projects'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'step1_newHLT_DIGI_L1_DIGI2RAW_HLT_PU.py'

config.Data.inputDataset = '/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/soffi-GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810-0164080974557022b1efd00e61df94f7/USER'
config.Data.inputDBS = 'phys03'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 1
config.Data.publication = True
config.Data.publishDBS = 'phys03'
config.Data.outputDatasetTag = 'GMSB_L180_Ctau2000_newHLT_DIGI_L1_DIGI2RAW_HLT_PU-v1'

config.Site.storageSite = 'T2_CH_CERN'

