#include "TreePlotter.hh"

TreePlotter::TreePlotter(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
			 const TString & plotconfig, const TString & miscconfig, const TString & era,
			 const Bool_t savemetadata, const TString & outfiletext) 
  : fInFileName(infilename), fInSignalFileName(insignalfilename), fCutConfig(cutconfig),
    fPlotConfig(plotconfig), fMiscConfig(miscconfig), fEra(era), fOutFileText(outfiletext)
{
  std::cout << "Initializing TreePlotter..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // Get input file
  fInFile = TFile::Open(Form("%s",fInFileName.Data()));
  Common::CheckValidFile(fInFile,fInFileName);
 
  // Get signal input file
  fInSignalFile = TFile::Open(Form("%s",fInSignalFileName.Data()));
  Common::CheckValidFile(fInSignalFile,fInSignalFileName);

  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(fTDRStyle);

  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");

  // setup config
  TreePlotter::SetupDefaults();
  TreePlotter::SetupSaveMetaData(savemetadata);
  TreePlotter::SetupCommon();
  TreePlotter::SetupMiscConfig(fMiscConfig);
  if (fSkipData) Common::RemoveGroup(SampleGroup::isData);
  if (fSignalsOnly) Common::KeepOnlySignals();
  TreePlotter::SetupPlotConfig(fPlotConfig);

  // setup hists
  TreePlotter::SetupHists();
  TreePlotter::SetupHistsStyle();
}

void TreePlotter::MakeTreePlot()
{
  // Fill Hists from TTrees
  TreePlotter::MakeHistFromTrees(fInFile,fInSignalFile);

  // Make Data Output
  TreePlotter::MakeDataOutput();

  // Make Bkgd Output
  TreePlotter::MakeBkgdOutput();

  // Make Signal Output
  TreePlotter::MakeSignalOutput();

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
  TreePlotter::SaveOutput(fOutFileText,fEra);

  // Write Out Config
  if (fSaveMetaData) TreePlotter::MakeConfigPave();

  // Dump integrals into text file
  TreePlotter::DumpIntegrals(fOutFileText);

  // Delete allocated memory
  TreePlotter::DeleteMemory(true);
}

void TreePlotter::MakeHistFromTrees(TFile *& inFile, TFile *& inSignalFile)
{
  std::cout << "Making hists from input trees..." << std::endl;

  // loop over sample groups for each tree
  for (const auto & TreeNamePair : Common::TreeNameMap)
  {
    // Init
    const auto & sample   = TreeNamePair.first;
    const auto & treename = TreeNamePair.second;
    std::cout << "Working on tree: " << treename.Data() << std::endl;
	
    // Get infile
    auto & infile = ((Common::GroupMap[sample] != SampleGroup::isSignal) ? inFile : inSignalFile);
    infile->cd();

    // Get TTree
    auto intree = (TTree*)infile->Get(Form("%s",treename.Data()));
    const auto isnull = Common::IsNullTree(intree);

    if (!isnull)
    {
      std::cout << "Filling hist from tree..." << std::endl;

      // get the hist we wish to write to (and holy crap, ROOT's internal memory residency is stupid)
      auto & hist = HistMap[sample];
      hist->SetDirectory(infile);

      intree->Draw(Form("%s>>%s",Common::XVarMap[sample].Data(),hist->GetName()),Form("%s",Common::CutWgtMap[sample].Data()),"goff");

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
      Common::Scale(hist,isUp);
      // const auto nBinsX = hist->GetXaxis()->GetNbins();
      // hist->SetBinContent(nBinsX,hist->GetBinContent(nBinsX)+hist->GetBinContent(nBinsX+1));
      // hist->SetBinError(nBinsX,std::sqrt(std::pow(hist->GetBinContent(nBinsX),2)+std::pow(hist->GetBinContent(nBinsX+1),2)));
    }
  }

  // save totals to output file
  Common::WriteMap(fOutFile,HistMap);
}

