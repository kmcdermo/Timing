#include "PlotComparator.hh"

PlotComparator::PlotComparator(const TString & compareconfig, const TString & outfiletext)
  : fCompareConfig(compareconfig), fOutFileText(outfiletext)
{
  std::cout << "Initializing PlotComparator..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(fTDRStyle);

  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");

  // setup config
  PlotComparator::SetupDefaults();
  PlotComparator::SetupCompareConfig();
}

PlotComparator::~PlotComparator()
{
  std::cout << "Tidying up in the destructor..." << std::endl;

  delete fConfigPave;
  delete fLowerPad;
  delete fUpperPad;
  delete fOutCanv;
  delete fLegend;
  delete fRatioLine;
  delete fRatioHist;
  delete fHist2;
  delete fFile2;
  delete fHist1;
  delete fFile1;
  delete fOutFile;
  delete fTDRStyle;
}

void PlotComparator::MakeComparisonPlot()
{
  // Get input files and hists. set colors and such
  PlotComparator::SetupInputs();
  
  // Make ratio outputs
  PlotComparator::MakeRatioOutputs();

  // Make legend
  PlotComparator::MakeLegend();

  // Setup canvas and pads
  PlotComparator::SetupCanvasAndPads();
  
  // Draw upper pad
  PlotComparator::DrawUpperPad();

  // Draw lower pad
  PlotComparator::DrawLowerPad();

  // Save output
  PlotComparator::SaveOutput();

  // Write out config
  PlotComparator::MakeConfigPave();
}

void PlotComparator::SetupInputs()
{
  std::cout << "Setting up inputs..." << std::endl;

  // hist1
  fFile1 = TFile::Open(Form("%s",fFileName1.Data()));
  fFile1->cd();

  fHist1 = (TH1F*)fFile1->Get(Form("%s",fHistName1.Data()));
  fHist1->SetName("Hist1");
  fHist1->SetLineColor(fColor1);
  fHist1->SetMarkerColor(fColor1);
  
  auto minY1 = PlotComparator::GetHistMinY(fHist1);
  auto maxY1 = PlotComparator::GetHistMaxY(fHist1);
  
  // hist2
  fFile2 = TFile::Open(Form("%s",fFileName2.Data()));
  fFile2->cd();

  fHist2 = (TH2F*)fFile2->Get(Form("%s",fHistName2.Data()));
  fHist2->SetName("Hist2");
  fHist2->SetLineColor(fColor2);
  fHist2->SetMarkerColor(fColor2);

  auto minY2 = PlotComparator::GetHistMinY(fHist2);
  auto maxY2 = PlotComparator::GetHistMaxY(fHist2);

  // get min / max Y
  fMinY = ( minY1 < minY2 ? minY1 : minY2 );
  fMaxY = ( maxY1 > maxY2 ? maxY1 : maxY2 );

  // save to output file
  fOutFile->cd();
  fHist1->Write(fHist1->GetName(),TObject::kWriteDelete);
  fHist2->Write(fHist2->GetName(),TObject::kWriteDelete);
}

