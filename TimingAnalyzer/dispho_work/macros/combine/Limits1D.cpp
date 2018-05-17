#include "Limits1D.hh"

Limits1D::Limits1D(const TString & indir, const TString & infilename, const TString & outtext)
  : fInDir(indir), fInFileName(infilename), fOutText(outtext)
{  
  Limits1D::SetupGMSB();

}

Limits1D::~Limits1D()
{

}


void Limits1D::MakeLimits1D()
{


}

void Limits1D::SetupGMSB()
{
  // read in parameters... 
  std::fstream inparams("signal_config/all_params.txt");
  Float_t mass, width, br;
  TString lambda, ctau;

  while (inparams >> lambda >> ctau >> mass >> width >> br)
  {
    const TString s_ctau = (ctau.EqualTo("0.1") ? "0p1" : ctau);
    const TString name = "GMSB_L"+lambda+"TeV_CTau"+s_ctau+"cm";

    const Int_t i_lambda = lambda.Atoi();
    const Float_t f_ctau = ctau.Atof();

    fGMSBMap[name] = {i_lambda,f_ctau,mass,width,br};
  }
  
  // read in xsecs...
  std::fstream inxsecs("signal_config/all_xsecs.txt");
  Float_t xsec, exsec;

  while (inxsecs >> lambda >> ctau >> xsec >> exsec)
  {
    const TString s_ctau = (ctau.EqualTo("0.1") ? "0p1" : ctau);
    const TString name = "GMSB_L"+lambda+"TeV_CTau"+s_ctau+"cm";

    fGMSBMap[name].xsec  = xsec;
    fGMSBMap[name].exsec = exsec;
  }

  // read in r-values...
  for (auto & GMSBPair : fGMSBMap)
  {
    const auto & name = GMSBPair.first;
    auto       & info = GMSBPair.second;
    
    // get file
    const TString filename = Form("%s/%s%s.root",fInDir.Data(),fInFileName.Data(),name.Data());
    auto infile = TFile::Open(Form("%s",filename.Data()));
    auto isnull = Config::IsNullFile(infile);

    if (!isnull)
    {
      const TString treename = "limit";
      auto intree = (TTree*)infile->Get(Form("%s",treename.Data()));
      Config::CheckValidTree(intree,treename,filename);

      // 5 Entries in tree, one for each quantile 

    }
    else
    {
      info.rexp      = -1.f;
      info.r1sigup   = -1.f;
      info.r1sigdown = -1.f;
      info.r2sigup   = -1.f;
      info.r2sigdown = -1.f;

      std::cout << "skipping this file: " << filename.Data() << std::endl;
    }


  }

}