void TreePlotter::MakeDataOutput()
{
  std::cout << "Making Data Output..." << std::endl;

  // Make new data hist in case we are blinded
  DataHist = TreePlotter::SetupHist(Form("%s_Plotted",Common::HistNameMap["Data"].Data()));
  if (!fSignalsOnly && !fSkipData) DataHist->Add(HistMap["Data"]);

  if (fBlindData)
  {
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
  }

  // scale to unity 
  if (fScaleToUnity) DataHist->Scale(1.f/DataHist->Integral(fXVarBins?"width":""));

  // save to output file
  if (!fSignalsOnly && !fSkipData) Common::Write(fOutFile,DataHist);
}

void TreePlotter::MakeBkgdOutput()
{
  std::cout << "Making Bkgd Output..." << std::endl;

  // Extra Hists
  BkgdHist = TreePlotter::SetupHist(Form("%s",Common::BkgdHistName.Data())); // Make Total Bkgd Hist: for ratio and error plotting
  EWKHist  = TreePlotter::SetupHist(Form("%s",Common::EWKHistName.Data())); // Make EWK Histogram for posterity : do not plot it though

  // Loop over histograms, adding up bkgd as needed
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const auto & hist = HistPair.second;

    if (Common::GroupMap[sample] == SampleGroup::isBkgd)
    {
      // add all MC bkgds for bkgd hist 
      BkgdHist->Add(hist);
      
      // ewk hist excludes GJets and QCD MC
      if (!Common::IsCR(sample))
      {
	EWKHist->Add(hist);
      }
    }
  }

  // set style for BkgdHist (used for plotting error bands)
  BkgdHist->SetMarkerSize(0);
  BkgdHist->SetFillStyle(3254);
  BkgdHist->SetFillColor(kGray+3);

  // scale to unity
  if (fScaleToUnity) TreePlotter::ScaleMCToUnity();

  // scale mc to data
  if (fScaleMCToData) TreePlotter::ScaleMCToData();

  // Make Background Stack
  BkgdStack = new THStack("Bkgd_Stack","");
  
  // sort by smallest to biggest, then add
  std::vector<TString> StackOrder;
  for (const auto & GroupPair : Common::GroupMap)
  {
    if (GroupPair.second == SampleGroup::isBkgd)
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
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    auto & hist = HistPair.second;

    if (Common::GroupMap[sample] == SampleGroup::isBkgd) Common::Write(fOutFile,hist,Form("%s_Plotted",hist->GetName()));
  }

  // save to outfile
  if (!fSignalsOnly)
  {
    Common::Write(fOutFile,BkgdHist);
    Common::Write(fOutFile,EWKHist);
    Common::Write(fOutFile,BkgdStack);
  }
}

void TreePlotter::MakeSignalOutput()
{
  std::cout << "Making Signal Output..." << std::endl;

  // no need to make new hists, just rescale and save as "_Plotted"
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    auto & hist = HistPair.second;

    if (Common::GroupMap[sample] == SampleGroup::isSignal)
    {
      if (fScaleToUnity) hist->Scale(1.f/hist->Integral(fXVarBins?"width":""));

      // save it regardless
      Common::Write(fOutFile,hist,Form("%s_Plotted",hist->GetName()));
    }
  }
}

