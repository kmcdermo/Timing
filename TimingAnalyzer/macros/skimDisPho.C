#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TDirectory.h"

void skimDisPho(const TString & dir, const TString & outname, const Bool_t applyTrigger = true)
{
  // get input full trees
  TFile * infile = TFile::Open(Form("%s/tree.root",dir.Data()));
  TTree * intree = (TTree*)infile->Get("tree/tree");

  // set branch addresses of input for skimming
  Bool_t  hltPho50;   intree->SetBranchAddress("hltPho50",&hltPho50);
  Float_t phopt_0;    intree->SetBranchAddress("phopt_0",&phopt_0);
  Float_t phosceta_0; intree->SetBranchAddress("phosceta_0",&phosceta_0);
  Int_t   njets;      intree->SetBranchAddress("njets",&njets);
  Int_t   phoID_0;    intree->SetBranchAddress("phoID_0",&phoID_0);

  // declare output skim files
  TFile * outfile = new TFile(Form("%s",outname.Data()),"RECREATE");
  TDirectory * subdir = outfile->mkdir("tree");
  subdir->cd();
  TTree * outtree = (TTree*)infile->Get("tree/tree");
  // clone structure of input tree, but storing no events
  outtree = intree->CloneTree(0);
  
  // loop over input tree
  for (UInt_t entry = 0; entry < intree->GetEntries(); entry++)
  {
    intree->GetEntry(entry);
    
    // apply selection
    if (!hltPho50 && applyTrigger)     continue;
    if (std::abs(phosceta_0) > 1.4442) continue; 
    if (phopt_0 < 60) continue;
    if (phoID_0 <= 0) continue;
    if (njets   <= 0) continue;

    outtree->Fill(); // fill output tree
  }

  // write the output skim tree
  outfile->cd();
  subdir->cd();
  outtree->Write();

  // now delete (close) everything
  delete intree;
  delete infile;
  delete outtree;
  delete outfile;
}
