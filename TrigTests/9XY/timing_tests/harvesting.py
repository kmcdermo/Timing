import FWCore.ParameterSet.Config as cms
process = cms.Process('HARVESTING')

# read all the DQMIO files produced by the previous jobs
process.source = cms.Source("DQMRootSource",
fileNames = cms.untracked.vstring( "file:DQM.root") )

# DQMStore service
process.load('DQMServices.Core.DQMStore_cfi')

# FastTimerService client
process.load('HLTrigger.Timer.fastTimerServiceClient_cfi')
process.fastTimerServiceClient.dqmPath = "HLT/TimerService"

# DQM file saver
process.load('DQMServices.Components.DQMFileSaver_cfi')
process.dqmSaver.workflow = "/HLT/FastTimerService/All"

process.DQMFileSaverOutput = cms.EndPath( process.fastTimerServiceClient + process.dqmSaver )

process.DQMStore.collateHistograms  = True
process.dqmSaver.saveByRun = cms.untracked.int32(-1) ## multirun harvesting
process.dqmSaver.saveAtJobEnd   = True
process.dqmSaver.forceRunNumber = 999999

process.load( "HLTrigger.Timer.FastTimerService_cfi" )
process.FastTimerService.enableDQMbyLumiSection = False

