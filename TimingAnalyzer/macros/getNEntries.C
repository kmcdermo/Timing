void getNEntries()
{
  std::vector<TString> qcdHTs = {"100To200","200To300","300To500","500To700","700To1000","1000To1500","1500To2000","2000ToInf"};
  for (auto&& qcd : qcdHTs)
  {
    TFile * file = TFile::Open(Form("input/MC/bkg/QCD/QCD_HT%s.root",qcd.Data()));
    TTree * tree = (TTree*)file->Get("tree/tree");
    std::cout << tree->GetEntries() << ",";
    delete tree;
    delete file;
  }
  std::cout << std::endl << std::endl;

  std::vector<TString> gjetHTs = {"40To100","100To200","200To400","400To600","600ToInf"};
  for (auto&& gjet : gjetHTs)
  {
    TFile * file = TFile::Open(Form("input/MC/bkg/GJets/GJets_HT%s.root",gjet.Data()));
    TTree * tree = (TTree*)file->Get("tree/tree");
    std::cout << tree->GetEntries() << ",";
    delete tree;
    delete file;
  }
  std::cout << std::endl << std::endl;
}
