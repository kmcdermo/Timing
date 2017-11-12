#include "TreePlotter.hh"

TreePlotter::TreePlotter(const TString & var, const TString & commoncut, const TString & text, const Int_t nbinsx, const Float_t xlow, const Float_t xhigh,
			 const Bool_t islogx, const Bool_t islogy, const TString & title, const TString & xtitle, const TString & ytitle) : 
  fVar(var), fCommonCut(commoncut), fText(text), fNbinsX(nbinsx), fXLow(xlow), fXHigh(xhigh),
  fIsLogX(islogx), fIsLogY(islogy), fTitle(title), fXTitle(xtitle), fYTitle(ytitle)
{
  std::cout << "Initializing..." << std::endl;

  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  SetTDRStyle(fTDRStyle);
  gROOT->ForceStyle();

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  SetupSamples();
  SetupColors();
  SetupCuts();
  SetupLabels();
  SetupHists();

  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s.root",text.Data()),"UPDATE");
}

TreePlotter::~TreePlotter() {}

void TreePlotter::MakePlot()
{
  ////////////////////////////
  //                        //
  // Fill Hists from TTrees //
  //                        //
  ////////////////////////////

  for (const auto & SamplePair : SampleMap)
  {
    // Init
    const auto & input  = SamplePair.first;
    const auto & sample = SamplePair.second;
    const Bool_t isMC = (sample != Data);
    std::cout << "Working on " << (isMC?"MC":"DATA") << " sample: " << input.Data() << std::endl;

    // Get File
    const TString filename = Form("/afs/cern.ch/work/k/kmcdermo/public/input/2017/%s/tree.root",input.Data());
    TFile * file = TFile::Open(Form("%s",filename.Data()));
    CheckValidFile(file,filename);
    file->cd();
	
    // Get TTree
    const TString treename = "tree/tree";
    TTree * tree = (TTree*)file->Get(Form("%s",treename.Data()));
    CheckValidTree(tree,treename,filename);

    // Make temp hist
    TString histname = input;
    histname.ReplaceAll("/","_");
    TH1F * hist = TreePlotter::SetupHist(Form("%s_Hist",histname.Data()));
    
    // Set weight
    const Float_t weight = (isMC ? GetSampleWeight(file) : 1.f);

    // Fill from tree
    tree->Draw(Form("%s>>%s",fVar.Data(),hist->GetName()),Form("(%s) * (%f%s)",CutMap[sample].Data(),weight,isMC?Form("* genwgt"):""),"goff");
    
    // Add to main hists
    HistMap[sample]->Add(hist);

    // save to output file
    fOutFile->cd();
    hist->Write(hist->GetName(),TObject::kWriteDelete);

    // delete everything
    delete hist;
    delete tree;
    delete file;
  }

  //////////////////////
  //                  //
  // Make Bkgd Output //
  //                  //
  //////////////////////

  std::cout << "Making Bkgd Output..." << std::endl;

  // Make Total Bkgd Hist: for error plotting
  TH1F * BkgdHist = TreePlotter::SetupHist("BkgdHist");
  BkgdHist->Add(HistMap[QCD]);
  BkgdHist->Add(HistMap[GJets]);
  BkgdHist->SetMarkerSize(0);
  BkgdHist->SetFillStyle(3254);
  BkgdHist->SetFillColor(kGray+3);

  // Make Background Stack
  THStack * BkgdStack = new THStack("BkgdStack","");
  BkgdStack->Add(HistMap[QCD]);
  BkgdStack->Add(HistMap[GJets]);

  // save to output file
  fOutFile->cd();
  BkgdHist->Write(BkgdHist->GetName(),TObject::kWriteDelete);
  BkgdStack->Write(BkgdStack->GetName(),TObject::kWriteDelete);

  ///////////////////////
  //                   //
  // Make Ratio Output //
  //                   //
  ///////////////////////

  std::cout << "Making Ratio Output..." << std::endl;

  // ratio value plot
  TH1F * RatioHist = TreePlotter::SetupHist("RatioHist");
  RatioHist->Add(HistMap[Data]);
  RatioHist->Divide(BkgdHist);  
  RatioHist->GetYaxis()->SetTitle("Data/MC");
  RatioHist->SetMinimum(-0.1); // Define Y ..
  RatioHist->SetMaximum( 2.1); // .. range
  RatioHist->SetLineColor(kBlack);
  RatioHist->SetMarkerColor(kBlack);
  RatioHist->SetStats(0);      // No statistics on lower plot
  
  // ratio MC error plot
  TH1F * RatioMCErrs = TreePlotter::SetupHist("RatioMCErrs");
  RatioMCErrs->Add(BkgdHist);
  RatioMCErrs->Divide(BkgdHist);
  RatioMCErrs->SetMarkerSize(0);
  RatioMCErrs->SetFillStyle(3254);
  RatioMCErrs->SetFillColor(kGray+3);

  // don't display empty bins
  for (Int_t ibin = 1; ibin <= RatioMCErrs->GetNbinsX(); ibin++) 
  {
    if (RatioMCErrs->GetBinContent(ibin) == 0) {RatioMCErrs->SetBinContent(ibin,-2);} 
  }

  // save to output file
  fOutFile->cd();
  RatioHist->Write(RatioHist->GetName(),TObject::kWriteDelete);
  RatioMCErrs->Write(RatioMCErrs->GetName(),TObject::kWriteDelete);
  
  // ratio line
  TLine * RatioLine = new TLine();
  RatioLine->SetLineColor(kRed);
  RatioLine->SetLineWidth(2);

  // save to output file
  fOutFile->cd();
  RatioLine->Write(RatioLine->GetName(),TObject::kWriteDelete);

  ///////////////////
  //               //
  // Create Legend //
  //               //
  ///////////////////

  std::cout << "Creating Legend..." << std::endl;

  TLegend * Legend = new TLegend(0.682,0.7,0.825,0.92);
  //  Legend->SetNColumns(2);
  Legend->SetBorderSize(1);
  Legend->SetLineColor(kBlack);

  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    TString fillType;
    if      (sample == Data) fillType = "epl";
    else if (sample == GMSB) fillType = "l";
    else                     fillType = "f";

    Legend->AddEntry(HistPair.second,LabelMap[sample].Data(),fillType.Data());
  }
  Legend->AddEntry(BkgdHist,"MC Unc.","f");

  // save to output file
  fOutFile->cd();
  Legend->Write(Legend->GetName(),TObject::kWriteDelete);

  ///////////////////////////
  //                       //
  // Init Output Canv+Pads //
  //                       //
  ///////////////////////////

  std::cout << "Initializing canvas and pads..." << std::endl;

  TCanvas * OutCanv = new TCanvas("OutCanv","");
  OutCanv->cd();
  
  TPad * UpperPad = new TPad("UpperPad","", Config::left_up, Config::bottom_up, Config::right_up, Config::top_up);
  UpperPad->SetBottomMargin(0); // Upper and lower plot are joined
  
  TPad * LowerPad = new TPad("LowerPad", "", Config::left_lp, Config::bottom_lp, Config::right_lp, Config::top_lp);
  LowerPad->SetTopMargin(0);
  LowerPad->SetBottomMargin(0.3);

  ////////////////////
  //                //
  // Draw Upper Pad //
  //                //
  ////////////////////

  std::cout << "Drawing upper pad..." << std::endl;

  // Pad Gymnastics
  OutCanv->cd();
  UpperPad->Draw();
  UpperPad->cd();
  UpperPad->SetLogx(fIsLogX);
  UpperPad->SetLogy(fIsLogY);
  
  // Get and Set Maximum
  const Float_t min = GetHistMinimum();
  const Float_t max = GetHistMaximum(BkgdHist);

  if (fIsLogY) 
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
  HistMap[Data]->GetYaxis()->SetLabelSize  (Config::LabelSize / Config::height_up); 
  HistMap[Data]->GetYaxis()->SetTitleSize  (Config::TitleSize / Config::height_up);
  HistMap[Data]->GetYaxis()->SetTitleOffset(Config::TitleFF * Config::TitleYOffset * Config::height_up);
  
  // Draw stack
  BkgdStack->Draw("HIST SAME"); 
  UpperPad->RedrawAxis("SAME"); // stack kills axis

  // Draw Signal
  HistMap[GMSB]->Draw("HIST SAME");

  // Draw MC sum total error as well on top of stack --> E2 makes error appear as rectangle
  BkgdHist->Draw("E2 SAME");

  // Redraw data to make it appear again!
  HistMap[Data]->Draw("PE SAME"); 
 
  // And lastly draw the legend
  Legend->Draw("SAME"); 

  ////////////////////
  //                //
  // Draw Lower Pad //
  //                //
  ////////////////////
  
  std::cout << "Drawing lower pad..." << std::endl;

  // Pad gymnastics
  OutCanv->cd(); 
  LowerPad->Draw();
  LowerPad->cd(); 

  // draw th1 first so line can appear, then draw over it (and set Y axis divisions)
  RatioHist->Draw("EP"); 
  RatioLine->SetX1(RatioHist->GetXaxis()->GetXmin());
  RatioLine->SetX2(RatioHist->GetXaxis()->GetXmax());
  RatioLine->SetY1(1.0);
  RatioLine->SetY2(1.0);
  RatioLine->Draw("SAME");

  // some style since apparently TDR Style is crapping out
  RatioHist->GetYaxis()->SetNdivisions(505);

  // sizes of titles is percent of height of pad --> want a constant size 
  RatioHist->GetXaxis()->SetLabelSize  (Config::LabelSize   / Config::height_lp); 
  RatioHist->GetXaxis()->SetLabelOffset(Config::LabelOffset / Config::height_lp); 
  RatioHist->GetXaxis()->SetTitleSize  (Config::TitleSize   / Config::height_lp);
  RatioHist->GetXaxis()->SetTickLength (Config::TickLength  / Config::height_lp);
  RatioHist->GetYaxis()->SetLabelSize  (Config::LabelSize   / Config::height_lp); 
  RatioHist->GetYaxis()->SetTitleSize  (Config::TitleSize   / Config::height_lp);
  RatioHist->GetYaxis()->SetTitleOffset(Config::TitleFF * Config::TitleYOffset * Config::height_lp);

  // redraw to go over line
  RatioHist->Draw("EP SAME"); 
  
  // plots MC error copy
  RatioMCErrs->Draw("E2 SAME");

  /////////////////
  //             //
  // Save Output //
  //             //
  /////////////////
  
  std::cout << "Saving hist as png..." << std::endl;

  OutCanv->cd(); // Go back to the main canvas before saving
  CMSLumi(OutCanv,Config::lumi); // write out Lumi info
  OutCanv->SaveAs(Form("%s.png",fText.Data()));

  // save to output file
  fOutFile->cd();
  OutCanv->Write(OutCanv->GetName(),TObject::kWriteDelete);
}

