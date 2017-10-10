#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TDirectory.h"

#include <map>
#include <string>
#include <fstream>

typedef std::map<UInt_t,Int_t> uiimap;

void getEntryMap(const std::string & input, uiimap & entryphomap)
{
  UInt_t ientry, run, lumi;
  ULong64_t event;
  Int_t goodpho;
  Float_t time;

  std::ifstream textfile;
  textfile.open(input.c_str(),std::ios::in);
  
  while (textfile >> ientry >> run >> lumi >> event >> goodpho >> time)
  {
    entryphomap[ientry] = goodpho;
  }

  textfile.close();
}

void simpleplot()
{
  // get input full trees
  TFile * infile = TFile::Open("badinfo.root");
  TTree * intree = (TTree*)infile->Get("tree/tree");

  // Branches
  std::vector<Float_t> * phsmaj = 0; TBranch * b_phsmaj; intree->SetBranchAddress("phsmaj", &phsmaj, &b_phsmaj);
  std::vector<Float_t> * phsmin = 0; TBranch * b_phsmin; intree->SetBranchAddress("phsmin", &phsmin, &b_phsmin);
  std::vector<Int_t> * phisOOT = 0; TBranch * b_phisOOT; intree->SetBranchAddress("phisOOT", &phisOOT, &b_phisOOT);

  // get input entry skim info
  std::string input = "badinfo.txt";
  uiimap entrymap;
  getEntryMap(input,entrymap);

  // Hists
  TH1F * smaj = new TH1F("hsmaj","S_{major}",10,0,1);
  TH1F * smin = new TH1F("hsmin","S_{minor}",10,0,1);
  TH1F * isoot = new TH1F("hisoot","isOOT",2,0,2);

  // loop over input tree
  for (UInt_t ientry = 0; ientry < intree->GetEntries(); ientry++)
  {
    intree->GetEntry(ientry);
    smaj->Fill((*phsmaj)[entrymap[ientry]]);
    smin->Fill((*phsmin)[entrymap[ientry]]);
    isoot->Fill((*phisOOT)[entrymap[ientry]]);
  }

  TCanvas * canv = new TCanvas(); canv->cd();
  smaj->Draw();
  canv->SaveAs("smaj.png");
  smin->Draw();
  canv->SaveAs("smin.png");
  isoot->Draw();
  canv->SaveAs("isoot.png");

  // now delete (close) everything
  delete canv;
  delete isoot;
  delete smin;
  delete smaj;
  delete intree;
  delete infile;
}
