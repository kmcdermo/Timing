#include "../interface/Common.hh"
#include "../interface/Analysis.hh"

#include <TROOT.h>

int main(){
  Float_t lumi = 2.301; // brilcalc lumi --normtag /afs/cern.ch/user/l/lumipro/public/normtag_file/moriond16_normtag.json -i rereco2015D.txt -u /fb

  // to do:
  // mc vs data stacking
  // mc vs data output in histos
  // sub directories
  
  // make analysis object for every input file
  // choose which "analysis" to do
  TString input = "input/tree.root";
  Analysis analysis(input.Data(),"first_results","png",lumi);
  analysis.TimeResPlots();
}
