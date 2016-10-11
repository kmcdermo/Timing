#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TDirectory.h"

void skim(TString path){
  // get input full trees
  TFile * infile = TFile::Open(Form("%s/tree.root",path.Data()));
  TTree * intree = (TTree*)infile->Get("tree/tree");
  const Bool_t isMC = path.Contains("MC",TString::kExact);

  // set branch addresses of input for skimming
  Float_t zmass;         intree->SetBranchAddress("zmass",&zmass);
  Bool_t  hltdoubleel33; intree->SetBranchAddress("hltdoubleel33",&hltdoubleel33);
  Bool_t  hltdoubleel37; intree->SetBranchAddress("hltdoubleel37",&hltdoubleel37);
  Int_t   el1pid;        intree->SetBranchAddress("el1pid",&el1pid); 
  Int_t   el2pid;        intree->SetBranchAddress("el2pid",&el2pid);
  Int_t   el1nrh;        intree->SetBranchAddress("el1nrh",&el1nrh); 
  Int_t   el2nrh;        intree->SetBranchAddress("el2nrh",&el2nrh);

  // declare output skim files
  TFile * outfile = new TFile(Form("%s/skimmedtree.root",path.Data()),"RECREATE");
  TDirectory * subdir = outfile->mkdir("tree");
  subdir->cd();
  TTree * outtree = (TTree*)infile->Get("tree/tree");
  // clone structure of input tree, but storing no events
  outtree = intree->CloneTree(0);
  
  // loop over input tree
  for (UInt_t entry = 0; entry < intree->GetEntries(); entry++)
  {
    intree->GetEntry(entry);
    const Bool_t triggered = (isMC)?true:(hltdoubleel33||hltdoubleel37);

    // skim cut
    if ( (zmass > 76.f && zmass < 106.f) && (triggered == true) && (el1pid == -el2pid) && (el1nrh > 0) && (el2nrh > 0) )
    {
      outtree->Fill(); // fill output tree
    }
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
