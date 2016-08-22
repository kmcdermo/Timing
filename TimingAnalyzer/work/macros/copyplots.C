#include "TString.h"
#include "TFile.h"
#include "TH1F.h"

#include <vector>
#include <fstream>
#include <iostream>

void copyplots()
{
  std::vector<TString> plotnames;
  
  std::ifstream inputplots;
  inputplots.open("config/finalplots.txt",std::ios::in);
  TString plotname, dirname;
  while (inputplots >> plotname >> dirname){
    if (!plotname.Contains("runs",TString::kExact)) 
    {
      plotnames.push_back(plotname);
    }
  }
  inputplots.close();

  TFile * infile  = TFile::Open("output/MC/dyll/plots.root");
  TFile * outfile = new TFile("plots/MC/dyll/plots.root","RECREATE");

  for (UInt_t iplot = 0; iplot < plotnames.size(); iplot++)
  {
    std::cout << plotnames[iplot].Data() << std::endl;

    infile->cd();
    TH1F * hist = (TH1F*)infile->Get(plotnames[iplot].Data());
    
    outfile->cd();
    hist->Write();
    delete hist;
  }  
}
