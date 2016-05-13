import os
import FWCore.ParameterSet.Config as cms
from PhysicsTools.SelectorUtils.tools.vid_id_tools import *

def PhotonTools(process):

    # Photon ValueMaps for identification
    dataFormat = DataFormat.MiniAOD;
    switchOnVIDPhotonIdProducer(process, dataFormat);
    ph_id_modules = []
    ph_id_modules.append('RecoEgamma.PhotonIdentification.Identification.cutBasedPhotonID_Spring15_25ns_V1_cff')

    for idmod in ph_id_modules:
        setupAllVIDIdsInModule(process,idmod,setupVIDPhotonSelection)
