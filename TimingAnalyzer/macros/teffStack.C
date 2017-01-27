#include "TStyle.h"
#include "TROOT.h"
#include "TFile.h"
#include "TEfficiency.h"
#include "TGraphAsymmErrors.h"
#include "TCanvas.h"
#include "TLegend.h"

#include "common/common.C"

#include <fstream>
#include <vector>

TString indir  = "output/vidEffs";
TString outdir = "output/vidEffs/stacks";

void getPlotnames(std::vector<TString> &);
void getQCD(std::vector<TGraphAsymmErrors*>&, std::vector<TString>&);
void getGJets(std::vector<TGraphAsymmErrors*>&, std::vector<TString>&);
void getData(std::vector<TEfficiency*>&, std::vector<TString>&);
void getSignals(std::vector<TEfficiency*>&, std::vector<TEfficiency*>&, std::vector<TEfficiency*>&, std::vector<TString>&);
void drawAll(std::vector<TEfficiency*>&, 
	     std::vector<TEfficiency*>&, std::vector<TEfficiency*>&, std::vector<TEfficiency*>&, 
	     std::vector<GraphAsymmErrors*>&, std::vector<GraphAsymmErrors*>&, std::vector<TString>&);
void delTEfficiencyVec(std::vector<TEfficiency*>&);
void delTGraphAsymmErrorsVec(std::vector<TGraphAsymmErrors*>&);

void teffStack()
{
  // set TDR Style (need to force it!)
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  std::vector<TString> plotnames;
  getPlotnames(plotnames);

  // Data
  std::vector<TEfficiency*> dataTEffs(plotnames.size());
  getData(dataTEffs,plotnames);

  // Signal MC
  std::vector<TEfficiency*> ctau100TEffs(plotnames.size()), ctau2000TEffs(plotnames.size()), ctau6000TEffs(plotnames.size());
  getSignals(ctau100TEffs,ctau2000TEffs,ctau6000TEffs,plotnames);

  // GJets MC
  std::vector<TGraphAsymmErrors*> gjetTGAEs(plotnames.size());
  getGJets(gjetTGAEs,plotnames);

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
  drawAll(dataTEffs,ctau100TEffs,ctau2000TEffs,ctau6000TEffs,gjetTGAEs,gjetTGAEs,plotnames);

  delTEfficiencyVec(dataTEffs);
  delTEfficiencyVec(ctau100TEffs);
  delTEfficiencyVec(ctau2000TEffs);
  delTEfficiencyVec(ctau6000TEffs);
  delTGraphAsymmErrorsVec(gjetTGAEs);
}

void getPlotnames(std::vector<TString> & plotnames) 
{
  std::ifstream histdump;
  histdump.open(Form("%s/histdump.txt",indir.Data()),std::ios::in);
  
  TString plotname; 
  while (histdump >> plotname)
  {
    plotnames.push_back(plotname);
  }
  histdump.close();
}

void getData(std::vector<TEfficiency*>& dataTEffs, std::vector<TString>& plotnames)
{
  TFile * datafile = TFile::Open(Form("%s/data/plots.root",indir.Data()));

  for (UInt_t ihist = 0; ihist < plotnames.size(); ihist++)
  {
    dataTEffs[ihist] = (TEfficiency*)datafile->Get(plotnames[ihist].Data());     
    dataTEffs[ihist]->SetDirectory(0);
    dataTEffs[ihist]->SetLineColor(kBlack);
  }

  delete datafile;
}

void getSignals(std::vector<TEfficiency*>& ctau100TEffs, std::vector<TEfficiency*>& ctau2000TEffs, std::vector<TEfficiency*>& ctau6000TEffs, std::vector<TString>& plotnames)
{
  // Signals
  TFile * file100  = TFile::Open(Form("%s/ctau100/plots.root" ,indir.Data()));
  TFile * file2000 = TFile::Open(Form("%s/ctau2000/plots.root",indir.Data()));
  TFile * file6000 = TFile::Open(Form("%s/ctau6000/plots.root",indir.Data()));

  for (UInt_t ihist = 0; ihist < plotnames.size(); ihist++)
  {
    ctau100TEffs[ihist] = (TEfficiency*)file100->Get(plotnames[ihist].Data());     
    ctau100TEffs[ihist]->SetDirectory(0);
    ctau100TEffs[ihist]->SetLineColor(kRed+1);

    ctau2000TEffs[ihist] = (TEfficiency*)file2000->Get(plotnames[ihist].Data());     
    ctau2000TEffs[ihist]->SetDirectory(0);
    ctau2000TEffs[ihist]->SetLineColor(kViolet-1);

    ctau6000TEffs[ihist] = (TEfficiency*)file6000->Get(plotnames[ihist].Data());     
    ctau6000TEffs[ihist]->SetDirectory(0);
    ctau6000TEffs[ihist]->SetLineColor(kBlue+1);
  }

  delete file100;
  delete file2000;
  delete file6000;
}

