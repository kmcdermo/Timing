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

  TString pd =  "DATA/doubleeg"; // "MC/dyll" ; // 

  TFile * infile  = TFile::Open(Form("gaus2fm/%s/plots.root",pd.Data()));
  TFile * outfile = new TFile(Form("plots/%s/plots.root",pd.Data()),"RECREATE");

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
