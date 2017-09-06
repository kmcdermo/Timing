void plotdump(TString sfile, TString shist)
{
  TFile * file = TFile::Open(sfile.Data());
  TH1F  * hist = (TH1F*)file->Get(shist.Data());

  for (Int_t ibin = 0; ibin <= hist->GetNbinsX() + 1; ibin++)
  {
    TString bin;
    if      (ibin == 0) {bin = "UF";}
    else if (ibin == hist->GetNbinsX() + 1) {bin = "OF";}
    else    {bin = Form("%i",ibin);}

    if (hist->GetBinContent(ibin) == 0) continue; 

    std::cout << "ibin: " << bin.Data() << " center: " << hist->GetXaxis()->GetBinCenter(ibin) 
	      << " low: " << hist->GetXaxis()->GetBinLowEdge(ibin) 
	      << " up: "  << hist->GetXaxis()->GetBinUpEdge(ibin) 
	      << " label: " << hist->GetXaxis()->GetBinLabel(ibin)
	      << " content: " << hist->GetBinContent(ibin) << " +/- " 
	      << hist->GetBinError(ibin) << std::endl;
  }
}
