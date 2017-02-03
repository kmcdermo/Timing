# Auto generated configuration file
# using: 
# Revision: 1.19 
# Source: /local/reps/CMSSW/CMSSW/Configuration/Applications/python/ConfigBuilder.py,v 
# with command line options: step4 --datatier AOD --process reRECO --conditions auto:run2_data --filtername RECOfromRECO -s RECO,EI --eventcontent AOD --no_exec -n -1 --filein root://eoscms.cern.ch//store/data/Run2016B/SingleMuon/RECO/PromptReco-v2/000/273/150/00000/1C609FC2-D919-E611-ACFB-02163E011C02.root --fileout file:rereco.root
import FWCore.ParameterSet.Config as cms

process = cms.Process('reRECO')

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Reconstruction_Data_cff')
process.load('CommonTools.ParticleFlow.EITopPAG_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)

# Input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('file:/afs/cern.ch/work/k/kmcdermo/files/RECO/test_2016B.root'),
    secondaryFileNames = cms.untracked.vstring()
)

process.options = cms.untracked.PSet(

)

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('step4 nevts:-1'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

# Output definition

process.AODoutput = cms.OutputModule("PoolOutputModule",
    compressionAlgorithm = cms.untracked.string('LZMA'),
    compressionLevel = cms.untracked.int32(4),
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('AOD'),
        filterName = cms.untracked.string('RECOfromRECO')
    ),
    eventAutoFlushCompressedSize = cms.untracked.int32(15728640),
    fileName = cms.untracked.string('file:rereco.root'),
    outputCommands = process.AODEventContent.outputCommands,
    SelectEvents = cms.untracked.PSet(
      SelectEvents = cms.vstring("eventinterpretaion_step")
    )
)

# Additional output definition

# Other statements
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_data', '')

process.particleFlowRecHitECAL.producers[0].qualityTests[1].timingCleaning = cms.bool(False)
process.particleFlowRecHitECAL.producers[1].qualityTests[1].timingCleaning = cms.bool(False)

process.highMET = cms.EDFilter('PtMinCandViewSelector',
    src = cms.InputTag('pfMet'),
    ptMin = cms.double(120)
)

process.highMETFilter = cms.EDFilter('CandViewCountFilter',
    src = cms.InputTag('highMET'),
    minNumber = cms.uint32(1)
)

process.highMETFilterSequence = cms.Sequence(
    process.highMET +
    process.highMETFilter
)

# Path and EndPath definitions
process.reconstruction_step = cms.Path(process.highMETFilterSequence + process.reconstruction_fromRECO)
process.eventinterpretaion_step = cms.Path(process.highMETFilterSequence + process.EIsequence)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.AODoutput_step = cms.EndPath(process.AODoutput)

# Schedule definition
process.schedule = cms.Schedule(process.reconstruction_step,process.eventinterpretaion_step,process.endjob_step,process.AODoutput_step)
