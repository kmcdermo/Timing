void totalrate(Int_t psc)
{
  std::ifstream input;
  input.open("stdpaths.txt",std::ios::in);
  
  TString ipath;
  std::vector<TString> paths;
  while (input >> ipath)
  {
    paths.push_back(ipath);
  }

  TString fullmenu;
  for (UInt_t i = 0; i < paths.size(); i++)
  {
    if (i != (paths.size() -1)) fullmenu += "(" + paths[i] + "==1) || ";
    else fullmenu += "(" + paths[i] + "==1)";
  }

  TString spsc = (psc == 1 ? "1p6e34/" : "1p35e34/");
  TString dir  = "GRunDisplacedPhotonHT_Full_V2/" + spsc + "hlt0/";
  Float_t nls  = (psc == 1 ? 640 : 638); //618;
  Float_t lsl  = 23.3;
  Float_t ps   = 428; // 107 major factor * 4 for PS of HLT_L1FatEvents_part0_v1
  TFile * file = TFile::Open(dir+"hltbits.root");
  TTree * tree = (TTree*)file->Get("HltTree");

  ROOT::v5::TFormula::SetMaxima(3000);
  
  Int_t total = tree->GetEntries();
  Int_t npass = tree->GetEntries(fullmenu.Data());
  std::cout << "Total entries:" << total << " nPassed: " << npass << std::endl;
  std::cout << "Rate: " << ((ps*npass)/(lsl*nls)) << " Hz" <<std::endl;
}
