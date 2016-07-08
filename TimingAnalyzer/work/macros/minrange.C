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

namespace Partition{
  static const Float_t etaEB     = 1.4442;
  static const Float_t etaEElow  = 1.566;
  static const Float_t etaEEhigh = 2.5;
};

inline Float_t rad2 (const Float_t e1, const Float_t e2){return e1*e1 + e2*e2;}
inline Float_t effA (const Float_t e1, const Float_t e2){return e1*e2/std::sqrt(rad2(e1,e2));}
inline Float_t theta(const Float_t r,  const Float_t z) {return std::atan2(r,z);}
inline Float_t eta  (const Float_t x,  const Float_t y, const Float_t z){return -1.0f*std::log(std::tan(theta(std::sqrt(rad2(x,y)),z)/2.f));}

void minrange()
{
  Bool_t     isMC = false;
  TString   indir = Form("input/%s", (isMC?"MC/dyll":"DATA/doubleeg") );
  TFile   *  file = TFile::Open(Form("%s/skimmedtree.root",indir.Data())); 
  TTree   *  tree = (TTree*)file->Get("tree/tree");
  TString  invar1 = "el1seedE";
  TString  invar2 = "el2seedE";
  TString outvar  = "effseedE";
  Int_t    minval = 20;
  Float_t diffval = 5.0;
  size_t  ndiff   = 1000;

  TString outtxt = Form("config/%s_EBEB_bins.txt",outvar.Data());
  ofstream outfile;
  outfile.open(outtxt.Data(),std::ios_base::trunc);
			
  Float_t var1 = 0; TBranch * b_var1 = tree->GetBranch(invar1.Data()); b_var1->SetAddress(&var1);
  Float_t var2 = 0; TBranch * b_var2 = tree->GetBranch(invar2.Data()); b_var2->SetAddress(&var2);
  
  Float_t x1 = 0; TBranch * b_x1 = tree->GetBranch("el1seedX"); b_x1->SetAddress(&x1);
  Float_t y1 = 0; TBranch * b_y1 = tree->GetBranch("el1seedY"); b_y1->SetAddress(&y1);
  Float_t z1 = 0; TBranch * b_z1 = tree->GetBranch("el1seedZ"); b_z1->SetAddress(&z1);
  
  Float_t x2 = 0; TBranch * b_x2 = tree->GetBranch("el2seedX"); b_x2->SetAddress(&x2);
  Float_t y2 = 0; TBranch * b_y2 = tree->GetBranch("el2seedY"); b_y2->SetAddress(&y2);
  Float_t z2 = 0; TBranch * b_z2 = tree->GetBranch("el2seedZ"); b_z2->SetAddress(&z2);

  std::vector<Float_t> values;
  for (UInt_t entry = 0; entry < tree->GetEntries(); entry++){
    b_var1->GetEvent(entry); 
    b_x1->GetEvent(entry); b_y1->GetEvent(entry); b_z1->GetEvent(entry);
    Float_t eta1 = eta(x1,y1,z1);
    Bool_t el1eb = false, el1ee = false, el1ep = false, el1em = false;
    if      (std::abs(eta1)<Parition::etaEB)        {el1eb = true;}
    else if (std::abs(eta1)<Parition::etaEEhigh 
	     && std::abs(eta1)>Partition::etaEElow) {el1ee = true;
      if      (z1>0)                                {el1ep = true;}
      else if (z1<0)                                {el1em = true;}
    }      

    b_var2->GetEvent(entry);
    b_x2->GetEvent(entry); b_y2->GetEvent(entry); b_z2->GetEvent(entry);
    Float_t eta2 = eta(x2,y2,z2);
    Bool_t el2eb = false, el2ee = false, el2ep = false, el2em = false;
    if      (std::abs(eta2)<Parition::etaEB)        {el2eb = true;}
    else if (std::abs(eta2)<Parition::etaEEhigh 
	     && std::abs(eta2)>Partition::etaEElow) {el2ee = true;
      if      (z2>0)                                {el2ep = true;}
      else if (z2<0)                                {el2em = true;}
    }      

    if (el1eb && el2eb) {
      if (var1>0 && var2>0) values.push_back(effA(var1,var2));
    }
  }

  std::sort(values.begin(), values.end(), std::less<Float_t>());

  size_t ilow   = 0;
  Int_t  front  = values.front();
  Int_t  lowval = (minval<front?minval:front);
  outfile << lowval << std::endl;
  Bool_t reset  = false;
  for (size_t i = 0; i < values.size(); i++) {
    if (reset) { 
      ilow   = i;
      reset  = false; 
    }

    if ( ((i-ilow) >= ndiff) && (values[i]-values[ilow] >= diffval) && (values[i]>10) ) { 
      std::cout << i-ilow << " : " << values[ilow] << " - " << values[i] << " = " << values[i] - values[ilow] << std::endl;
      reset  = true;
      outfile << int(values[i]) << std::endl;
    }
  }
  std::cout << "last bin: " << values.size()-ilow << " : " << values[ilow] << " - " << values.back() << " = " << values.back() - values[ilow] << std::endl;

  outfile << int(values.back()) << std::endl;
}
