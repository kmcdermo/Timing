#include "common/common.h"
#include "TreePlotter.hh"

void TreePlotter(const TString & var, const TString & title, 
		 const Int_t nbinsx, const Float_t xlow, const Float_t xhigh,
		 const TString & xtitle, const TString & ytitle)
{
  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  TStyle * tdrStyle;
  InitPlotter(tdrStyle,title,nbinsx,xlow,xhigh,xtitle,ytitle);
  
  ////////////////////////////
  //                        //
  // Fill Hists from TTrees //
  //                        //
  ////////////////////////////

  for (const auto & SamplePair : SampleMap)
  {
    const auto & sample = SamplePair.second;
    const Bool_t isMC = (sample != Data);
    TFile * file = TFile::Open(Form("/afs/cern.ch/work/k/kmcdermo/public/input/2017/%s/tree.root",SamplePair.first.Data()));
    TTree * tree = (TTree*)file->Get("tree/tree.root");
    TH1F  * hist = SetupHist("tmp_hist",title,nbinsx,xlow,xhigh,xtitle,ytitle);

    const Float_t weight = (isMC ? GetSampleWeight(file) : 1.f);
    tree->Draw(Form("%s>>%s",var.Data(),hist->GetName()),Form("(%s) * (%f%s)",CutMap[sample].Data(),isMC?Form("* genwgt *%f",lumi):""),"goff");
    
    HistMap[sample]->Add(hist);

    delete hist;
    delete tree;
    delete file;
  }

  //////////////////////
  //                  //
  // Make Bkgd Output //
  //                  //
  //////////////////////

  // Make Total Bkgd Hist: for error plotting
  TH1F * BkgdHist = SetupHist("BkgdHist",title,nbinsx,xlow,xhigh,xtitle,ytitle);
  BkgdHist->Add(HistMap[QCD]);
  BkgdHist->Add(HistMap[GJets]);
  BkgdHist->SetMarkerSize(0);
  BkgdHist->SetFillStyle(3254);
  BkgdHist->SetFillColor(kGray+3);

  // Make Background Stack
  THStack * BkgdStack = new THStack("BkgdStack","");
  BkgdHist->Add(HistMap[QCD]);
  BkgdHist->Add(HistMap[GJets]);

  ///////////////////////
  //                   //
  // Make Ratio Output //
  //                   //
  ///////////////////////

  // ratio value plot
  TH1F * RatioHist = (TH1F*)HistMap[Data]->Clone();
  RatioHist->Divide(BkgdHist);  
  RatioHist->GetYaxis()->SetTitle("Data/MC");
  RatioHist->SetMinimum(-1.5); // Define Y ..
  RatioHist->SetMaximum( 1.5); // .. range
  RatioHist->SetLineColor(kBlack);
  RatioHist->SetStats(0);      // No statistics on lower plot
  
  // ratio MC error plot
  TH1F * RatioMCErrs = (TH1F*)BkgdHist->Clone();
  RatioMCErrs->Divide(BkgdHist);
  // don't display empty bins
  for (Int_t ibin = 1; ibin <= RatioMCErrs->GetNbinsX(); ibin++) 
  {
    if (RatioMCErrs->GetBinContent(ibin) == 0) {RatioMCErrs->SetBinContent(ibin,-1000);} 
  }
  
  // ratio line
  TLine * RatioLine = new TLine();
  RatioLine->SetX1(RatioHist->GetXaxis()->GetXmin());
  RatioLine->SetX2(RatioHist->GetXaxis()->GetXmax());
  RatioLine->SetY1(0.0);
  RatioLine->SetY2(0.0);
  RatioLine->SetLineColor(kRed);
  RatioLine->SetLineWidth(2);

  ///////////////////
  //               //
  // Create Legend //
  //               //
  ///////////////////

  TLegend * OutLeg = new TLegend(0.682,0.7,0.825,0.92);
  OutLeg->SetBorderSize(1);
  OutLeg->SetLineColor(kBlack);

  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const Bool_t isMC = (sample != Data);

    OutLeg->AddEntry(HistPair.second,LabelMap[sample],(isMC?"f":"epl"));
  }
  OutLeg->AddEntry(BkgdHist,"MC Unc.","f");
  
  ///////////////////////////
  //                       //
  // Init Output Canv+Pads //
  //                       //
  ///////////////////////////

  OutCanv = new TCanvas("OutCanv","");
  OutCanv->cd();
  
  OutUpPad = new TPad("OutUpPad","", left_up, bottom_up, right_up, top_up);
  OutUpPad->SetBottomMargin(0); // Upper and lower plot are joined
  
  OutLowPad = new TPad("OutLowPad", "", left_lp, bottom_lp, right_lp, top_lp);
  OutLowPad->SetTopMargin(0);
  OutLowPad->SetBottomMargin(0.3);

  ////////////////////
  //                //
  // Draw Upper Pad //
  //                //
  ////////////////////

  // Pad Gymnastics
  OutCanv->cd();
  OutUpPad->Draw();
  OutUpPad->cd();
  OutUpPad->SetLogx(isLogX);
  OutUpPad->SetLogy(isLogY);
  
  // Get and Set Maximum
  const Float_t min = GetMinimum();
  const Float_t max = GetMaximum();

  if (isLogY) 
  { 
    HistMap[Data]->SetMinimum(min/1.5);
    HistMap[Data]->SetMaximum(max*1.5);
  }
  else 
  {
    HistMap[Data]->SetMaximum( max > 0 ? max*1.05 : max/1.05 );      
    HistMap[Data]->SetMinimum( min > 0 ? min/1.05 : min*1.05 );
  }

  // now draw the plots for upper pad in absurd order because ROOT is dumb
  HistMap[Data]->Draw("PE"); // draw first so labels appear

  // Have to scale TDR style values by height of upper pad
  HistMap[Data]->GetYaxis()->SetLabelSize  (LabelSize / height_up); 
  HistMap[Data]->GetYaxis()->SetTitleSize  (TitleSize / height_up);
  HistMap[Data]->GetYaxis()->SetTitleOffset(TitleFF * TitleYOffset * height_up);
  
  // Draw stack
  BkgdStack->Draw("HIST SAME"); 
  OutUpPad->RedrawAxis("SAME"); // stack kills axis

  // Draw MC sum total error as well on top of stack --> E2 makes error appear as rectangle
  BkgdHist->Draw("E2 SAME");

  // Redraw data to make it appear again!
  HistMap[Data]->Draw("PE SAME"); 
 
  // And lastly draw the legend
  OutLeg->Draw("SAME"); 

  ////////////////////
  //                //
  // Draw Lower Pad //
  //                //
  ////////////////////
  
  // Pad gymnastics
  OutCanv->cd(); 
  OutLowPad->Draw();
  OutLowPad->cd(); 

  // draw th1 first so line can appear, then draw over it (and set Y axis divisions)
  RatioHist->Draw("EP"); 
  RatioLine->Draw("SAME");

  // some style since apparently TDR Style is crapping out
  RatioHist->GetYaxis()->SetNdivisions(505);

  // sizes of titles is percent of height of pad --> want a constant size 
  RatioHist->GetXaxis()->SetLabelSize  (LabelSize   / height_lp); 
  RatioHist->GetXaxis()->SetLabelOffset(LabelOffset / height_lp); 
  RatioHist->GetXaxis()->SetTitleSize  (TitleSize   / height_lp);
  RatioHist->GetXaxis()->SetTickLength (TickLength  / height_lp);
  RatioHist->GetYaxis()->SetLabelSize  (LabelSize   / height_lp); 
  RatioHist->GetYaxis()->SetTitleSize  (TitleSize   / height_lp);
  RatioHist->GetYaxis()->SetTitleOffset(TitleFF * TitleYOffset * height_lp);

  // redraw to go over line
  RatioHist->Draw("EP SAME"); 
  
  // plots MC error copy
  RatioMCErrs->Draw("E2 SAME");

  
}

