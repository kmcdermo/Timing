# Auto generated configuration file
# using: 
# Revision: 1.19 
# Source: /local/reps/CMSSW/CMSSW/Configuration/Applications/python/ConfigBuilder.py,v 
# with command line options: reHLT -s HLT:hltdev:/users/kmcdermo/TargetPath/HLTDisplacedPhotonHT/V6 --processName userHLT --data --datatier MINIAOD --eventcontent MINIAOD --era Run2_2016 --conditions auto:run2_hlt_GRun --filein /store/data/Run2016H/SinglePhoton/MINIAOD/PromptReco-v2/000/284/035/00000/D2866817-459F-E611-A40C-02163E014156.root --secondfilein root://eoscms.cern.ch//eos/cms/store/group/phys_higgs/cmshgg/soffi/SinglePhotonRunHRaw/6CC3EF80-E29E-E611-B455-02163E011F8F.root --fileout file:singlephoton_reHLT.root -n 10 --no_exec

import FWCore.ParameterSet.Config as cms
import FWCore.PythonUtilities.LumiList as LumiList  
import FWCore.ParameterSet.Types as CfgTypes  

from Configuration.StandardSequences.Eras import eras

process = cms.Process('userHLT',eras.Run2_2016)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_AutoFromDBCurrent_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(10)
)

# Input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('/store/data/Run2016H/SinglePhoton/MINIAOD/PromptReco-v2/000/284/035/00000/D2866817-459F-E611-A40C-02163E014156.root'),
    secondaryFileNames = cms.untracked.vstring('root://eoscms.cern.ch//eos/cms/store/group/phys_higgs/cmshgg/soffi/SinglePhotonRunHRaw/6CC3EF80-E29E-E611-B455-02163E011F8F.root')
)

process.source.lumisToProcess = CfgTypes.untracked(CfgTypes.VLuminosityBlockRange())  
JSONfile = '/afs/cern.ch/user/k/kmcdermo/public/small.json'
myLumis = LumiList.LumiList(filename = JSONfile).getCMSSWString().split(',')  
process.source.lumisToProcess.extend(myLumis)                              

process.options = cms.untracked.PSet(

)

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('reHLT nevts:10'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

# Output definition

process.MINIAODoutput = cms.OutputModule("PoolOutputModule",
    compressionAlgorithm = cms.untracked.string('LZMA'),
    compressionLevel = cms.untracked.int32(4),
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('MINIAOD'),
        filterName = cms.untracked.string('')
    ),
    dropMetaData = cms.untracked.string('ALL'),
    eventAutoFlushCompressedSize = cms.untracked.int32(15728640),
    fastCloning = cms.untracked.bool(False),
    fileName = cms.untracked.string('file:singlephoton_reHLT.root'),
    outputCommands = process.MINIAODEventContent.outputCommands,
    overrideInputFileSplitLevels = cms.untracked.bool(True)
)

# Additional output definition

# Other statements
import HLTrigger.Configuration.Utilities
process.loadHltConfiguration("hltdev:/users/kmcdermo/TargetPath/HLTDisplacedPhotonHT/V6",type='GRun')
from HLTrigger.Configuration.CustomConfigs import ProcessName
process = ProcessName(process)

from Configuration.AlCa.GlobalTag import GlobalTag
#process.GlobalTag = GlobalTag(process.GlobalTag, '80X_dataRun2_Prompt_v14', '')
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_hlt_GRun', '')

# Path and EndPath definitions
process.endjob_step = cms.EndPath(process.endOfProcess)
process.MINIAODoutput_step = cms.EndPath(process.MINIAODoutput)

# Schedule definition
process.schedule = cms.Schedule()
process.schedule.extend(process.HLTSchedule)
process.schedule.extend([process.endjob_step,process.MINIAODoutput_step])


