// Class include
#include "TnPPlotter.hh"

TnPPlotter::TnPPlotter(const TString & infilename, const Bool_t savemetadata, const TString & outfiletext)
  : fInFileName(infilename), fSaveMetaData(savemetadata), fOutFileText(outfiletext)
{
  std::cout << "Initializing TnPPlotter..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // init configuration
  TnPPlotter::SetupCommon();

  // open input skim file
  fInFile = TFile::Open(Form("%s",fInFileName.Data()),"UPDATE");
  Common::CheckValidFile(fInFile,fInFileName);

  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(fTDRStyle);

  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");

  // Setup Functions
  TnPPlotter::SetupCommon();
  TnPPlotter::SetupStrVecs();
  TnPPlotter::SetupOutputEffs();
}

TnPPlotter::~TnPPlotter()
{
  std::cout << "Tidying up in the destructor..." << std::endl;

  Common::DeleteMap(LowerPadMap);
  Common::DeleteMap(UpperPadMap);
  Common::DeleteMap(OutCanvMap);
  Common::DeleteMap(RatioLineMap);
  Common::DeleteMap(HistMap);
  Common::DeleteMap(GraphMap);
  Common::DeleteMap(EffMap);

  delete fOutFile;
  delete fTDRStyle;
  delete fInFile;
}

void TnPPlotter::MakeTnPPlots()
{
  std::cout << "Making efficiency plots..." << std::endl;

  // loop over trees and fill effs
  TnPPlotter::EventLoop();

  // Make Graphs
  TnPPlotter::MakeGraph();

  // Make Ratio Output
  TnPPlotter::MakeRatioOutput();

  // Get Graph min/max : set it for lines and hists
  TnPPlotter::GetGraphMinMax();

  // Make Legend
  TnPPlotter::MakeLegend();

  // Init Output Canv+Pads
  TnPPlotter::InitOutputCanvPads();

  // Draw Upper Pad
  TnPPlotter::DrawUpperPad();

  // Draw Lower Pad
  TnPPlotter::DrawLowerPad();

  // Save Output
  TnPPlotter::SaveOutput();

  // Write Out Config
  if (fSaveMetaData) TnPPlotter::MakeConfigPave();
}

