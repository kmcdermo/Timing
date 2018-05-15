#include "TreePlotter.hh"

TreePlotter::TreePlotter(const TString & infilename, const TString & insignalfilename, 
			 const TString & cutconfig, const TString & plotconfig, const Bool_t scalearea, const TString & outfiletext) 
  : fInFileName(infilename), fInSignalFileName(insignalfilename), 
    fCutConfig(cutconfig), fPlotConfig(plotconfig), fScaleArea(scalearea), fOutFileText(outfiletext)
{
  std::cout << "Initializing TreePlotter..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // Get input file
  fInFile = TFile::Open(Form("%s",fInFileName.Data()));
  Config::CheckValidFile(fInFile,fInFileName);
 
  // Get signal input file
  fInSignalFile = TFile::Open(Form("%s",fInSignalFileName.Data()));
  Config::CheckValidFile(fInSignalFile,fInSignalFileName);

  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Config::SetTDRStyle(fTDRStyle);
  gROOT->ForceStyle();

  // setup hists
  TreePlotter::SetupDefaults();
  TreePlotter::SetupConfig();
  TreePlotter::SetupPlotConfig();
  TreePlotter::SetupSignalsToPlot();
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
  delete DataHist;

  delete fOutFile;
  for (auto & HistPair : HistMap) delete HistPair.second;
  delete fTDRStyle;
  delete fInFile;
}

void TreePlotter::MakePlot()
{
  // Fill Hists from TTrees
  TreePlotter::MakeHistFromTrees();

  // Make Data Output
  TreePlotter::MakeDataOutput();

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

  // Dump integrals into text file
  TreePlotter::DumpIntegrals();
}

