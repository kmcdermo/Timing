# cmsenv
# git cms-addpkg RecoEcal/Configuration
# cp RecoEcal_CustomiseTimeCleaning.py ${CMSSW_BASE}/src/RecoEcal/Configuration/python

import FWCore.ParameterSet.Config as cms

def customiseTimeCleaning(process):
   process.particleFlowRecHitECAL.producers[0].qualityTests[1].timingCleaning = cms.bool(False)
   process.particleFlowRecHitECAL.producers[1].qualityTests[1].timingCleaning = cms.bool(False)

   return process
