from CRABClient.UserUtilities import config
config = config()

base_name = 'GMSB_L200TeV_CTau400cm_933'
NJOBS     = 100

config.General.requestName       = base_name+'_step0'
config.General.workArea          = 'crab_projects'
config.General.transferLogs      = True

config.JobType.pluginName        = 'PrivateMC'
config.JobType.psetName          = 'step0_GEN_SIM.py'
config.JobType.numCores          = 4

config.Data.outputPrimaryDataset = base_name
config.Data.splitting            = 'EventBased'
config.Data.unitsPerJob          = 500
config.Data.totalUnits           = config.Data.unitsPerJob * NJOBS

config.Data.publication          = True
config.Data.publishDBS           = 'phys03'
config.Data.outputDatasetTag     = config.General.requestName

config.Site.storageSite          = 'T2_CH_CERN'
config.Data.outLFNDirBase        = '/store/user/kmcdermo/'
