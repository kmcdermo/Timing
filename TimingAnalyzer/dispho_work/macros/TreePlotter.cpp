#include "TreePlotter.hh"
#include "TVector2.h"

TreePlotter::TreePlotter(const TString & cutconfig, const TString & plotconfig, const TString & outfiletext) 
  : fCutConfig(cutconfig), fPlotConfig(plotconfig), fOutFileText(outfiletext)
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

    // relevant variables
    float e0,pt0,phi0,eta0;
    TBranch * b_e0, * b_pt0, * b_phi0, * b_eta0;
    int gedid0,ootid0;
    TBranch * b_gedid0, * b_ootid0;
    bool isoot0;
    TBranch * b_isoot0;

    float e1,pt1,phi1,eta1;
    TBranch * b_e1, * b_pt1, * b_phi1, * b_eta1;
    int gedid1,ootid1;
    TBranch * b_gedid1, * b_ootid1;
    bool isoot1;
    TBranch * b_isoot1;

    float jphi0,jeta0;
    TBranch * b_jphi0, * b_jeta0;

    int njets;
    TBranch * b_njets;

    int nvtx;
    TBranch * b_nvtx;

    bool hlt;
    TBranch * b_hlt;
    
    float evtwgt;
    TBranch * b_evtwgt;

    tree->SetBranchAddress("phoE_0",&e0,&b_e0);
    tree->SetBranchAddress("phopt_0",&pt0,&b_pt0);
    tree->SetBranchAddress("phophi_0",&phi0,&b_phi0);
    tree->SetBranchAddress("phoeta_0",&eta0,&b_eta0);
    tree->SetBranchAddress("phogedID_0",&gedid0,&b_gedid0);
    tree->SetBranchAddress("phoootID_0",&ootid0,&b_ootid0);
    tree->SetBranchAddress("phoisOOT_0",&isoot0,&b_isoot0);

    tree->SetBranchAddress("phoE_1",&e1,&b_e1);
    tree->SetBranchAddress("phopt_1",&pt1,&b_pt1);
    tree->SetBranchAddress("phophi_1",&phi1,&b_phi1);
    tree->SetBranchAddress("phoeta_1",&eta1,&b_eta1);
    tree->SetBranchAddress("phogedID_1",&gedid1,&b_gedid1);
    tree->SetBranchAddress("phoootID_1",&ootid1,&b_ootid1);
    tree->SetBranchAddress("phoisOOT_1",&isoot1,&b_isoot1);

    tree->SetBranchAddress("jetphi_0",&jphi0,&b_jphi0);
    tree->SetBranchAddress("jeteta_0",&jeta0,&b_jeta0);

    tree->SetBranchAddress("njetsidL",&njets,&b_njets);
    tree->SetBranchAddress("nvtx",&nvtx,&b_nvtx);
    tree->SetBranchAddress("hltDiPho3022M90",&hlt,&b_hlt);

    const bool isMC = (Config::GroupMap[sample] != isData);

    std::vector<float> puweights;
    if (isMC)
    { 
      tree->SetBranchAddress("evtwgt",&evtwgt,&b_evtwgt);

      // Get puweights file for now
      const TString pufilename = Config::puwgtFileName+".root";
      TFile * pufile = TFile::Open(Form("%s",pufilename.Data()));
      Config::CheckValidFile(pufile,pufilename);
      TH1F * puhist = (TH1F*)pufile->Get(Config::puwgtHistName.Data());
      Config::CheckValidTH1F(puhist,Config::puwgtHistName,pufilename);

      puweights.clear();
      for (Int_t ibin = 1; ibin <= puhist->GetNbinsX(); ibin++)
      {
	puweights.emplace_back(puhist->GetBinContent(ibin));
      }

      delete puhist;
      delete pufile;
    }

    // Make temp hist
    TString histname = Config::ReplaceSlashWithUnderscore(input);
    TH1F * hist = TreePlotter::SetupHist(Form("%s_Hist",histname.Data()));

    const UInt_t entries = tree->GetEntries();
    for (UInt_t ientry = 0; ientry < entries; ientry++)
    {
      if (ientry % 10000 == 0 || ientry == 0) std::cout << ientry << " out of " << entries << std::endl;

      b_hlt->GetEntry(ientry);
      if (!hlt) continue;

      b_njets->GetEntry(ientry);
      if (njets>=3) continue;
      
      b_pt0->GetEntry(ientry);
      if (pt0<=70) continue;

      b_isoot0->GetEntry(ientry);
      b_gedid0->GetEntry(ientry);
      b_ootid0->GetEntry(ientry);
      if (!isoot0 && gedid0<3) continue;
      if ( isoot0 && ootid0<3) continue;

      b_pt1->GetEntry(ientry);
      if (pt1<=40) continue;

      b_isoot1->GetEntry(ientry);
      b_gedid1->GetEntry(ientry);
      b_ootid1->GetEntry(ientry);
      if (!isoot1 && gedid1<1) continue;
      if ( isoot1 && ootid1<1) continue;
      
      b_e0->GetEntry(ientry);
      b_phi0->GetEntry(ientry);
      b_eta0->GetEntry(ientry);
      b_e1->GetEntry(ientry);
      b_phi1->GetEntry(ientry);
      b_eta1->GetEntry(ientry);
      if (std::sqrt(std::pow(e0+e1,2)
    		    -std::pow(pt0*std::cos (phi0)+pt1*std::cos (phi1),2)
    		    -std::pow(pt0*std::sin (phi0)+pt1*std::sin (phi1),2)
    		    -std::pow(pt0*std::sinh(eta0)+pt1*std::sinh(eta1),2)
    		    )<=90) continue;

      b_jphi0->GetEntry(ientry);
      b_jeta0->GetEntry(ientry);
      if (std::sqrt(std::pow(TVector2::Phi_mpi_pi(jphi0-phi0),2)+std::pow(jeta0-eta0,2))<=2.1) continue;

      b_nvtx->GetEntry(ientry);
      
      float weight = 1.0;
      if (isMC)
      {
    	b_evtwgt->GetEntry(ientry);
    	weight = evtwgt * puweights[nvtx-1];
    	if (sample == DYLL)
    	{
    	  if      (input.Contains("base")) weight *= 0.4982;
    	  else if (input.Contains("ext"))  weight *= 0.5018;
    	  else    
    	  {
    	    std::cerr << "Somehow you specified a DYLL sample that does not exist: " << input.Data() << " ...exiting..." << std::endl;
    	    exit(1);
    	  }
    	}
      }
      
      hist->Fill(nvtx,weight);
    }

    // Fill from tree
    //    tree->Draw(Form("%s>>%s",fXVar.Data(),hist->GetName()),Form("(%s) * (%s)",Config::CutMap[sample].Data(),Config::WeightString(input,sample).Data()),"goff");
    
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

  OutCanv->cd(); // Go back to the main canvas before saving
  Config::CMSLumi(OutCanv,0); // write out Lumi info
  OutCanv->SaveAs(Form("%s.png",fOutFileText.Data()));

  // save to output file
  fOutFile->cd();
  OutCanv->Write(OutCanv->GetName(),TObject::kWriteDelete);
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

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
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
      Config::SetupBins(str,fXBins);
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