void PlotComparator::MakeRatioOutputs()
{
  std::cout << "Making ratio outputs..." << std::endl;
  
  // clone first to make ratio hist
  fRatioHist = (TH1F*)fHist1->Clone("RatioHist");

  // setup ratio hist style
  fRatioHist->SetLineColor(fColorRatio);
  fRatioHist->SetMarkerColor(fColorRatio);
  fRatioHist->GetYaxis()->SetRangeUser(fRatioMinY,fRatioMaxY);
  fRatioHist->GetYaxis()->SetTitle(fRatioYTitle.Data());
  fRatioHist->GetYaxis()->SetNdivisions(505);

  // scale label and title sizes
  fRatioHist->GetXaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
  fRatioHist->GetXaxis()->SetLabelOffset(Common::LabelOffset / Common::height_lp); 
  fRatioHist->GetXaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
  fRatioHist->GetXaxis()->SetTickLength (Common::TickLength  / Common::height_lp);
  fRatioHist->GetYaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
  fRatioHist->GetYaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
  fRatioHist->GetYaxis()->SetTitleOffset(Common::TitleFF * Common::TitleYOffset * Common::height_lp);

  // make ratio line
  fRatioLine = new TLine();
  fRatioLine->SetLineColor(kRed);
  fRatioLine->SetLineWidth(2);

  // set range of ratio line
  fRatioLine->SetX1(fRatioHist->GetXaxis()->GetXmin());
  fRatioLine->SetX2(fRatioHist->GetXaxis()->GetXmax());

  // set ratio type dependent options
  if (fRatioType == DIVONLY)
  {
    fRatioHist->Divide(fHist2);

    fRatioLine->SetY1(1.0);
    fRatioLine->SetY2(1.0);
  }
  else if (fRatioType == RES)
  {
    fRatioHist->Add(fHist2,-1);
    fRatioHist->Divide(fHist2);

    fRatioLine->SetY1(0.0);
    fRatioLine->SetY2(0.0);
  }
  else
  {
    std::cerr << "Somehow fRatioType is unset... Exiting..." << std::endl;
    exit(1);
  }

  // save to output file
  fOutFile->cd();
  fRatioHist->Write(fRatioHist->GetName(),TObject::kWriteDelete);
  fRatioLine->Write("RatioLine",TObject::kWriteDelete);
}

void PlotComparator::MakeLegend()
{
  std::cout << "Creating Legend..." << std::endl;

  // instantiate the legend
  fLegend = new TLegend(0.55,0.65,0.825,0.92);
  fLegend->SetName("Legend");
  fLegend->SetBorderSize(1);
  fLegend->SetLineColor(kBlack);

  // add entries
  fLegend->AddEntry(fHist1,fLabel1.Data(),"lep");
  fLegend->AddEntry(fHist2,fLabel2.Data(),"lep");

  // save to output file
  fOutFile->cd();
  fLegend->Write(Legend->GetName(),TObject::kWriteDelete);
}

void PlotComparator::SetupCanvasAndPads()
{
  std::cout << "Initializing canvas and pads..." << std::endl;

  fOutCanv = new TCanvas("OutCanv","");
  fOutCanv->cd();

  fUpperPad = new TPad("UpperPad","", Common::left_up, Common::bottom_up, Common::right_up, Common::top_up);
  fUpperPad->SetBottomMargin(0); // 0.04

  fLowerPad = new TPad("LowerPad", "", Common::left_lp, Common::bottom_lp, Common::right_lp, Common::top_lp);
  fLowerPad->SetTopMargin(0);  // 0.04
  fLowerPad->SetBottomMargin(0.35);
}

void PlotComparator::DrawUpperPad()
{
  std::cout << "Drawing upper pad..." << std::endl;

  // Pad Gymnastics
  fOutCanv->cd();
  fUpperPad->Draw();
  fUpperPad->cd();
  
  // Ensure x-axis is not visible
  fHist1->GetXaxis()->SetLabelSize(0);
  fHist1->GetXaxis()->SetTitleSize(0);

  // Have to scale TDR style values by height of upper pad
  fHist1->GetYaxis()->SetLabelSize  (Common::LabelSize / Common::height_up); 
  fHist1->GetYaxis()->SetTitleSize  (Common::TitleSize / Common::height_up);
  fHist1->GetYaxis()->SetTitleOffset(Common::TitleFF * Common::TitleYOffset * Common::height_up);

  // draw hists
  fHist1->Draw("EP");
  fHist2->Draw("EP SAME");

  // draw legend
  fLegend->Draw("SAME");
}

void PlotComparator::DrawLowerPad()
{  
  std::cout << "Drawing lower pad..." << std::endl;

  // Pad gymnastics
  fOutCanv->cd(); 
  fLowerPad->Draw();
  fLowerPad->cd(); 

  // draw th1 first so line can appear, then draw over it (and set Y axis divisions)
  fRatioHist->Draw("EP");

  // set params for line then draw
  fRatioLine->Draw();

  // redraw
  fRatioHist->Draw("EP SAME");
}

