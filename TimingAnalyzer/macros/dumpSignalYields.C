#include "Common.cpp+"

// god-awful hardcoded badness everywhere!
// just need this to be done

struct BinInfo
{
  BinInfo(){}
  BinInfo(const TString & name, const Int_t ibinX, const Int_t ibinY)
    : name(name), ibinX(ibinX), ibinY(ibinY) {}

  TString name;
  Int_t ibinX;
  Int_t ibinY;
};

void dumpSignalYields(const TString & infiletext, const TString & outfiletext)
{
  const std::vector<BinInfo> binInfos = 
  {
    {"A",1,1},
    {"B",1,2},
    {"C",2,2},
    {"D",2,1}
  };

  const std::vector<TString> lambdas = {"100","150","200","250","300","350","400"};
  const std::vector<TString> ctaus = {"10","200","400","600","800","1000","1200","10000"};

  for (const auto & lambda : lambdas)
  {
    // output dump
    std::ofstream outfile(lambda+"_"+outfiletext+"."+Common::outTextExt,std::ios::trunc);

    for (const auto & ctau : ctaus)
    {
      const TString indir = (ctau.EqualTo("10") ? "x_0.5_y_200" : "x_1.5_y_200");
      outfile << ctau.Data();

      const auto filename = infiletext+"/"+indir+"/plots2D/met_vs_time.root";
      if (Common::IsNullFile(filename)) continue;
      auto file = TFile::Open(filename.Data());
      file->cd();

      const auto histname = "GMSB_L"+lambda+"_CTau"+ctau+"_Hist";
      const auto & hist = (TH2F*)file->Get(histname.Data());
      Common::CheckValidHist(hist,histname,filename);
      
      for (const auto & binInfo : binInfos)
      {
	const auto yield = hist->GetBinContent(binInfo.ibinX,binInfo.ibinY);
	const auto error = hist->GetBinError  (binInfo.ibinX,binInfo.ibinY);
	
	outfile << Form(" & $%.3f \\pm %.3f$",yield,error);
      }
      outfile << " \\\\" << std::endl;

      delete hist;
      delete file;
    }
  }
}
