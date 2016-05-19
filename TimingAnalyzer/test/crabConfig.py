from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'dataTime_singleEl'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'tree.py'

#config.Data.ignoreLocality = True
#config.Data.inputDataset = '/DoubleEG/Run2015D-16Dec2015-v2/MINIAOD'
config.Data.inputDataset = '/SingleElectron/Run2015D-16Dec2015-v1/MINIAOD'
config.Data.splitting = 'EventAwareLumiBased' #'LumiBased' 
config.Data.unitsPerJob = 500000

config.Data.lumiMask = 'rereco2015.txt'

config.Data.publication = False
config.Site.storageSite = 'T2_CH_CERN'
