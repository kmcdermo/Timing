#include "common/Common.cpp+"

#include "TFile.h"
#include "TH1F.h"
#include "TString.h"

void computePUWeights(const TString & text)
{
  // get input
  const TString filename = Form("%s.root",text.Data());
  TFile * file = TFile::Open(Form("%s",filename.Data()),"UPDATE");
  Config::CheckValidFile(file,filename);
  file->cd();
  
  const TString dataname = "Data_Hist";
  TH1F * dataHist = (TH1F*)file->Get(Form("%s",dataname.Data()));
  Config::CheckValidTH1F(dataHist,dataname,filename);

  const TString bkgdname = "Bkgd_Hist";
  TH1F * bkgdHist = (TH1F*)file->Get(Form("%s",bkgdname.Data()));
  Config::CheckValidTH1F(bkgdHist,bkgdname,filename);

  std::cout << "Data: " << dataHist->Integral() << std::endl;
  std::cout << "Bkgd: " << bkgdHist->Integral() << std::endl;

  // normalize
  dataHist->Scale(1.0/dataHist->Integral());
  bkgdHist->Scale(1.0/bkgdHist->Integral());

  // clone and divide
  TH1F * PUWeights = (TH1F*)dataHist->Clone(Form("%s",Config::puwgtHistName.Data()));
  PUWeights->Divide(bkgdHist);

  // save
  file->cd();
  PUWeights->Write(PUWeights->GetName(),TObject::kWriteDelete);
  
  // delete
  delete PUWeights;
  delete bkgdHist;
  delete dataHist;
  delete file;
}
