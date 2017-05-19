# Auto generated configuration file
# using: 
# Revision: 1.19 
# Source: /local/reps/CMSSW/CMSSW/Configuration/Applications/python/ConfigBuilder.py,v 
# with command line options: ReRECO --data --era Run2_2016 --runUnscheduled --customise Configuration/DataProcessing/RecoTLR.customisePostEra_Run2_2016,RecoTracker/Configuration/customizeMinPtForHitRecoveryInGluedDet.customizeHitRecoveryInGluedDetOn --process reRECO --filtername RECOfromRECO -s RECO --eventcontent RECO --datatier RECO --conditions 80X_dataRun2_2016SeptRepro_v4 --filein file:/afs/cern.ch/work/k/kmcdermo/files/RECO/test_2016B.root --fileout file:rereco_test.root --no_exec -n 100
import FWCore.ParameterSet.Config as cms

# with --era Run2_2016 --> RECO output module RP for totem not found
#from Configuration.StandardSequences.Eras import eras
#process = cms.Process('reRECO',eras.Run2_2016)

process = cms.Process('reRECO')

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff')
process.load('Configuration.StandardSequences.Reconstruction_Data_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100)
)

# include cpu timing
process.Timing = cms.Service("Timing",
  summaryOnly = cms.untracked.bool(False),
  useJobReport = cms.untracked.bool(False)
)

# Input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('file:/afs/cern.ch/work/k/kmcdermo/public/files/RECO/test_sph_2016C.root'),
    secondaryFileNames = cms.untracked.vstring()
)

process.options = cms.untracked.PSet(
    allowUnscheduled = cms.untracked.bool(True)
)

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('ReRECO nevts:100'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

#ReRECO customization

from RecoParticleFlow.PFClusterProducer.particleFlowRecHitECAL_cfi import *
process.particleFlowOOTRecHitECAL = process.particleFlowRecHitECAL.clone()

process.particleFlowOOTRecHitECAL.producers[0].qualityTests[1].timingCleaning = cms.bool(False) 
process.particleFlowOOTRecHitECAL.producers[1].qualityTests[1].timingCleaning = cms.bool(False)

from RecoParticleFlow.PFClusterProducer.particleFlowClusterECALUncorrected_cfi import *
process.particleFlowClusterOOTECALUncorrected = process.particleFlowClusterECALUncorrected.clone()
process.particleFlowClusterOOTECALUncorrected.recHitsSource = cms.InputTag("particleFlowOOTRecHitECAL")

from RecoParticleFlow.PFClusterProducer.particleFlowClusterECAL_cfi import *
process.particleFlowClusterOOTECAL = process.particleFlowClusterECAL.clone()
process.particleFlowClusterOOTECAL.inputECAL = cms.InputTag("particleFlowClusterOOTECALUncorrected")

from RecoEcal.EgammaClusterProducers.particleFlowSuperClusterECAL_cfi import *
process.particleFlowSuperClusterOOTECAL = process.particleFlowSuperClusterECAL.clone()
process.particleFlowSuperClusterOOTECAL.PFClusters = cms.InputTag("particleFlowClusterOOTECAL")
process.particleFlowSuperClusterOOTECAL.ESAssociation = cms.InputTag("particleFlowClusterOOTECAL")
process.particleFlowSuperClusterOOTECAL.PFBasicClusterCollectionBarrel = cms.string("particleFlowBasicClusterOOTECALBarrel")
process.particleFlowSuperClusterOOTECAL.PFSuperClusterCollectionBarrel = cms.string("particleFlowSuperClusterOOTECALBarrel")
process.particleFlowSuperClusterOOTECAL.PFBasicClusterCollectionEndcap = cms.string("particleFlowBasicClusterOOTECALEndcap")
process.particleFlowSuperClusterOOTECAL.PFSuperClusterCollectionEndcap = cms.string("particleFlowSuperClusterOOTECALEndcap")
process.particleFlowSuperClusterOOTECAL.PFBasicClusterCollectionPreshower = cms.string("particleFlowBasicClusterOOTECALPreshower")
process.particleFlowSuperClusterOOTECAL.PFSuperClusterCollectionEndcapWithPreshower = cms.string("particleFlowSuperClusterOOTECALEndcapWithPreshower")

from RecoEgamma.EgammaPhotonProducers.photonCore_cfi import *
from RecoEgamma.EgammaPhotonProducers.photons_cfi import *
process.mustacheOOTPhotonCore = process.photonCore.clone()
process.mustacheOOTPhotonCore.scHybridBarrelProducer = cms.InputTag("particleFlowSuperClusterOOTECAL:particleFlowSuperClusterOOTECALBarrel")
process.mustacheOOTPhotonCore.scIslandEndcapProducer = cms.InputTag("particleFlowSuperClusterOOTECAL:particleFlowSuperClusterOOTECALEndcapWithPreshower")
process.mustacheOOTPhotonCore.conversionProducer = cms.InputTag("conversions")
process.mustacheOOTPhotons = process.photons.clone()
process.mustacheOOTPhotons.photonCoreProducer = cms.InputTag('mustacheOOTPhotonCore')
process.mustacheOOTPhotons.isOOTCollection = cms.bool(True)

process.OOTPhotonSequence = cms.Sequence(process.particleFlowOOTRecHitECAL*process.particleFlowClusterOOTECALUncorrected*process.particleFlowClusterOOTECAL*process.particleFlowSuperClusterOOTECAL*process.mustacheOOTPhotonCore*process.mustacheOOTPhotons)

process.particleFlowEGammaFull += cms.Sequence(process.OOTPhotonSequence)

process.RecoOOTPhoton = cms.untracked.PSet( 
    outputCommands = cms.untracked.vstring( 
        "keep *_particleFlowClusterOOTECAL_*_*", 
        "keep *_particleFlowSuperClusterOOTECAL_*_*",
        "keep *_mustacheOOTPhotonCore_*_*", 
        "keep *_mustacheOOTPhotons_*_*" 
))

# Output definition
process.RECOoutput = cms.OutputModule("PoolOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('RECO'),
        filterName = cms.untracked.string('RECOfromRECO')
    ),
    eventAutoFlushCompressedSize = cms.untracked.int32(5242880),
    fileName = cms.untracked.string('file:rereco_sph_2016C.root'),
    outputCommands = process.RECOEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)

