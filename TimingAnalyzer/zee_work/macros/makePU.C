#include "TH1D.h"
#include "TFile.h"

void makePU() {

  TFile fileData("input/PU/MyDataPileupHistogram.root");
  TFile fileMC("input/PU/mcpu.root");
  
  TH1D *generated_pu = (TH1D*)fileMC.Get("pileupMC");
  generated_pu->Sumw2();
  generated_pu->Scale(1./generated_pu->Integral());
  generated_pu->SetTitle("generated_pu");
  generated_pu->SetName("generated_pu");

  TH1D *target_pu = (TH1D*)fileData.Get("pileup");
  target_pu->Sumw2();
  target_pu->Scale(1./target_pu->Integral());
  target_pu->SetTitle("target_pu");
  target_pu->SetName("target_pu");
    
  TH1D *myClone = (TH1D*)target_pu->Clone("myClone");
  myClone->Divide(generated_pu);
  myClone->SetTitle("weights");
  myClone->SetName("weights");
  myClone->Scale(1./myClone->Integral());
  // for (int ii=0; ii<53; ii++) myClone->SetBinError(ii,0);
  
  TFile *fileOut = new TFile("input/PU/pileupWeights.root","RECREATE");
  fileOut->cd();
  target_pu->Write();
  generated_pu->Write();
  myClone->Write();
  fileOut->Close();
  
}
