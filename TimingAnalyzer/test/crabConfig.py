from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'DYll_Spring16'

config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'tree.py'

# MC samples
config.Data.inputDataset = '/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/RunIISpring16MiniAODv2-PUSpring16RAWAODSIM_reHLT_80X_mcRun2_asymptotic_v14-v1/MINIAODSIM'
config.Data.unitsPerJob = 250000

############################################
#                Data samples              #
# /DoubleEG/Run2016*-PromptReco-v*/MINIAOD #
############################################
# /DoubleEG/Run2016B-PromptReco-v2/MINIAOD
# /DoubleEG/Run2016C-PromptReco-v2/MINIAOD
# /DoubleEG/Run2016D-PromptReco-v2/MINIAOD
# /DoubleEG/Run2016E-PromptReco-v2/MINIAOD
# /DoubleEG/Run2016F-PromptReco-v1/MINIAOD
# /DoubleEG/Run2016G-PromptReco-v1/MINIAOD
# /DoubleEG/Run2016H-PromptReco-v2/MINIAOD # no json yet!

#config.Data.inputDataset = '/DoubleEG/Run2016G-PromptReco-v1/MINIAOD'
#config.Data.lumiMask = 'reco2016_noL1T-27p22ifb.txt'
#config.Data.unitsPerJob = 500000

#config.Data.ignoreLocality = True
config.Data.splitting = 'EventAwareLumiBased' #'LumiBased' 

config.Data.publication = False
config.Site.storageSite = 'T2_CH_CERN'
