#include "TStyle.h"
#include "TROOT.h"
#include "TFile.h"
#include "TH1F.h"
#include "THStack.h"
#include "TCanvas.h"
#include "TLegend.h"

#include "common/common.C"

#include <vector>

static const Float_t lumi = 36.56 * 1000; // pb

void getQCD(TH1F*&, TH1F *&);
void getGJets(TH1F*&, TH1F *&);

void precutshapes()
{
  // set TDR Style (need to force it!)
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  // BKGs
  TH1F * qcdnjets;
  TH1F * qcdph1pt;
  getQCD(qcdnjets,qcdph1pt);

  TH1F * gjetnjets;
  TH1F * gjetph1pt;
  getGJets(gjetnjets,gjetph1pt);

  // Totals 
  TH1F * bkgnjets = qcdnjets;
  TH1F * bkgph1pt = qcdph1pt;
  bkgnjets->Add(gjetnjets);
  bkgph1pt->Add(gjetph1pt);

  // scale that sum == 1.0
  float total     = bkgph1pt ->Integral();
  float frac_qcd  = qcdph1pt ->Integral() / total;
  float frac_gjet = gjetph1pt->Integral() / total;
  
  // scale to 1.0
  bkgnjets ->Scale(1.0/bkgnjets->Integral());
  bkgph1pt ->Scale(1.0/bkgph1pt->Integral());
  qcdnjets ->Scale((1.0/qcdnjets ->Integral())*(frac_qcd));
  qcdph1pt ->Scale((1.0/qcdph1pt ->Integral())*(frac_qcd));
  gjetnjets->Scale((1.0/gjetnjets->Integral())*(frac_gjet));
  gjetph1pt->Scale((1.0/gjetph1pt->Integral())*(frac_gjet));
  
  //Make stacks
  THStack * njetsstack = new THStack("njetsstack","");
  njetsstack->Add(gjetnjets);
  njetsstack->Add(qcdnjets);

  THStack * ph1ptstack = new THStack("ph1ptstack","");
  ph1ptstack->Add(gjetph1pt);
  ph1ptstack->Add(qcdph1pt);

  // Signals
  TFile * file100 = TFile::Open("output/MC/signal/withReReco/nocuts/ctau100/cuts_jetpt35.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  TH1F  * ctau100njets = (TH1F*)file100->Get("njets"); 
  TH1F  * ctau100ph1pt = (TH1F*)file100->Get("ph1pt"); 

  ctau100njets->Scale(1.0/ctau100njets->Integral());
  ctau100ph1pt->Scale(1.0/ctau100ph1pt->Integral());
  ctau100njets->SetLineColor(kViolet-1);
  ctau100ph1pt->SetLineColor(kViolet-1);

  TFile * file2000 = TFile::Open("output/MC/signal/withReReco/nocuts/ctau2000/cuts_jetpt35.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  TH1F  * ctau2000njets = (TH1F*)file2000->Get("njets"); 
  TH1F  * ctau2000ph1pt = (TH1F*)file2000->Get("ph1pt"); 

  ctau2000njets->Scale(1.0/ctau2000njets->Integral());
  ctau2000ph1pt->Scale(1.0/ctau2000ph1pt->Integral());
  ctau2000njets->SetLineColor(kRed+1);
  ctau2000ph1pt->SetLineColor(kRed+1);

  TFile * file6000 = TFile::Open("output/MC/signal/withReReco/nocuts/ctau6000/cuts_jetpt35.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  TH1F  * ctau6000njets = (TH1F*)file6000->Get("njets"); 
  TH1F  * ctau6000ph1pt = (TH1F*)file6000->Get("ph1pt"); 

  ctau6000njets->Scale(1.0/ctau6000njets->Integral());
  ctau6000ph1pt->Scale(1.0/ctau6000ph1pt->Integral());
  ctau6000njets->SetLineColor(kBlue+1);
  ctau6000ph1pt->SetLineColor(kBlue+1);

  // Draw it all
  TCanvas * njetscanv = new TCanvas("njetscanv","njetscanv");
  njetscanv->cd();
  njetscanv->SetLogy();
  bkgnjets     ->GetXaxis()->SetRangeUser(0,25);
  bkgnjets     ->GetXaxis()->SetTitle("nJets [p_{T} > 35 GeV/c]");
  bkgnjets     ->GetYaxis()->SetRangeUser(5e-5,3);
  bkgnjets     ->Draw("HIST");
  ctau100njets ->Draw("HIST same");
  ctau2000njets->Draw("HIST same");
  ctau6000njets->Draw("HIST same");

  TLegend * njetsleg = new TLegend(0.6,0.7,0.8,0.9);
  njetsleg->AddEntry(bkgnjets,"Background","f");
  njetsleg->AddEntry(ctau100njets ,"c#tau = 36.5 mm" ,"l");
  njetsleg->AddEntry(ctau2000njets,"c#tau = 730.5 mm","l");
  njetsleg->AddEntry(ctau6000njets,"c#tau = 2192 mm","l");
  njetsleg->Draw("same");

  CMSLumi(njetscanv,"Simulation");
  njetscanv->SaveAs("njets.png");
    
  TCanvas * ph1ptcanv = new TCanvas("ph1ptcanv","ph1ptcanv");
  ph1ptcanv->cd();
  ph1ptcanv->SetLogy();
  bkgph1pt     ->GetXaxis()->SetRangeUser(0,1500);
  bkgph1pt     ->GetXaxis()->SetTitle("Leading Medium ID Photon p_{T} [GeV/c]");
  bkgph1pt     ->GetYaxis()->SetRangeUser(1e-5,3);
  bkgph1pt     ->GetYaxis()->SetTitle("Events");
  bkgph1pt     ->Draw("HIST");
  ctau100ph1pt ->Draw("HIST same");
  ctau2000ph1pt->Draw("HIST same");
  ctau6000ph1pt->Draw("HIST same");

  TLegend * ph1ptleg = new TLegend(0.6,0.7,0.8,0.9);
  ph1ptleg->AddEntry(bkgph1pt,"Background","f");
  ph1ptleg->AddEntry(ctau100ph1pt ,"c#tau = 36.5 mm","l");
  ph1ptleg->AddEntry(ctau2000ph1pt,"c#tau = 730.5 mm","l");
  ph1ptleg->AddEntry(ctau6000ph1pt,"c#tau = 2192 mm","l");
  ph1ptleg->Draw("same");

  CMSLumi(ph1ptcanv,"Simulation");
  ph1ptcanv->SaveAs("ph1pt.png");
}

void getQCD(TH1F*& qcdnjets, TH1F *& qcdph1pt)
{
  std::vector<TString> qcdHTs = {"100To200","200To300","300To500","500To700","700To1000","1000To1500","1500To2000","2000ToInf"};
  std::vector<Float_t> qcdnes = {171217,140932,176621,184492,187035,180495,174838,170585}; //nEvents
  std::vector<Float_t> qcdxss = {27540000,1717000,351300,31630,6802,1206,120.4,25.25}; //pb
  std::vector<TFile*>  qcdfiles(qcdHTs.size());

  std::vector<TH1F*> qcdnjetss(qcdHTs.size());
  std::vector<TH1F*> qcdph1pts(qcdHTs.size());
  for (UInt_t iqcd = 0; iqcd < qcdHTs.size(); iqcd++)
  {
    qcdfiles[iqcd]  = TFile::Open(Form("output/MC/bkg/QCD/nocuts/%s/cuts_jetpt35.0_phVIDmedium_rhE1.0_ecalaccept/plots.root",qcdHTs[iqcd].Data()));
    qcdnjetss[iqcd] = (TH1F*)qcdfiles[iqcd]->Get("njets"); 
    qcdph1pts[iqcd] = (TH1F*)qcdfiles[iqcd]->Get("ph1pt"); 
    qcdnjetss[iqcd] ->Scale((1.0/qcdnes[iqcd])*(qcdxss[iqcd]*lumi));
    qcdph1pts[iqcd] ->Scale((1.0/qcdnes[iqcd])*(qcdxss[iqcd]*lumi));
  }

  for (UInt_t iqcd = 0; iqcd < qcdHTs.size(); iqcd++)
  {
    if (iqcd > 0)
    {
      qcdnjets->Add(qcdnjetss[iqcd]);
      qcdph1pt->Add(qcdph1pts[iqcd]);
    }
    else
    {
      qcdnjets = qcdnjetss[iqcd];
      qcdph1pt = qcdph1pts[iqcd];
    }
  }

  qcdnjets->SetLineColor(kBlack);
  qcdnjets->SetFillColor(kGreen+2);
  qcdph1pt->SetLineColor(kBlack);
  qcdph1pt->SetFillColor(kGreen+2);
}

void getGJets(TH1F*& gjetnjets, TH1F *& gjetph1pt)
{
  std::vector<TString> gjetHTs = {"40To100","100To200","200To400","400To600","600ToInf"};
  std::vector<Float_t> gjetnes = {110327,147189,117893,125644,125788}; //nEvents
  std::vector<Float_t> gjetxss = {20730,9226,2300,277.4,93.38}; //pb
  std::vector<TFile*>  gjetfiles(gjetHTs.size());

  std::vector<TH1F*> gjetnjetss(gjetHTs.size());
  std::vector<TH1F*> gjetph1pts(gjetHTs.size());
  for (UInt_t igjet = 0; igjet < gjetHTs.size(); igjet++)
  {
    gjetfiles[igjet]  = TFile::Open(Form("output/MC/bkg/GJets/nocuts/%s/cuts_jetpt35.0_phVIDmedium_rhE1.0_ecalaccept/plots.root",gjetHTs[igjet].Data()));
    gjetnjetss[igjet] = (TH1F*)gjetfiles[igjet]->Get("njets"); 
    gjetph1pts[igjet] = (TH1F*)gjetfiles[igjet]->Get("ph1pt"); 
    gjetnjetss[igjet] ->Scale((1.0/gjetnes[igjet])*(gjetxss[igjet]*lumi));
    gjetph1pts[igjet] ->Scale((1.0/gjetnes[igjet])*(gjetxss[igjet]*lumi));
  }

  for (UInt_t igjet = 0; igjet < gjetHTs.size(); igjet++)
  {
    if (igjet > 0)
    {
      gjetnjets->Add(gjetnjetss[igjet]);
      gjetph1pt->Add(gjetph1pts[igjet]);
    }
    else
    {
      gjetnjets = gjetnjetss[igjet];
      gjetph1pt = gjetph1pts[igjet];
    }
  }

  gjetnjets->SetLineColor(kBlack);
  gjetnjets->SetFillColor(kYellow-7);
  gjetph1pt->SetLineColor(kBlack);
  gjetph1pt->SetFillColor(kYellow-7);
}