void TreePlotter::MakeHistFromTrees()
{
  std::cout << "Making hists from input trees..." << std::endl;

  // loop over sample groups for each tree
  for (const auto & TreeNamePair : Config::TreeNameMap)
  {
    // Init
    const auto & sample   = TreeNamePair.first;
    const auto & treename = TreeNamePair.second;
    std::cout << "Working on tree: " << treename.Data() << std::endl;
	
    // Get infile
    auto & infile = ((Config::GroupMap[sample] != isSignal) ? fInFile : fInSignalFile);
    infile->cd();

    // Get TTree
    auto intree = (TTree*)infile->Get(Form("%s",treename.Data()));
    const auto isnull = Config::IsNullTree(intree);

    if (!isnull)
    {
      std::cout << "Filling hist from tree..." << std::endl;

      // get the hist we wish to write to
      auto & hist = HistMap[sample];
      
      // Fill from tree
      intree->Draw(Form("%s>>%s",fXVar.Data(),hist->GetName()),Form("(%s) * (%s)",Config::CutMap[sample].Data(),Config::WeightString(sample).Data()),"goff");

      // delete tree;
      delete intree;
    }
    else
    {
      std::cout << "Skipping null tree..." << std::endl;
    }
  }

  // rescale bins by widths if variable size
  if (fXVarBins)
  {
    const Bool_t isUp = false;
    for (auto & HistPair : HistMap)
    {
      auto & hist = HistPair.second;
      Config::Scale(hist,isUp);
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

void TreePlotter::MakeDataOutput()
{
  std::cout << "Making Data Output..." << std::endl;

  // Make new data hist in case we are blinded
  DataHist = TreePlotter::SetupHist("Data_Hist_Plotted");
  DataHist->Add(Hist_Map[Data]);

  for (const auto & Blind : fBlinds)
  {
    const auto binXlow = DataHist->GetXaxis()->FindBin(Blind.xlow);
    const auto binXup  = DataHist->GetXaxis()->FindBin(Blind.xup);

    for (auto ibinX = binXlow; ibinX <= binXup; ibinX++) 
    {
      DataHist->SetBinContent(ibinX,0.f);
      DataHist->SetBinError  (ibinX,0.f);
    }
  }

  DataHist->Write(DataHist->GetName(),TObject::kWriteDelete);
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
  RatioHist->Add(DataHist);
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
  RatioLine->Write("RatioLine",TObject::kWriteDelete);
}

void TreePlotter::MakeLegend()
{
  std::cout << "Creating Legend..." << std::endl;

  // instantiate the legend
  Legend = new TLegend(0.682,0.7,0.825,0.92);
  Legend->SetName("Legend");
  //  Legend->SetNColumns(2);
  Legend->SetBorderSize(1);
  Legend->SetLineColor(kBlack);

  // add all bkgd samples and data to legend
  for (const auto & HistPair : HistMap)
  {
    // get sample
    const auto & sample = HistPair.first;
    const auto & hist   = HistPair.second;

    // if signal, check to see if this is a plotted one
    if (Config::GroupMap[sample] == isSignal)
    {
      if (!fPlotSignalMap[sample]) continue;
    }

    TString fillType;
    if      (Config::GroupMap[sample] == isData) fillType = "epl";
    else if (Config::GroupMap[sample] == isBkgd) fillType = "f";
    else                                         fillType = "l";

    Legend->AddEntry(hist,Config::LabelMap[sample].Data(),fillType.Data());
  }

  // add the mc unc. to legend
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

  DataHist->SetMinimum(fMinY);
  DataHist->SetMaximum(fMaxY);
  
  // now draw the plots for upper pad in absurd order because ROOT is dumb
  DataHist->Draw("PE"); // draw first so labels appear

  // Have to scale TDR style values by height of upper pad
  DataHist->GetYaxis()->SetLabelSize  (Config::LabelSize / Config::height_up); 
  DataHist->GetYaxis()->SetTitleSize  (Config::TitleSize / Config::height_up);
  DataHist->GetYaxis()->SetTitleOffset(Config::TitleFF * Config::TitleYOffset * Config::height_up);
  
  // Draw stack
  BkgdStack->Draw("HIST SAME"); 
  UpperPad->RedrawAxis("SAME"); // stack kills axis

  // Draw Signal : loop over ones to plot
  for (const auto & PlotSignalPair : fPlotSignalMap)
  {
    const auto & sample = PlotSignalPair.first;
    const auto & isplot = PlotSignalPair.second;
    if (!isplot) continue;
    
    Hist_Map[sample]->Draw("HIST SAME");
  }

  // Draw MC sum total error as well on top of stack --> E2 makes error appear as rectangle
  BkgdHist->Draw("E2 SAME");

  // Redraw data to make it appear again!
  DataHist->Draw("PE SAME"); 
 
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
    DataHist->SetMinimum(fMinY/1.5);
    DataHist->SetMaximum(fMaxY*1.5);
  }
  else 
  {
    DataHist->SetMinimum( fMinY > 0 ? fMinY/1.05 : fMinY*1.05 );
    DataHist->SetMaximum( fMaxY > 0 ? fMaxY*1.05 : fMaxY/1.05 );      
  }

  // save canvas as png
  OutCanv->cd();
  OutCanv->SaveAs(Form("%s_%s.png",fOutFileText.Data(),(isLogy?"log":"lin")));
}

void TreePlotter::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  fOutFile->cd();
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Config::pavename.Data()));
  std::string str; // tmp string

  // give grand title
  fConfigPave->AddText("***** TreePlotter Config *****");

  // dump plot cut config first
  fConfigPave->AddText(Form("TreePlotter Cut Config: %s",fCutConfig.Data()));
  std::ifstream cutfile(Form("%s",fCutConfig.Data()),std::ios::in);
  while (std::getline(cutfile,str))
  {
    fConfigPave->AddText(str.c_str());
  }

  // dump plot config second
  fConfigPave->AddText(Form("Plot Config: %s",fPlotConfig.Data()));
  std::ifstream plotfile(Form("%s",fPlotConfig.Data()),std::ios::in);
  while (std::getline(plotfile,str))
  {
    fConfigPave->AddText(str.c_str());
  }

  // dump scaling choices
  fConfigPave->AddText(fScaleArea?"Scale MC to data area":"Scale MC to luminosity");

  // save name of infile, redundant
  fConfigPave->AddText(Form("Infile name: %s",fInFileName.Data()));

  // dump in old config
  Config::AddTextFromInputPave(fConfigPave,fInFile);

  // dump in old signal config
  Config::AddTextFromInputPave(fConfigPave,fInSignalFile);
  
  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void TreePlotter::DumpIntegrals()
{
  std::cout << "Dumping integrals into text file..." << std::endl;

  // make dumpfile object
  std::ofstream dumpfile(Form("%s_integrals.txt",fOutFileText.Data()),std::ios_base::out);

  // Signal MC zeroth
  for (const auto & HistPair : HistMap)
  {
    if (Config::GroupMap[HistPair.first] == isSignal)
    {
      auto sign_err = 0.;
      const auto sign_int = HistPair.second->IntegralAndError(1,HistPair.second->GetXaxis()->GetNbins(),sign_err,(fXVarBins?"width":""));
      dumpfile << HistPair.second->GetName() << " : " << sign_int << " +/- " << sign_err << std::endl;
    }
  }
  dumpfile << "-------------------------------------" << std::endl;

  // Individual MC first
  for (const auto & HistPair : HistMap)
  {
    if (Config::GroupMap[HistPair.first] == isBkgd)
    {
      auto bkgd_err = 0.0;
      const auto bkgd_int = HistPair.second->IntegralAndError(1,HistPair.second->GetXaxis()->GetNbins(),bkgd_err,(fXVarBins?"width":""));
      dumpfile << HistPair.second->GetName() << " : " << bkgd_int << " +/- " << bkgd_err << std::endl;
    }
  }
  dumpfile << "-------------------------------------" << std::endl;
  
  // Sum MC second
  auto mc_err = 0.0;
  const auto mc_int = BkgdHist->IntegralAndError(1,BkgdHist->GetXaxis()->GetNbins(),mc_err,(fXVarBins?"width":""));
  dumpfile << BkgdHist->GetName() << " : " << mc_int << " +/- " << mc_err << std::endl;

  // Data third
  auto data_err = 0.0;
  const auto data_int = DataHist->IntegralAndError(1,DataHist->GetXaxis()->GetNbins(),data_err,(fXVarBins?"width":""));
  dumpfile << DataHist->GetName() << " : " << data_int << " +/- " << data_err << std::endl;
  dumpfile << "-------------------------------------" << std::endl;

  // Make ratio
  const auto ratio = data_int/mc_int;
  const auto ratio_err = ratio*std::sqrt(std::pow(data_err/data_int,2)+std::pow(mc_err/mc_int,2));
  dumpfile << "Data/MC : " << ratio << " +/- " << ratio_err << std::endl;
}

