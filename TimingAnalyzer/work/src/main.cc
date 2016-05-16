#include "../interface/Common.hh"
#include "../interface/Analysis.hh"

#include <TROOT.h>

int main(){

  TString input = "input/tree.root";
  Analysis analysis(input.Data(),"first_results","png");
  analysis.TimeResPlots();
  //  MoveInput(input.Data());

}
