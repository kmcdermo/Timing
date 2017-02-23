#include "TFile.h"
#include "TH1F.h"
#include "TString.h"
#include "TObject.h"

void quickRename()
{
  TString input  = "ph1suisseXXX";
  TString output = "ph1suisseXXXXX";
  
  TFile * file = TFile::Open("test.root","UPDATE");
  TH1F  * hist = (TH1F*)file->Get(input.Data());
  
  hist->SetName(output.Data());
  file->cd();
  hist->Write(hist->GetName(),TObject::kWriteDelete);
  file->Delete(Form("%s;2",input.Data()));

//   hist->SetDirectory(0);
//   TH1F * copy = (TH1F*)hist->Clone(output.Data());
//   delete hist;
//   file->cd();
//   copy->Write();
//   file->Delete(Form("%s;1",input.Data()));

  delete file;
}
