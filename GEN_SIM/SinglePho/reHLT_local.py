# Auto generated configuration file
# using: 
# Revision: 1.19 
# Source: /local/reps/CMSSW/CMSSW/Configuration/Applications/python/ConfigBuilder.py,v 
# with command line options: reHLT_HLTDEBUG -s HLT:hltdev:/users/kmcdermo/TargetPath/HLTDisplacedPhotonHT/V6 --processName userHLT --data --datatier HLTDEBUG --eventcontent HLTDEBUG --era Run2_2016 --conditions auto:run2_hlt_GRun --filein /store/data/Run2016H/SinglePhoton/MINIAOD/PromptReco-v2/000/284/035/00000/D2866817-459F-E611-A40C-02163E014156.root --secondfilein root://eoscms.cern.ch//eos/cms/store/group/phys_higgs/cmshgg/soffi/SinglePhotonRunHRaw/6CC3EF80-E29E-E611-B455-02163E011F8F.root --fileout file:singlephoton_reHLT.root -n 10 --no_exec

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
    input = cms.untracked.int32(-1)
)

# Input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
        '/store/data/Run2016H/SinglePhoton/MINIAOD/PromptReco-v2/000/284/035/00000/1098E856-459F-E611-9438-02163E013685.root',
        '/store/data/Run2016H/SinglePhoton/MINIAOD/PromptReco-v2/000/284/035/00000/1AE29A23-459F-E611-871F-FA163E1DE70D.root',
        '/store/data/Run2016H/SinglePhoton/MINIAOD/PromptReco-v2/000/284/035/00000/2213A86C-479F-E611-8BCF-02163E0127E5.root',
        '/store/data/Run2016H/SinglePhoton/MINIAOD/PromptReco-v2/000/284/035/00000/D2866817-459F-E611-A40C-02163E014156.root',
        '/store/data/Run2016H/SinglePhoton/MINIAOD/PromptReco-v2/000/284/035/00000/D4129043-4A9F-E611-8EFB-FA163EE30F6B.root'
    ),
    secondaryFileNames = cms.untracked.vstring(
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/0A2C49D0-E29E-E611-92CB-02163E0142E4.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/0ACE81B4-E29E-E611-8B2F-FA163E760E68.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/0ED443F7-E29E-E611-B428-02163E012A88.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/148FFFCA-E29E-E611-97DD-02163E01470C.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/160BCBBD-E29E-E611-AE95-FA163E06D0F1.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/1EFA9258-E39E-E611-8181-02163E01390C.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/206462CD-E29E-E611-AB9A-02163E0124AE.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/38002ABB-E29E-E611-9943-02163E013780.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/48D9C648-E39E-E611-A3A4-FA163E27C52B.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/4AA9619B-E29E-E611-9550-02163E01414A.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/589DA1D6-E29E-E611-A58E-02163E011E3B.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/60C3E79E-E29E-E611-BC41-02163E01346E.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/641412B6-E29E-E611-8CEE-02163E0129AD.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/6CC3EF80-E29E-E611-B455-02163E011F8F.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/74DEB57E-E29E-E611-8A3B-02163E014320.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/76411CCC-E29E-E611-842E-FA163EFC9C91.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/786CAF75-E29E-E611-9701-02163E012756.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/7890374B-E39E-E611-B534-02163E012A1E.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/78BE5801-E39E-E611-A443-02163E01342B.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/7CD31A7C-E29E-E611-A394-02163E01198B.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/A48AF53E-E39E-E611-937F-FA163E35BABF.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/A6E8EDBE-E29E-E611-B0D0-02163E013611.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/A836356D-E39E-E611-A65A-02163E01349A.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/ACCE09E4-E29E-E611-9BB0-02163E013779.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/B0532152-E39E-E611-B2FE-02163E011FA8.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/BCB430F4-E29E-E611-B7A4-02163E014152.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/BEBE749F-E29E-E611-A49B-02163E013768.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/C6E77392-E29E-E611-94ED-02163E011EA1.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/C8074A7C-E29E-E611-BD88-FA163E8A9CFA.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/C84E6880-E29E-E611-A570-02163E0145C7.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/CCE29984-E29E-E611-B8A4-02163E0146A0.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/D075EEB1-E29E-E611-AD4D-02163E0138B7.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/DCD0ACEB-E29E-E611-8205-02163E014123.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/E08E52D4-E29E-E611-B6AD-FA163EF898AE.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/F017B84D-E39E-E611-8A05-02163E014276.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/F67FBDFD-E29E-E611-9728-02163E013658.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/F6C09481-E29E-E611-850B-02163E014466.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/F6D00286-E29E-E611-8B9B-02163E014725.root',
        'root://cmsxrootd.fnal.gov//store/data/Run2016H/SinglePhoton/RAW/v1/000/284/035/00000/F6F0D97F-E29E-E611-A936-02163E012A1E.root'
    )
)

process.source.lumisToProcess = CfgTypes.untracked(CfgTypes.VLuminosityBlockRange())  
JSONfile = '/afs/cern.ch/user/k/kmcdermo/private/dispho/Analysis/CMSSW_8_0_26_patch2/src/Timing/GEN_SIM/SinglePho/block_3e69b694-9ee5-11e6-9951-001e67abf228.json'
myLumis = LumiList.LumiList(filename = JSONfile).getCMSSWString().split(',')  
process.source.lumisToProcess.extend(myLumis)                              

process.options = cms.untracked.PSet(
    numberOfThreads = cms.untracked.uint32(4),
    numberOfStreams = cms.untracked.uint32(4),
    allowUnscheduled = cms.untracked.bool(True)
)

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
    fileName = cms.untracked.string('root://eoscms.cern.ch//eos/cms/store/group/phys_exotica/displacedPhotons/SinglePhoton/test/reHLT_full.root'),
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