void TreePlotter::MakeRatioOutput()
{
  std::cout << "Making Ratio Output..." << std::endl;

  // ratio value plot
  RatioHist = TreePlotter::SetupHist("Ratio_Hist");
  if (fSignalsOnly || fSkipData)
  {
    for (auto ibinX = 1; ibinX <= RatioHist->GetXaxis()->GetNbins(); ibinX++)
    {
      RatioHist->SetBinContent(ibinX,-1.f);
    }
  }
  else
  {
    RatioHist->Add(DataHist);
    RatioHist->Divide(BkgdHist);  
  }

  // set style for ratio plot
  RatioHist->GetYaxis()->SetTitle("Data/MC");
  RatioHist->SetMinimum(-0.1); // Define Y ..
  RatioHist->SetMaximum( 2.1); // .. range
  RatioHist->SetLineColor(kBlack);
  RatioHist->SetMarkerColor(kBlack);
  RatioHist->SetStats(0);      // No statistics on lower plot

  // rescale titles and labels
  RatioHist->GetYaxis()->SetNdivisions(505);
  RatioHist->GetXaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
  RatioHist->GetXaxis()->SetLabelOffset(Common::LabelOffset / Common::height_lp); 
  RatioHist->GetXaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
  RatioHist->GetXaxis()->SetTickLength (Common::TickLength  / Common::height_lp);
  RatioHist->GetYaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
  RatioHist->GetYaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
  RatioHist->GetYaxis()->SetTitleOffset(Common::TitleFF * Common::TitleYOffset * Common::height_lp);
  
  // ratio MC error plot
  RatioMCErrs = TreePlotter::SetupHist("Ratio_MCErrs");
  if (!fSignalsOnly && !fSkipData)
  {
    RatioMCErrs->Add(BkgdHist);
    RatioMCErrs->Divide(BkgdHist);
  }
  RatioMCErrs->SetMarkerSize(0);
  RatioMCErrs->SetFillStyle(3254);
  RatioMCErrs->SetFillColor(kGray+3);

  // don't display empty bins
  for (auto ibin = 1; ibin <= RatioMCErrs->GetNbinsX(); ibin++) 
  {
    if (RatioMCErrs->GetBinContent(ibin) == 0) {RatioMCErrs->SetBinContent(ibin,-2);} 
  }

  // save to output file
  if (!fSignalsOnly && !fSkipData)
  {
    Common::Write(fOutFile,RatioHist);
    Common::Write(fOutFile,RatioMCErrs);
  }  

  // ratio line
  RatioLine = new TLine();
  RatioLine->SetLineColor(kRed);
  RatioLine->SetLineWidth(2);

  // set line params
  RatioLine->SetX1(RatioHist->GetXaxis()->GetXmin());
  RatioLine->SetX2(RatioHist->GetXaxis()->GetXmax());
  RatioLine->SetY1(1.0);
  RatioLine->SetY2(1.0);

  // save to output file
  if (!fSignalsOnly && !fSkipData) Common::Write(fOutFile,RatioLine,"RatioLine");
}

void TreePlotter::MakeLegend()
{
  std::cout << "Creating Legend..." << std::endl;

  // instantiate the legend
  Legend = new TLegend(0.55,0.65,0.825,0.92);
  Legend->SetName("Legend");
  //  Legend->SetNColumns(2);
  Legend->SetBorderSize(1);
  Legend->SetLineColor(kBlack);

  // add data to legend
  if (!fSignalsOnly && !fSkipData) Legend->AddEntry(DataHist,Common::LabelMap["Data"].Data(),"epl");

  // add bkgd to legend
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const auto & hist   = HistPair.second;

    // only do background
    if (Common::GroupMap[sample] != SampleGroup::isBkgd) continue;

    if (!fSignalsOnly) Legend->AddEntry(hist,Common::LabelMap[sample].Data(),"f");
  }

  // add signals that we want to plot to the legend
  for (const auto & sample : fPlotSignalVec)
  {
    Legend->AddEntry(HistMap[sample],Common::LabelMap[sample].Data(),"l");
  }

  // add the mc unc. to legend
  // Legend->AddEntry(BkgdHist,"MC Unc.","f");

  // save to output file
  Common::Write(fOutFile,Legend);
}

void TreePlotter::InitOutputCanvPads()
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

