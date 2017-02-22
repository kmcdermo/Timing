#include "TFile.h"
#include "TH1F.h"
#include "TString.h"
#include "TObject.h"

#include <vector>

void renameQCD  (const std::vector<TString>&, const std::vector<TString>&, const TString &);
void renameGJets(const std::vector<TString>&, const std::vector<TString>&, const TString &);
void renameGMSB (const std::vector<TString>&, const std::vector<TString>&, const TString &);
void renameHVDS (const std::vector<TString>&, const std::vector<TString>&, const TString &);

void renameHist()
{
  std::vector<TString> inputnames  = {"ph1suisse"};
  std::vector<TString> outputnames = {"ph1suisseX"};

  // generic settings
  TString cuts = "cuts_jetpt35.0_phpt50.0_phVIDnone_rhE1.0_EBOnly";

  renameQCD  (inputnames,outputnames,cuts);
  renameGJets(inputnames,outputnames,cuts);
  renameGMSB (inputnames,outputnames,cuts);
  renameHVDS (inputnames,outputnames,cuts);
}

void renameQCD(const std::vector<TString>& inputnames, const std::vector<TString>& outputnames, const TString & cuts)
{
  const std::vector<TString> qcdHTs = {"100To200","200To300","300To500","500To700","700To1000","1000To1500","1500To2000","2000ToInf"};

  std::vector<TFile*> qcdfiles(qcdHTs.size());
  for (UInt_t iqcd = 0; iqcd < qcdHTs.size(); iqcd++)
  {
    qcdfiles[iqcd] = TFile::Open(Form("output/MC/bkg/QCD/photondump/HT%s/%s/plots.root",qcdHTs[iqcd].Data(),cuts.Data()),"UPDATE");
    for (UInt_t ihist = 0; ihist < inputnames.size(); ihist++)
    {
      TH1F * hist  = (TH1F*)qcdfiles[iqcd]->Get(inputnames[ihist].Data());
      hist->SetDirectory(0);
      TH1F * clone = (TH1F*)hist->Clone(outputnames[ihist].Data());
      delete hist;
      
      qcdfiles[iqcd]->cd();
      clone->Write(clone->GetName(),TObject::kWriteDelete);
      qcdfiles[iqcd]->Delete(Form("%s;1",inputnames[ihist].Data()));
    }
    delete qcdfiles[iqcd];
  }
}

void renameGJets(const std::vector<TString>& inputnames, const std::vector<TString>& outputnames, const TString & cuts)
{
  std::vector<TString> gjetsHTs = {"40To100","100To200","200To400","400To600","600ToInf"};

  std::vector<TFile*> gjetsfiles(gjetsHTs.size());
  for (UInt_t igjets = 0; igjets < gjetsHTs.size(); igjets++)
  {
    gjetsfiles[igjets] = TFile::Open(Form("output/MC/bkg/GJets/photondump/HT%s/%s/plots.root",gjetsHTs[igjets].Data(),cuts.Data()),"UPDATE");
    for (UInt_t ihist = 0; ihist < inputnames.size(); ihist++)
    {
      TH1F * hist  = (TH1F*)gjetsfiles[igjets]->Get(inputnames[ihist].Data());
      hist->SetDirectory(0);
      TH1F * clone = (TH1F*)hist->Clone(outputnames[ihist].Data());
      delete hist;
      
      gjetsfiles[igjets]->cd();
      clone->Write(clone->GetName(),TObject::kWriteDelete);
      gjetsfiles[igjets]->Delete(Form("%s;1",inputnames[ihist].Data()));
    }
    delete gjetsfiles[igjets];
  }
}

void renameGMSB(const std::vector<TString>& inputnames, const std::vector<TString>& outputnames, const TString & cuts)
{
  std::vector<TString> gmsbCTaus = {"100","2000","6000"};

  std::vector<TFile*> gmsbfiles(gmsbCTaus.size());
  for (UInt_t igmsb = 0; igmsb < gmsbCTaus.size(); igmsb++)
  {
    gmsbfiles[igmsb] = TFile::Open(Form("output/MC/signal/GMSB/photondump/ctau%s/%s/plots.root",gmsbCTaus[igmsb].Data(),cuts.Data()),"UPDATE");
    for (UInt_t ihist = 0; ihist < inputnames.size(); ihist++)
    {
      TH1F * hist  = (TH1F*)gmsbfiles[igmsb]->Get(inputnames[ihist].Data());
      hist->SetDirectory(0);
      TH1F * clone = (TH1F*)hist->Clone(outputnames[ihist].Data());
      delete hist;
      
      gmsbfiles[igmsb]->cd();
      clone->Write(clone->GetName(),TObject::kWriteDelete);
      gmsbfiles[igmsb]->Delete(Form("%s;1",inputnames[ihist].Data()));
    }
    delete gmsbfiles[igmsb];
  }
}

void renameHVDS(const std::vector<TString>& inputnames, const std::vector<TString>& outputnames, const TString & cuts)
{
  TFile * hvdsfile = TFile::Open(Form("output/MC/signal/HVDS/photondump/%s/plots.root",cuts.Data()),"UPDATE");
  for (UInt_t ihist = 0; ihist < inputnames.size(); ihist++)
  {
    TH1F * hist  = (TH1F*)hvdsfile->Get(inputnames[ihist].Data());
    hist->SetDirectory(0);
    TH1F * clone = (TH1F*)hist->Clone(outputnames[ihist].Data());
    delete hist;
    
    hvdsfile->cd();
    clone->Write(clone->GetName(),TObject::kWriteDelete);
    hvdsfile->Delete(Form("%s;1",inputnames[ihist].Data()));
  }
  delete hvdsfile;
}
