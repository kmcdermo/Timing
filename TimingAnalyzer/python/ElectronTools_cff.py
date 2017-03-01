import os
import FWCore.ParameterSet.Config as cms
from PhysicsTools.SelectorUtils.tools.vid_id_tools import *
import random

def ElectronTools(process,isMC):

    # smear first, then egmID
    # https://twiki.cern.ch/twiki/bin/viewauth/CMS/EGMSmearer    
    process.selectedElectrons = cms.EDFilter("PATElectronSelector",
                                             src = cms.InputTag("slimmedElectrons"),
                                             cut = cms.string("pt > 5 && abs(eta)<2.5")
                                             )
		
    from EgammaAnalysis.ElectronTools.calibratedElectronsRun2_cfi import calibratedPatElectrons,files
    setattr(process,"calibratedElectrons",calibratedPatElectrons.clone(
            isMC = cms.bool(isMC),				
            electrons = cms.InputTag('selectedElectrons'),
            correctionFile = cms.string(files["Moriond17_23Jan"])
            ))

    process.RandomNumberGeneratorService.calibratedElectrons = cms.PSet(
        initialSeed = cms.untracked.uint32(int(random.uniform(0,1000000))),
#        initialSeed = cms.untracked.uint32(81),
        engineName = cms.untracked.string('TRandom3')
        )

    # Electron ValueMaps for identification
    dataFormat = DataFormat.MiniAOD
    switchOnVIDElectronIdProducer(process, dataFormat);
    ele_id_modules = [];
    ele_id_modules.append('RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Summer16_80X_V1_cff');
    ele_id_modules.append('RecoEgamma.ElectronIdentification.Identification.heepElectronID_HEEPV60_cff');
    
    for idmod in ele_id_modules:
        setupAllVIDIdsInModule(process,idmod,setupVIDElectronSelection)

    process.egmGsfElectronIDs.physicsObjectSrc = cms.InputTag('calibratedElectrons')



