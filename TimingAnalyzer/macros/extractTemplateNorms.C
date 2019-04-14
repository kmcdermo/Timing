#include "Common.cpp+"

void dumpValues(TH1D * hist, const std::vector<Float_t> & boundaries, const TString & label)
{
  for (const auto boundary : boundaries)
  {
    const auto ibin = hist->FindBin(boundary);
    const auto nlow = hist->Integral(1,ibin-1);
    const auto nup  = hist->Integral(ibin,hist->GetXaxis()->GetNbins());

    // get ratio
    const auto ratio     = nup/nlow;
    const auto ratio_unc = ratio * std::sqrt(1.f/nup + 1.f/nlow);

    // print
    std::cout << label.Data() << " [" << boundary << "] " << ratio << " +/- " << ratio_unc << std::endl;
  }
}

void extractTemplateNorms()
{
  // get file
  auto filename = Common::eosPreFix+"/"+Common::eosDir+"/"+Common::calibDir+"/"+Common::templateFileName;
  auto file = TFile::Open(filename.Data());
  Common::CheckValidFile(file,filename);

  // get hists
  auto xhist = (TH1D*)file->Get(Common::templateXHistName.Data());
  Common::CheckValidHist(xhist,Common::templateXHistName,filename);

  auto yhist = (TH1D*)file->Get(Common::templateYHistName.Data());
  Common::CheckValidHist(yhist,Common::templateYHistName,filename);

  // xy-boundaries
  std::vector<Float_t> xboundaries = {0.5,1.5};
  std::vector<Float_t> yboundaries = {200};

  // dump values
  dumpValues(xhist,xboundaries,"X");
  dumpValues(yhist,yboundaries,"Y");

  // delete 
  delete yhist;
  delete xhist;
  delete file;
}