void TnPPlotter::EventLoop()
{
  std::cout << "Looping over trees..." << std::endl;

  /////////////////////
  // Loop over Trees //
  /////////////////////

  for (const auto & TreeNamePair : Common::TreeNameMap)
  {
    // Init
    const auto & sample   = TreeNamePair.first;
    const auto & treename = TreeNamePair.second;
    
    std::cout << "Working on tree: " << treename.Data() << std::endl;

    // data vs mc
    const Bool_t isMC = (Common::GroupMap[sample] != isData);
    const TString sample_label = (isMC ? "MC" : "Data");
	
    // Get tree
    fInFile->cd();
    auto intree = (TTree*)fInFile->Get(Form("%s",treename.Data()));
    const auto isnull = Common::IsNullTree(intree);

    if (!isnull)
    {
      //////////////////////////
      // Initialize variables //
      //////////////////////////

      // weights
      Float_t evtwgt; TBranch * b_evtwgt; intree->SetBranchAddress("evtwgt",&evtwgt,&b_evtwgt);
      Float_t puwgt; TBranch * b_puwgt; if (isMC) intree->SetBranchAddress("puwgt",&puwgt,&b_puwgt);

      // HLT 
      Bool_t hltDiEle33MW; TBranch * b_hltDiEle33MW; intree->SetBranchAddress("hltDiEle33MW",&hltDiEle33MW,&b_hltDiEle33MW);

      ///////////////////////
      // Setup TnP Structs //
      ///////////////////////

      TnPStruct leadingEl; // leading electron can be tag
      intree->SetBranchAddress("phohasPixSeed_0",&leadingEl.phohasPixSeed_tag,&leadingEl.b_phohasPixSeed_tag);
      intree->SetBranchAddress("phoisEB_1"      ,&leadingEl.phoisEB_probe    ,&leadingEl.b_phoisEB_probe);
      intree->SetBranchAddress("phoisTrk_1"     ,&leadingEl.phoisTrk_probe   ,&leadingEl.b_phoisTrk_probe);
      intree->SetBranchAddress("phopt_1"        ,&leadingEl.phopt_probe      ,&leadingEl.b_phopt_probe);

      TnPStruct subleadingEl; // subleading electron can be tag
      intree->SetBranchAddress("phohasPixSeed_1",&subleadingEl.phohasPixSeed_tag,&subleadingEl.b_phohasPixSeed_tag);
      intree->SetBranchAddress("phoisEB_0"      ,&subleadingEl.phoisEB_probe    ,&subleadingEl.b_phoisEB_probe);
      intree->SetBranchAddress("phoisTrk_0"     ,&subleadingEl.phoisTrk_probe   ,&subleadingEl.b_phoisTrk_probe);
      intree->SetBranchAddress("phopt_0"        ,&subleadingEl.phopt_probe      ,&subleadingEl.b_phopt_probe);

      ///////////////////////////////
      // Loop over entries in tree //
      ///////////////////////////////

      const auto nEntries = intree->GetEntries();
      for (auto entry = 0U; entry < nEntries; entry++)
      {
	// dump status check
	if (entry%Common::nEvCheck == 0 || entry == 0) std::cout << "Processing Entry: " << entry << " out of " << nEntries << std::endl;
	
	// require hlt
	b_hltDiEle33MW->GetEntry(entry);
	if (!hltDiEle33MW) continue;

	// require pt cuts (probes on opposite leg)
	subleadingEl.b_phopt_probe->GetEntry(entry);
	if (subleadingEl.phopt_probe < 40.f) continue;
	leadingEl   .b_phopt_probe->GetEntry(entry);
	if (leadingEl.phopt_probe < 40.f) continue;

	// get weights
	b_evtwgt->GetEntry(entry);
	if (isMC) b_puwgt->GetEntry(entry);

	// Fill plots
	TnPPlotter::FillTnP(leadingEl   ,entry,sample_label,(isMC?evtwgt*puwgt:evtwgt));
	TnPPlotter::FillTnP(subleadingEl,entry,sample_label,(isMC?evtwgt*puwgt:evtwgt));
      } // end loop over entries
      
      // delete it all
      delete intree;

    } // tree is valid
    else
    {
      std::cout << "Skipping null tree..." << std::endl;
    } // isnull tree
  } // end loop over tree names
  
  // save totals to output file
  Common::WriteMap(fOutFile,EffMap);
}

void TnPPlotter::FillTnP(TnPStruct & info, const UInt_t entry, const TString & sample_label, const Float_t wgt)
{
  // require tag
  info.b_phohasPixSeed_tag->GetEntry(entry);
  if (info.phohasPixSeed_tag)
  {
    // get remaining pieces (pts already loaded if cutting on them, but redo it anyway)
    info.b_phoisEB_probe ->GetEntry(entry);
    info.b_phoisTrk_probe->GetEntry(entry);
    info.b_phopt_probe   ->GetEntry(entry);
  
    // eta label
    const TString eta_label = (info.phoisEB_probe ? "EB" : "EE");
  
    // fill tefficiency
    EffMap[sample_label+"_"+eta_label]->FillWeighted(info.phoisTrk_probe,wgt,info.phopt_probe);
  }
}

void TnPPlotter::MakeGraph()
{
  std::cout << "Make graphs..." << std::endl;

  for (const auto & sample : fSamples)
  {
    for (const auto & eta : fEtas)
    {
      const TString label = sample+"_"+eta;
      const auto & eff = EffMap[label];
      auto & graph = GraphMap[label];

      graph = eff->CreateGraph();
      graph->SetName(label+"_Graph");
    }
  }
}

