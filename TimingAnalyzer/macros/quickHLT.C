void quickHLT()
{
  TFile * file = TFile::Open("photondump-gmsb-ctau6000-HLT4.root");
  TTree * tree = (TTree*)file->Get("tree/tree");

  std::vector<Bool_t> * triggerBits = 0; TBranch * b_triggerBits; tree->SetBranchAddress("triggerBits", &triggerBits, &b_triggerBits);
  Int_t nphotons; TBranch * b_nphotons; tree->SetBranchAddress("nphotons", &nphotons, &b_nphotons);
  std::vector<std::vector<Int_t> > * phIsHLTMatched = 0; TBranch * b_phIsHLTMatched; tree->SetBranchAddress("phIsHLTMatched", &phIsHLTMatched, &b_phIsHLTMatched);
  
  for (UInt_t ientry = 0; ientry < tree->GetEntries(); ientry++)
  {
    tree->GetEntry(ientry);
    if (!(*triggerBits)[9]) continue;

    for (Int_t iph = 0; iph < nphotons; iph++)
    {
      std::cout << iph << " : " << (*phIsHLTMatched)[iph][1] << std::endl;
    }
    std::cout << "------------" << std::endl;
  }

}
