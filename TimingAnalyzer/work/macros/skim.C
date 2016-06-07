void skim(TString path){
  // get input full trees
  TFile * infile = TFile::Open(Form("%s/smearedtree.root",path.Data()));
  TTree * intree = (TTree*)infile->Get("tree/tree");

  // set branch addresses of input for skimming
  Float_t zmass;
  Bool_t  hltdoubleel;
  Int_t   el1pid,el2pid;
  intree->SetBranchAddress("zmass",&zmass);
  intree->SetBranchAddress("hltdoubleel",&hltdoubleel);
  intree->SetBranchAddress("el1pid",&el1pid); 
  intree->SetBranchAddress("el2pid",&el2pid);

  // declare output skim files
  TFile * outfile = new TFile(Form("%s/skimmedtree.root",path.Data()),"RECREATE");
  TDirectory * subdir = outfile->mkdir("tree");
  subdir->cd();
  TTree * outtree = (TTree*)infile->Get("tree/tree");
  // clone structure of input tree, but storing no events
  outtree = intree->CloneTree(0);
  
  // loop over input tree
  for (UInt_t entry = 0; entry < intree->GetEntries(); entry++){
    intree->GetEntry(entry);
    // skim cut
    if ( (zmass>76. && zmass<106.) && hltdoubleel && (el1pid == -el2pid) ) {
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
