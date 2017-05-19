from CRABClient.UserUtilities import config
config = config()

config.General.requestName   = 'SinglePhoton2016C-ReRECO_RECO-v2'
config.General.workArea      = 'crab_projects'

config.JobType.pluginName    = 'Analysis'
config.JobType.psetName      = 'ReRECO_RECO.py'

config.Data.outputPrimaryDataset = 'DoubleEG'
config.Data.userInputFiles   = ['/afs/cern.ch/work/k/kmcdermo/files/RECO/test_singlephoton_2016C.root'] #open('input_reco_files.txt').readlines()
config.Data.splitting        = 'FileBased'
config.Data.unitsPerJob      = 1
config.Data.totalUnits       = 1

config.Data.publication      = True
config.Data.outputDatasetTag = config.General.requestName

config.Site.storageSite      = 'T2_CH_CERN'
config.Site.whitelist        = ['T2_CH_CERN']