void TreePlotter::DrawUpperPad()
{
  std::cout << "Drawing upper pad..." << std::endl;

  // Pad Gymnastics
  OutCanv->cd();
  UpperPad->Draw();
  UpperPad->cd();
  UpperPad->SetLogx(fIsLogX);

  // get relevant hist for setting options
  auto & hist = (fSignalsOnly || fSkipData) ? HistMap[fPlotSignalVec.front()] : DataHist;
  
  // Get and Set Global Minimum and Maximum
  TreePlotter::GetHistMinimum();
  TreePlotter::GetHistMaximum();

  // Have to scale TDR style values by height of upper pad
  hist->GetXaxis()->SetLabelSize(0);
  hist->GetXaxis()->SetTitleSize(0);

  hist->GetYaxis()->SetLabelSize  (Common::LabelSize / Common::height_up); 
  hist->GetYaxis()->SetTitleSize  (Common::TitleSize / Common::height_up);
  hist->GetYaxis()->SetTitleOffset(Common::TitleFF * Common::TitleYOffset * Common::height_up);
  
  // full info
  if (!fSignalsOnly)
  {
    // now draw the plots for upper pad in absurd order because ROOT is dumb
    if (!fSkipData)
    {
      DataHist->Draw("PE"); // draw first so labels appear
    }
    else
    {
      hist->Draw("HIST"); // tmp draw
    }
    
    // Draw stack
    BkgdStack->Draw("HIST SAME"); 
    UpperPad->RedrawAxis("SAME"); // stack kills axis
    
    // Draw MC sum total error as well on top of stack --> E2 makes error appear as rectangle
    BkgdHist->Draw("E2 SAME");

    // Redraw data to make it appear again!
    DataHist->Draw("PE SAME"); 
  } 

  // Draw Signal : loop over ones to plot
  for (auto isample = 0U; isample < fPlotSignalVec.size(); isample++)
  {
    const auto & sample = fPlotSignalVec[isample];
    auto & hist = HistMap[sample];

    hist->Draw(((isample==0 && fSignalsOnly)?"HIST":"HIST SAME"));
  }

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

  // some style since apparently TDR Style is crapping out

  // draw th1 first so line can appear, then draw over it (and set Y axis divisions)
  RatioHist->Draw("EP"); 

  // set params for line then draw
  RatioLine->Draw(fSignalsOnly?"":"SAME");

  if (!fSignalsOnly && !fSkipData)
  {
    // redraw to go over line
    RatioHist->Draw("EP SAME");

    // plots MC error copy
    RatioMCErrs->Draw("E2 SAME");  
  }
}

void TreePlotter::SaveOutput(const TString & outfiletext, const TString & era)
{
  std::cout << "Saving hist as images..." << std::endl;

  // Go back to the main canvas before saving and write out lumi info
  OutCanv->cd();
  Common::CMSLumi(OutCanv,0,era);

  // Save a log version first
  TreePlotter::PrintCanvas(outfiletext,true);

  // Save a linear version second
  TreePlotter::PrintCanvas(outfiletext,false);

  // save to output file
  Common::Write(fOutFile,OutCanv);
}  

void TreePlotter::PrintCanvas(const TString & outfiletext, const Bool_t isLogy)
{
  // pad gymnastics
  OutCanv->cd();
  UpperPad->cd();
  UpperPad->SetLogy(isLogy);

  // get relevant hist
  auto & hist = (fSignalsOnly || fSkipData) ? HistMap[fPlotSignalVec.front()] : DataHist;

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
  Common::SaveAs(OutCanv,Form("%s_%s",outfiletext.Data(),(isLogy?"log":"lin")));
}

