// gamma+jets old was kyellow-7

#include "TStyle.h"
#include "TROOT.h"
#include "TFile.h"
#include "TH1F.h"
#include "THStack.h"
#include "TCanvas.h"
#include "TLegend.h"

#include "common/common.C"

#include <vector>

//static const Float_t lumi = 36.46 * 1000; // pb

void makeDir(const TString &);
void getQCD(std::vector<TH1F*>&, std::vector<TString>&, const TString &, const TString &);
void getGJets(std::vector<TH1F*>&, std::vector<TString>&, const TString &, const TString &);
void getGMSB(std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TString>&, const TString &, const TString &);
void getHVDS(std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TString>&, const TString &, const TString &);
void drawAll(std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TString>&, const TString &);
void drawStack(std::vector<THStack*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TString>&, const TString &);
void drawAllSeparate(std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TH1F*>&, std::vector<TString>&, const TString &);
void delTH1FVec(std::vector<TH1F*>&);
void delTHStackVec(std::vector<THStack*>&);

void quickStack()
{
  // set TDR Style (need to force it!)
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  //std::vector<TString> histnames = {"phdelaypt","phdelayHoE","phdelayr9","phdelaysieie","phdelaysmaj","phdelaysmin","phdelayseedtime"};
  std::vector<TString> histnames = {"ph1pt","ph1HoE","ph1r9","ph1sieie","ph1smaj","ph1smin","ph1seedtime"};

  // generic settings
  TString cuts = "cuts_jetpt35.0_njets3_ph1pt50.0_ph1VIDmedium_phanypt10.0_phanyVIDloose_rhE1.0";
  TString region = "EBOnly";
  TString outdir = "output/stacks/photondump/"+cuts+"_"+region;
  makeDir(outdir);

  // BKGs
  std::vector<TH1F*> qcdTH1Fs(histnames.size());
  getQCD(qcdTH1Fs,histnames,cuts,region);

  std::vector<TH1F*> gjetsTH1Fs(histnames.size());
  getGJets(gjetsTH1Fs,histnames,cuts,region);

  // Totals and scale
  std::vector<TH1F*> bkgTH1Fs(histnames.size());
  std::vector<THStack*> bkgStacks(histnames.size());
  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  { 
    bkgTH1Fs[ihist] = (TH1F*)qcdTH1Fs[ihist]->Clone(Form("%s_bkg",histnames[ihist].Data()));
    bkgTH1Fs[ihist]->Add(gjetsTH1Fs[ihist]);

    const Float_t total      = bkgTH1Fs  [ihist]->Integral();
    const Float_t frac_qcd   = qcdTH1Fs  [ihist]->Integral() / total;
    const Float_t frac_gjets = gjetsTH1Fs[ihist]->Integral() / total;

    bkgTH1Fs  [ihist]->Scale(1.f/bkgTH1Fs[ihist]->Integral());
    qcdTH1Fs  [ihist]->Scale((1.f/qcdTH1Fs[ihist]->Integral())*(frac_qcd));  
    gjetsTH1Fs[ihist]->Scale((1.f/gjetsTH1Fs[ihist]->Integral())*(frac_gjets));  
  
    //Make stacks
    bkgStacks[ihist] = new THStack(Form("%s_stack",histnames[ihist].Data()),"");
    bkgStacks[ihist]->Add(gjetsTH1Fs[ihist]);
    bkgStacks[ihist]->Add(qcdTH1Fs[ihist]);
  }

  // GMSB
  std::vector<TH1F*> gmsb100TH1Fs(histnames.size()), gmsb2000TH1Fs(histnames.size()), gmsb6000TH1Fs(histnames.size());
  getGMSB(gmsb100TH1Fs,gmsb2000TH1Fs,gmsb6000TH1Fs,histnames,cuts,region);

  // HVDS
  std::vector<TH1F*> hvds100TH1Fs(histnames.size()), hvds1000TH1Fs(histnames.size());
  getHVDS(hvds100TH1Fs,hvds1000TH1Fs,histnames,cuts,region);

  // Draw everything together
  drawAll(bkgTH1Fs,gmsb100TH1Fs,gmsb2000TH1Fs,gmsb6000TH1Fs,hvds100TH1Fs,hvds1000TH1Fs,histnames,outdir);
  drawStack(bkgStacks,qcdTH1Fs,gjetsTH1Fs,gmsb100TH1Fs,gmsb2000TH1Fs,gmsb6000TH1Fs,hvds100TH1Fs,hvds1000TH1Fs,histnames,outdir);
  drawAllSeparate(qcdTH1Fs,gjetsTH1Fs,gmsb100TH1Fs,gmsb2000TH1Fs,gmsb6000TH1Fs,hvds100TH1Fs,hvds1000TH1Fs,histnames,outdir);

  delTH1FVec(qcdTH1Fs);
  delTH1FVec(gjetsTH1Fs);
  delTH1FVec(bkgTH1Fs);
  delTHStackVec(bkgStacks);
  delTH1FVec(gmsb100TH1Fs);
  delTH1FVec(gmsb2000TH1Fs);
  delTH1FVec(gmsb6000TH1Fs);
  delTH1FVec(hvds100TH1Fs);
  delTH1FVec(hvds1000TH1Fs);
}