void PlotComparator::SaveOutput(const Float_t lumi)
{
  std::cout << "Saving hist as images..." << std::endl;

  // Go back to the main canvas before saving and write out lumi info
  fOutCanv->cd();
  Common::CMSLumi(fOutCanv,0,lumi);

  // Save a log version first
  PlotComparator::PrintCanvas(true);

  // Save a linear version second
  PlotComparator::PrintCanvas(false);

  // save to output file
  fOutFile->cd();
  fOutCanv->Write(fOutCanv->GetName(),TObject::kWriteDelete);
}  

void PlotComparator::PrintCanvas(const Bool_t isLogy)
{
  // pad gymnastics
  fOutCanv->cd();
  fUpperPad->cd();
  fUpperPad->SetLogy(isLogy);

  // set min and max
  if (isLogy)
  {
    fHist1->SetMinimum(fMinY/1.5);
    fHist1->SetMaximum(fMaxY*1.5);
  }
  else 
  {
    fHist1->SetMinimum( fMinY > 0 ? fMinY/1.05 : fMinY*1.05 );
    fHist1->SetMaximum( fMaxY > 0 ? fMaxY*1.05 : fMaxY/1.05 );      
  }

  // save canvas as images
  Common::SaveAs(fOutCanv,Form("%s_%s",fOutFileText.Data(),(isLogy?"log":"lin")));
}

void PlotComparator::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  fOutFile->cd();
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));

  // give grand title
  fConfigPave->AddText("***** PlotComparator Config *****");

  // dump compare config first
  Common::AddTextFromInputConfig(fConfigPave,"PlotComparator Config",fCompareConfig);

  // Add in File1 config
  Common::AddPaddingToPave(fConfigPave,3);
  fConfigPave->AddText("***** File1 Config *****");
  Common::AddTextFromInputPave(fConfigPave,fFile1);

  // Add in File2 config
  Common::AddPaddingToPave(fConfigPave,3);
  fConfigPave->AddText("***** File2 Config *****");
  Common::AddTextFromInputPave(fConfigPave,fFile2);
  
  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void PlotComparator::SetupDefaults()
{
  std::cout << "Setting defaults..." << std::endl;

  fRatioType = DIVONLY;
  fRatioYTitle = "After/Before";
  fRatioMinY = 0.f;
  fRatioMaxY = 2.f;
}

void PlotComparator::SetupCompareConfig()
{
  std::cout << "Reading compare config..." << std::endl;

  std::ifstream infile(Form("%s",fCompareConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("ratio_type=") != std::string::npos)
    {
      const auto type_str = Common::RemoveDelim(str,"ratio_type=");
      if (type_str.find("DIVONLY") != std::string::npos)
      {
	fRatioType = DIVONLY;
      }
      else if (type_str.find("RES") != std::string::npos)
      {
	fRatioType = RES;
      }
      else
      {
	std::cerr << type_str.c_str() << " is not a valid option for ratio_type! Exiting..." << std::endl;
	exit(1);
      }
    }
    else if (str.find("ratio_y_title=") != std::string::npos)
    {
      fRatioYTitle = Common::RemoveDelim(str,"ratio_y_title=");
    }
    else if (str.find("ratio_min_y=") != std::string::npos)
    {
      fRatioMinY = Common::Atof(str);
    }
    else if (str.find("ratio_max_y=") != std::string::npos)
    {
      fRatioMaxY = Common::Atof(str);
    }
    else 
    {
      std::cerr << "Aye... your compare config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

Float_t PlotComparator::GetHistMinY(const TH1F * hist)
{
  Float_t minY = 1e9;
  
  for (Int_t ibin = 0; ibin < hist->GetXaxis()->GetNbins(); ibin++)
  {
    const auto content = hist->GetBinContent(ibin);
    if (content > 0 && content < minY) minY = content;
  }

  return minY;
}

Float_t PlotComparator::GetHistMaxY(const TH1F * hist)
{
  return hist->GetBinContent(hist->GetMaximumBin());
}
