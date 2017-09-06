void test(){

  TFile * file = TFile::Open("pileupWeights.root");
  TH1F  * hist = (TH1F*)file->Get("weights");

  for (int i = 1; i <= hist->GetNbinsX(); i++){
    std::cout << "i: " << i << " entries: " << hist->GetBinContent(i) << std::endl;

  }

}