void getQCD(std::vector<TH1F*>& qcdTH1Fs, std::vector<TString>& histnames, const TString & cuts, const TString & region)
{
  std::vector<TString> qcdHTs = {"100To200","200To300","300To500","500To700","700To1000","1000To1500","1500To2000","2000ToInf"};
  std::vector<Float_t> qcdnes = {387775,356732,323810,299995,186869,237396,203266,228166}; //nEvents
  std::vector<Float_t> qcdxss = {27540000,1717000,351300,31630,6802,1206,120.4,25.25}; //pb

  std::vector<TFile*> qcdfiles(qcdHTs.size());
  for (UInt_t iqcd = 0; iqcd < qcdHTs.size(); iqcd++)
  {
    qcdfiles[iqcd] = TFile::Open(Form("output/MC/bkg/QCD/photondump/HT%s/%s_antiphmc_%s/plots.root",qcdHTs[iqcd].Data(),cuts.Data(),region.Data()));
  }

  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  {
    std::vector<TH1F*> qcdhists(qcdHTs.size());
    for (UInt_t iqcd = 0; iqcd < qcdHTs.size(); iqcd++)
    {
      qcdhists[iqcd] = (TH1F*)qcdfiles[iqcd]->Get(histnames[ihist].Data());
      //      qcdhists[iqcd]->Scale((1.0/qcdnes[iqcd])*(qcdxss[iqcd]*lumi));
      qcdhists[iqcd]->Scale((1.0/qcdnes[iqcd])*(qcdxss[iqcd]));
    }
    
    for (UInt_t iqcd = 0; iqcd < qcdHTs.size(); iqcd++)
    {
      if   (iqcd>0) qcdTH1Fs[ihist]->Add(qcdhists[iqcd]);
      else          qcdTH1Fs[ihist] = (TH1F*)qcdhists[iqcd]->Clone(Form("%s_qcd",histnames[ihist].Data()));
    }
    delTH1FVec(qcdhists);

    qcdTH1Fs[ihist]->SetDirectory(0);
    qcdTH1Fs[ihist]->SetLineColor(kBlack);
    qcdTH1Fs[ihist]->SetFillColor(kGreen+2);
  }

  for (UInt_t iqcd = 0; iqcd < qcdHTs.size(); iqcd++)  
  {
    delete qcdfiles[iqcd];
  }
}

