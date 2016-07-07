void dumprawcontent(TH1F *& hist, Int_t nbinsX);
void getminbinrange(TH1F *& hist, Int_t nbinsX);

void dumpints() 
{
  Bool_t  isMC     = false;
  TString indir    = "output";
  TString name     = "effE";

  TString outdir = Form("%s/%s",indir.Data(), (isMC?"MC/dyll":"DATA/doubleeg") );
  TFile   * file = TFile::Open(Form("%s/plots.root",outdir.Data()));
  TH1F    * hist = (TH1F*)file->Get(name.Data());
  Int_t   nbinsX = hist->GetNbinsX();
  std::cout << "nBins:" << nbinsX << " integral: " << hist->Integral() << std::endl;
  std::cout << "UF: " << hist->GetBinContent(0) << "+/-" << hist->GetBinError(0) << std::endl;
  //  dumprawcontent(hist,nbinsX);
  getminbinrange(hist,nbinsX);
  std::cout << "OF: " << hist->GetBinContent(nbinsX+1) << "+/-" << hist->GetBinError(nbinsX+1) << std::endl;
}

void dumprawcontent(TH1F *& hist, Int_t nbinsX)
{
  for (Int_t ibin = 1; ibin <= nbinsX; ibin++){
    const Double_t xlow    = hist->GetXaxis()->GetBinLowEdge(ibin); 
    const Double_t xhigh   = hist->GetXaxis()->GetBinUpEdge(ibin);
    const Double_t content = hist->GetBinContent(ibin);
    const Double_t error   = hist->GetBinError(ibin);
    std::cout << xlow << "-" << xhigh << ": " << content << "+/-" << error << std::endl;
  }
}

void getminbinrange(TH1F *& hist, Int_t nbinsX)
{
  Double_t tmpcontent = 0.0;
  Int_t        lowbin = 0;
  Bool_t        reset = true;
  for (Int_t ibin = 1; ibin <= nbinsX; ibin++){
    if (reset) { 
      tmpcontent = 0.0; 
      lowbin     = ibin; 
      reset      = false; 
    }

    tmpcontent += hist->GetBinContent(ibin);
    if (tmpcontent > 1000){
      const Double_t xlow  = hist->GetXaxis()->GetBinLowEdge(lowbin); 
      const Double_t xhigh = hist->GetXaxis()->GetBinUpEdge(ibin);
      std::cout << xlow << "-" << xhigh << ": " << tmpcontent << std::endl;
      reset      = true;
    }
  }
}
