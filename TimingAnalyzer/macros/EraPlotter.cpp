#include "EraPlotter.hh"

EraPlotter::EraPlotter(const TString & eraplotconfig, const TString & plotconfig, 
		       const Bool_t savemetadata, const TString & outfiletext)
  : fEraPlotConfig(eraplotconfig), fPlotConfig(plotconfig), fSaveMetaData(savemetadata), fOutFileText(outfiletext)
{
  //////////////////
  // Initializing //
  //////////////////

  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(fTDRStyle);

  // setup config
  EraPlotter::SetupDefaults();
  EraPlotter::SetupCommon();
  EraPlotter::SetupEraPlotConfig();
  EraPlotter::SetupPlotConfig();

  // get input files
  EraPlotter::SetupInFiles();

  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");
}

EraPlotter::~EraPlotter()
{
  if (fSaveMetaData) delete fConfigPave;
  
  delete LowerPad;
  delete UpperPad;
  delete OutCanv;
  delete Legend;
  delete RatioLine;
  for (auto & ratiohist : RatioHistVec) delete ratiohist;
  for (auto & datahist : DataHistVec) delete datahist;
  delete BkgdHistErrRatio;
  delete BkgdHistErr;
  delete BkgdHist;

  delete fOutFile;
  for (auto & infile : fInFiles) delete infile;
  delete fTDRStyle;
}

void EraPlotter::MakeEraPlot()
{
  // get bkgd hist ready
  EraPlotter::SetupBkgdHists();

  // get data hists ready
  EraPlotter::SetupDataHists();

  // make ratios
  EraPlotter::SetupRatioOutput();

  // make legend
  EraPlotter::SetupLegend();

  // make canvas
  EraPlotter::InitOutputCanvPads();

  // make upper pad
  EraPlotter::DrawUpperPad();

  // make lower pad
  EraPlotter::DrawLowerPad();

  // save the output as images
  EraPlotter::SaveOutput();

  // save metadata
  if (fSaveMetaData) EraPlotter::MakeConfigPave();
}

void EraPlotter::SetupInFiles()
{
  for (const auto & era : fEras)
  {
    fInFiles.emplace_back(TFile::Open(Form("%s_%s.root",fInFileText.Data(),era.Data())));
  }
}

void EraPlotter::SetupBkgdHists()
{
  std::cout << "Setting up bkgd hists..." << std::endl;

  // only need one bkgd hist since they are the same once they are scaled to unity
  const auto & infile = fInFiles.front();
  BkgdHist = (TH1F*)infile->Get(Form("%s",Common::BkgdHistName.Data()));

  BkgdHist->Scale(1.f/BkgdHist->Integral(fXVarBins?"width":"")); 
  BkgdHist->SetFillColor(kGray);
  BkgdHist->SetFillColor(kGray);
  BkgdHist->SetMarkerSize(0);

  // output
  Common::Write(fOutFile,BkgdHist);

  // make a copy for plotting the errors
  BkgdHistErr = (TH1F*)BkgdHist->Clone(Form("%sErr",BkgdHist->GetName()));
  BkgdHistErr->SetFillStyle(3254);
  BkgdHistErr->SetFillColor(kBlack);

  // output
  Common::Write(fOutFile,BkgdHistErr);
}

void EraPlotter::SetupDataHists()
{
  std::cout << "Setting up data era hists..." << std::endl;

  for (auto iera = 0U; iera < fEras.size(); iera++)
  {
    auto & infile = fInFiles[iera];
    infile->cd();
    DataHistVec.emplace_back((TH1F*)infile->Get(Form("%s",Common::HistNameMap["Data"].Data())));

    auto & datahist = DataHistVec.back();
    datahist->SetName(Form("%s_Hist",fEras[iera].Data()));
    datahist->Scale(1.f/datahist->Integral(fXVarBins?"width":""));

    auto color = Common::ColorVec[iera];
    datahist->SetLineColor(color);
    datahist->SetMarkerColor(color);

    // output
    Common::Write(fOutFile,datahist);
  }
}

void EraPlotter::SetupRatioOutput()
{
  std::cout << "Setting up ratio outputs..." << std::endl;

  // get data ratios
  for (auto & datahist : DataHistVec)
  {
    RatioHistVec.emplace_back((TH1F*)datahist->Clone(Form("%sRatio",datahist->GetName())));

    auto & ratiohist = RatioHistVec.back();
    ratiohist->Divide(BkgdHist);

    // output
    Common::Write(fOutFile,ratiohist);
  }

  // mc errors
  BkgdHistErrRatio = (TH1F*)BkgdHistErr->Clone(Form("%sRatio",BkgdHistErr->GetName()));
  BkgdHistErrRatio->Divide(BkgdHistErrRatio);

  // output
  Common::Write(fOutFile,BkgdHistErrRatio);

  // ratio line
  RatioLine = new TLine();
  RatioLine->SetLineColor(kRed);
  RatioLine->SetLineWidth(2);

  // output
  Common::Write(fOutFile,RatioLine,"RatioLine");
}