TH1F * SetupHist(const TString & name, const TString & title, 
		 const Int_t nbinsx, const Float_t xlow, const Float_t xhigh,
		 const TString & xtitle, const TString & ytitle)
{
  TH1F * hist = new TH1F(name.Data(),title.Data(),nbinsx,xlow,xhigh);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  return hist;
}

Float_t GetSampleWeight(TFile * file)
{
  TTree * configtree = (TTree*)file->Get("tree/configtree");
  TH1F * h_cutflow = (TH1F*)file->Get("tree/h_cutflow");

  Float_t xsec = 0.f;      configtree->SetBranchAddress("xsec",&xsec);
  Float_t filterEff = 0.f; configtree->SetBranchAddress("filterEff",&filterEff);
  Float_t BR = 0.f;        configtree->SetBranchAddress("BR",&BR);
  configtree->GetEntry(0);

  const Float_t weight = h_cutflow->GetBinContent(1) * xsec * filterEff * BR;

  delete h_cutflow;
  delete configtree;

  return weight;
}

void InitPlotter(TStyle * tdrStyle, const TString & title, 
		 const Int_t nbinsx, const Float_t xlow, const Float_t xhigh,
		 const TString & xtitle, const TString & ytitle)
{
  tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  SetupSamples();
  SetupColors();
  SetupHists(title,nbinsx,xlow,xhigh,xtitle,ytitle);
}