Float_t TreePlotter::GetHistMinimum()
{
  Float_t min = 1e9;

  // need to loop through to check bin != 0
  for (const auto & HistPair : HistMap)
  {
    const auto & hist = HistPair.second;
    for (Int_t bin = 1; bin <= hist->GetNbinsX(); bin++)
    {
      const Float_t tmpmin = hist->GetBinContent(bin);
      if ((tmpmin < min) && (tmpmin > 0)) min = tmpmin;
    }
  }

  return min;
}

Float_t TreePlotter::GetHistMaximum(const TH1F * BkgdHist)
{
  const Float_t datamax = HistMap[Data]->GetBinContent(HistMap[Data]->GetMaximumBin());
  const Float_t bkgdmax = BkgdHist     ->GetBinContent(BkgdHist     ->GetMaximumBin());
  return (datamax > bkgdmax ? datamax : bkgdmax);
}

TH1F * TreePlotter::SetupHist(const TString & name)
{
  TH1F * hist = new TH1F(name.Data(),fTitle.Data(),fNbinsX,fXLow,fXHigh);
  hist->GetXaxis()->SetTitle(fXTitle.Data());
  hist->GetYaxis()->SetTitle(fYTitle.Data());
  hist->Sumw2();

  return hist;
}

Float_t TreePlotter::GetSampleWeight(TFile * file)
{
  // Get Configtree
  const TString treename = "tree/configtree";
  TTree * configtree = (TTree*)file->Get(Form("%s",treename.Data()));
  CheckValidTree(configtree,treename,file->GetName());

  // Get Historgram
  const TString histname = "tree/h_cutflow";
  TH1F * h_cutflow = (TH1F*) file->Get(Form("%s",histname.Data()));
  CheckValidTH1F(h_cutflow,histname,file->GetName());

  Float_t xsec = 0.f;      configtree->SetBranchAddress("xsec",&xsec);
  Float_t filterEff = 0.f; configtree->SetBranchAddress("filterEff",&filterEff);
  Float_t BR = 0.f;        configtree->SetBranchAddress("BR",&BR);
  configtree->GetEntry(0);

  const Float_t weight = Config::lumi * xsec * filterEff * BR / h_cutflow->GetBinContent(1);

  delete h_cutflow;
  delete configtree;

  return weight;
}

