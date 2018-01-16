#include "TreePlotter.hh"

TreePlotter::TreePlotter(const TString & var, const TString & commoncut, const TString & text, const Int_t nbinsx, const Float_t xlow, const Float_t xhigh,
			 const Bool_t islogx, const Bool_t islogy, const TString & title, const TString & xtitle, const TString & ytitle, const TString & delim) : 
  fVar(var), fCommonCut(commoncut), fText(text), fNbinsX(nbinsx), fXLow(xlow), fXHigh(xhigh),
  fIsLogX(islogx), fIsLogY(islogy), fDelim(delim)
{
  std::cout << "Initializing..." << std::endl;

  // setup up titles
  fTitle  = Config::ReplaceDelimWithSpace(title,fDelim);
  fXTitle = Config::ReplaceDelimWithSpace(xtitle,fDelim);
  fYTitle = Config::ReplaceDelimWithSpace(ytitle,fDelim);

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Config::SetTDRStyle(fTDRStyle);
  gROOT->ForceStyle();

  // setup hists
  TreePlotter::InitConfig();
  TreePlotter::SetupHists();

  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s.root",fText.Data()),"UPDATE");
}

TreePlotter::~TreePlotter() 
{
  // delete everything
  delete LowerPad;
  delete UpperPad;
  delete OutCanv;
  delete Legend;
  delete RatioLine;
  delete RatioMCErrs;
  delete RatioHist;
  delete BkgdStack;
  delete BkgdHist;

  delete fOutFile;
  for (auto & HistPair : HistMap) delete HistPair.second;
  delete fTDRStyle;
}

void TreePlotter::MakePlot()
{
  // Fill Hists from TTrees
  TreePlotter::MakeHistFromTrees();

  // Make Bkgd Output
  TreePlotter::MakeBkgdOutput();

  // Make Ratio Output
  TreePlotter::MakeRatioOutput();

  // Make Legend
  TreePlotter::MakeLegend();

  // Init Output Canv+Pads
  TreePlotter::InitOutputCanvPads();

  // Draw Upper Pad
  TreePlotter::DrawUpperPad();

  // Draw Lower Pad
  TreePlotter::DrawLowerPad();

  // Save Output
  TreePlotter::SaveOutput();
}

void TreePlotter::MakeHistFromTrees()
{
  for (const auto & SamplePair : Config::SampleMap)
  {
    // Init
    const auto & input  = SamplePair.first;
    const auto & sample = SamplePair.second;
    const Bool_t isMC = (sample != Data);
    std::cout << "Working on " << (isMC?"MC":"DATA") << " sample: " << input.Data() << std::endl;

    // Get File
    const TString filename = Form("%s/%s/%s/%s",Config::eosDir.Data(),Config::baseDir.Data(),input.Data(),Config::tupleFileName.Data());
    TFile * file = TFile::Open(Form("%s",filename.Data()));
    Config::CheckValidFile(file,filename);
    file->cd();
	
    // Get TTree
    TTree * tree = (TTree*)file->Get(Form("%s",Config::disphotreename.Data()));
    Config::CheckValidTree(tree,Config::disphotreename,filename);

    // Make temp hist
    TString histname = Config::ReplaceSlashWithUnderscore(input);
    TH1F * hist = TreePlotter::SetupHist(Form("%s_Hist",histname.Data()));
    
    // Fill from tree
    tree->Draw(Form("%s>>%s",fVar.Data(),hist->GetName()),Form("(%s) * (%s)",Config::CutMap[sample].Data(),Config::WeightString(isMC).Data()),"goff");
    
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

  // save totals to output file
  fOutFile->cd();
  for (const auto & HistPair : HistMap)
  { 
    const auto & hist = HistPair.second;
    hist->Write(hist->GetName(),TObject::kWriteDelete);
  }
}

void TreePlotter::MakeBkgdOutput()
{
  std::cout << "Making Bkgd Output..." << std::endl;

  // Make Total Bkgd Hist: for ratio and error plotting
  BkgdHist = TreePlotter::SetupHist("Bkgd_Hist");
  BkgdHist->Add(HistMap[GJets]);
  BkgdHist->Add(HistMap[QCD]);
  BkgdHist->SetMarkerSize(0);
  BkgdHist->SetFillStyle(3254);
  BkgdHist->SetFillColor(kGray+3);

  // ****** TEMP HACK : SCALE TO AREA OF DATA ******* //
  // const Float_t data_int = HistMap[Data]->Integral();
  // const Float_t bkgd_int = BkgdHist     ->Integral();
  // BkgdHist      ->Scale(data_int/bkgd_int);
  // HistMap[GJets]->Scale(data_int/bkgd_int);
  // HistMap[QCD]  ->Scale(data_int/bkgd_int);

  // Make Background Stack
  BkgdStack = new THStack("Bkgd_Stack","");
  if (fText.Contains("gjets_ctrl",TString::kExact) || fText.Contains("signal",TString::kExact))
  {
    BkgdStack->Add(HistMap[QCD]);
    BkgdStack->Add(HistMap[GJets]);
  }
  else
  {
    BkgdStack->Add(HistMap[GJets]);
    BkgdStack->Add(HistMap[QCD]);
  }

  // save to output file
  fOutFile->cd();
  BkgdHist->Write(BkgdHist->GetName(),TObject::kWriteDelete);
  BkgdStack->Write(BkgdStack->GetName(),TObject::kWriteDelete);
}

void TreePlotter::MakeRatioOutput()
{
  std::cout << "Making Ratio Output..." << std::endl;

  // ratio value plot
  RatioHist = TreePlotter::SetupHist("Ratio_Hist");
  RatioHist->Add(HistMap[Data]);
  RatioHist->Divide(BkgdHist);  
  RatioHist->GetYaxis()->SetTitle("Data/MC");
  RatioHist->SetMinimum(-0.1); // Define Y ..
  RatioHist->SetMaximum( 2.1); // .. range
  RatioHist->SetLineColor(kBlack);
  RatioHist->SetMarkerColor(kBlack);
  RatioHist->SetStats(0);      // No statistics on lower plot
  
  // ratio MC error plot
  RatioMCErrs = TreePlotter::SetupHist("Ratio_MCErrs");
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
  RatioLine = new TLine();
  RatioLine->SetLineColor(kRed);
  RatioLine->SetLineWidth(2);

  // save to output file
  fOutFile->cd();
  RatioLine->Write(RatioLine->GetName(),TObject::kWriteDelete);
}

void TreePlotter::MakeLegend()
{
  std::cout << "Creating Legend..." << std::endl;

  Legend = new TLegend(0.682,0.7,0.825,0.92);
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

    Legend->AddEntry(HistPair.second,Config::LabelMap[sample].Data(),fillType.Data());
  }
  Legend->AddEntry(BkgdHist,"MC Unc.","f");

  // save to output file
  fOutFile->cd();
  Legend->Write(Legend->GetName(),TObject::kWriteDelete);
}

