from CRABClient.UserUtilities import config
config = config()

config.General.requestName = 'DoubleEG_reMAD_PRHv2-v1'
 
config.JobType.pluginName = 'Analysis'
config.JobType.psetName = 'zeetree.py'

# MC samples
#config.Data.inputDataset = '/DYJetsToLL_M-50_TuneCUETP8M1_13TeV-amcatnloFXFX-pythia8/RunIISpring16MiniAODv2-PUSpring16RAWAODSIM_reHLT_80X_mcRun2_asymptotic_v14-v1/MINIAODSIM'
#config.Data.unitsPerJob = 250000

############################################
#                Data samples              #
############################################
# /DoubleEG/Run2016B-03Feb2017_ver1-v1/MINIAOD --> NOT IN Golden JSON
# /DoubleEG/Run2016B-03Feb2017_ver2-v2/MINIAOD --> GT: 80X_dataRun2_2016SeptRepro_v7
# /DoubleEG/Run2016C-03Feb2017-v1/MINIAOD --> GT: 80X_dataRun2_2016SeptRepro_v7
# /DoubleEG/Run2016D-03Feb2017-v1/MINIAOD --> GT: 80X_dataRun2_2016SeptRepro_v7
# /DoubleEG/Run2016E-03Feb2017-v1/MINIAOD --> GT: 80X_dataRun2_2016SeptRepro_v7
# /DoubleEG/Run2016F-03Feb2017-v1/MINIAOD --> GT: 80X_dataRun2_2016SeptRepro_v7
# /DoubleEG/Run2016G-03Feb2017-v1/MINIAOD --> GT: 80X_dataRun2_2016SeptRepro_v7
# /DoubleEG/Run2016H-03Feb2017_ver2-v1/MINIAOD --> GT: 80X_dataRun2_Prompt_v16
# /DoubleEG/Run2016H-03Feb2017_ver3-v1/MINIAOD --> GT: 80X_dataRun2_Prompt_v16

config.Data.inputDataset = '/DoubleEG/Run2016H-03Feb2016_ver2-v1/MINIAOD'
config.Data.lumiMask = 'final2016json.txt'
config.Data.unitsPerJob = 500000

#config.Data.ignoreLocality = True
config.Data.splitting = 'EventAwareLumiBased' #'LumiBased' 

config.Data.publication = False
config.Site.storageSite = 'T2_CH_CERN'