void getGJets(std::vector<TGraphAsymmErrors*>& gjetsTGAEs, std::vector<TString>& plotnames)
{
  std::vector<TString> gjetsHTs = {"40To100","100To200","200To400","400To600","600ToInf"};
  std::vector<Float_t> gjetsnes = {227636,281313,234157,282915,196590}; //nEvents
  std::vector<Float_t> gjetsxss = {20730,9226,2300,277.4,93.38}; //pb
  Double_t gjetsws[gjetsHTs.size()];

  for( UInt_t igjets = 0; igjets < gjetsHTs.size(); igjets++)
  {
    gjetsws[igjets] = gjetsxss[igjets] / gjetsnes[igjets];
  }

  std::vector<TFile*> gjetsfiles(gjetsHTs.size());
  for (UInt_t igjets = 0; igjets < gjetsHTs.size(); igjets++)
  {
    gjetsfiles[igjets] = TFile::Open(Form("%s/GJets/%s/plots.root",indir.Data(),gjetsHTs[igjets].Data()));
  }

  for (UInt_t ihist = 0; ihist < plotnames.size(); ihist++)
  {
    std::vector<TEfficiency*> gjetsteffs(gjetsHTs.size());
    TList * gjetsteffslist = new TList();
    for (UInt_t igjets = 0; igjets < gjetsHTs.size(); igjets++)
    {
      gjetsteffs[igjets] = (TEfficiency*)gjetsfiles[igjets]->Get(plotnames[ihist].Data());
      gjetsteffslist->Add(gjetsteffs[igjets]);
    }
    
    // now combine 
    gjetsTGAEs[igjets] = TEFficiency::Combine(gjetsteffslist,"",gjetsHTs.size(),gjetsws);

    // delete the temporary list stuff
    gjetseffslist->Clear();
    delete gjetseffslist;
    delTEfficiencyVec(gjetsteffs);
    
    gjetsTGAEs[ihist]->SetDirectory(0);
    gjetsTGAEs[ihist]->SetLineColor(kBlack);
    gjetsTGAEs[ihist]->SetFillColor(kOrange);
  }

  for (UInt_t igjets = 0; igjets < gjetsHTs.size(); igjets++)  
  {
    delete gjetsfiles[igjets];
  }
}

void drawAll(std::vector<TEfficiency*>& dataTEffs, 
	     std::vector<TEfficiency*>& ctau100TEffs, std::vector<TEfficiency*>& ctau2000TEffs, std::vector<TEfficiency*>& ctau6000TEffs, 
	     std::vector<TGraphAssymErrors*>& gjetTGAEs, std::vector<TGraphAssymErrors*>& qcdTGAEs, 
	     std::vector<TString>& plotnames)
{
  // Draw it all
  for (UInt_t ihist = 0; ihist < plotnames.size(); ihist++)
  {
    TCanvas * canv = new TCanvas("canv","canv");
    canv->cd();
    dataTEffs    [ihist]->Draw("AP");
    ctau100TEffs [ihist]->Draw("P same");
    ctau2000TEffs[ihist]->Draw("P same");
    ctau6000TEffs[ihist]->Draw("P same");

    // change ranges
    canv->Update();
    dataTEffs[ihist]->GetPaintedGraph()->GetXaxis()->SetLimits(1,1.5e3);
    dataTEffs[ihist]->GetPaintedGraph()->GetYaxis()->SetRangeUser(2e-2,3e0);
    canv->Update();    

    TLegend * leg = new TLegend(0.35,0.77,0.55,0.92);
    leg->AddEntry(dataTEffs    [ihist],"Data","lp");
    leg->AddEntry(ctau100TEffs [ihist],"c#tau = 36.5 mm" ,"l");
    leg->AddEntry(ctau2000TEffs[ihist],"c#tau = 730.5 mm","l");
    leg->AddEntry(ctau6000TEffs[ihist],"c#tau = 2192 mm","l");
    leg->Draw("same");
    
    CMSLumi(canv,"Preliminary");

    // save as log first
    canv->cd();
    canv->SetLogy(1);
    canv->SetLogx(1);
    canv->SaveAs(Form("%s/log/%s.png",outdir.Data(),plotnames[ihist].Data()));

    // save as lin second
    canv->cd();
    canv->SetLogy(0);
    canv->SetLogx(0);
    dataTEffs    [ihist]->Draw("AP");
    ctau100TEffs [ihist]->Draw("P same");
    ctau2000TEffs[ihist]->Draw("P same");
    ctau6000TEffs[ihist]->Draw("P same");

    // change ranges
    canv->Update();
    dataTEffs[ihist]->GetPaintedGraph()->GetXaxis()->SetRangeUser(0,1.5e3);
    dataTEffs[ihist]->GetPaintedGraph()->GetYaxis()->SetRangeUser(0.f,1.f);
    canv->Update();    

    leg->Draw("same");

    CMSLumi(canv,"Preliminary");

    canv->SaveAs(Form("%s/lin/%s.png",outdir.Data(),plotnames[ihist].Data()));

    delete leg;
    delete canv;
  }
}

void delTEfficiencyVec(std::vector<TEfficiency*>& teffvec)
{
  for (auto&& teff : teffvec)
  {
    delete teff;
  }
}

void delTGraphAsymmErrorsVec(std::vector<TGraphAsymmErrors*>& teffvec)
{
  for (auto&& teff : teffvec)
  {
    delete teff;
  }
}

