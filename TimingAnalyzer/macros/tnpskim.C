#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TDirectory.h"

void skim()
{
  // get input full trees
  TFile * infile = TFile::Open("zeetnptree.root");
  TTree * intree = (TTree*)infile->Get("tree/tree");

  // set branch addresses of input for skimming
  Float_t zmass; intree->SetBranchAddress("zmass",&zmass);

  // declare output skim files
  TFile * outfile = new TFile("skimmedtnptree.root","RECREATE");
  TDirectory * subdir = outfile->mkdir("tree");
  subdir->cd();
  TTree * outtree = (TTree*)infile->Get("tree/tree");
  // clone structure of input tree, but storing no events
  outtree = intree->CloneTree(0);
  
  // loop over input tree
  for (UInt_t entry = 0; entry < intree->GetEntries(); entry++)
  {
    intree->GetEntry(entry);

    if (zmass == -9999.0) continue;

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
