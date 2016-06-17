from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'dyll_smear'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'tree.py'

# MC samples
config.Data.inputDataset = '/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/RunIIFall15MiniAODv2-PU25nsData2015v1_76X_mcRun2_asymptotic_v12-v1/MINIAODSIM'

# Data samples
#config.Data.inputDataset = '/DoubleEG/Run2015D-16Dec2015-v2/MINIAOD'
#config.Data.inputDataset = '/SingleElectron/Run2015D-16Dec2015-v1/MINIAOD'
#config.Data.lumiMask = 'rereco2015.txt'

#config.Data.ignoreLocality = True
config.Data.splitting = 'EventAwareLumiBased' #'LumiBased' 
config.Data.unitsPerJob = 100000

config.Data.publication = False
config.Site.storageSite = 'T2_CH_CERN'
