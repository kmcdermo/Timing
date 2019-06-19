const auto nBinsX = 1000;

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

  auto hist = new TH1F("hist","Average Pedestal Noise",nBinsX,0,nBinsX);
  hist->Sumw2();

  for (auto ifile = 0; ifile < filenames.size(); ifile++)
  {
    if (ifile % 10 != 0) continue;

    const auto & filename = filenames[ifile];
    std::cout <<  "working on file: " << filename.Data() << std::endl;

    const auto file = TFile::Open(filename.Data());
    auto tree = (TTree*)file->Get("tree/disphotree");
    std::vector<Float_t> * rhE = 0; TBranch * b_rhE = 0; tree->SetBranchAddress("rhE", &rhE, &b_rhE);
    std::vector<Float_t> * rhpedrms12 = 0; TBranch * b_rhpedrms12 = 0; tree->SetBranchAddress("rhpedrms12", &rhpedrms12, &b_rhpedrms12);
    std::vector<Float_t> * rhadcToGeV = 0; TBranch * b_rhadcToGeV = 0; tree->SetBranchAddress("rhadcToGeV", &rhadcToGeV, &b_rhadcToGeV);
    
    const auto nentries = tree->GetEntries();
    for (auto ientry = 0; ientry < nentries; ientry++)
    {
      b_rhE->GetEntry(ientry);
      b_rhpedrms12->GetEntry(ientry);
      b_rhadcToGeV->GetEntry(ientry);

      const auto nrh = rhE->size();
      for (auto irh = 0; irh < nrh; irh++)
      {
	if ( (*rhE)[irh] < 1.f || (*rhE)[irh] > 120.f) continue;
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
