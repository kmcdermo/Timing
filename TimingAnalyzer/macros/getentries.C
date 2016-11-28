void getentries()
{
  TFile* file = TFile::Open("input/QCD/QCD-HT2000ToInf.root");
  TTree* tree= (TTree*)file->Get("tree/tree");

  std::cout << tree->GetEntries() << std::endl;
}
