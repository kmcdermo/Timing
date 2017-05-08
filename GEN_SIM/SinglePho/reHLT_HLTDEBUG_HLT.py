# Auto generated configuration file
# using: 
# Revision: 1.19 
# Source: /local/reps/CMSSW/CMSSW/Configuration/Applications/python/ConfigBuilder.py,v 
# with command line options: reHLT_HLTDEBUG -s HLT:hltdev:/users/kmcdermo/TargetPath/HLTDisplacedPhotonHT/V6 --processName userHLT --data --datatier HLTDEBUG --eventcontent HLTDEBUG --era Run2_2016 --conditions auto:run2_hlt_GRun --filein /store/data/Run2016H/SinglePhoton/MINIAOD/PromptReco-v2/000/284/035/00000/D2866817-459F-E611-A40C-02163E014156.root --secondfilein root://eoscms.cern.ch//eos/cms/store/group/phys_higgs/cmshgg/soffi/SinglePhotonRunHRaw/6CC3EF80-E29E-E611-B455-02163E011F8F.root --fileout file:singlephoton_reHLT.root -n 10 --no_exec

import FWCore.ParameterSet.Config as cms
#import FWCore.PythonUtilities.LumiList as LumiList  
#import FWCore.ParameterSet.Types as CfgTypes  

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
    input = cms.untracked.int32(-1)
)

# Input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('/store/data/Run2016H/SinglePhoton/MINIAOD/PromptReco-v2/000/284/035/00000/D2866817-459F-E611-A40C-02163E014156.root'),
    secondaryFileNames = cms.untracked.vstring('root://eoscms.cern.ch//eos/cms/store/group/phys_higgs/cmshgg/soffi/SinglePhotonRunHRaw/6CC3EF80-E29E-E611-B455-02163E011F8F.root')
)

# process.source.lumisToProcess = CfgTypes.untracked(CfgTypes.VLuminosityBlockRange())  
# JSONfile = '/afs/cern.ch/user/k/kmcdermo/public/small.json'
# myLumis = LumiList.LumiList(filename = JSONfile).getCMSSWString().split(',')  
# process.source.lumisToProcess.extend(myLumis)                              

# process.options = cms.untracked.PSet(
#     numberOfThreads = cms.untracked.uint32(4),
#     numberOfStreams = cms.untracked.uint32(4),
#     allowUnscheduled = cms.untracked.bool(True)
# )

# Production Info
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('reHLT_HLTDEBUG nevts:10'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

# Output definition

process.HLTDEBUGoutput = cms.OutputModule("PoolOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('HLTDEBUG'),
        filterName = cms.untracked.string('')
    ),
    eventAutoFlushCompressedSize = cms.untracked.int32(5242880),
    fileName = cms.untracked.string('root://eoscms.cern.ch//eos/cms/store/group/phys_exotica/displacedPhotons/SinglePhoton/test/reHLT.root'),
    outputCommands = process.HLTDEBUGEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)

# customized event content
from Timing.GEN_SIM.MINIAODEventContent_Extension_cff import MINIAODEventContent
process.HLTDEBUGEventContent.outputCommands.extend(MINIAODEventContent.outputCommands)

# Additional output definition

# Other statements
import HLTrigger.Configuration.Utilities
process.loadHltConfiguration("hltdev:/users/kmcdermo/TargetPath/HLTDisplacedPhotonHT/V6",type='GRun')
from HLTrigger.Configuration.CustomConfigs import ProcessName
process = ProcessName(process)

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_hlt_GRun', '')

# Path and EndPath definitions
process.endjob_step = cms.EndPath(process.endOfProcess)
process.HLTDEBUGoutput_step = cms.EndPath(process.HLTDEBUGoutput)

# Schedule definition
process.schedule = cms.Schedule()
process.schedule.extend(process.HLTSchedule)
process.schedule.extend([process.endjob_step,process.HLTDEBUGoutput_step])
