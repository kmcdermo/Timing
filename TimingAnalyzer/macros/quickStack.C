#include "TStyle.h"
#include "TROOT.h"
#include "TFile.h"
#include "TH1F.h"
#include "THStack.h"
#include "TCanvas.h"
#include "TLegend.h"

#include "common/common.C"

#include <vector>

static const Float_t lumi = 36.46 * 1000; // pb

void getQCD(std::vector<TH1F*>&, std::vector<TString>&);
void getGJets(std::vector<TH1F*>&, std::vector<TString>&);
void getSignals(std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TString>&);
void drawAll(std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TString>&);
void delTH1FVec(std::vector<TH1F*>&);

void quickStack()
{
  // set TDR Style (need to force it!)
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  std::vector<TString> histnames = {"njets","ph1pt","ph1smaj","ph1smin"};

  // BKGs
  std::vector<TH1F*> qcdTH1Fs;
  getQCD(qcdTH1Fs,histnames);

  std::vector<TH1F*> gjetsTH1Fs;
  getGJets(gjetsTH1Fs,histnames);

  // Totals and scale
  std::vector<TH1F*> bkgTH1Fs;
  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  {
    bkgTH1Fs[ihist] = qcdTH1Fs[ihist];
    bkgTH1Fs[ihist]->Add(gjetsTH1Fs[ihist]);
    bkgTH1Fs[ihist]->Scale(1.f/bkgTH1Fs[ihist]->Integral());
  }
  delTH1FVec(qcdTH1Fs);
  delTH1FVec(gjetsTH1Fs);

  // Signals
  std::vector<TH1F*> ctau100TH1Fs, ctau2000TH1Fs, ctau6000TH1Fs;
  getSignals(ctau100TH1Fs,ctau2000TH1Fs,ctau6000TH1Fs,histnames);

  // Draw everything together
  drawAll(bkgTH1Fs,ctau100TH1Fs,ctau2000TH1Fs,ctau6000TH1Fs,histnames);

  delTH1FVec(bkgTH1Fs);
  delTH1FVec(ctau100TH1Fs);
  delTH1FVec(ctau2000TH1Fs);
  delTH1FVec(ctau6000TH1Fs);
}

void getQCD(std::vector<TH1F*>& qcdTH1Fs, std::vector<TString>& histnames)
{
  std::vector<TString> qcdHTs = {"100To200","200To300","300To500","500To700","700To1000","1000To1500","1500To2000","2000ToInf"};
  std::vector<Float_t> qcdnes = {387775,356732,323810,299995,186869,237396,203266,228166}; //nEvents
  std::vector<Float_t> qcdxss = {27540000,1717000,351300,31630,6802,1206,120.4,25.25}; //pb

  std::vector<TFile*>  qcdfiles(qcdHTs.size());
  for (UInt_t iqcd = 0; iqcd < qcdHTs.size(); iqcd++)
  {
    qcdfiles[iqcd] = TFile::Open(Form("output/MC/bkg/QCD/nocuts/%s/cuts_jetpt35.0_phVIDmedium_rhE1.0_ecalaccept/plots.root",qcdHTs[iqcd].Data()));
  }

  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  {
    std::vector<TH1F*> qcdhists(qcdHTs.size());
    for (UInt_t iqcd = 0; iqcd < qcdHTs.size(); iqcd++)
    {
      qcdhists[iqcd] = (TH1F*)qcdfiles[iqcd]->Get(histnames[ihist].Data());
      qcdhists[iqcd]->Scale((1.0/qcdnes[iqcd])*(qcdxss[iqcd]*lumi));
    }
    
    for (UInt_t iqcd = 0; iqcd < qcdHTs.size(); iqcd++)
    {
      if   (iqcd>0) qcdTH1Fs[ihist]->Add(qcdhists[iqcd]);
      else          qcdTH1Fs[ihist] = qcdhists[iqcd];
    }
    delTH1FVec(qcdhists);

    qcdTH1Fs[ihist]->SetLineColor(kBlack);
    qcdTH1Fs[ihist]->SetFillColor(kGreen+2);
  }

  for (UInt_t iqcd = 0; iqcd < qcdHTs.size(); iqcd++)  
  {
    delete qcdfiles[iqcd];
  }
}

