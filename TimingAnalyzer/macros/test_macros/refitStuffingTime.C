void doMoreStuff(TFile * file, const TString & label, const TString & histname)
{
  // get hist
  auto hist2D = (TH2F*)file->Get(histname.Data());
  
  // this macro rebins the data to hide large uncertainties: science!
  auto hist1D = new TH1F("hist1D","",30,-3,3);
  
  // fill hist
  for (auto ibinY = 1; ibinY <= hist2D->GetYaxis()->GetNbins(); ibinY++)
  { 
    auto val = 0.0;
    auto err = 0.0;
    for (auto ibinX = 12; ibinX <= 14; ibinX++)
    {
      val += hist2D->GetBinContent(ibinX,ibinY);
      err += std::pow(hist2D->GetBinError(ibinX,ibinY),2);
    }
    hist1D->SetBinContent(ibinY,val);
    hist1D->SetBinError  (ibinY,std::sqrt(err));
  }

  // set tmp init vals
  const auto norm  = hist1D->Integral() / 1.77245385090551602729;
  const auto mu    = hist1D->GetMean();
  const auto sigma = hist1D->GetStdDev(); 

  const auto rangelow = (mu-2.0*sigma);
  const auto rangeup  = (mu+2.0*sigma);
  
  const TString & fitname = "fit";
  auto fit = new TF1(fitname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)",rangelow,rangeup);
  fit->SetParameter(0,norm);
  fit->SetParameter(1,mu);
  fit->SetParameter(2,sigma); fit->SetParLimits(2,0,10);

  // fit and print
  hist1D->Fit(fitname.Data(),"RBQ0");
  std::cout << label.Data() << " " << histname.Data() << std::endl;
  std::cout << "N: "     << fit->GetParameter(0) << " +/- " << fit->GetParError(0) << std::endl;
  std::cout << "mu: "    << fit->GetParameter(1) << " +/- " << fit->GetParError(1) << std::endl;
  std::cout << "sigma: " << fit->GetParameter(2) << " +/- " << fit->GetParError(2) << std::endl << std::endl;

  // delete all
  delete fit;
  delete hist1D;
  delete hist2D;
}

void doStuff(const TString & label)
{
  // get file
  auto file = TFile::Open("inputfiles/"+label+".root");

  doMoreStuff(file,label,"Data_Hist");
  doMoreStuff(file,label,"Bkgd_Hist");

  // delete
  delete file;
}
 

void refitStuffingTime()
{
  doStuff("dixtal_inclusive");
  doStuff("dixtal_sameTT");
  doStuff("dixtal_diffTT");
  doStuff("zee");
}