void TnPPlotter::MakeRatioOutput()
{
  std::cout << "Make ratio outputs..." << std::endl;

  // make input hists for ratio
  for (const auto & sample : fSamples)
  {
    for (const auto & eta : fEtas)
    {
      const TString label = sample+"_"+eta;
      const auto & eff = EffMap[label];
      auto & hist = HistMap[label];

      for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
      { 
	const auto val     = eff->GetEfficiency(ibinX);
	const auto err_low = eff->GetEfficiencyErrorLow(ibinX);
	const auto err_up  = eff->GetEfficiencyErrorUp (ibinX);
	const auto err     = ((err_low > err_up) ? err_low : err_up);

	hist->SetBinContent(ibinX,val);
	hist->SetBinError  (ibinX,err);
      }
    }
  }
  
  // make ratio output hists and lines
  for (const auto & eta : fEtas)
  {
    const auto & data_hist = HistMap["Data_"+eta];
    const auto & mc_hist   = HistMap["MC_"  +eta];
    
    // output ratio hist
    auto & ratio_hist = HistMap[eta];
    ratio_hist->Add(data_hist);
    ratio_hist->Divide(mc_hist);

    ratio_hist->SetMinimum( 0.98); // Define Y ..
    ratio_hist->SetMaximum( 1.02); // .. range
    
    // ratio line
    auto & ratioline = RatioLineMap[eta];
    ratioline = new TLine();
    ratioline->SetLineColor(kRed);
    ratioline->SetLineWidth(2);

    // set y pos, x comes later
    ratioline->SetY1(1.0);
    ratioline->SetY2(1.0);
    
    // save to output
    Common::Write(fOutFile,ratio_hist);
  }
}

void TnPPlotter::GetGraphMinMax()
{
  std::cout << "Getting graph mins and maxes..." << std::endl;

  for (const auto & eta : fEtas)
  {
    auto & data_graph = GraphMap["Data_"+eta];
    auto & mc_graph   = GraphMap["MC_"  +eta];
    auto & minmax     = fMinMaxMap[eta];

    // minmax.xmin = ((data_graph->GetXaxis()->GetXmin() < mc_graph->GetXaxis()->GetXmin()) ? data_graph->GetXaxis()->GetXmin() : mc_graph->GetXaxis()->GetXmin());
    // minmax.xmax = ((data_graph->GetXaxis()->GetXmax() > mc_graph->GetXaxis()->GetXmax()) ? data_graph->GetXaxis()->GetXmax() : mc_graph->GetXaxis()->GetXmax());
    // minmax.ymin = ((data_graph->GetYaxis()->GetXmin() < mc_graph->GetYaxis()->GetXmin()) ? data_graph->GetYaxis()->GetXmin() : mc_graph->GetYaxis()->GetXmin());
    // minmax.ymax = ((data_graph->GetYaxis()->GetXmax() > mc_graph->GetYaxis()->GetXmax()) ? data_graph->GetYaxis()->GetXmax() : mc_graph->GetYaxis()->GetXmax());
    minmax.xmin = 0.f;
    minmax.xmax = 1000.f;
    minmax.ymin = 0.97f;
    minmax.ymax = 1.02f;
  }
}

void TnPPlotter::MakeLegend()
{
  std::cout << "Making legends..." << std::endl;

  // make output hists
  fOutFile->cd();
  for (const auto & eta : fEtas)
  {
    const auto & data_graph = GraphMap["Data_"+eta];
    const auto & mc_graph   = GraphMap["MC_"  +eta];

    // make legend
    auto & leg = LegendMap[eta];
    leg = new TLegend(0.72,0.755,0.82,0.925);
    leg->SetName("Legend_"+eta);
    leg->SetBorderSize(1);
    leg->SetLineColor(kBlack);
    
    // add to legend
    leg->AddEntry(data_graph,"Data","epl");
    leg->AddEntry(mc_graph  ,"MC"  ,"epl");

    // save to output
    Common::Write(fOutFile,leg);
  }
}

void TnPPlotter::InitOutputCanvPads()
{
  std::cout << "Initializing canvas and pads..." << std::endl;

  for (const auto & eta : fEtas)
  {
    auto & canv = OutCanvMap[eta];

    OutCanvMap[eta] = new TCanvas("OutCanv_"+eta,"");
    OutCanvMap[eta]->cd();

    UpperPadMap[eta] = new TPad("UpperPad_"+eta,"", Common::left_up, Common::bottom_up, Common::right_up, Common::top_up);
    UpperPadMap[eta]->SetBottomMargin(Common::merged_margin);
    
    LowerPadMap[eta] = new TPad("LowerPad_"+eta, "", Common::left_lp, Common::bottom_lp, Common::right_lp, Common::top_lp);
    LowerPadMap[eta]->SetTopMargin(Common::merged_margin);
    LowerPadMap[eta]->SetBottomMargin(Common::bottom_margin);
  }
}