void TreePlotter::InitOutputCanvPads()
{
  std::cout << "Initializing canvas and pads..." << std::endl;

  OutCanv = new TCanvas("OutCanv","");
  OutCanv->cd();
  
  UpperPad = new TPad("UpperPad","", Config::left_up, Config::bottom_up, Config::right_up, Config::top_up);
  UpperPad->SetBottomMargin(0); // Upper and lower plot are joined
  
  LowerPad = new TPad("LowerPad", "", Config::left_lp, Config::bottom_lp, Config::right_lp, Config::top_lp);
  LowerPad->SetTopMargin(0);
  LowerPad->SetBottomMargin(0.35); 
}

void TreePlotter::DrawUpperPad()
{
  std::cout << "Drawing upper pad..." << std::endl;

  // Pad Gymnastics
  OutCanv->cd();
  UpperPad->Draw();
  UpperPad->cd();
  UpperPad->SetLogx(fIsLogX);
  UpperPad->SetLogy(fIsLogY);
  
  // Get and Set Maximum
  const Float_t min = TreePlotter::GetHistMinimum();
  const Float_t max = TreePlotter::GetHistMaximum();

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
}

void TreePlotter::DrawLowerPad()
{  
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
}

void TreePlotter::SaveOutput()
{
  std::cout << "Saving hist as png..." << std::endl;

  OutCanv->cd(); // Go back to the main canvas before saving
  Config::CMSLumi(OutCanv,0); // write out Lumi info
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

Float_t TreePlotter::GetHistMaximum()
{
  const Float_t datamax = HistMap[Data]->GetBinContent(HistMap[Data]->GetMaximumBin());
  const Float_t bkgdmax = BkgdHist     ->GetBinContent(BkgdHist     ->GetMaximumBin());
  return (datamax > bkgdmax ? datamax : bkgdmax);
}

void TreePlotter::InitConfig()
{
  Config::SetupSamples();
  Config::SetupColors();
  Config::SetupCuts(fCommonCut);
  Config::SetupLabels();
}

void TreePlotter::SetupHists()
{
  HistMap[QCD]   = SetupHist("QCD_Hist");
  HistMap[GJets] = SetupHist("GJets_Hist");
  HistMap[GMSB]  = SetupHist("GMSB_Hist");
  HistMap[Data]  = SetupHist("Data_Hist");
  
  for (auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const auto & hist   = HistPair.second;
    const Bool_t isSignal = (sample == GMSB);
    const Bool_t isBkgd   = (sample == GJets || sample == QCD);
    
    hist->SetLineColor(Config::ColorMap[sample]);
    hist->SetMarkerColor(Config::ColorMap[sample]);
    if (isBkgd)
    {
      hist->SetFillColor(Config::ColorMap[sample]);
      hist->SetFillStyle(1001);
    }
    else if (isSignal)
    {
      hist->SetLineWidth(2);
    }
  }
}

TH1F * TreePlotter::SetupHist(const TString & name)
{
  TH1F * hist = new TH1F(name.Data(),fTitle.Data(),fNbinsX,fXLow,fXHigh);
  hist->GetXaxis()->SetTitle(fXTitle.Data());
  hist->GetYaxis()->SetTitle(fYTitle.Data());
  hist->Sumw2();
  
  return hist;
}