void getGJets(std::vector<TH1F*>& gjetsTH1Fs, std::vector<TString>& histnames, const TString & cuts, const TString & region)
{
  std::vector<TString> gjetsHTs = {"40To100","100To200","200To400","400To600","600ToInf"};
  std::vector<Float_t> gjetsnes = {227636,281313,234157,282915,196590}; //nEvents
  std::vector<Float_t> gjetsxss = {20730,9226,2300,277.4,93.38}; //pb

  std::vector<TFile*> gjetsfiles(gjetsHTs.size());
  for (UInt_t igjets = 0; igjets < gjetsHTs.size(); igjets++)
  {
    gjetsfiles[igjets] = TFile::Open(Form("output/MC/bkg/GJets/photondump/HT%s/%s_exactphmc_%s/plots.root",gjetsHTs[igjets].Data(),cuts.Data(),region.Data()));
  }

  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  {
    std::vector<TH1F*> gjetshists(gjetsHTs.size());
    for (UInt_t igjets = 0; igjets < gjetsHTs.size(); igjets++)
    {
      gjetshists[igjets] = (TH1F*)gjetsfiles[igjets]->Get(histnames[ihist].Data());
      //gjetshists[igjets]->Scale((1.0/gjetsnes[igjets])*(gjetsxss[igjets]*lumi));
      gjetshists[igjets]->Scale((1.0/gjetsnes[igjets])*(gjetsxss[igjets]));
    }
    
    for (UInt_t igjets = 0; igjets < gjetsHTs.size(); igjets++)
    {
      if   (igjets>0) gjetsTH1Fs[ihist]->Add(gjetshists[igjets]);
      else            gjetsTH1Fs[ihist] = (TH1F*)gjetshists[igjets]->Clone(Form("%s_gjets",histnames[ihist].Data()));
    }
    delTH1FVec(gjetshists);
    
    gjetsTH1Fs[ihist]->SetDirectory(0);
    gjetsTH1Fs[ihist]->SetLineColor(kBlack);
    gjetsTH1Fs[ihist]->SetFillColor(kOrange);
  }

  for (UInt_t igjets = 0; igjets < gjetsHTs.size(); igjets++)  
  {
    delete gjetsfiles[igjets];
  }
}

void getGMSB(std::vector<TH1F*>& gmsb100TH1Fs, std::vector<TH1F*>& gmsb2000TH1Fs, std::vector<TH1F*>& gmsb6000TH1Fs, std::vector<TString>& histnames, const TString & cuts, const TString & region)
{
  // GMSB
  TFile * file100  = TFile::Open(Form("output/MC/signal/GMSB/photondump/ctau100/%s_exactphmc_%s/plots.root",cuts.Data(),region.Data()));
  TFile * file2000 = TFile::Open(Form("output/MC/signal/GMSB/photondump/ctau2000/%s_exactphmc_%s/plots.root",cuts.Data(),region.Data()));
  TFile * file6000 = TFile::Open(Form("output/MC/signal/GMSB/photondump/ctau6000/%s_exactphmc_%s/plots.root",cuts.Data(),region.Data()));

  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  {
    gmsb100TH1Fs[ihist] = (TH1F*)file100->Get(histnames[ihist].Data());     
    gmsb100TH1Fs[ihist]->SetDirectory(0);
    gmsb100TH1Fs[ihist]->Scale(1.0/gmsb100TH1Fs[ihist]->Integral());
    gmsb100TH1Fs[ihist]->SetLineColor(kRed+1);

    gmsb2000TH1Fs[ihist] = (TH1F*)file2000->Get(histnames[ihist].Data());     
    gmsb2000TH1Fs[ihist]->SetDirectory(0);
    gmsb2000TH1Fs[ihist]->Scale(1.0/gmsb2000TH1Fs[ihist]->Integral());
    gmsb2000TH1Fs[ihist]->SetLineColor(kViolet-1);

    gmsb6000TH1Fs[ihist] = (TH1F*)file6000->Get(histnames[ihist].Data());     
    gmsb6000TH1Fs[ihist]->SetDirectory(0);
    gmsb6000TH1Fs[ihist]->Scale(1.0/gmsb6000TH1Fs[ihist]->Integral());
    gmsb6000TH1Fs[ihist]->SetLineColor(kBlue+1);
  }

  delete file100;
  delete file2000;
  delete file6000;
}

void getHVDS(std::vector<TH1F*>& hvds100TH1Fs, std::vector<TH1F*>& hvds1000TH1Fs, std::vector<TString>& histnames, const TString & cuts, const TString & region)
{
  // HVDS
  TFile * file100  = TFile::Open(Form("output/MC/signal/HVDS/photondump/ctau100/%s_exactphmc_%s/plots.root",cuts.Data(),region.Data()));
  TFile * file1000 = TFile::Open(Form("output/MC/signal/HVDS/photondump/ctau1000/%s_exactphmc_%s/plots.root",cuts.Data(),region.Data()));

  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  {
    hvds100TH1Fs[ihist] = (TH1F*)file100->Get(histnames[ihist].Data());     
    hvds100TH1Fs[ihist]->SetDirectory(0);
    hvds100TH1Fs[ihist]->Scale(1.0/hvds100TH1Fs[ihist]->Integral());
    hvds100TH1Fs[ihist]->SetLineColor(kPink-4);

    hvds1000TH1Fs[ihist] = (TH1F*)file1000->Get(histnames[ihist].Data());     
    hvds1000TH1Fs[ihist]->SetDirectory(0);
    hvds1000TH1Fs[ihist]->Scale(1.0/hvds1000TH1Fs[ihist]->Integral());
    hvds1000TH1Fs[ihist]->SetLineColor(kAzure+6);
  }

  delete file100;
  delete file1000;
}

