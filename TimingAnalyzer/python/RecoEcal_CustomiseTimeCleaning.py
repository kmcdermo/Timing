import FWCore.ParameterSet.Config as cms

def customiseTimeCleaning(process):
   process.particleFlowRecHitECAL.producers[0].qualityTests[1].timingCleaning = cms.bool(False)
   process.particleFlowRecHitECAL.producers[1].qualityTests[1].timingCleaning = cms.bool(False)

   return process