void TreePlotter::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  fOutFile->cd();
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));

  // give grand title
  fConfigPave->AddText("***** TreePlotter Config *****");

  // Add era info
  Common::AddEraInfoToPave(fConfigPave,fEra);

  // dump plot cut config first
  Common::AddTextFromInputConfig(fConfigPave,"TreePlotter Cut Config",fCutConfig);

  // dump plot config
  Common::AddTextFromInputConfig(fConfigPave,"Plot Config",fPlotConfig);

  // store last bits of info from misc
  Common::AddTextFromInputConfig(fConfigPave,"Miscellaneous Config",fMiscConfig); 

  // padding
  Common::AddPaddingToPave(fConfigPave,3);

  // save name of infile, redundant
  fConfigPave->AddText(Form("InFile name: %s",fInFileName.Data()));

  // dump in old config
  Common::AddTextFromInputPave(fConfigPave,fInFile);

  // save name of insignalfile, redundant
  fConfigPave->AddText(Form("InSignalFile name: %s",fInSignalFileName.Data()));

  // dump in old signal config
  Common::AddTextFromInputPave(fConfigPave,fInSignalFile);
  
  // save to output file
  Common::Write(fOutFile,fConfigPave);
}

TString TreePlotter::DumpIntegrals(const TString & outfiletext)
{
  std::cout << "Dumping integrals into text file..." << std::endl;

  // make dumpfile object
  const TString filename = outfiletext+Common::outIntText+"."+Common::outTextExt; 
  std::ofstream dumpfile(Form("%s",filename.Data()),std::ios_base::out);

  // Signal MC zeroth
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const auto & hist = HistPair.second;

    if (Common::GroupMap[sample] == SampleGroup::isSignal)
    {
      auto sign_err = 0.;
      const auto sign_int = hist->IntegralAndError(1,hist->GetXaxis()->GetNbins(),sign_err,(fXVarBins?"width":""));
      dumpfile << hist->GetName() << " : " << sign_int << " +/- " << sign_err << std::endl;
    }
  }
  dumpfile << "-------------------------------------" << std::endl;

  // Individual MC first
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const auto & hist = HistPair.second;

    if (Common::GroupMap[sample] == SampleGroup::isBkgd)
    {
      auto bkgd_err = 0.0;
      const auto bkgd_int = hist->IntegralAndError(1,hist->GetXaxis()->GetNbins(),bkgd_err,(fXVarBins?"width":""));
      dumpfile << hist->GetName() << " : " << bkgd_int << " +/- " << bkgd_err << std::endl;
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
  
  // Report ratio prior to scaling
  if (fScaleMCToData)
  {
    dumpfile << "kFactor (Pre-Scaling) : " << fkFactor << " +/- " << fkFactorErr << std::endl;
  }

  return filename;
}

void TreePlotter::DeleteMemory(const Bool_t deleteInternal)
{
  std::cout << "Deleting memory in TreePlotter... deleting internal: " << Common::PrintBool(deleteInternal).Data() << std::endl;

  // delete everything
  if (fSaveMetaData) delete fConfigPave;

  delete LowerPad;
  delete UpperPad;
  delete OutCanv;
  delete Legend;
  delete RatioLine;
  delete RatioMCErrs;
  delete RatioHist;
  delete BkgdStack;
  delete EWKHist;
  delete BkgdHist;
  delete DataHist;

  Common::DeleteMap(HistMap);

  delete fOutFile;
  delete fTDRStyle;

  if (deleteInternal) 
  { 
    delete fInSignalFile;
    delete fInFile;
  }
}

void TreePlotter::ScaleMCToUnity()
{
  std::cout << "Scaling MC Bkgd to unity..." << std::endl;
  
  const auto mctotal = BkgdHist->Integral(fXVarBins?"width":"");

  // first scale each individual sample
  for (auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    auto & hist = HistPair.second;

    if (Common::GroupMap[sample] == SampleGroup::isBkgd)
    {
      hist->Scale(1.f/mctotal);
    }
  }

  // scale total
  BkgdHist->Scale(1.f/mctotal);
}

