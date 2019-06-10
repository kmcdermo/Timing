#include "Common.cpp+"

void draw_nRecHits(const TString & filename, const TString & text, const TString & label, const Double_t max);
TH1F * prepHist(const TString & name, const Color_t color, const Double_t max, const TString & var, TTree * tree, const TString & label, TLegend * leg);

void drawSharedHits()
{
  gStyle->SetOptStat(0);  

  draw_nRecHits("rechits/L200_CTau200.root","GMSB: #Lambda=200TeV, c#tau=200cm","L200_CTau200",1e5);
  draw_nRecHits("rechits/sph_2017F.root","SinglePhoton 2017F","sph_2017F",5e5);
}

void draw_nRecHits(const TString & filename, const TString & text, const TString & label, const Double_t max)
{
  // get inputs
  auto file = TFile::Open(filename.Data());
  Common::CheckValidFile(file,filename);

  const TString treename = "tree/tree";
  auto tree = (TTree*)file->Get(treename.Data());
  Common::CheckValidTree(tree,treename,filename);  
  
  // get legend
  auto leg = new TLegend(0.5,0.7,0.82,0.82);

  // make hists
  auto hist_pre  = prepHist("hist_pre" ,kRed ,max,"nSharedRH_Pre" ,tree,"Pre-Cleaning" ,leg);
  auto hist_post = prepHist("hist_post",kBlue,max,"nSharedRH_Post",tree,"Post-Cleaning",leg);
  
  // Text 
  auto pave_text = new TPaveText(0.5,0.6,0.82,0.68,"NDC");
  pave_text->AddText(text.Data());
  pave_text->SetTextAlign(11);
  pave_text->SetFillColorAlpha(pave_text->GetFillColor(),0);

  // canvas
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy();
  canv->SetTickx();
  canv->SetTicky();
  
  // draw
  hist_pre ->Draw("ep");
  hist_post->Draw("ep same");
  leg->Draw("same");
  pave_text->Draw("same");

  // save
  Common::SaveAs(canv,"nSharedHits_"+label);

  // delete it all
  delete canv;
  delete pave_text;
  delete hist_post;
  delete hist_pre;
  delete leg;
  delete tree;
  delete file;
}

TH1F * prepHist(const TString & name, const Color_t color, const Double_t max, const TString & var, TTree * tree, const TString & label, TLegend * leg)
{
  auto hist = new TH1F(name.Data(),"Number of Shared RecHits / Event;Number of Shared RecHits;Number of Events",40,0,40);
  hist->Sumw2(); 
  hist->SetLineColor(color);
  hist->SetMarkerColor(color); 
  hist->SetMarkerStyle(20);
  hist->GetYaxis()->SetRangeUser(0.5,2*max);
  leg->AddEntry(hist,label.Data(),"epl");

  tree->Draw(var+">>"+name,"","goff");

  return hist;
}
