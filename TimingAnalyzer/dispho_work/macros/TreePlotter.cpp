#include "TreePlotter.hh"

TreePlotter::TreePlotter(const TString & cutconfig, const TString & plotconfig, const Bool_t scalearea, const TString & outfiletext) 
  : fCutConfig(cutconfig), fPlotConfig(plotconfig), fScaleArea(scalearea), fOutFileText(outfiletext)
{
  std::cout << "Initializing..." << std::endl;

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
  TreePlotter::SetupConfig();
  TreePlotter::SetupHists();

  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");
}

TreePlotter::~TreePlotter() 
{
  // delete everything
  delete fConfigPave;

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

  // Write Out Config
  TreePlotter::MakeConfigPave();
}

void TreePlotter::MakeHistFromTrees()
{
  for (const auto & SamplePair : Config::SampleMap)
  {
    // Init
    const auto & input  = SamplePair.first;
    const auto & sample = SamplePair.second;
    std::cout << "Working on input: " << input.Data() << std::endl;

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
    tree->Draw(Form("%s>>%s",fXVar.Data(),hist->GetName()),Form("(%s) * (%s)",Config::CutMap[sample].Data(),Config::WeightString(input,sample).Data()),"goff");
    
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

  // rescale bins by widths if variable size
  if (fXVarBins)
  {
    for (auto & HistPair : HistMap)
    {
      auto & hist = HistPair.second;
      const UInt_t nbinsX = hist->GetXaxis()->GetNbins();
      for (UInt_t ibinX = 1; ibinX <= nbinsX; ibinX++)
      {
	const Float_t binwidthX = hist->GetXaxis()->GetBinWidth(ibinX);
	hist->SetBinContent(ibinX,(hist->GetBinContent(ibinX)/binwidthX));
	hist->SetBinError  (ibinX,(hist->GetBinError  (ibinX)/binwidthX));
      }
    }
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
  for (const auto & HistPair : HistMap)
  {
    if (Config::GroupMap[HistPair.first] == isBkgd)
    {
      BkgdHist->Add(HistPair.second);
    }
  }
  BkgdHist->SetMarkerSize(0);
  BkgdHist->SetFillStyle(3254);
  BkgdHist->SetFillColor(kGray+3);

  // ***** SCALE TO AREA OF DATA ***** //
  if (fScaleArea) TreePlotter::ScaleToArea();

  // Make Background Stack
  BkgdStack = new THStack("Bkgd_Stack","");
  
  // sort by smallest to biggest, then add
  std::vector<SampleType> StackOrder;
  for (const auto & GroupPair : Config::GroupMap)
  {
    if (GroupPair.second == isBkgd)
    {
      StackOrder.push_back(GroupPair.first);
    }
  }
  std::sort(StackOrder.begin(),StackOrder.end(),
	    [&](const auto & sample1, const auto & sample2)
	    { return HistMap[sample1]->Integral() < HistMap[sample2]->Integral(); });
  
  // add hists to stack from smallest to biggest
  for (const auto & Sample : StackOrder)
  {
    BkgdStack->Add(HistMap[Sample]);
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
    if      (Config::GroupMap[sample] == isData) fillType = "epl";
    else if (Config::GroupMap[sample] == isBkgd) fillType = "f";
    else                                         fillType = "l";

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

  // Get and Set Global Maximum and Minimum
  fMinY = TreePlotter::GetHistMinimum();
  fMaxY = TreePlotter::GetHistMaximum();

  HistMap[Data]->SetMinimum(fMinY);
  HistMap[Data]->SetMaximum(fMaxY);
  
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
  LowerPad->SetLogx(fIsLogX);

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

  // Go back to the main canvas before saving and write out lumi info
  OutCanv->cd();
  Config::CMSLumi(OutCanv,0);

  // Save a log version first
  TreePlotter::PrintCanvas(true);

  // Save a linear version second
  TreePlotter::PrintCanvas(false);

  // save to output file
  fOutFile->cd();
  OutCanv->Write(OutCanv->GetName(),TObject::kWriteDelete);
}  

void TreePlotter::PrintCanvas(const Bool_t isLogy)
{
  // pad gymnastics
  OutCanv->cd();
  UpperPad->cd();
  UpperPad->SetLogy(isLogy);

  // set min and max
  if (isLogy)
  {
    HistMap[Data]->SetMinimum(fMinY/1.5);
    HistMap[Data]->SetMaximum(fMaxY*1.5);
  }
  else 
  {
    HistMap[Data]->SetMinimum( fMinY > 0 ? fMinY/1.05 : fMinY*1.05 );
    HistMap[Data]->SetMaximum( fMaxY > 0 ? fMaxY*1.05 : fMaxY/1.05 );      
  }

  // save canvas as png
  OutCanv->cd();
  OutCanv->SaveAs(Form("%s_%s.png",fOutFileText.Data(),(isLogy?"log":"lin")));
}

void TreePlotter::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave
  fConfigPave = new TPaveText();
  fConfigPave->SetName("Config");
  std::string str; // tmp string
  
  // dump cut config first
  fConfigPave->AddText("Cut Config");
  std::ifstream cutfile(Form("%s",fCutConfig.Data()),std::ios::in);
  while (std::getline(cutfile,str))
  {
    fConfigPave->AddText(str.c_str());
  }

  // dump plot config second
  fConfigPave->AddText("Plot Config");
  std::ifstream plotfile(Form("%s",fPlotConfig.Data()),std::ios::in);
  while (std::getline(plotfile,str))
  {
    fConfigPave->AddText(str.c_str());
  }

  // dump scaling choices
  fConfigPave->AddText(fScaleArea?"Scale MC to data area":"Scale MC to luminosity");

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void TreePlotter::DumpIntegral()
{
  std::ofstream dumpfile(Form("%s_integrals.txt",outfiletext.Data()),std::ios_base::out);

  // Individual MC first
  for (const auto & HistPair : HistMap)
  {
    if (Config::GroupMap[HistPair.first] == isBkgd)
    {
      Double_t bkgd_err = 0.;
      const Double_t bkgd_int = HistPair.second->IntegralAndError(1,HistPair->GetXaxis()->GetNbinsX(),bkgd_err,(fXVarBins?"width":""));
      dumpfile << HistPair.second->GetName() << " : " << bkgd_int << " +/- " << bkgd_err << std::endl;
    }
  }
  dumpfile << "-------------------------------------" << std::endl;
  
  // Sum MC second
  Double_t mc_err = 0;
  const Double_t mc_int = BkgdHist->IntegralAndError(1,Bkgd_Hist->GetXaxis()->GetNbinsX(),mc_err,(fXVarBins?"width":""));
  dumpfile << BkgdHist->GetName() << " : " << mc_int << " +/- " << mc_err << std::endl;

  // Data third
  Double_t data_err = 0;
  const Double_t data_int = HistMap[Data]->IntegralAndError(1,HistMap[Data]->GetXaxis()->GetNbinsX(),data_err,(fXVarBins?"width":""));
  dumpfile << HistMap[Data]->GetName() << " : " << data_int << " +/- " << data_err << std::endl;
  dumpfile << "-------------------------------------" << std::endl;

  // Make ratio
  const Double_t ratio = data_int/mc_int;
  const Double_t ratio_err = ratio*std::sqrt(std::pow(data_err/data_int,2)+std::pow(mc_err/mc_int,2));
  dumpfile << "Data/MC : " << ratio << " +/- " << ratio_err << std::endl;
}

void TreePlotter::ScaleToArea()
{
  const Float_t area_sf = HistMap[Data]->Integral()/BkgdHist->Integral();

  for (auto & HistPair : HistMap)
  {
    if (Config::GroupMap[HistPair.first] == isBkgd)
    {
      HistPair.second->Scale(area_sf);
    }
  }
  BkgdHist->Scale(area_sf);
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

void TreePlotter::SetupConfig()
{
  Config::SetupSamples();
  Config::SetupGroups();
  Config::SetupHistNames();
  Config::SetupColors();
  Config::SetupLabels();
  Config::SetupCuts(fCutConfig);
}

void TreePlotter::ReadPlotConfig()
{
  std::cout << "Reading plot config..." << std::endl;

  std::ifstream infile(Form("%s",fPlotConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("plot_title=") != std::string::npos)
    {
      fTitle = Config::RemoveDelim(str,"plot_title=");
    }
    else if (str.find("x_title=") != std::string::npos)
    {
      fXTitle = Config::RemoveDelim(str,"x_title=");
    }
    else if (str.find("x_scale=") != std::string::npos)
    {
      Config::SetupScale(str,fIsLogX);
    }
    else if (str.find("x_var=") != std::string::npos)
    {
      fXVar = Config::RemoveDelim(str,"x_var=");
    }
    else if (str.find("x_bins=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"x_bins=");
      Config::SetupBins(str,fXBins,fXVarBins);
    }
    else if (str.find("y_title=") != std::string::npos)
    {
      fYTitle = Config::RemoveDelim(str,"y_title=");
    }
    else if (str.find("y_scale=") != std::string::npos)
    {
      Config::SetupScale(str,fIsLogY);
    }
    else 
    {
      std::cerr << "Aye... your plot config is messed up, try again!" << std::endl;
      exit(1);
    }
  }
}

void TreePlotter::SetupHists()
{
  TreePlotter::ReadPlotConfig();
  
  for (const auto & HistNamePair : Config::HistNameMap)
  {
    HistMap[HistNamePair.first] = SetupHist(HistNamePair.second);
  }
  
  for (auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const auto & hist   = HistPair.second;
    
    hist->SetLineColor(Config::ColorMap[sample]);
    hist->SetMarkerColor(Config::ColorMap[sample]);
    if (Config::GroupMap[sample] == isBkgd)
    {
      hist->SetFillColor(Config::ColorMap[sample]);
      hist->SetFillStyle(1001);
    }
    else if (Config::GroupMap[sample] == isSignal)
    {
      hist->SetLineWidth(2);
    }
  }
}

TH1F * TreePlotter::SetupHist(const TString & name)
{
  const Double_t * xbins = &fXBins[0];

  TH1F * hist = new TH1F(name.Data(),fTitle.Data(),fXBins.size()-1,xbins);
  hist->GetXaxis()->SetTitle(fXTitle.Data());
  hist->GetYaxis()->SetTitle(fYTitle.Data());
  hist->Sumw2();
  
  return hist;
}