void EraPlotter::SetupLegend()
{
  std::cout << "Setting up the legend..." << std::endl;

  Legend = new TLegend(0.55,0.65,0.825,0.92);
  Legend->SetName("Legend");
  Legend->SetNColumns(2);
  Legend->SetBorderSize(1);
  Legend->SetLineColor(kBlack);

  for (auto iera = 0U; iera < fEras.size(); iera++)
  {
    Legend->AddEntry(DataHistVec[iera],Form("%s",fEras[iera].Data()),"elp");
  }

  Legend->AddEntry(BkgdHist,Common::BkgdSampleName.Data(),"f");
  Legend->AddEntry(BkgdHistErr,Form("%s Unc.",Common::BkgdSampleName.Data()),"f");

  // output
  Common::Write(fOutFile,Legend);
}

void EraPlotter::InitOutputCanvPads()
{
  std::cout << "Initializing canvas and pads..." << std::endl;

  OutCanv = new TCanvas("OutCanv","");
  OutCanv->cd();

  UpperPad = new TPad("UpperPad","", Common::left_up, Common::bottom_up, Common::right_up, Common::top_up);
  UpperPad->SetBottomMargin(Common::merged_margin);

  LowerPad = new TPad("LowerPad", "", Common::left_lp, Common::bottom_lp, Common::right_lp, Common::top_lp);
  LowerPad->SetTopMargin(Common::merged_margin);
  LowerPad->SetBottomMargin(Common::bottom_margin);
}

void EraPlotter::DrawUpperPad()
{
  std::cout << "Drawing upper pad..." << std::endl;

  // Pad Gymnastics
  OutCanv->cd();
  UpperPad->Draw();
  UpperPad->cd();

  // get relevant hist for setting options
  auto & hist = BkgdHist;
  
  // Get and Set Global Minimum and Maximum
  EraPlotter::GetHistMinimum();
  EraPlotter::GetHistMaximum();

  hist->SetMinimum(fMinY);
  hist->SetMaximum(fMaxY);
  
  // Have to scale TDR style values by height of upper pad
  hist->GetYaxis()->SetLabelSize  (Common::LabelSize / Common::height_up); 
  hist->GetYaxis()->SetTitleSize  (Common::TitleSize / Common::height_up);
  hist->GetYaxis()->SetTitleOffset(Common::TitleFF * Common::TitleYOffset * Common::height_up);

  // draw hists
  BkgdHist->Draw("HIST");
  BkgdHistErr->Draw("E2 SAME");
  
  for (auto & datahist : DataHistVec)
  {
    datahist->Draw("PE SAME");
  }

  // And lastly draw the legend
  Legend->Draw("SAME"); 
}

void EraPlotter::DrawLowerPad()
{  
  std::cout << "Drawing lower pad..." << std::endl;

  // Pad gymnastics
  OutCanv->cd(); 
  LowerPad->Draw();
  LowerPad->cd(); 

  // get first hist to be drawn
  auto & hist = BkgdHistErrRatio;

  // set title and range
  hist->GetYaxis()->SetTitle("Data/MC");
  hist->SetMinimum(-0.1); // Define Y ..
  hist->SetMaximum( 2.1); // .. range
  hist->SetStats(0);      // No statistics on lower plot

  // some style since apparently TDR Style is crapping out
  hist->GetYaxis()->SetNdivisions(505);

  // sizes of titles is percent of height of pad --> want a constant size 
  hist->GetXaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
  hist->GetXaxis()->SetLabelOffset(Common::LabelOffset / Common::height_lp); 
  hist->GetXaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
  hist->GetXaxis()->SetTickLength (Common::TickLength  / Common::height_lp);
  hist->GetYaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
  hist->GetYaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
  hist->GetYaxis()->SetTitleOffset(Common::TitleFF * Common::TitleYOffset * Common::height_lp);

  // draw th1 first so line can appear, then draw over it (and set Y axis divisions)
  BkgdHistErrRatio->Draw("E2"); 

  // set params for line then draw
  RatioLine->SetX1(BkgdHistErrRatio->GetXaxis()->GetXmin());
  RatioLine->SetX2(BkgdHistErrRatio->GetXaxis()->GetXmax());
  RatioLine->SetY1(1.0);
  RatioLine->SetY2(1.0);
  RatioLine->Draw("SAME");

  // draw again
  BkgdHistErrRatio->Draw("E2 SAME");

  // draw data ratios
  for (auto & ratiohist : RatioHistVec)
  {
    ratiohist->Draw("EP SAME");
  }
}

