void minrange()
{
  Bool_t  isMC  = false;
  TString indir = Form("input/%s", (isMC?"MC/dyll":"DATA/doubleeg") );
  TFile  * file = TFile::Open(Form("%s/skimmedtree.root",indir.Data())); 
  TTree  * tree = (TTree*)file->Get("tree/tree");
  

  TString name  = "effE";


  TH1F    * hist = (TH1F*)file->Get(name.Data());
  Int_t   nbinsX = hist->GetNbinsX();
  std::cout << "nBins:" << nbinsX << " integral: " << hist->Integral() << std::endl;
  std::cout << "UF: " << hist->GetBinContent(0) << "+/-" << hist->GetBinError(0) << std::endl;
  //  dumprawcontent(hist,nbinsX);
  getminbinrange(hist,nbinsX);
  std::cout << "OF: " << hist->GetBinContent(nbinsX+1) << "+/-" << hist->GetBinError(nbinsX+1) << std::endl;




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
