from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'GMSB_L180_Ctau2000_userHLT_customiseTiming_RAW2DIGI_L1Reco_RECO-AODSIM-v1'
config.General.workArea = 'crab_projects'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'step2_customiseTiming_RAW2DIGI_L1Reco_RECO-AODSIM.py'

config.Data.inputDataset = '/GMSB_L180_Ctau2000_Pythia8_13TeV_cff_py_GEN_SIM_1810/kmcdermo-GMSB_L180_Ctau2000_userHLT_DIGI_L1_DIGI2RAW_HLT_PU-RAWSIMv1-4763f36c0c6ad7117eaff44776e31715/USER'
config.Data.inputDBS = 'phys03'
config.Data.splitting = 'FileBased'
config.Data.unitsPerJob = 5
config.Data.publication = True
config.Data.publishDBS = 'phys03'
config.Data.outputDatasetTag = 'GMSB_L180_Ctau2000_userHLT_customiseTiming_RAW2DIGI_L1Reco_RECO-AODSIM-v1'

config.Site.storageSite = 'T2_CH_CERN'

