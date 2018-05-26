#include "Common.cpp+"

#include "TString.h"
#include "TFile.h"
#include "TH1F.h"

#include <fstream>

void computeSumWeights(const TString & indir, const TString & files, const TString & wgtfile)
{
  // get list of files
  std::ifstream infiles(files.Data(),std::ios::in);

  // sum the weights
  TString infile = "";
  Float_t sumwgts = 0.f;
  while (infiles >> infile)
  {
    // first get the file
    const TString infilename = Form("%s/%s",indir.Data(),infile.Data());
    auto file = TFile::Open(infilename.Data());
    Common::CheckValidFile(file,infilename);

    // Get the cut histogram
    const TString inh_cutflowname = Form("%s/%s",Common::rootdir.Data(),Common::h_cutflowname.Data());
    auto cutflow = (TH1F*)file->Get(inh_cutflowname.Data());
    Common::CheckValidTH1F(cutflow,inh_cutflowname,infilename);

    // now add to sum of weights
    sumwgts += cutflow->GetBinContent(1);
    
    delete cutflow;
    delete file;
  }
  
  // dump the sum of weights
  std::ofstream outfile(wgtfile.Data(),std::ios_base::trunc);
  outfile << "Sum_of_weights: " << sumwgts << std::endl;
}
