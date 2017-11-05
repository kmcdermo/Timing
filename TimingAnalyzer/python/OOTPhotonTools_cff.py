import os
import FWCore.ParameterSet.Config as cms
from PhysicsTools.SelectorUtils.tools.vid_id_tools import *
import random

def OOTPhotonTools(process,isMC):

    # Photon ValueMaps for identification
    dataFormat = DataFormat.MiniAOD
    switchOnVIDPhotonIdProducer(process, dataFormat);
    ph_id_modules = [];
    ph_id_modules.append('RecoEgamma.PhotonIdentification.Identification.cutBasedPhotonID_OOT_2017_V1_cff');
    for idmod in ph_id_modules:
        setupAllVIDIdsInModule(process,idmod,setupVIDPhotonSelection)

    process.egmPhotonIDs.physicsObjectSrc = cms.InputTag('slimmedPhotons')
    #ootPhotonIDs = process.egmPhotonIDs.clone()
    #ootPhotonIDs.physicsObjectSrc = cms.InputTag('slimmedOOTPhotons')
