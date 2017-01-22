#include "TStyle.h"
#include "TROOT.h"
#include "TFile.h"
#include "TH1F.h"
#include "THStack.h"
#include "TCanvas.h"
#include "TLegend.h"

#include "common/common.C"

#include <fstream>
#include <vector>

TString indir  = "output/skim";
TString outdir = "output/skim/stacks";

void getData(std::vector<TH1F*>&, std::vector<TString>&);
void getSignals(std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TString>&);
void getHistNames(std::vector<TString> &);
void drawAll(std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TString>&);
void delTH1FVec(std::vector<TH1F*>&);

void skimStack()
{
  // set TDR Style (need to force it!)
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  std::vector<TString> histnames;
  getHistNames(histnames);

  // Data
  std::vector<TH1F*> dataTH1Fs(histnames.size());
  getData(dataTH1Fs,histnames);

  // Signals
  std::vector<TH1F*> ctau100TH1Fs(histnames.size()), ctau2000TH1Fs(histnames.size()), ctau6000TH1Fs(histnames.size());
  getSignals(ctau100TH1Fs,ctau2000TH1Fs,ctau6000TH1Fs,histnames);

  // make out dir
  FileStat_t dummyFileStat; 
  if (gSystem->GetPathInfo(outdir.Data(), dummyFileStat) == 1)
  {
    TString mkDir = Form("mkdir -p %s",outdir.Data());
    gSystem->Exec(mkDir.Data());

    TString mkLin = Form("mkdir -p %s/lin",outdir.Data());
    gSystem->Exec(mkLin.Data());

    TString mkLog = Form("mkdir -p %s/log",outdir.Data());
    gSystem->Exec(mkLog.Data());
  }
  
  // Draw everything together
  drawAll(dataTH1Fs,ctau100TH1Fs,ctau2000TH1Fs,ctau6000TH1Fs,histnames);

  delTH1FVec(dataTH1Fs);
  delTH1FVec(ctau100TH1Fs);
  delTH1FVec(ctau2000TH1Fs);
  delTH1FVec(ctau6000TH1Fs);
}

void getHistNames(std::vector<TString> & histnames) 
{
  std::ifstream histdump;
  histdump.open(Form("%s/histdump.txt",indir.Data()),std::ios::in);
  
  TString plotname; 
  while (histdump >> plotname)
  {
    histnames.push_back(plotname);
  }
  histdump.close();
}

void getData(std::vector<TH1F*>& dataTH1Fs, std::vector<TString>& histnames)
{
  TFile * datafile = TFile::Open(Form("%s/data/plots.root",indir.Data()));

  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  {
    dataTH1Fs[ihist] = (TH1F*)datafile->Get(histnames[ihist].Data());     
    dataTH1Fs[ihist]->SetDirectory(0);
    dataTH1Fs[ihist]->SetLineColor(kGray);
  }

  delete datafile;
}

void getSignals(std::vector<TH1F*>& ctau100TH1Fs, std::vector<TH1F*>& ctau2000TH1Fs, std::vector<TH1F*>& ctau6000TH1Fs, std::vector<TString>& histnames)
{
  // Signals
  TFile * file100  = TFile::Open(Form("%s/ctau100/plots.root" ,indir.Data()));
  TFile * file2000 = TFile::Open(Form("%s/ctau2000/plots.root",indir.Data()));
  TFile * file6000 = TFile::Open(Form("%s/ctau6000/plots.root",indir.Data()));

  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  {
    ctau100TH1Fs[ihist] = (TH1F*)file100->Get(histnames[ihist].Data());     
    ctau100TH1Fs[ihist]->SetDirectory(0);
    ctau100TH1Fs[ihist]->SetLineColor(kRed+1);

    ctau2000TH1Fs[ihist] = (TH1F*)file2000->Get(histnames[ihist].Data());     
    ctau2000TH1Fs[ihist]->SetDirectory(0);
    ctau2000TH1Fs[ihist]->SetLineColor(kViolet-1);

    ctau6000TH1Fs[ihist] = (TH1F*)file6000->Get(histnames[ihist].Data());     
    ctau6000TH1Fs[ihist]->SetDirectory(0);
    ctau6000TH1Fs[ihist]->SetLineColor(kBlue+1);
  }

  delete file100;
  delete file2000;
  delete file6000;
}

void drawAll(std::vector<TH1F*>& dataTH1Fs, std::vector<TH1F*>& ctau100TH1Fs, std::vector<TH1F*>& ctau2000TH1Fs, std::vector<TH1F*>& ctau6000TH1Fs, std::vector<TString>& histnames)
{
  // Draw it all
  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  {
    TCanvas * canv = new TCanvas("canv","canv");
    canv->cd();
    dataTH1Fs    [ihist]->GetYaxis()->SetRangeUser(5e-5,1.3);
    dataTH1Fs    [ihist]->Draw("EP");
    ctau100TH1Fs [ihist]->Draw("HIST same");
    ctau2000TH1Fs[ihist]->Draw("HIST same");
    ctau6000TH1Fs[ihist]->Draw("HIST same");
    
    TLegend * leg = new TLegend(0.6,0.7,0.8,0.9);
    leg->AddEntry(dataTH1Fs    [ihist],"Data","lp");
    leg->AddEntry(ctau100TH1Fs [ihist],"c#tau = 36.5 mm" ,"l");
    leg->AddEntry(ctau2000TH1Fs[ihist],"c#tau = 730.5 mm","l");
    leg->AddEntry(ctau6000TH1Fs[ihist],"c#tau = 2192 mm","l");
    leg->Draw("same");
    
    CMSLumi(canv,"Preliminary");

    // save as log first
    canv->cd();
    canv->SetLogy(1);
    canv->SaveAs(Form("%s/log/%s.png",outdir.Data(),histnames[ihist].Data()));

    // save as lin second
    canv->cd();
    canv->SetLogy(0);
    dataTH1Fs    [ihist]->GetYaxis()->SetRangeUser(0.f,1.f);
    dataTH1Fs    [ihist]->Draw("EP");
    canv->SaveAs(Form("%s/lin/%s.png",outdir.Data(),histnames[ihist].Data()));

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

