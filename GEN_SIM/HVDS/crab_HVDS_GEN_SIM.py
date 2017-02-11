from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'HVDS_13TeV_GEN_SIM'
config.General.workArea = 'crab_projects'

config.JobType.pluginName = 'PrivateMC'
config.JobType.psetName   = 'HVDS_13TeV_GEN_SIM.py'

config.Data.outputPrimaryDataset = 'HVDS_13TeV_GEN_SIM'
config.Data.splitting = 'EventBased'
config.Data.unitsPerJob = 100
NJOBS = 500
config.Data.totalUnits = config.Data.unitsPerJob * NJOBS
config.Data.outLFNDirBase = '/store/user/kmcdermo/' # or '/store/group/<subdir>'
config.Data.publication = True
config.Data.publishDBS = 'phys03'
config.Data.outputDatasetTag = 'HVDS_13TeV_GEN_SIM'

config.Site.storageSite = 'T2_CH_CERN'

