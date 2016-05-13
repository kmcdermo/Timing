from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'singleElTime'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'tree.py'

#config.Data.ignoreLocality = True
#config.Data.inputDataset = '/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/RunIISpring15MiniAODv2-74X_mcRun2_asymptotic_v2-v1/MINIAODSIM'
config.Data.inputDataset = '/SingleElectron/Run2015D-16Dec2015-v1/MINIAOD'
config.Data.splitting = 'EventAwareLumiBased' #'LumiBased' 
config.Data.unitsPerJob = 50000 #50000

config.Data.lumiMask = 'rereco2015.txt'

config.Data.publication = False
config.Site.storageSite = 'T2_CH_CERN'
