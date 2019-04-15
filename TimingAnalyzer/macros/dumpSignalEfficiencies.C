#include "Common.cpp+"
#include "TString.h"

// more hard-coded badness!

void dumpSignalEfficiencies()
{
  const TString filename = "skims/v4/final/signals_sr.root";
  auto file = TFile::Open(filename.Data());
  Common::CheckValidFile(file,filename);

  const std::vector<TString> lambdas = {"100","150","200","250","300","350","400"};
  const std::vector<TString> ctaus = {"10","200","400","600","800","1000","1200","10000"};

  std::ofstream output("sig_effs."+Common::outTextExt,std::ios::trunc);
  
  for (const auto & ctau : ctaus)
  {
    output << ctau;
    for (const auto & lambda : lambdas)
    {
      const auto histname = "GMSB_L"+lambda+"_CTau"+ctau+"_"+Common::h_cutflow_scaledname;
      auto hist = (TH1F*)file->Get(histname.Data());
      Common::CheckValidHist(hist,histname,filename);

      const auto denom = hist->GetBinContent(1);
      const auto numer = hist->GetBinContent(hist->GetXaxis()->GetNbins());
      const auto prob  = numer/denom;
      const auto err   = 1.96*std::sqrt((prob*(1-prob))/denom);
      
      output << Form(" & $%.2f \\pm %.2f$",100.f*prob,100.f*err);

      delete hist;
    }
    output << " \\\\" << std::endl;
  }

  delete file;
}
