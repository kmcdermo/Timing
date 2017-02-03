# Timing
Displaced Photon Analysis Framework

Lots of analyzers, macros, many of the files/functions are duplicated.  

To run this code, we need to check the latest version of CMSSW, as well as a few other packages:
https://twiki.cern.ch/twiki/bin/viewauth/CMS/EGMSmearer
https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedPhotonIdentificationRun2
https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedElectronIdentificationRun2

You may have to move the Configuration subdirectory to CMSSW_X_Y_Z/src, and also issue the cmsDriver commands from there for at least the first step of GEN SIM: 

https://github.com/lsoffi/GMSB-MC2016-PRODUCTION
https://twiki.cern.ch/twiki/bin/viewauth/CMS/PdmVMCcampaignRunIISummer16DR80
https://twiki.cern.ch/twiki/bin/view/CMS/PdmVMCcampaignRunIISummer16MiniAODv2
