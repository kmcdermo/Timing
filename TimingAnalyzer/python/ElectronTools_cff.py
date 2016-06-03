import os
import FWCore.ParameterSet.Config as cms
from PhysicsTools.SelectorUtils.tools.vid_id_tools import *

def ElectronTools(process,isMC):

    # Electron ValueMaps for identification
    dataFormat = DataFormat.MiniAOD
    switchOnVIDElectronIdProducer(process, dataFormat);
    ele_id_modules = [];
    ele_id_modules.append('RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Spring15_25ns_V1_cff');
    ele_id_modules.append('RecoEgamma.ElectronIdentification.Identification.heepElectronID_HEEPV60_cff');
    
    for idmod in ele_id_modules:
        setupAllVIDIdsInModule(process,idmod,setupVIDElectronSelection)

    # https://twiki.cern.ch/twiki/bin/viewauth/CMS/EGMSmearer    
    # add EGM smearing to eLeCtRoNs
    process.selectedElectrons = cms.EDFilter("PATElectronSelector",
                                             src = cms.InputTag("slimmedElectrons"),
                                             cut = cms.string("pt > 5 && abs(eta)<2.5")
                                             )
		
    from EgammaAnalysis.ElectronTools.calibratedElectronsRun2_cfi import calibratedPatElectrons,files
    setattr(process,"calibratedElectrons",calibratedPatElectrons.clone(
            isMC = cms.bool(isMC),				
            electrons = cms.InputTag('selectedElectrons'),
            correctionFile = cms.string(files["76XReReco"])
            ))