void TreePlotter::SetupSamples()
{
  // QCD
  SampleMap["MC/qcd/Pt-15to20"] = QCD;
  SampleMap["MC/qcd/Pt-20to30"] = QCD;
  SampleMap["MC/qcd/Pt-30to50"] = QCD;
  SampleMap["MC/qcd/Pt-50to80"] = QCD;
  SampleMap["MC/qcd/Pt-80to120"] = QCD;
  //    SampleMap["MC/qcd/Pt-120to170"] = QCD;
  SampleMap["MC/qcd/Pt-170to300"] = QCD;
  
  // GJets
  SampleMap["MC/gjets-EM"] = GJets;
   
  // GMSB
  SampleMap["MC/gmsb"] = GMSB;

  // Data
  SampleMap["DATA/singleph"] = Data;
}

void TreePlotter::SetupColors()
{
  ColorMap[QCD] = kGreen;
  ColorMap[GJets] = kRed;
  ColorMap[GMSB] = kBlue;
  ColorMap[Data] = kBlack;
}

void TreePlotter::SetupCuts()
{
  CutMap[QCD]   = Form("%s",fCommonCut.Data());
  CutMap[GJets] = Form("%s",fCommonCut.Data());
  CutMap[GMSB]  = Form("%s",fCommonCut.Data());
  CutMap[Data]  = Form("%s",fCommonCut.Data());
}

void TreePlotter::SetupLabels()
{
  LabelMap[QCD]   = "QCD";
  LabelMap[GJets] = "#gamma+Jets"; //"#gamma + Jets (EM Enriched)";
  LabelMap[GMSB]  = "GMSB c#tau=4m"; //"GMSB c#tau = 4m, #Lambda = 200 TeV";
  LabelMap[Data]  = "Data";
}

void TreePlotter::SetupHists()
{
  HistMap[QCD]   = SetupHist("QCD_hist");
  HistMap[GJets] = SetupHist("GJets_hist");
  HistMap[GMSB]  = SetupHist("GMSB_hist");
  HistMap[Data]  = SetupHist("Data_hist");
  
  for (auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const auto & hist   = HistPair.second;
    const Bool_t isSignal = (sample == GMSB);
    const Bool_t isBkgd   = (sample == GJets || sample == QCD);

    hist->SetLineColor(ColorMap[sample]);
    hist->SetMarkerColor(ColorMap[sample]);
    if (isBkgd)
    {
      hist->SetFillColor(ColorMap[sample]);
      hist->SetFillStyle(1001);
    }
    else if (isSignal)
    {
      hist->SetLineWidth(2);
    }
  }
}