void TreePlotter::ScaleMCToData()
{
  std::cout << "Scaling MC Bkgd to data..." << std::endl;

  // Get MC Integral + Error
  auto mc_err = 0.0;
  const auto mc_int = BkgdHist->IntegralAndError(1,BkgdHist->GetXaxis()->GetNbins(),mc_err,(fXVarBins?"width":""));

  // Get Data Integral + Error
  auto data_err = 0.0;
  const auto data_int = DataHist->IntegralAndError(1,DataHist->GetXaxis()->GetNbins(),data_err,(fXVarBins?"width":""));

  // Make Ratio
  fkFactor = data_int/mc_int;
  fkFactorErr = fkFactor*std::sqrt(std::pow(data_err/data_int,2)+std::pow(mc_err/mc_int,2));

  // first scale each individual sample
  for (auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    auto & hist = HistPair.second;

    if (Common::GroupMap[sample] == SampleGroup::isBkgd)
    {
      hist->Scale(fkFactor);
    }
  }

  // lastly, scale combo bkgd hist
  BkgdHist->Scale(fkFactor);
}

void TreePlotter::GetHistMinimum()
{
  // initialize to dummy value
  fMinY = 1e9;

  // need to loop through to check bin != 0
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const auto & hist = HistPair.second;

    // skip data and check afterwards
    if (Common::GroupMap[sample] == SampleGroup::isData) continue;

    // skip samples not plotted
    if (Common::GroupMap[sample] == SampleGroup::isSignal)
    {
      if (std::find(fPlotSignalVec.begin(),fPlotSignalVec.end(),sample) == fPlotSignalVec.end()) continue;
    }

    // get and set tmp min
    for (auto bin = 1; bin <= hist->GetNbinsX(); bin++)
    {
      const auto tmpmin = hist->GetBinContent(bin);
      if ((tmpmin < fMinY) && (tmpmin > 0)) fMinY = tmpmin;
    }
  }

  // now check data --> safely
  if (!fSignalsOnly && !fSkipData)
  {
    for (auto bin = 1; bin <= DataHist->GetNbinsX(); bin++)
    {
      const auto tmpmin = DataHist->GetBinContent(bin);
      if ((tmpmin < fMinY) && (tmpmin > 0)) fMinY = tmpmin;
    }
  }
}

void TreePlotter::GetHistMaximum()
{
  // initialize to dummy value
  fMaxY = -1e9;

  // check signals first
  for (const auto & sample : fPlotSignalVec)
  {
    const auto & hist = HistMap[sample];

    // get tmp max
    const auto tmpmax = hist->GetBinContent(hist->GetMaximumBin());

    if (tmpmax > fMaxY) fMaxY = tmpmax;
  }

  // for safety
  if (!fSignalsOnly)
  {
    // check combined background
    const auto bkgdmax = BkgdHist->GetBinContent(BkgdHist->GetMaximumBin());
    if (bkgdmax > fMaxY) fMaxY = bkgdmax;
    
    if (!fSkipData)
    {
      // check final data (scaled and/or blinded)
      const auto datamax = DataHist->GetBinContent(DataHist->GetMaximumBin());
      if (datamax > fMaxY) fMaxY = datamax;
    }
  }
}

void TreePlotter::SetupDefaults()
{
  fIsLogX   = false;
  fXVarBins = false;
  fIsLogY   = false;

  fScaleToUnity = false;
  fScaleMCToData = false;
  fBlindData = false;
  fSkipData = false;
  fSignalsOnly = false;
  
  fSaveMetaData = false;
}

void TreePlotter::SetupSaveMetaData(const Bool_t savemetadata)
{
  fSaveMetaData = savemetadata;
}

void TreePlotter::SetupCommon()
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupSignalGroups();
  Common::SetupSignalSubGroups();
  Common::SetupTreeNames();
  Common::SetupHistNames();
  Common::SetupSignalSubGroupColors();
  Common::SetupColors();
  Common::SetupLabels();
  Common::SetupCuts(fCutConfig);
  Common::SetupEraCuts(fEra);
  Common::SetupWeights();
  Common::SetupEraWeights(fEra);
}