void EraPlotter::SaveOutput()
{
  std::cout << "Saving hist as images..." << std::endl;

  // Go back to the main canvas before saving and write out lumi info
  OutCanv->cd();
  Common::CMSLumi(OutCanv,0);

  // Save a log version first
  EraPlotter::PrintCanvas(true);

  // Save a linear version second
  EraPlotter::PrintCanvas(false);

  // save to output file
  Common::Write(fOutFile,OutCanv);
}  

void EraPlotter::PrintCanvas(const Bool_t isLogy)
{
  // pad gymnastics
  OutCanv->cd();
  UpperPad->cd();
  UpperPad->SetLogy(isLogy);

  // get relevant hist
  auto & hist = BkgdHist;

  // set min and max
  if (isLogy)
  {
    hist->SetMinimum(fMinY/1.5);
    hist->SetMaximum(fMaxY*1.5);
  }
  else 
  {
    hist->SetMinimum( fMinY > 0 ? fMinY/1.05 : fMinY*1.05 );
    hist->SetMaximum( fMaxY > 0 ? fMaxY*1.05 : fMaxY/1.05 );      
  }

  // save canvas as images
  Common::SaveAs(OutCanv,Form("%s_%s",fOutFileText.Data(),(isLogy?"log":"lin")));
}

void EraPlotter::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  fOutFile->cd();
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));

  // give grand title
  fConfigPave->AddText("***** EraPlotter Config *****");

  // dump era plot config first
  Common::AddTextFromInputConfig(fConfigPave,"EraPlotter Config",fEraPlotConfig);

  // dump plot config
  Common::AddTextFromInputConfig(fConfigPave,"Plot Config",fPlotConfig);

  for (auto & infile : fInFiles)
  {
    // padding
    Common::AddPaddingToPave(fConfigPave,3);

    // save name of infile, redundant
    fConfigPave->AddText(Form("In file name: %s",infile->GetName()));

    // dump in old config
    Common::AddTextFromInputPave(fConfigPave,infile);
  }

  // save to output file
  Common::Write(fOutFile,fConfigPave);
}

void EraPlotter::GetHistMinimum()
{
  // initialize to dummy value
  fMinY = 1e9;

  // need to loop through to check bin != 0
  for (const auto & datahist : DataHistVec)
  {
    // get and set tmp min
    for (auto bin = 1; bin <= datahist->GetNbinsX(); bin++)
    {
      const auto tmpmin = datahist->GetBinContent(bin);
      if ((tmpmin < fMinY) && (tmpmin > 0)) fMinY = tmpmin;
    }
  }

  for (auto bin = 1; bin <= BkgdHist->GetNbinsX(); bin++)
  {
    const auto tmpmin = BkgdHist->GetBinContent(bin);
    if ((tmpmin < fMinY) && (tmpmin > 0)) fMinY = tmpmin;
  }
}

void EraPlotter::GetHistMaximum()
{
  // initialize to dummy value
  fMaxY = -1e9;

  // check data first
  for (const auto & datahist : DataHistVec)
  {
    const auto max = datahist->GetBinContent(datahist->GetMaximumBin());
    if (max > fMaxY) fMaxY = max;
  }

  // check combined background
  const auto bkgdmax = BkgdHist->GetBinContent(BkgdHist->GetMaximumBin());
  if (bkgdmax > fMaxY) fMaxY = bkgdmax;
}

void EraPlotter::SetupCommon()
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupGroups();
  Common::SetupHistNames();
}

void EraPlotter::SetupEraPlotConfig()
{
  std::cout << "Reading era plot config..." << std::endl;

  std::ifstream infile(Form("%s",fEraPlotConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("in_file_text=") != std::string::npos)
    {
      fInFileText = Common::RemoveDelim(str,"in_file_text=");
    }
    else if (str.find("eras=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"eras=");
      std::stringstream ss(str);
      std::string era;
      while (ss >> era) fEras.push_back(era);
    }
    else 
    {
      std::cerr << "Aye... your era plot config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void EraPlotter::SetupPlotConfig()
{
  std::cout << "Reading plot config..." << std::endl;

  std::ifstream infile(Form("%s",fPlotConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str.find("x_bins=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_bins=");
      Common::SetupBins(str,fXBins,fXVarBins);
    }
  }
}

void EraPlotter::SetupDefaults()
{
  fXBins.clear();
  fXVarBins = false;
}
