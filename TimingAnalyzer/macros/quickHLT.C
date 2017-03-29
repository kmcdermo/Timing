void quickHLT()
{
  TFile * file = TFile::Open("input/MC/signal/GMSB/photondump-gmsb-ctau6000-HLT2.root");
  TTree * tree = (TTree*)file->Get("tree/tree");
  
  const Int_t total = tree->GetEntries();
  std::vector<TString> paths = {"hltdispho60","hltdispho80"};
  
  for (UInt_t ipath = 0; ipath < paths.size(); ipath++)
  {
    const Int_t   passed      = tree->GetEntries(paths[ipath].Data());
    const Float_t efficiency  = float(passed)/float(total);
    const Float_t uncertainty = std::sqrt((efficiency*(1.f-efficiency))/float(total));

    std::cout << paths[ipath] << " " << efficiency << " " << uncertainty << std::endl;
  }

}