process.RECOoutput.outputCommands.extend(process.RecoOOTPhoton.outputCommands)

# Additional output definition

# Other statements
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '80X_dataRun2_2016SeptRepro_v4', '')

# Path and EndPath definitions
process.reconstruction_step = cms.Path(process.reconstruction_fromRECO)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.RECOoutput_step = cms.EndPath(process.RECOoutput)

# Schedule definition
process.schedule = cms.Schedule(process.reconstruction_step,process.endjob_step,process.RECOoutput_step)

# customisation of the process.

# Automatic addition of the customisation function from Configuration.DataProcessing.RecoTLR
from Configuration.DataProcessing.RecoTLR import customisePostEra_Run2_2016 

#call to customisation function customisePostEra_Run2_2016 imported from Configuration.DataProcessing.RecoTLR
process = customisePostEra_Run2_2016(process)

# Automatic addition of the customisation function from RecoTracker.Configuration.customizeMinPtForHitRecoveryInGluedDet
from RecoTracker.Configuration.customizeMinPtForHitRecoveryInGluedDet import customizeHitRecoveryInGluedDetOn 

#call to customisation function customizeHitRecoveryInGluedDetOn imported from RecoTracker.Configuration.customizeMinPtForHitRecoveryInGluedDet
process = customizeHitRecoveryInGluedDetOn(process)

# End of customisation functions
#do not add changes to your config after this point (unless you know what you are doing)
from FWCore.ParameterSet.Utilities import convertToUnscheduled
process=convertToUnscheduled(process)
from FWCore.ParameterSet.Utilities import cleanUnscheduled
process=cleanUnscheduled(process)

