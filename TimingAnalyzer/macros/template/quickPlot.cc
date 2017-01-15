#include "quickPlot.hh"
#include <iostream>

quickPlot::quickPlot()
{
  fInFile = TFile::Open("input/MC/signal/withReReco/ctau6000.root");
  fInTree = (TTree*)fInFile->Get("tree/tree");

  quickPlot::InitTree();
}

quickPlot::~quickPlot()
{
  delete fInTree;
  delete fInFile;
}

void quickPlot::InitTree()
{
  nphotons  = 0;
  phnrh     = 0;
  phseedpos = 0;
  phrhE     = 0;

  fInTree->SetBranchAddress("nphotons" , &nphotons , &b_nphotons);
  fInTree->SetBranchAddress("phnrh"    , &phnrh    , &b_phnrh);
  fInTree->SetBranchAddress("phseedpos", &phseedpos, &b_phseedpos);
  fInTree->SetBranchAddress("phrhE"    , &phrhE    , &b_phrhE);
}

void quickPlot::doPlot()
{
  // Output histogram
  TH1F * hist = new TH1F("hist","hist",100,0.,1000.);

  for (UInt_t ientry = 0; ientry < fInTree->GetEntries(); ientry++)
  {
    fInTree->GetEntry(ientry);
    for (Int_t iph = 0; iph < nphotons; iph++)
    {
      for (Int_t irh = 0; irh < (*phnrh)[iph]; irh++)
      {
	if ( (*phseedpos)[iph] == irh ) // seed info
        {
	  hist->Fill((*phrhE)[iph][irh]);
	}
      }
    }
  }
  
  // Output canvas
  TCanvas * canv = new TCanvas();
  canv->cd();
  hist->Draw();
  canv->SaveAs(Form("%s.png",hist->GetName()));
}

