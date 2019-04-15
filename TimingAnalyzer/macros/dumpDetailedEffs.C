#include "Common.cpp+"

void dumpDetailedEffs()
{
  // get files
  const TString indir = "skims/v4/final";

  const auto data_file_name = indir+"/"+"sr.root";
  auto data_file = TFile::Open(data_file_name);
  Common::CheckValidFile(data_file,data_file_name);

  const auto sign_file_name = indir+"/"+"signals_sr.root";
  auto sign_file = TFile::Open(sign_file_name);
  Common::CheckValidFile(sign_file,sign_file_name);
  
  // vector of hist names
  std::vector<TString> histnames = {"Data","GMSB_L200_CTau10","GMSB_L200_CTau200","GMSB_L200_CTau1200"};
  for (auto & histname : histnames) histname += "_"+Common::h_cutflow_scaledname;
  const auto nhists = histnames.size();

  // vector of hists
  std::vector<TH1F*> hists(nhists);
  for (auto ihist = 0U; ihist < nhists; ihist++)
  {
    const auto & histname = histnames[ihist];
    auto & file = (ihist > 0 ? sign_file : data_file);
    auto & hist = hists[ihist];
    hist = (TH1F*)file->Get(histname.Data());
    Common::CheckValidHist(hist,histname,file->GetName());
  }

  // output
  std::ofstream output("detailed_effs."+Common::outTextExt,std::ios::trunc);

  // loop over nbins
  const auto nbins = hists.front()->GetXaxis()->GetNbins();
  for (auto ibin = 1; ibin <= nbins; ibin++)
  {
    std::cout << "working on bin: " << ibin << std::endl;
    for (auto ihist = 0U; ihist < nhists; ihist++)
    {
      const auto & hist = hists[ihist];
      const auto denom = hist->GetBinContent(1);
      const auto numer = hist->GetBinContent(ibin);
      const auto percent = 100.f * (numer / denom);

      if (ihist == 0) 
      {
	output << Form("%s & %i (%.2f\\%%)",hist->GetXaxis()->GetBinLabel(ibin),Int_t(numer),percent);
      }
      else
      {
	output << Form(" & %.2f (%.2f\\%%)",numer,percent);
      }
    }
    output << " \\\\" << std::endl;
  }

  // delete it all
  Common::DeleteVec(hists);
  delete sign_file;
  delete data_file;
}