void TreePlotter::ScaleToArea()
{
  const auto area_sf = DataHist->Integral()/BkgdHist->Integral();

  // first scale each individual sample
  for (auto & HistPair : HistMap)
  {
    if (Config::GroupMap[HistPair.first] == isBkgd)
    {
      HistPair.second->Scale(area_sf);
    }
  }

  // lastly, scale combo bkgd hist
  BkgdHist->Scale(area_sf);
}

Float_t TreePlotter::GetHistMinimum()
{
  auto min = 1.0e9;

  // need to loop through to check bin != 0
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const auto & hist = HistPair.second;

    // skip samples not plotted
    if (Config::GroupMap[sample] == isSignal)
    {
      if (!fPlotSignalMap[sample]) continue;
    }

    // get tmp min
    for (auto bin = 1; bin <= hist->GetNbinsX(); bin++)
    {
      const auto tmpmin = hist->GetBinContent(bin);
      if ((tmpmin < min) && (tmpmin > 0)) min = tmpmin;
    }
  }

  return min;
}

Float_t TreePlotter::GetHistMaximum()
{
  const auto datamax = DataHist->GetBinContent(DataHist->GetMaximumBin());
  const auto bkgdmax = BkgdHist->GetBinContent(BkgdHist->GetMaximumBin());
  return (datamax > bkgdmax ? datamax : bkgdmax);
}

void TreePlotter::SetupDefaults()
{
  fIsLogX      = false;
  fXVarBins    = false;
  fIsLogY      = false;
}

void TreePlotter::SetupConfig()
{
  std::cout << "Setting up Config..." << std::endl;

  Config::SetupSamples();
  Config::SetupSignalSamples();
  Config::SetupGroups();
  Config::SetupTreeNames();
  Config::SetupHistNames();
  Config::SetupColors();
  Config::SetupLabels();
  Config::SetupCuts(fCutConfig);
}

void TreePlotter::SetupPlotConfig()
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
    else if (str.find("x_labels=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"x_labels=");
      Config::SetupBinLabels(str,fXLabels);
    }
    else if (str.find("y_title=") != std::string::npos)
    {
      fYTitle = Config::RemoveDelim(str,"y_title=");
    }
    else if (str.find("y_scale=") != std::string::npos)
    {
      Config::SetupScale(str,fIsLogY);
    }
    else if (str.find("blinding=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"blinding=");
      Config::SetupBlinding(str,fBlinds);
    }
    else if (str.find("signals_to_plot=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"signals_to_plot=");
      Config::SetupSignalsToPlot(str,fPlotSignalMap);
    }
    else 
    {
      std::cerr << "Aye... your plot config is messed up, try again!" << std::endl;
      exit(1);
    }
  }
}

void TreePlotter::SetupSignalsToPlot()
{
  // loop over signals
  for (const auto & GroupPair : Config::GroupMap)
  {
    const auto & sample = GroupPair.first;
    const auto & group  = GroupPair.second;

    // skip non-signal samples
    if (group != isSignal) continue;
    
    // only mark samples false for those that are not marked true
    if (!fPlotSignalMap.count(sample)) fPlotSignalMap[sample] = false;
  }
}

void TreePlotter::SetupHists()
{
  std::cout << "Setting up output hists..." << std::endl;

  // instantiate each histogram
  for (const auto & HistNamePair : Config::HistNameMap)
  {
    HistMap[HistNamePair.first] = TreePlotter::SetupHist(HistNamePair.second);
  }
  
  // set colors for each histogram
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
  // get the bins in a struct for ROOT
  const auto xbins = &fXBins[0];
  
  // initialize new histogram
  auto hist = new TH1F(name.Data(),fTitle.Data(),fXBins.size()-1,xbins);

  // set axis titles
  hist->GetXaxis()->SetTitle(fXTitle.Data());
  hist->GetYaxis()->SetTitle(fYTitle.Data());

  // set axis labels only if read in
  if (fXLabels.size() > 0)
  {
    for (auto ibin = 1; ibin <= hist->GetXaxis()->GetNbins(); ibin++)
    {
      hist->GetXaxis()->SetBinLabel(ibin,fXLabels[ibin-1].Data());
    }
  }

  // still do not understand why ROOT does not do this by default...
  hist->Sumw2();
  
  return hist;
}
