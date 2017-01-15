#ifndef _quick_plot_
#define _quick_plot_

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TString.h"

#include <vector>

class quickPlot
{
public:
  quickPlot();
  ~quickPlot();

  void InitTree();
  void doPlot();

private:
  TFile * fInFile;
  TTree * fInTree;

  // input variables
  Int_t                                nphotons;
  std::vector<Int_t>                 * phnrh;
  std::vector<Int_t>                 * phseedpos;
  std::vector<std::vector<Float_t> > * phrhE;
  
  TBranch * b_nphotons;  
  TBranch * b_phnrh;     
  TBranch * b_phseedpos; 
  TBranch * b_phrhE;
};

#endif
