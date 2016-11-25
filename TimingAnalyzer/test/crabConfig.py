from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'DoubleEG_ReRecoBv3-v1'
 
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'zeetree.py'

# MC samples
#config.Data.inputDataset = '/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/RunIISpring16MiniAODv2-PUSpring16RAWAODSIM_reHLT_80X_mcRun2_asymptotic_v14-v1/MINIAODSIM'
#config.Data.unitsPerJob = 250000

############################################
#                Data samples              #
############################################
# /DoubleEG/Run2016B-23Sep2016-v3/MINIAOD --> GT: 80X_dataRun2_2016SeptRepro_v4
# /DoubleEG/Run2016C-23Sep2016-v1/MINIAOD --> GT: 80X_dataRun2_2016SeptRepro_v4
# /DoubleEG/Run2016D-23Sep2016-v1/MINIAOD --> GT: 80X_dataRun2_2016SeptRepro_v4
# /DoubleEG/Run2016E-23Sep2016-v1/MINIAOD --> GT: 80X_dataRun2_2016SeptRepro_v4
# /DoubleEG/Run2016F-23Sep2016-v1/MINIAOD --> GT: 80X_dataRun2_2016SeptRepro_v4
# /DoubleEG/Run2016G-23Sep2016-v1/MINIAOD --> GT: 80X_dataRun2_2016SeptRepro_v4
# /DoubleEG/Run2016H-PromptReco-v2/MINIAOD --> GT: 80X_dataRun2_Prompt_v14
# /DoubleEG/Run2016H-PromptReco-v3/MINIAOD --> GT: 80X_dataRun2_Prompt_v14

config.Data.inputDataset = '/DoubleEG/Run2016B-23Sep2016-v3/MINIAOD'
config.Data.lumiMask = 'rereco2016.txt'
config.Data.unitsPerJob = 500000

#config.Data.ignoreLocality = True
config.Data.splitting = 'EventAwareLumiBased' #'LumiBased' 

config.Data.publication = False
config.Site.storageSite = 'T2_CH_CERN'
