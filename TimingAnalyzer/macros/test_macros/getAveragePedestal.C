namespace Config
{
  const auto nBinsX = 1000;
 
  inline Float_t rad2 (const Float_t x, const Float_t y){return x*x + y*y;}
  inline Float_t hypo (const Float_t x, const Float_t y){return std::sqrt(Config::rad2(x,y));}
  inline Float_t theta(const Float_t r, const Float_t z){return std::atan2(r,z);}
  inline Float_t eta  (const Float_t r, const Float_t z){return -1.0f*std::log(std::tan(Config::theta(r,z)/2.f));}
  inline Float_t eta  (const Float_t x, const Float_t y, const Float_t z){return Config::eta(Config::hypo(x,y),z);}
};

void getFiles(std::vector<TString> & filenames)
{
  std::ifstream input("files.txt",std::ios::in);
  TString filename;

  while (input >> filename) filenames.emplace_back(filename);
}

void getAveragePedestal()
{
  std::vector<TString> filenames;
  getFiles(filenames);

  auto hist = new TH1F("hist","Average Pedestal Noise",Config::nBinsX,0,Config::nBinsX);
  hist->Sumw2();

  for (auto ifile = 0; ifile < filenames.size(); ifile++)
  {
    if (ifile % 10 != 0) continue;

    const auto & filename = filenames[ifile];
    std::cout <<  "working on file: " << filename.Data() << std::endl;

    const auto file = TFile::Open(filename.Data());
    auto tree = (TTree*)file->Get("tree/disphotree");
    Bool_t hltDiEle33MW; TBranch * b_hltDiEle33MW; tree->SetBranchAddress("hltDiEle33MW", &hltDiEle33MW, &b_hltDiEle33MW);
    std::vector<Float_t> * rhE = 0; TBranch * b_rhE = 0; tree->SetBranchAddress("rhE", &rhE, &b_rhE);
    std::vector<Float_t> * rhX = 0; TBranch * b_rhX = 0; tree->SetBranchAddress("rhX", &rhX, &b_rhX);
    std::vector<Float_t> * rhY = 0; TBranch * b_rhY = 0; tree->SetBranchAddress("rhY", &rhY, &b_rhY);
    std::vector<Float_t> * rhZ = 0; TBranch * b_rhZ = 0; tree->SetBranchAddress("rhZ", &rhZ, &b_rhZ);
    std::vector<Float_t> * rhpedrms12 = 0; TBranch * b_rhpedrms12 = 0; tree->SetBranchAddress("rhpedrms12", &rhpedrms12, &b_rhpedrms12);
    std::vector<Float_t> * rhadcToGeV = 0; TBranch * b_rhadcToGeV = 0; tree->SetBranchAddress("rhadcToGeV", &rhadcToGeV, &b_rhadcToGeV);
    
    const auto nentries = tree->GetEntries();
    for (auto ientry = 0; ientry < nentries; ientry++)
    {
      b_hltDiEle33MW->GetEntry(ientry);
      if (!hltDiEle33MW) continue;

      b_rhE->GetEntry(ientry);
      b_rhX->GetEntry(ientry);
      b_rhY->GetEntry(ientry);
      b_rhZ->GetEntry(ientry);
      b_rhpedrms12->GetEntry(ientry);
      b_rhadcToGeV->GetEntry(ientry);

      const auto nrh = rhE->size();
      for (auto irh = 0; irh < nrh; irh++)
      {
	if ( (*rhE)[irh] < 1.f || (*rhE)[irh] > 120.f ) continue;
	if ( std::abs( Config::eta( (*rhX)[irh] , (*rhY)[irh] , (*rhZ)[irh] ) ) > 1.4442 ) continue;
	hist->Fill( (*rhpedrms12)[irh] * (*rhadcToGeV)[irh] * 1000.f );
      }
    }

    // delete it all
    delete tree;
    delete file;
  }

  std::cout << hist->GetMean() << " +/- " << hist->GetStdDev() << std::endl;

  // delete hist
  delete hist;
}