void drawAll(std::vector<TH1F*>& bkgTH1Fs, std::vector<TH1F*>& gmsb100TH1Fs, std::vector<TH1F*>& gmsb2000TH1Fs, std::vector<TH1F*>& gmsb6000TH1Fs, std::vector<TH1F*>& hvds100TH1Fs, std::vector<TH1F*>& hvds1000TH1Fs, std::vector<TString>& histnames, const TString & outdir)
{
  // Draw it all
  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  {
    TCanvas * canv = new TCanvas("canv","canv");
    canv->cd();
    canv->SetLogy();
    bkgTH1Fs     [ihist]->GetYaxis()->SetRangeUser(5e-6,5e1);
    bkgTH1Fs     [ihist]->Draw("HIST");
    gmsb100TH1Fs [ihist]->Draw("HIST same");
    gmsb2000TH1Fs[ihist]->Draw("HIST same");
    gmsb6000TH1Fs[ihist]->Draw("HIST same");
    hvds100TH1Fs [ihist]->Draw("HIST same");
    hvds1000TH1Fs[ihist]->Draw("HIST same");
    
    TLegend * leg = new TLegend(0.6,0.7,0.8,0.9);
    leg->AddEntry(bkgTH1Fs     [ihist],"Background","f");
    leg->AddEntry(gmsb100TH1Fs [ihist],"GMSB c#tau = 36.5 mm" ,"l");
    leg->AddEntry(gmsb2000TH1Fs[ihist],"GMSB c#tau = 730.5 mm","l");
    leg->AddEntry(gmsb6000TH1Fs[ihist],"GMSB c#tau = 2192 mm","l");
    leg->AddEntry(hvds100TH1Fs [ihist],"HVDS c#tau = 100 mm","l");
    leg->AddEntry(hvds1000TH1Fs[ihist],"HVDS c#tau = 1000 mm","l");
    leg->Draw("same");
    
    CMSLumi(canv,"Simulation");
    canv->SaveAs(Form("%s/hist_%s.png",outdir.Data(),histnames[ihist].Data()));

    delete leg;
    delete canv;
  }
}

void drawStack(std::vector<THStack*>& bkgStacks, std::vector<TH1F*>& qcdTH1Fs, std::vector<TH1F*>& gjetsTH1Fs, std::vector<TH1F*>& gmsb100TH1Fs, std::vector<TH1F*>& gmsb2000TH1Fs, std::vector<TH1F*>& gmsb6000TH1Fs, std::vector<TH1F*>& hvds100TH1Fs, std::vector<TH1F*>& hvds1000TH1Fs, std::vector<TString>& histnames, const TString & outdir)
{
  // Draw it all
  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  {
    TCanvas * canv = new TCanvas("canv","canv");
    canv->cd();
    canv->SetLogy();
    gmsb100TH1Fs [ihist]->GetYaxis()->SetRangeUser(5e-6,5e1);
    gmsb100TH1Fs [ihist]->Draw("HIST");    
    bkgStacks    [ihist]->Draw("HIST same");
    gmsb100TH1Fs [ihist]->Draw("HIST same");    
    gmsb2000TH1Fs[ihist]->Draw("HIST same");
    gmsb6000TH1Fs[ihist]->Draw("HIST same");
    hvds100TH1Fs [ihist]->Draw("HIST same");
    hvds1000TH1Fs[ihist]->Draw("HIST same");
    canv->RedrawAxis("same");

    TLegend * leg = new TLegend(0.6,0.7,0.8,0.9);
    leg->AddEntry(qcdTH1Fs     [ihist],"QCD","f");
    leg->AddEntry(gjetsTH1Fs   [ihist],"#gamma+jets","f");
    leg->AddEntry(gmsb100TH1Fs [ihist],"GMSB c#tau = 36.5 mm" ,"l");
    leg->AddEntry(gmsb2000TH1Fs[ihist],"GMSB c#tau = 730.5 mm","l");
    leg->AddEntry(gmsb6000TH1Fs[ihist],"GMSB c#tau = 2192 mm","l");
    leg->AddEntry(hvds100TH1Fs [ihist],"HVDS c#tau = 100 mm","l");
    leg->AddEntry(hvds1000TH1Fs[ihist],"HVDS c#tau = 1000 mm","l");
    leg->Draw("same");
    
    CMSLumi(canv,"Simulation");
    canv->SaveAs(Form("%s/stack_%s.png",outdir.Data(),histnames[ihist].Data()));

    delete leg;
    delete canv;
  }
}

