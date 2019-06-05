TH1F * prepHist(const TString & name, const Color_t color, const TString & var, TTree * tree, const TString & label, TLegend * leg)
{
  auto hist = new TH1F(name.Data(),"Number of Shared RecHits / Event;Number of Shared RecHits;Number of Events",40,0,40);
  hist->Sumw2(); 
  hist->SetLineColor(color);
  hist->SetMarkerColor(color); 
  hist->SetMarkerStyle(20);
  hist->GetYaxis()->SetRangeUser(0.5,2e5);
  leg->AddEntry(hist,label.Data(),"epl");

  tree->Draw(var+">>"+name,"","goff");

  return hist;
}

void drawSharedHits()
{
  gStyle->SetOptStat(0);

  // get inputs
  auto file = TFile::Open("recHitCounter.root");
  auto tree = (TTree*)file->Get("tree/tree");
  
  // get legend
  auto leg = new TLegend(0.5,0.7,0.82,0.82);

  // make hists
  auto hist_pre  = prepHist("hist_pre" ,kRed ,"nSharedRH_Pre" ,tree,"Pre-Cleaning" ,leg);
  auto hist_post = prepHist("hist_post",kBlue,"nSharedRH_Post",tree,"Post-Cleaning",leg);
  
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

  // save
  canv->SaveAs("nSharedHits.png");
  canv->SaveAs("nSharedHits.pdf");

  // delete it all
  delete canv;
  delete hist_post;
  delete hist_pre;
  delete leg;
  delete tree;
  delete file;
}