void TnPPlotter::DrawUpperPad()
{
  std::cout << "Drawing upper pad..." << std::endl;

  for (const auto & eta : fEtas)
  {
    // Pad Gymnastics
    OutCanvMap[eta]->cd();
    UpperPadMap[eta]->Draw();
    UpperPadMap[eta]->cd();

    // get inputs
    const auto & data_graph = GraphMap["Data_"+eta];
    const auto & mc_graph = GraphMap["MC_"+eta];
    const auto & minmax = fMinMaxMap[eta];

    // draw
    data_graph->Draw("AP");
    mc_graph->Draw("P SAME");

    // once grapahs are drawn, have axis objects to use: have to scale TDR style values by height of upper pad
    data_graph->GetXaxis()->SetLimits(minmax.xmin,minmax.xmax);
    data_graph->Draw("P SAME");
    UpperPadMap[eta]->Update();
    
    data_graph->GetXaxis()->SetLabelSize(0);
    data_graph->GetXaxis()->SetTitleSize(0);

    data_graph->GetYaxis()->SetLabelSize  (Common::LabelSize / Common::height_up); 
    data_graph->GetYaxis()->SetTitleSize  (Common::TitleSize / Common::height_up);
    data_graph->GetYaxis()->SetTitleOffset(Common::TitleFF * Common::TitleYOffset * Common::height_up);

    // And lastly draw the legend
    LegendMap[eta]->Draw("SAME"); 
  }
}

void TnPPlotter::DrawLowerPad()
{  
  std::cout << "Drawing lower pad..." << std::endl;

  for (const auto & eta : fEtas)
  {
    // Pad gymnastics
    OutCanvMap[eta]->cd(); 
    LowerPadMap[eta]->Draw();
    LowerPadMap[eta]->cd();

    // set ratio hist, line x-size
    auto & minmax    = fMinMaxMap[eta];
    auto & ratiohist = HistMap[eta];
    auto & ratioline = RatioLineMap[eta];

    ratiohist->GetXaxis()->SetRangeUser(minmax.xmin,minmax.xmax);

    ratioline->SetX1(minmax.xmin);
    ratioline->SetX2(minmax.xmax);
    
    // save to output (lines do not have name function)
    fOutFile->cd();
    ratioline->Write("RatioLine_"+eta,TObject::kWriteDelete);

    // draw th1 first so line can appear, then draw over it (and set Y axis divisions)
    HistMap[eta]->Draw("EP"); 

    // set params for line then draw
    RatioLineMap[eta]->Draw("SAME");

    // redraw to go over line
    HistMap[eta]->Draw("EP SAME");
  }
}

void TnPPlotter::SaveOutput()
{
  std::cout << "Saving hist as images..." << std::endl;

  for (const auto & eta : fEtas)
  {
    auto & outcanv = OutCanvMap[eta];

    // Go back to the main canvas before saving and write out lumi info
    outcanv->cd();
    Common::CMSLumi(outcanv,0,"Full");
    
    // Save a log version first
    TnPPlotter::PrintCanvas(eta,true);
    
    // Save a linear version second
    TnPPlotter::PrintCanvas(eta,false);

    // save to output file
    fOutFile->cd();
    outcanv->Write(outcanv->GetName(),TObject::kWriteDelete);
  }
}