void drawAllSeparate(std::vector<TH1F*>& qcdTH1Fs, std::vector<TH1F*>& gjetsTH1Fs, std::vector<TH1F*>& gmsb100TH1Fs, std::vector<TH1F*>& gmsb2000TH1Fs, std::vector<TH1F*>& gmsb6000TH1Fs, std::vector<TH1F*>& hvds100TH1Fs, std::vector<TH1F*>& hvds1000TH1Fs, std::vector<TString>& histnames, const TString & outdir)
{
  // Draw it all
  for (UInt_t ihist = 0; ihist < histnames.size(); ihist++)
  {
    // rescale back to unity
    qcdTH1Fs  [ihist]->Scale(1.f/qcdTH1Fs[ihist]->Integral());  
    gjetsTH1Fs[ihist]->Scale(1.f/gjetsTH1Fs[ihist]->Integral());  

    TCanvas * canv = new TCanvas("canv","canv");
    canv->cd();
    canv->SetLogy();
    qcdTH1Fs     [ihist]->GetYaxis()->SetRangeUser(5e-6,5e1);
    qcdTH1Fs     [ihist]->SetFillStyle(3005);
    qcdTH1Fs     [ihist]->Draw("HIST");
    //    gjetsTH1Fs   [ihist]->SetFillColorAlpha(gjetsTH1Fs[ihist]->GetFillColor(),0.5);
    gjetsTH1Fs   [ihist]->SetFillStyle(3004);
    gjetsTH1Fs   [ihist]->Draw("HIST same");
    gmsb100TH1Fs [ihist]->Draw("HIST same");
    gmsb2000TH1Fs[ihist]->Draw("HIST same");
    gmsb6000TH1Fs[ihist]->Draw("HIST same");
    hvds100TH1Fs [ihist]->Draw("HIST same");
    hvds1000TH1Fs[ihist]->Draw("HIST same");
    
    TLegend * leg = new TLegend(0.6,0.7,0.8,0.9);
    leg->AddEntry(qcdTH1Fs     [ihist],"QCD","f");
    leg->AddEntry(gjetsTH1Fs   [ihist],"#gamma+jets","f");
    leg->AddEntry(gmsb100TH1Fs [ihist],"GMSB c#tau = 36.5 mm" ,"l");
    leg->AddEntry(gmsb2000TH1Fs[ihist],"GMSB c#tau = 730.5 mm","l");
    leg->AddEntry(gmsb6000TH1Fs[ihist],"GMSB c#tau = 2192 mm","l");
    leg->AddEntry(hvds100TH1Fs [ihist],"HVDS c#tau = 100 mm","l");
    leg->AddEntry(hvds1000TH1Fs[ihist],"HVDS c#tau = 1000 mm","l");
    leg->Draw("same");
    
    CMSLumi(canv,"Simulation");
    canv->SaveAs(Form("%s/separate_%s.png",outdir.Data(),histnames[ihist].Data()));

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

void delTHStackVec(std::vector<THStack*>& stackvec)
{
  for (auto&& stack : stackvec)
  {
    delete stack;
  }
}

void makeDir(const TString & outdir)
{
  FileStat_t dummyFileStat; 
  if (gSystem->GetPathInfo(outdir.Data(), dummyFileStat) == 1)
  {
    TString mkDir = Form("mkdir -p %s",outdir.Data());
    gSystem->Exec(mkDir.Data());
  }
}
