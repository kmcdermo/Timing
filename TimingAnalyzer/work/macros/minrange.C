// to run this BS... need to compile with ACLiC
// root -l minrange.C+

#include "TString.h"
#include "TFile.h"
#include "TTree.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <functional> 
#include <algorithm>
#include <cmath>

//inline Float_t effA(const Float_t e1, const Float_t e2){return e1*e2/std::sqrt(rad2(e1,e2));}

void minrange()
{
  Bool_t     isMC = false;
  TString   indir = Form("input/%s", (isMC?"MC/dyll":"DATA/doubleeg") );
  TFile   *  file = TFile::Open(Form("%s/skimmedtree.root",indir.Data())); 
  TTree   *  tree = (TTree*)file->Get("tree/tree");
  TString  invar1 = "el1E";
  //  TString  invar2 = "el2E";
  Float_t diffval = 5.0;
  size_t  ndiff   = 1000;

  TString outvar = invar1;
  TString outtxt = Form("config/%s_bins.txt",outvar.Data());
  ofstream outfile;
  outfile.open(outtxt.Data(),std::ios_base::trunc);
			
  Float_t var1 = 0; TBranch * b_var1 = tree->GetBranch(invar1.Data()); b_var1->SetAddress(&var1);
  //  Float_t var2 = 0; TBranch * b_var2 = tree->GetBranch(invar2.Data()); b_var2->SetAddress(&var2);
  
  std::vector<Float_t> values;
  for (UInt_t entry = 0; entry < tree->GetEntries(); entry++){
    b_var1->GetEvent(entry);
    //b_var2->GetEvent(entry);
    values.push_back(var1);
  }

  std::sort(values.begin(), values.end(), std::less<Float_t>());

  size_t lowval = 0;
  Bool_t reset  = false;

  for (size_t i = 0; i < values.size(); i++) {
    if (reset) { 
      lowval = i;
      reset  = false; 
    }

    if ( ((i-lowval) >= ndiff) && (values[i]-values[lowval] >= diffval) ){
      std::cout << i-lowval << " : " << values[lowval] << " - " << values[i] << " = " << values[i] - values[lowval] << std::endl;
      reset  = true;
    }
  }
  std::cout << "last bin: " << values.size()-lowval << " : " << values[lowval] << " - " << values[values.size()-1] << " = " << values[values.size()-1] - values[lowval] << std::endl;
}
