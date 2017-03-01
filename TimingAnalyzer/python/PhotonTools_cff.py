import os
import FWCore.ParameterSet.Config as cms
from PhysicsTools.SelectorUtils.tools.vid_id_tools import *
import random

def PhotonTools(process,isMC):

    # smear first, then egmID
    # https://twiki.cern.ch/twiki/bin/viewauth/CMS/EGMSmearer    
    process.selectedPhotons = cms.EDFilter("PATPhotonSelector",
                                           src = cms.InputTag("slimmedPhotons"),
                                           cut = cms.string("pt > 5 && abs(eta)<2.5")
                                           )
		
    from EgammaAnalysis.ElectronTools.calibratedPhotonsRun2_cfi import calibratedPatPhotons,files
    setattr(process,"calibratedPhotons",calibratedPatPhotons.clone(
            isMC = cms.bool(isMC),				
            photons = cms.InputTag('selectedPhotons'),
            correctionFile = cms.string(files["Moriond17_23Jan"])
            ))

    process.RandomNumberGeneratorService.calibratedPhotons = cms.PSet(
        initialSeed = cms.untracked.uint32(int(random.uniform(0,1000000))),
        engineName = cms.untracked.string('TRandom3')
        )

    # Photon ValueMaps for identification
    dataFormat = DataFormat.MiniAOD
    switchOnVIDPhotonIdProducer(process, dataFormat);
    ph_id_modules = [];
    ph_id_modules.append('RecoEgamma.PhotonIdentification.Identification.cutBasedPhotonID_Spring16_V2p2_cff');
    for idmod in ph_id_modules:
        setupAllVIDIdsInModule(process,idmod,setupVIDPhotonSelection)

    process.egmPhotonIDs.physicsObjectSrc = cms.InputTag('calibratedPhotons')