void TreePlotter::SetupPlotConfig(const TString & plotconfig)
{
  std::cout << "Reading plot config..." << std::endl;

  std::ifstream infile(Form("%s",plotconfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("plot_title=") != std::string::npos)
    {
      fTitle = Common::RemoveDelim(str,"plot_title=");
    }
    else if (str.find("x_title=") != std::string::npos)
    {
      fXTitle = Common::RemoveDelim(str,"x_title=");
    }
    else if (str.find("x_scale=") != std::string::npos)
    {
      Common::SetupScale(str,fIsLogX);
    }
    else if (str.find("x_var=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_var=");
      Common::SetVar(str,Variable::X);
    }
    else if (str.find("x_var_data=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_var_data=");
      Common::SetVarMod(str,Variable::X,SampleGroup::isData);
    }
    else if (str.find("x_var_bkgd=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_var_bkgd=");
      Common::SetVarMod(str,Variable::X,SampleGroup::isBkgd);
    }
    else if (str.find("x_var_sign=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_var_sign=");
      Common::SetVarMod(str,Variable::X,SampleGroup::isSignal);
    }
    else if (str.find("x_bins=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_bins=");
      Common::SetupBins(str,fXBins,fXVarBins);
    }
    else if (str.find("x_labels=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_labels=");
      Common::SetupBinLabels(str,fXLabels);
    }
    else if (str.find("y_title=") != std::string::npos)
    {
      fYTitle = Common::RemoveDelim(str,"y_title=");
    }
    else if (str.find("y_scale=") != std::string::npos)
    {
      Common::SetupScale(str,fIsLogY);
    }
    else if (str.find("blinding=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"blinding=");
      Common::SetupBlockRanges(str,fBlinds);
    }
    else 
    {
      std::cerr << "Aye... your plot config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void TreePlotter::SetupMiscConfig(const TString & miscconfig)
{
  std::cout << "Reading miscellaneous plot config..." << std::endl;

  std::ifstream infile(Form("%s",miscconfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("signal_to_model=") != std::string::npos) 
    {
      std::cout << "signal_to_model not implemented in plotter, skipping..." << std::endl;
    }
    else if (str.find("signals_to_plot=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"signals_to_plot=");
      Common::SetupWhichSignals(str,fPlotSignalVec);
    }
    else if (str.find("scale_mc_to_data=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"scale_mc_to_data=");
      Common::SetupBool(str,fScaleMCToData);
    }
    else if (str.find("scale_to_unity=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"scale_to_unity=");
      Common::SetupBool(str,fScaleToUnity);
    }
    else if (str.find("blind_data=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"blind_data=");
      Common::SetupBool(str,fBlindData);
    }
    else if (str.find("skip_data=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"skip_data=");
      Common::SetupBool(str,fSkipData);
    }
    else if (str.find("signals_only=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"signals_only=");
      Common::SetupBool(str,fSignalsOnly);
    }
    else 
    {
      std::cerr << "Aye... your miscellaneous plot config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void TreePlotter::SetupHists()
{
  std::cout << "Setting up output hists..." << std::endl;

  // instantiate each histogram
  for (const auto & HistNamePair : Common::HistNameMap)
  {
    HistMap[HistNamePair.first] = TreePlotter::SetupHist(HistNamePair.second);
  }
}

void TreePlotter::SetupHistsStyle()
{
  std::cout << "Setting up output hists styles..." << std::endl;

  for (auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const auto & hist   = HistPair.second;
    
    hist->SetLineColor(Common::ColorMap[sample]);
    hist->SetMarkerColor(Common::ColorMap[sample]);
    if (Common::GroupMap[sample] == SampleGroup::isBkgd)
    {
      hist->SetFillColor(Common::ColorMap[sample]);
      hist->SetFillStyle(1001);
    }
    else if (Common::GroupMap[sample] == SampleGroup::isSignal)
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
