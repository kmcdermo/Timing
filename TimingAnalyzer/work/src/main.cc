#include "../interface/Common.hh"
#include "../interface/Analysis.hh"

#include <TROOT.h>

int main(){
  // to do:
  // mc vs data stacking
  // mc vs data output in histos
  // sub directories
  
  // make analysis object for every input file
  // choose which "analysis" to do
  TString input = "input/tree.root";
  Analysis analysis(input.Data(),"first_results","png",2.5);
  analysis.TimeResPlots();
  //  MoveInput(input.Data());

}
