// use this to compare data/data or MC/MC

void quickmean()
{
  gStyle->SetOptStat(0);

  TString label1 = "notof";
  TString label2 = "tof";
  Bool_t  isData = true;
  TString el     = "el2";
  TString hname  = Form("%s_vtxZ_mean_gaus%i",el.Data(),(isData?2:1));
  
  TString indir = isData?"DATA/doubleeg":"MC/dyll";
  TFile * file1 = TFile::Open(Form("test/%s/%s/plots.root",label1.Data(),indir.Data()));
  TFile * file2 = TFile::Open(Form("test/%s/%s/plots.root",label2.Data(),indir.Data()));
  TH1F * h1 = (TH1F*)file1->Get(hname.Data());
  TH1F * h2 = (TH1F*)file2->Get(hname.Data());

  Double_t weighted_numer1 = 0; Double_t weighted_denom1;
  for (Int_t ibin = 1; ibin < h1->GetNbinsX(); ibin++)
  {
    Double_t binc = h1->GetBinContent(ibin);
    Double_t bine = h1->GetBinError(ibin);

    weighted_numer1 += binc / (bine*bine);
    weighted_denom1 += 1.0  / (bine*bine);
  }

  Double_t weighted_numer2 = 0; Double_t weighted_denom2;
  for (Int_t ibin = 1; ibin < h2->GetNbinsX(); ibin++)
  {
    Double_t binc = h2->GetBinContent(ibin);
    Double_t bine = h2->GetBinError(ibin);

    weighted_numer2 += binc / (bine*bine);
    weighted_denom2 += 1.0  / (bine*bine);
  }

  std::cout << "isData: " << (isData?"true":"false") << " el: " << el.Data() <<  std::endl;
  std::cout << label1.Data() << ": " << weighted_numer1 / weighted_denom1  << std::endl;
  std::cout << label2.Data() << ": " << weighted_numer2 / weighted_denom2  << std::endl;
}