void TnPPlotter::PrintCanvas(const TString & eta, const Bool_t isLogy)
{
  // get canvas
  auto & outcanv = OutCanvMap[eta];

  // pad gymnastics
  outcanv->cd();
  UpperPadMap[eta]->cd();
  UpperPadMap[eta]->SetLogy(isLogy);

  // get relevant graph and min+max
  auto & graph = GraphMap["Data_"+eta];
  auto & minmax = fMinMaxMap[eta];

  // get tmp min/max
  const auto min = minmax.ymin;
  const auto max = minmax.ymax;

  // set min and max
  // if (isLogy)
  // {
  //   graph->GetHistogram()->SetMinimum(min/1.5);
  //   graph->GetHistogram()->SetMaximum(max*1.5);
  // }
  // else 
  // {
  //   graph->GetHistogram()->SetMinimum( min > 0 ? min/1.05 : min*1.05 );
  //   graph->GetHistogram()->SetMaximum( max > 0 ? max*1.05 : max/1.05 );      
  // }

  graph->GetHistogram()->SetMinimum(min);
  graph->GetHistogram()->SetMaximum(max);

  // save canvas as images
  Common::SaveAs(outcanv,Form("%s_%s_%s",fOutFileText.Data(),eta.Data(),(isLogy?"log":"lin")));
}

void TnPPlotter::MakeConfigPave()
{
  std::cout << "Dumping config to a pave" << std::endl;

  // create the pave, copying in old info
  fOutFile->cd();
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));

  // add some padding to new stuff
  Common::AddPaddingToPave(fConfigPave,3);

  // give grand title
  fConfigPave->AddText("***** TnPPlotter Config *****");

  // dump in old configs... will be a lot
  fConfigPave->AddText(Form("InFileName: %s",fInFile->GetName())); 
  Common::AddTextFromInputPave(fConfigPave,fInFile);

  // save to output file
  Common::Write(fOutFile,fConfigPave);

  // delete pave
  delete fConfigPave;
}

void TnPPlotter::SetupCommon()
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupGroups();
  Common::SetupTreeNames();
}

void TnPPlotter::SetupStrVecs()
{
  fSamples = {"Data","MC"};
  fEtas = {"EB","EE"};
}

void TnPPlotter::SetupOutputEffs()
{
  std::cout << "Setup Output Efficiencies..." << std::endl;

  const TString x_title = "Probe p_{T} [GeV]";
  const TString y_title = "Efficiency of isTrk";
  const TString title = y_title+" vs. "+x_title;
  const std::vector<Double_t> xBins = {0,50,100,250,500,1000};
  const Double_t * x_bins = &xBins[0];

  std::map<TString,Color_t> colorMap;
  colorMap["Data"] = kBlue;
  colorMap["MC"] = kRed;

  // make raw efficiencies
  for (const auto & sample : fSamples)
  {
    for (const auto & eta : fEtas)
    {
      const TString label = sample+"_"+eta;
      const TString title_label = "("+sample+" "+eta+")";

      auto & eff = EffMap[label];
      eff = new TEfficiency(label+"_Eff",title+" "+title_label+";"+x_title+" "+title_label+";"+y_title,xBins.size()-1,x_bins);
      eff->SetLineColor  (colorMap[sample]);
      eff->SetMarkerColor(colorMap[sample]);
      eff->SetUseWeightedEvents();

      auto & hist = HistMap[label];
      hist = new TH1F(label+"_Hist",title+" "+x_title+" "+title_label,xBins.size()-1,x_bins);
      hist->GetXaxis()->SetTitle(x_title+" "+title_label);
      hist->GetYaxis()->SetTitle(y_title);
      hist->Sumw2();
    }
  }

  // make ratios
  for (const auto & eta : fEtas)
  {
    auto & hist = HistMap[eta];
    hist = new TH1F(eta+"_Hist","Ratio of "+title+" ("+eta+")",xBins.size()-1,x_bins);
    hist->GetXaxis()->SetTitle(x_title+" ("+eta+")");
    hist->GetYaxis()->SetTitle("Data/MC");
    hist->SetLineColor(kBlack);
    hist->SetMarkerColor(kBlack);
    hist->Sumw2();

    // set other options (from TTreePlotter)
    hist->SetStats(0);      // No statistics on lower plot
    hist->GetYaxis()->SetNdivisions(505);
    hist->GetXaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
    hist->GetXaxis()->SetLabelOffset(Common::LabelOffset / Common::height_lp); 
    hist->GetXaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
    hist->GetXaxis()->SetTickLength (Common::TickLength  / Common::height_lp);
    hist->GetYaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
    hist->GetYaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
    hist->GetYaxis()->SetTitleOffset(Common::TitleFF * Common::TitleYOffset * Common::height_lp);
  }
}