void getGJets(std::vector<TH1F*>& gjetsTH1Fs, std::vector<TString>& histnames)
{
  std::vector<TString> gjetsHTs = {"40To100","100To200","200To400","400To600","600ToInf"};
  std::vector<Float_t> gjetsnes = {227636,281313,234157,282915,196590}; //nEvents
  std::vector<Float_t> gjetsxss = {20730,9226,2300,277.4,93.38}; //pb

  std::vector<TFile*>  gjetsfiles(gjetsHTs.size());
  for (UInt_t igjets = 0; igjets < gjetsHTs.size(); igjets++)
  {
    gjetsfiles[igjets] = TFile::Open(Form("output/MC/bkg/GJets/nocuts/%s/cuts_jetpt35.0_phVIDmedium_rhE1.0_ecalaccept/plots.root",gjetsHTs[igjets].Data()));
  }

  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  {
    std::vector<TH1F*> gjetshists(gjetsHTs.size());
    for (UInt_t igjets = 0; igjets < gjetsHTs.size(); igjets++)
    {
      gjetshists[igjets] = (TH1F*)gjetsfiles[igjets]->Get(histnames[ihist].Data());
      gjetshists[igjets]->Scale((1.0/gjetsnes[igjets])*(gjetsxss[igjets]*lumi));
    }
    
    for (UInt_t igjets = 0; igjets < gjetsHTs.size(); igjets++)
    {
      if   (igjets>0) gjetsTH1Fs[ihist]->Add(gjetshists[igjets]);
      else            gjetsTH1Fs[ihist] = gjetshists[igjets];
    }
    delTH1FVec(gjetshists);
    
    gjetsTH1Fs[ihist]->SetLineColor(kBlack);
    gjetsTH1Fs[ihist]->SetFillColor(kYellow-7);
  }

  for (UInt_t igjets = 0; igjets < gjetsHTs.size(); igjets++)  
  {
    delete gjetsfiles[igjets];
  }
}

void getSignals(std::vector<TH1F*>& ctau100TH1Fs, std::vector<TH1F*>& ctau2000TH1Fs, std::vector<TH1F*>& ctau6000TH1Fs, std::vector<TString>& histnames)
{
  // Signals
  TFile * file100  = TFile::Open("output/MC/signal/withReReco/nocuts/ctau100/cuts_jetpt35.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  TFile * file2000 = TFile::Open("output/MC/signal/withReReco/nocuts/ctau2000/cuts_jetpt35.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  TFile * file6000 = TFile::Open("output/MC/signal/withReReco/nocuts/ctau6000/cuts_jetpt35.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");

  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  {
    ctau100TH1Fs[ihist] = (TH1F*)file100->Get(histnames[ihist].Data());     
    ctau100TH1Fs[ihist]->Scale(1.0/ctau100TH1Fs[ihist]->Integral());
    ctau100TH1Fs[ihist]->SetLineColor(kViolet-1);

    ctau2000TH1Fs[ihist] = (TH1F*)file2000->Get(histnames[ihist].Data());     
    ctau2000TH1Fs[ihist]->Scale(1.0/ctau2000TH1Fs[ihist]->Integral());
    ctau2000TH1Fs[ihist]->SetLineColor(kRed+1);

    ctau6000TH1Fs[ihist] = (TH1F*)file6000->Get(histnames[ihist].Data());     
    ctau6000TH1Fs[ihist]->Scale(1.0/ctau6000TH1Fs[ihist]->Integral());
    ctau6000TH1Fs[ihist]->SetLineColor(kBlue+1);
  }

  delete file100;
  delete file2000;
  delete file6000;
}

void drawAll(std::vector<TH1F*>& bkgTH1Fs, std::vector<TH1F*>& ctau100TH1Fs, std::vector<TH1F*>& ctau2000TH1Fs, std::vector<TH1F*>& ctau6000TH1Fs, std::vector<TString>& histnames)
{
  // Draw it all
  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  {
    TCanvas * canv = new TCanvas("canv","canv");
    canv->cd();
    canv->SetLogy();
    bkgTH1Fs     [ihist]->GetYaxis()->SetRangeUser(5e-5,3);
    bkgTH1Fs     [ihist]->Draw("HIST");
    ctau100TH1Fs [ihist]->Draw("HIST same");
    ctau2000TH1Fs[ihist]->Draw("HIST same");
    ctau6000TH1Fs[ihist]->Draw("HIST same");
    
    TLegend * leg = new TLegend(0.6,0.7,0.8,0.9);
    leg->AddEntry(bkgTH1Fs     [ihist],"Background","f");
    leg->AddEntry(ctau100TH1Fs [ihist],"c#tau = 36.5 mm" ,"l");
    leg->AddEntry(ctau2000TH1Fs[ihist],"c#tau = 730.5 mm","l");
    leg->AddEntry(ctau6000TH1Fs[ihist],"c#tau = 2192 mm","l");
    leg->Draw("same");
    
    CMSLumi(canv,"Simulation");
    canv->SaveAs(Form("%s.png",histnames[ihist].Data()));

    delete leg;
    delete canv;
  }
}

void delTH1FVec(std::vector<TH1F*>& histvec)
{
  for (auto&& hist : histvec)
  {
    delete hist;
  }
}

