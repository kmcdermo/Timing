import os
import FWCore.ParameterSet.Config as cms
from PhysicsTools.SelectorUtils.tools.vid_id_tools import *

def ElectronTools(process):

    # Electron ValueMaps for identification
    dataFormat = DataFormat.MiniAOD
    switchOnVIDElectronIdProducer(process, dataFormat);
    ele_id_modules = [];
    ele_id_modules.append('RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Spring15_25ns_V1_cff');
    ele_id_modules.append('RecoEgamma.ElectronIdentification.Identification.heepElectronID_HEEPV60_cff');
    
    for idmod in ele_id_modules:
        setupAllVIDIdsInModule(process,idmod,setupVIDElectronSelection)
