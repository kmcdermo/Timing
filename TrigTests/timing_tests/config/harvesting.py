import FWCore.ParameterSet.Config as cms
process = cms.Process('HARVESTING')

# read all the DQMIO files produced by the previous jobs
process.source = cms.Source("DQMRootSource",
fileNames = cms.untracked.vstring( "file:DQMIO.root") )

# DQMStore service
process.load('DQMServices.Core.DQMStore_cfi')
process.load('Configuration.StandardSequences.DQMSaverAtJobEnd_cff') 

# FastTimerService client
process.load('HLTrigger.Timer.fastTimerServiceClient_cfi')
process.fastTimerServiceClient.dqmPath = "HLT/TimerService"

# DQM file saver
process.load('DQMServices.Components.DQMFileSaver_cfi')
process.dqmSaver.workflow = "/HLT/FastTimerService/All"
process.DQMFileSaverOutput = cms.EndPath( process.fastTimerServiceClient + process.dqmSaver )

process.dqmSaver.saveByRun = cms.untracked.int32(-1) ## multirun harvesting
process.dqmSaver.forceRunNumber = 999999
