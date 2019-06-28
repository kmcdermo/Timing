#include "Common.cpp+"
#include "TString.h"

// more hard-coded badness!

void dumpSignalEfficiencies()
{
  const TString filename = "skims/v4p1/final/categories/signals_exclusive_1pho.root";
  auto file = TFile::Open(filename.Data());
  Common::CheckValidFile(file,filename);

  const std::vector<TString> lambdas = {"100","150","200","250","300","350","400"};
  const std::vector<TString> ctaus = {"10","50","100","200","400","600","800","1000","1200","10000"};

  std::ofstream output("sig_effs."+Common::outTextExt,std::ios::trunc);
  
  for (const auto & ctau : ctaus)
  {
    output << ctau;
    for (const auto & lambda : lambdas)
    {
      const auto sample = "GMSB_L"+lambda+"_CTau"+ctau;

      const auto histname = sample+"_"+Common::h_cutflow_scaledname;
      auto hist = (TH1F*)file->Get(histname.Data());
      Common::CheckValidHist(hist,histname,filename);

      // get input tree for rescaling errors
      const auto treename = sample+"_Tree";
      auto tree = (TTree*)file->Get(treename.Data());
      Common::CheckValidTree(tree,treename,file->GetName());
      
      // get scale factor
      Float_t evtwgt; TBranch * b_evtwgt; tree->SetBranchAddress("evtwgt",&evtwgt,&b_evtwgt);
      tree->GetEntry(0);

      const auto denom = hist->GetBinContent(1);
      const auto numer = hist->GetBinContent(hist->GetXaxis()->GetNbins());
      const auto prob  = numer/denom;
      const auto err   = 1.96*std::sqrt((prob*(1-prob))/(denom/evtwgt));
      
      output << Form(" & %s",Common::PrintValueAndError(100.f*prob,100.f*err).Data());

      // delete it all
      delete tree;
      delete hist;
    }
    output << " \\\\" << std::endl;
  }

  delete file;
}
