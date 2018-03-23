#include "common/Common.cpp+"

#include "TString.h"
#include "TFile.h"
#include "TH1F.h"

#include <fstream>

void runSumWeights(const TString & indir, const TString & files, const TString & wgtfile, const TString extra)
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
    TFile * file = TFile::Open(infilename.Data());
    Config::CheckValidFile(file,infilename);

    // Get the cut histogram
    const TString inh_cutflowname = Form("%s%s",extra.Data(),Config::h_cutflowname.Data());
    TH1F * cutflow = (TH1F*)file->Get(inh_cutflowname.Data());
    Config::CheckValidTH1F(cutflow,inh_cutflowname,infilename);

    // now add to sum of weights
    sumwgts += cutflow->GetBinContent(1);
    
    delete cutflow;
    delete file;
  }
  infiles.close();
  
  // dump the sum of weights
  std::ofstream outfile(wgtfile.Data(),std::ios_base::trunc);
  outfile << "Sum of weights: " << sumwgts << std::endl;
  outfile.close();
}
