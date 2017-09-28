from CRABClient.UserUtilities import config
config = config()

## Name of output directory ##
config.General.requestName = 'HLT_DCS_SP_2017B-v2'
config.General.workArea    = 'crab_projects'

## Input analyzer pset ## 
config.JobType.pluginName  = 'Analysis'
config.JobType.psetName    = 'hltdump.py'
config.JobType.pyCfgParams = ['globalTag=92X_dataRun2_Prompt_v5','useOOTPhotons=False']
config.JobType.inputFiles  = ['HLTpaths.txt','HLTfilters.txt']

## Input Data ##
config.Data.inputDataset   = '/SinglePhoton/Run2017B-PromptReco-v2/MINIAOD'
config.Data.lumiMask       = 'golden_2017.json'
config.Data.unitsPerJob    = 1000000
config.Data.splitting      = 'EventAwareLumiBased' 
#config.Data.totalUnits     = 1000

## Output Data ##
config.Data.publication   = False
config.Site.storageSite   = 'T2_CH_CERN'
config.Data.outLFNDirBase = '/store/group/phys_exotica/displacedPhotons/'

### 2017 Datasets ###
# /SinglePhoton/Run2017A-PromptReco-v1/MINIAOD, GT: 92X_dataRun2_Prompt_v4, useOOTPhotons = false
# /SinglePhoton/Run2017A-PromptReco-v2/MINIAOD, GT: 92X_dataRun2_Prompt_v4, useOOTPhotons = false
# /SinglePhoton/Run2017A-PromptReco-v3/MINIAOD, GT: 92X_dataRun2_Prompt_v4, useOOTPhotons = false
# /SinglePhoton/Run2017B-PromptReco-v1/MINIAOD, GT: 92X_dataRun2_Prompt_v4, useOOTPhotons = false
# /SinglePhoton/Run2017B-PromptReco-v2/MINIAOD, GT: 92X_dataRun2_Prompt_v5, useOOTPhotons = false
# /SinglePhoton/Run2017C-PromptReco-v1/MINIAOD, GT: 92X_dataRun2_Prompt_v6, useOOTPhotons = true
# /SinglePhoton/Run2017C-PromptReco-v2/MINIAOD, GT: 92X_dataRun2_Prompt_v7, useOOTPhotons = true
# /SinglePhoton/Run2017C-PromptReco-v3/MINIAOD, GT: 92X_dataRun2_Prompt_v8, useOOTPhotons = true
# /SinglePhoton/Run2017D-PromptReco-v1/MINIAOD, GT: 92X_dataRun2_Prompt_v8, useOOTPhotons = true
# /SinglePhoton/Run2017E-PromptReco-v1/MINIAOD, GT: 92X_dataRun2_Prompt_v9, useOOTPhotons = true
