void scale(TH2F * hist)
{
  for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
  {
    const auto binwidthX = hist->GetXaxis()->GetBinWidth(ibinX);
    for (auto ibinY = 1; ibinY <= hist->GetYaxis()->GetNbins(); ibinY++)
    {
      const auto binwidthY = hist->GetYaxis()->GetBinWidth(ibinY);
      
      // get multiplier/divisor
      auto multiplier = binwidthX * binwidthY;

      hist->SetBinContent(ibinX,ibinY,hist->GetBinContent(ibinX,ibinY)*multiplier);
      hist->SetBinError  (ibinX,ibinY,hist->GetBinError  (ibinX,ibinY)*multiplier);
    }
  }
}

void table()
{
  auto file = TFile::Open("sr_met_vs_time_box.root");
  
  const std::vector<TString> lambdas = {"100","150","200","250","300","350","400","500","600"}; // TeV
  const std::vector<TString> gctaus  = {"400"}; //{"0p001","0p1","10","200","400","600","800","1000","1200","10000"}; // cm
  
  std::ofstream norms("norms.csv",std::ios_base::trunc);
  norms << "Sample,A,B,C,D" << std::endl;
  
  auto bkgd = (TH2F*)file->Get("Bkgd_Hist");
  scale(bkgd);

  norms << "MC Background,"
	<< bkgd->GetBinContent(1,1) << "," << bkgd->GetBinContent(1,2) << ","
	<< bkgd->GetBinContent(2,2) << "," << bkgd->GetBinContent(2,1) << std::endl;
  
  delete bkgd;

  // loop over all possible GMSBs...
  for (const auto & lambda : lambdas)
  {
    for (const auto & ctau : gctaus)
    {
      const TString label = Form("GMSB_L%s_CTau%s",lambda.Data(),ctau.Data());
      auto hist = (TH2F*)file->Get(Form("%s_Hist",label.Data()));
      scale(hist);

      norms << label.Data() << "," 
	    << hist->GetBinContent(1,1) << "," << hist->GetBinContent(1,2) << ","
	    << hist->GetBinContent(2,2) << "," << hist->GetBinContent(2,1) << std::endl;

      delete hist;
    }
  }
}
