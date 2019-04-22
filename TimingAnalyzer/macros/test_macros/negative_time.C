void do_plot(const Bool_t is1Pho, const Bool_t isCut);
void do_proj(TH2F * hist, TCanvas * canv, const TString & label, const Bool_t isX);

void negative_time()
{
  // get 2D style
  gStyle->SetNumberContours(255);

  do_plot(true,true);
  do_plot(true,false);
  do_plot(false,true);
  do_plot(false,false);
}

void do_plot(const Bool_t is1Pho, const Bool_t isCut)
{
  // 2D style
  gStyle->SetOptStat("eMRou");

  // filename
  const TString filename = (is1Pho ? "exclusive_1pho.root" : "inclusive_2pho.root");

  // file
  auto file = TFile::Open(filename.Data()); 
  auto tree = (TTree*)file->Get("Data_Tree");

  // bins
  const std::vector<Double_t> xbins = {-25,-20,-18,-16,-14,-12,-10,-9,-8,-7,-6,-5,-4.5,-4,-3.5,-3,-2.5,-2};
  const std::vector<Double_t> ybins = {0,10,20,30,40,50,60,70,80,90,100,120,140,160,180,200,250,300,350,500,1000};
  const auto binsX = &xbins[0];
  const auto binsY = &ybins[0];

  // hist
  auto hist2D = new TH2F("hist2D",Form("Signal Region Negative Time [%s Category : nJets%s3];Photon Weighted Time [ns];p_{T}^{miss} [GeV]",
				   (is1Pho?"1-#gamma":"2-#gamma"),(isCut?"<":"#geq")),xbins.size()-1,&xbins[0],ybins.size()-1,&ybins[0]);
  hist2D->Sumw2();

  // draw from tree
  tree->Draw("t1pfMETpt:(phoweightedtimeLT120_0+phoweightedtimeLT120SHIFT_0)>>hist2D",Form("(Sum$(1*(jetpt>30))%s3)",(isCut?"<":">=")),"goff");

  // canvas
  auto canv = new TCanvas();
  canv->SetGridy();
  canv->SetTickx();
  canv->SetTicky();

  // draw
  hist2D->Draw("colz");
  canv->Update();
  
  // print text
  auto text = new TPaveText(0.5,0.7,0.86,0.86,"NDC");
  text->AddText(Form("Covariance: %.2f",hist2D->GetCovariance()));
  text->AddText(Form("Corr. Factor: %.3f",hist2D->GetCorrelationFactor()));
  text->SetTextAlign(11);                                                                                                                                                    
  text->SetFillColorAlpha(text->GetFillColor(),0);                                                                                                                        
  text->Draw("same");
  
  // stats
  auto stats = (TPaveStats*)(hist2D->GetListOfFunctions()->FindObject("stats"));
  stats->SetX1NDC(0.14);
  stats->SetX2NDC(0.39);
  stats->SetY1NDC(0.45); 
  stats->SetY2NDC(0.86);
  
  // save as
  const TString label = Form("neg_time_%sPho_nJ%s3",(is1Pho?"1":"2"),(isCut?"LT":"GTE"));
  canv->SaveAs(label+".png");
  canv->SaveAs(label+".pdf");

  // new canv settings
  canv->SetGridy(0);
  delete text;
  delete stats;

  // now, project out
  do_proj(hist2D,canv,label,false);
  do_proj(hist2D,canv,label,true);
 
  // delete it all
  delete canv;
  delete hist2D;
  delete tree;
  delete file;
}

void do_proj(TH2F * hist2D, TCanvas * canv, const TString & label, const Bool_t isX)
{
  // 1D style
  gStyle->SetOptStat("MR");

  // project
  TH1D * hist1D = NULL;
  if (isX) hist1D = (TH1D*)hist2D->ProjectionX("histX",1,hist2D->GetYaxis()->GetNbins());
  else     hist1D = (TH1D*)hist2D->ProjectionY("histY",1,hist2D->GetXaxis()->GetNbins());
  
  // title
  if (isX) hist1D->GetYaxis()->SetTitle("Events/ns");
  else     hist1D->GetYaxis()->SetTitle("Events/GeV");

  // style
  hist1D->SetLineColor(kBlack);
  hist1D->SetMarkerColor(kBlack);
  hist1D->SetMarkerStyle(20);
  hist1D->SetMarkerSize(0.6);

  // scale
  for (auto ibin = 1; ibin <= hist1D->GetXaxis()->GetNbins(); ibin++)
  {
    const auto width = hist1D->GetXaxis()->GetBinWidth(ibin);
    hist1D->SetBinContent(ibin,hist1D->GetBinContent(ibin)/width);
    hist1D->SetBinError  (ibin,hist1D->GetBinError  (ibin)/width);
  }

  // draw
  hist1D->Draw("ep");
  canv->Update();
  
  // stats
  auto stats = (TPaveStats*)(hist1D->GetListOfFunctions()->FindObject("stats"));
  const auto widthX = 0.25;
  stats->SetX1NDC(isX?0.14:0.61);
  stats->SetX2NDC(stats->GetX1NDC()+widthX);
  stats->SetY1NDC(0.75); 
  stats->SetY2NDC(0.86);
  
  // save
  canv->SetLogy();
  canv->SaveAs(Form("%s_%s.png",label.Data(),(isX?"X":"Y")));
  canv->SaveAs(Form("%s_%s.pdf",label.Data(),(isX?"X":"Y")));

  // delete
  delete stats;
  delete hist1D;
}
