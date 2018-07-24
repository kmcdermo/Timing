Core chain of analysis:

NTuplizer  : plugins/Dispho.cc/hh
HelperUtils: plugins/CommonUtils.cc/hh

Produce ntuples with multicrab:
test/multicrab_dispho_Data.py
test/multicrab_dispho_Bkgd.py
test/multicrab_dispho_Signal.py

Which calls config: test/dispho.py
Make sure to configure correctly!

XSecs from signals: see GEN_SIM examples
XSecs from bkgds: see genproductions GH (local fork branch: xsec)

Output of ntuples end up in EOS space. Now begin macro chain of analysis. cd to dispho_work/macros

Produce skims with new time branches, event weights, basic core skim + met flags: scripts/skimAndMerge/*.sh
Also makes PU weights...
Will have to launch each process... annoying. loops are serial and long... looking to parallelize

Produce data+bkgd mc skims: scripts/runFastSkimmer.sh
Produce signal skims: scripts/runSignalSkimmer.sh
Make sure to do both for GJets CR, QCD CR, and SR selection

Then run the scripts for making the variable weight trees in CR to SR: scripts/makeWgtsAndPlots.sh
Then check 1D final plots: scripts/makePlotsForSR.sh
Finally, run 2D plots for fis: scripts/makeLimits.sh
