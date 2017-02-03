from CRABClient.UserUtilities import config
config = config()

config.General.requestName   = 'ReRECO_PAT_CleanedECALRecHits-v1'
config.General.workArea      = 'crab_projects'

config.JobType.pluginName    = 'Analysis'
config.JobType.psetName      = 'ReRECO_RECO_PAT_cleaned.py'

config.Data.outputPrimaryDataset = 'DoubleEG'
config.Data.userInputFiles   = open('input_reco_files.txt').readlines()
config.Data.splitting        = 'FileBased'
config.Data.unitsPerJob      = 1
config.Data.totalUnits       = 22

config.Data.publication      = True
config.Data.outputDatasetTag = config.General.requestName

config.Site.storageSite      = 'T2_CH_CERN'
config.Site.whitelist        = ['T2_IT_Rome']
