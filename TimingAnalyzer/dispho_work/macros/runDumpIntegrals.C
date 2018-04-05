#include "common/Common.cpp+"

#include "TString.h"
#include "TFile.h"
#include "TH1F.h"

#include <fstream>

void runDumpIntegrals(const TString & outfiletext)
{
  // Set Config
  Config::SetupGroups();
  Config::SetupHistNames();

  // Get Input
  TFile * infile = TFile::Open(Form("%s.root",outfiletext.Data()));
  std::map<SampleType,TH1F*> HistMap;
  for (const auto & HistNamePair : Config::HistNameMap)
  {
    HistMap[HistNamePair.first] = (TH1F*)infile->Get(HistNamePair.second.Data());
  }
  TH1F * BkgdHist = (TH1F*)infile->Get("Bkgd_Hist");

  // Make output
  std::ofstream outfile(Form("%s_integrals.txt",outfiletext.Data()),std::ios_base::out);

  // Individual MC first
  for (const auto & HistPair : HistMap)
  {
    if (Config::GroupMap[HistPair.first] == isBkgd)
    {
      outfile << HistPair.second->GetName() << " : " << HistPair.second->Integral() << std::endl;
    }
  }
  outfile << "-------------------------------------" << std::endl;
  
  // Sum MC second
  const Float_t mc_int = BkgdHist->Integral();
  outfile << BkgdHist->GetName() << " : " << mc_int << std::endl;

  // Data third
  const Float_t data_int = HistMap[Data]->Integral();
  outfile << HistMap[Data]->GetName() << " : " << data_int << std::endl;
  outfile << "-------------------------------------" << std::endl;

  // Make ratio
  outfile << "Data/MC : " << data_int/mc_int << std::endl;
}
