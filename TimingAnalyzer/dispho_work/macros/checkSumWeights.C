#include "Common.cpp+"

void checkSumWeights(const TString & unskimmedwgtfile, const TString & skimdir)
{
  // read back unskimmed sum
  std::ifstream unskimmedstream(Form("%s",unskimmedwgtfile.Data()),std::ios::in);
  TString text; 
  Float_t unskimmedsum;
  while (unskimmedstream >> text >> unskimmedsum) std::cout << "Unskimmed sum: " << unskimmedsum << std::endl;

  // get skimmed sum of weights
  const TString skimfilename = Form("%s/%s",skimdir.Data(),Common::tupleFileName.Data());
  auto skimfile = TFile::Open(skimfilename.Data());
  const Bool_t isnull = Common::IsNullFile(skimfile);
  if (isnull) 
  {
    std::cerr << "Bad skimmed file... exiting..." << std::endl;
    exit(1);
  }

  // Get the cut histogram
  const TString skimhistname = Form("%s",Common::h_cutflowname.Data());
  auto skimhist = (TH1F*)skimfile->Get(skimhistname.Data());
  Common::CheckValidTH1F(skimhist,skimhistname,skimfilename);

  // get sum
  const Float_t skimmedsum = skimhist->GetBinContent(1);
  std::cout << "Skimmed sum:   " << skimmedsum << std::endl;
}
