#include "../interface/StackDataMC.hh"

StackDataMC::StackDataMC() 
{
  // input data members
  for (TStrBoolMapIter iter = Config::SampleMap.begin(); iter != Config::SampleMap.end(); ++iter) 
  {
    if ((*iter).second) 
    { // isMC == true
      fMCNames.push_back((*iter).first);
    }
    else 
    { // data
      fDataNames.push_back((*iter).first);
    }
  }

  // store for later ... would rather have move semantics ... iterators too annoying
  fNData = fDataNames.size();
  fNMC   = fMCNames.size();

  // output data members
  fOutDir = Form("%s/datamcstacks",Config::outdir.Data()); // where to put output stack plots 
  MakeOutDir(fOutDir); // make output directory 
  fOutFile = new TFile(Form("%s/stackplots_canvases.root",fOutDir.Data()),"RECREATE"); // make output tfile --> store canvas images here too, for quick editting

  // Read in names of plots to be stacked
  StackDataMC::InitTH1FNamesAndSubDNames();
  
  // make stack outputs
  MakeSubDirs(fTH1FSubDMap,fOutDir);

  // with all that defined, initialize everything in constructor
  StackDataMC::OpenInputFiles();
  StackDataMC::InitInputPlots();
  StackDataMC::InitOutputPlots();
  StackDataMC::InitOutputLegends();
  StackDataMC::InitRatioPlots();
  StackDataMC::InitRatioLines();
  StackDataMC::InitOutputCanvPads();
}

StackDataMC::~StackDataMC()
{
  // delete all pointers
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    delete fOutRatioMCErrs[th1f];
    delete fOutRatioTH1FHists[th1f];
    delete fOutDataTH1FHists[th1f];
    delete fOutMCTH1FHists[th1f];
    delete fOutMCTH1FStacks[th1f];
    delete fTH1FLegends[th1f];
    delete fOutTH1FRatioLines[th1f];
    delete fOutTH1FStackPads[th1f];
    delete fOutTH1FRatioPads[th1f];
    delete fOutTH1FCanvases[th1f];
    
    for (Int_t data = 0; data < fNData; data++) 
    {
      delete fInDataTH1FHists[th1f][data];
    }
  
    for (Int_t mc = 0; mc < fNMC; mc++)
    {
      delete fInMCTH1FHists[th1f][mc];
    }
  }

  for (Int_t data = 0; data < fNData; data++) 
  {
    delete fDataFiles[data];
  }

  for (Int_t mc = 0; mc < fNMC; mc++) 
  {
    delete fMCFiles[mc];
  }

  delete fOutFile;
}

void StackDataMC::DoStacks(std::ofstream & yields) 
{
  StackDataMC::MakeStackDataMC(yields);
  StackDataMC::MakeRatioPlots();
  StackDataMC::MakeOutputCanvas();
}

void StackDataMC::MakeStackDataMC(std::ofstream & yields)
{
  // copy th1f plots into output hists/stacks
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++) //th1f hists
  {
    // data, copy + add only
    for (Int_t data = 0; data < fNData; data++) 
    {
      if (data == 0)
      {
	fOutDataTH1FHists[th1f] = (TH1F*)fInDataTH1FHists[th1f][data]->Clone();
      }
      else
      {
	fOutDataTH1FHists[th1f]->Add(fInDataTH1FHists[th1f][data]);
      }

      if (fTH1FNames[th1f].Contains("nvtx",TString::kExact)) // save individual yields for data using nvtx, as only the selection on this plot is event selection
      {
	yields << fDataNames[data].Data() << ": " << fInDataTH1FHists[th1f][data]->Integral() << std::endl;
      }
    } // end loop over data samples

    // add legend entry for total data
    fTH1FLegends[th1f]->AddEntry(fOutDataTH1FHists[th1f],"Data","epl"); // add data entry to legend

    // add total yield for data here if nvtx plot
    if (fTH1FNames[th1f].Contains("nvtx",TString::kExact)) // save total yields for data
    {
      yields << "--------------------" << std::endl;
      yields << "Data Total: " << fOutDataTH1FHists[th1f]->Integral() << std::endl << std::endl;
    }

    // mc, copy + add to hists
    for (Int_t mc = 0; mc < fNMC; mc++) 
    {
      if (mc == 0) // add first for ratio
      {
	fOutMCTH1FHists[th1f] = (TH1F*)fInMCTH1FHists[th1f][mc]->Clone();
      }
      else // add first for ratio
      { 
	fOutMCTH1FHists[th1f]->Add(fInMCTH1FHists[th1f][mc]);
      }

      fTH1FLegends[th1f]->AddEntry(fInMCTH1FHists[th1f][mc],Config::SampleTitleMap[fMCNames[mc]],"f");

      if (fTH1FNames[th1f].Contains("nvtx",TString::kExact))  // save individual contributions for yields for MC
      {
	yields << fMCNames[mc].Data() << ": " << fInMCTH1FHists[th1f][mc]->Integral() << std::endl;
      }
    } // end loop over mc samples

    // add stacks once renormalized
    for (Int_t mc = 0; mc < fNMC; mc++) 
    {
      fOutMCTH1FStacks[th1f]->Add(fInMCTH1FHists[th1f][mc]); // just add input to stacks
    }

    // will use the output MC added Hists for plotting uncertainties and add it to legend once
    fOutMCTH1FHists[th1f]->SetMarkerSize(0);
    fOutMCTH1FHists[th1f]->SetFillStyle(3254);
    fOutMCTH1FHists[th1f]->SetFillColor(kGray+3);

    fTH1FLegends[th1f]->AddEntry(fOutMCTH1FHists[th1f],"MC Unc.","f");
  
    // add total yield for MC here if nvtx plot
    if (fTH1FNames[th1f].Contains("nvtx",TString::kExact)) // save total yields for MC
    {
      yields << "--------------------" << std::endl;
      yields << "MC Total: " << fOutMCTH1FHists[th1f]->Integral() << std::endl;
    }
  } // end loop over th1f plots
}

void StackDataMC::MakeRatioPlots()
{
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  { 
    // ratio value plot
    fOutRatioTH1FHists[th1f] = (TH1F*)fOutDataTH1FHists[th1f]->Clone();
    fOutRatioTH1FHists[th1f]->Divide(fOutMCTH1FHists[th1f]);  
    fOutRatioTH1FHists[th1f]->GetYaxis()->SetTitle("Data/MC");
    fOutRatioTH1FHists[th1f]->SetMinimum(-1.5); // Define Y ..
    fOutRatioTH1FHists[th1f]->SetMaximum( 1.5); // .. range
    fOutRatioTH1FHists[th1f]->SetLineColor(kBlack);
    fOutRatioTH1FHists[th1f]->SetStats(0);      // No statistics on lower plot

    // ratio MC error plot
    fOutRatioMCErrs[th1f] = (TH1F*)fOutMCTH1FHists[th1f]->Clone();
    fOutRatioMCErrs[th1f]->Divide(fOutMCTH1FHists[th1f]);
  }
}

void StackDataMC::MakeOutputCanvas() 
{
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    // draw first with logy scale
    Bool_t isLogY = true;
    StackDataMC::DrawUpperPad(th1f,isLogY); // upper pad is stack
    StackDataMC::DrawLowerPad(th1f); // lower pad is ratio
    StackDataMC::SaveCanvas(th1f,isLogY);  // now save the canvas, w/ logy

    // draw second with lin scale
    isLogY = false;
    StackDataMC::DrawUpperPad(th1f,isLogY); // upper pad is stack
    StackDataMC::DrawLowerPad(th1f); // lower pad is ratio
    StackDataMC::SaveCanvas(th1f,isLogY); // now save the canvas, w/o logy
  }
}

void StackDataMC::DrawUpperPad(const Int_t th1f, const Bool_t isLogY) 
{    
  // pad gymnastics
  fOutTH1FCanvases[th1f]->cd();
  fOutTH1FStackPads[th1f]->Draw(); // draw upper pad   
  fOutTH1FStackPads[th1f]->cd(); // upper pad is current pad
  
  // set maximum by comparing added mc vs added data
  Float_t min = StackDataMC::GetMinimum(th1f);
  Float_t max = StackDataMC::GetMaximum(th1f);

  if (isLogY) // set min for log only... maybe consider min for linear eventually
  { 
    fOutDataTH1FHists[th1f]->SetMinimum(min/1.5);
    // set max with 2.0 scale to give enough space 
    fOutDataTH1FHists[th1f]->SetMaximum(max*1.5);
  }
  else 
  {
    fOutDataTH1FHists[th1f]->SetMaximum( max > 0 ? max*1.05 : max/1.05 );      
    fOutDataTH1FHists[th1f]->SetMinimum( min > 0 ? min/1.05 : min*1.05 );
  }
  
  // now draw the plots for upper pad in absurd order because ROOT is dumb
  fOutDataTH1FHists[th1f]->Draw("PE"); // draw first so labels appear

  // again, have to scale TDR style values by height of upper pad
  fOutDataTH1FHists[th1f]->GetYaxis()->SetLabelSize  (Config::LabelSize / Config::height_up); 
  fOutDataTH1FHists[th1f]->GetYaxis()->SetTitleSize  (Config::TitleSize / Config::height_up);
  fOutDataTH1FHists[th1f]->GetYaxis()->SetTitleOffset(Config::TitleFF * Config::TitleYOffset * Config::height_up);
  
  fOutMCTH1FStacks[th1f]->Draw("HIST SAME"); 
  fOutTH1FStackPads[th1f]->RedrawAxis("SAME"); // stack kills axis

  //Draw MC sum total error as well on top of stack --> E2 makes error appear as rectangle
  fOutMCTH1FHists[th1f]->Draw("E2 SAME");

  // redraw data (ROOT IS SO DUMBBBBBB)
  fOutDataTH1FHists[th1f]->Draw("PE SAME"); 
  fTH1FLegends[th1f]->Draw("SAME"); // make sure to include the legend!
}

Float_t StackDataMC::GetMaximum(const Int_t th1f) 
{
  Float_t max = -1e9;
  if (fOutDataTH1FHists[th1f]->GetBinContent(fOutDataTH1FHists[th1f]->GetMaximumBin()) > fOutMCTH1FHists[th1f]->GetBinContent(fOutMCTH1FHists[th1f]->GetMaximumBin())) 
  {
    max = fOutDataTH1FHists[th1f]->GetBinContent(fOutDataTH1FHists[th1f]->GetMaximumBin());
  }
  else 
  {
    max = fOutMCTH1FHists[th1f]->GetBinContent(fOutMCTH1FHists[th1f]->GetMaximumBin());
  }
  return max;
}

Float_t StackDataMC::GetMinimum(const Int_t th1f) 
{
  // need to loop through to check bin != 0
  Float_t datamin  = 1e9;
  Bool_t newdatamin = false;

  for (Int_t bin = 1; bin <= fOutDataTH1FHists[th1f]->GetNbinsX(); bin++)
  {
    Float_t tmpmin = fOutDataTH1FHists[th1f]->GetBinContent(bin);
    if ((tmpmin < datamin) && (tmpmin > 0))
    {
      datamin    = tmpmin;
      newdatamin = true;
    }
  }

  Float_t mcmin  = 1e9;
  Bool_t newmcmin = false;
  for (Int_t mc = 0; mc < fNMC; mc++) 
  {
    for (Int_t bin = 1; bin <= fInMCTH1FHists[th1f][mc]->GetNbinsX(); bin++)
    {
      Float_t tmpmin = fInMCTH1FHists[th1f][mc]->GetBinContent(bin);
      if ((tmpmin < mcmin) && (tmpmin > 0)) 
      {
	mcmin    = tmpmin;
	newmcmin = true;
      }
    }
  }
  
  // now set return variable min
  Float_t min = 1; // to not royally mess up logy plots in case where plots have no fill and no new min is set for data or mc
  if (newdatamin || newmcmin) 
  {
    min = ((datamin < mcmin) ? datamin : mcmin); 
  }
  return min;
}

void StackDataMC::DrawLowerPad(const Int_t th1f) 
{    
  // pad gymnastics
  fOutTH1FCanvases[th1f]->cd();   // Go back to the main canvas before defining pad2
  fOutTH1FRatioPads[th1f]->Draw(); // draw lower pad
  fOutTH1FRatioPads[th1f]->cd(); // lower pad is current pad

  // make red line at ratio of 1.0
  StackDataMC::SetLines(th1f);

  // draw th1 first so line can appear, then draw over it (and set Y axis divisions)
  fOutRatioTH1FHists[th1f]->Draw("EP"); // draw first so line can appear
  fOutTH1FRatioLines[th1f]->Draw("SAME");

  // some style since apparently TDR Style is crapping out --> would really not like this here
  fOutRatioTH1FHists[th1f]->GetYaxis()->SetNdivisions(505);

  // sizes of titles is percent of height of pad --> want a constant size ... so take TDRStyle value, which is evaulated at unity pad height, and divide by height of pad
  fOutRatioTH1FHists[th1f]->GetXaxis()->SetLabelSize  (Config::LabelSize   / Config::height_lp); 
  fOutRatioTH1FHists[th1f]->GetXaxis()->SetLabelOffset(Config::LabelOffset / Config::height_lp); 
  fOutRatioTH1FHists[th1f]->GetXaxis()->SetTitleSize  (Config::TitleSize   / Config::height_lp);
  fOutRatioTH1FHists[th1f]->GetXaxis()->SetTickLength (Config::TickLength  / Config::height_lp);
  fOutRatioTH1FHists[th1f]->GetYaxis()->SetLabelSize  (Config::LabelSize   / Config::height_lp); 
  fOutRatioTH1FHists[th1f]->GetYaxis()->SetTitleSize  (Config::TitleSize   / Config::height_lp);
  fOutRatioTH1FHists[th1f]->GetYaxis()->SetTitleOffset(Config::TitleFF * Config::TitleYOffset * Config::height_lp);

  // redraw to go over line
  fOutRatioTH1FHists[th1f]->Draw("EP SAME"); 
  
  // plots MC error copy
  for (Int_t bin = 1; bin <= fOutRatioMCErrs[th1f]->GetNbinsX(); bin++) 
  {
    if (fOutRatioMCErrs[th1f]->GetBinContent(bin) == 0) { fOutRatioMCErrs[th1f]->SetBinContent(bin,-1); } // don't display empty bins
  }
  fOutRatioMCErrs[th1f]->Draw("E2 SAME");
}

void StackDataMC::SetLines(const Int_t th1f)
{
  // have line held at ratio of 1.0 over whole x range
  fOutTH1FRatioLines[th1f]->SetX1(fOutRatioTH1FHists[th1f]->GetXaxis()->GetXmin());
  fOutTH1FRatioLines[th1f]->SetX2(fOutRatioTH1FHists[th1f]->GetXaxis()->GetXmax());
  fOutTH1FRatioLines[th1f]->SetY1(0.0);
  fOutTH1FRatioLines[th1f]->SetY2(0.0);

  // customize appearance
  fOutTH1FRatioLines[th1f]->SetLineColor(kRed);
  fOutTH1FRatioLines[th1f]->SetLineWidth(2);
}

void StackDataMC::SaveCanvas(const Int_t th1f, const Bool_t isLogY)
{
  TString suffix;

  if   (isLogY) {suffix = "log";}
  else          {suffix = "lin";}

  // cd to upper pad to make it log or not
  fOutTH1FStackPads[th1f]->cd(); // upper pad is current pad
  fOutTH1FStackPads[th1f]->SetLogy(isLogY); //  set logy on this pad

  fOutTH1FCanvases[th1f]->cd();    // Go back to the main canvas before saving
  CMSLumi(fOutTH1FCanvases[th1f]); // write out Lumi info
  fOutTH1FCanvases[th1f]->SaveAs(Form("%s/%s/%s/%s.%s",fOutDir.Data(),fTH1FSubDMap[fTH1FNames[th1f]].Data(),suffix.Data(),fTH1FNames[th1f].Data(),Config::outtype.Data()));
  fOutFile->cd();
  if (!isLogY) fOutTH1FCanvases[th1f]->Write(Form("%s",fTH1FNames[th1f].Data()));
}

void StackDataMC::OpenInputFiles() 
{
  // open input files into TFileVec --> data 
  fDataFiles.resize(fNData);
  for (Int_t data = 0; data < fNData; data++) 
  {
    TString datafile = Form("%s/DATA/%s/plots.root",Config::outdir.Data(),fDataNames[data].Data());
    fDataFiles[data] = TFile::Open(datafile.Data());
    CheckValidFile(fDataFiles[data],datafile);
  }

  // open input files into TFileVec --> mc 
  fMCFiles.resize(fNMC);
  for (Int_t mc = 0; mc < fNMC; mc++) 
  {
    TString mcfile = Form("%s/MC/%s/plots.root",Config::outdir.Data(),fMCNames[mc].Data());
    fMCFiles[mc] = TFile::Open(mcfile.Data());
    CheckValidFile(fMCFiles[mc],mcfile);
  }
}

void StackDataMC::InitInputPlots() 
{
  // init input th1f hists
  fInDataTH1FHists.resize(fNTH1F);
  fInMCTH1FHists.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    // data first
    fInDataTH1FHists[th1f].resize(fNData); 
    for (Int_t data = 0; data < fNData; data++)  // init data double hists
    {
      fInDataTH1FHists[th1f][data] = (TH1F*)fDataFiles[data]->Get(Form("%s",fTH1FNames[th1f].Data()));	
      CheckValidTH1F(fInDataTH1FHists[th1f][data],fTH1FNames[th1f],fDataFiles[data]->GetName());
    }

    // mc second
    fInMCTH1FHists[th1f].resize(fNMC); 
    for (Int_t mc = 0; mc < fNMC; mc++)  // init mc double hists
    {
      fInMCTH1FHists[th1f][mc] = (TH1F*)fMCFiles[mc]->Get(Form("%s",fTH1FNames[th1f].Data()));
      CheckValidTH1F(fInMCTH1FHists[th1f][mc],fTH1FNames[th1f],fMCFiles[mc]->GetName());
      fInMCTH1FHists[th1f][mc]->SetFillColor(Config::colorMap[fMCNames[mc]]);

      fInMCTH1FHists[th1f][mc]->SetLineColor(kBlack);
    }
  }
}

void StackDataMC::InitOutputPlots() 
{
  fOutDataTH1FHists.resize(fNTH1F); // make enough space for data double hists
  fOutMCTH1FHists.resize(fNTH1F); // make enough space for MC double hists
  fOutMCTH1FStacks.resize(fNTH1F); // same with stack 
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fOutMCTH1FStacks[th1f] = new THStack(Form("%s_stack",fTH1FNames[th1f].Data()),"");
  }
}

void StackDataMC::InitOutputLegends() 
{
  fTH1FLegends.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fTH1FLegends[th1f] = new TLegend(0.682,0.7,0.825,0.92);
    fTH1FLegends[th1f]->SetBorderSize(1);
    fTH1FLegends[th1f]->SetLineColor(kBlack);
  }
}

void StackDataMC::InitRatioPlots() 
{
  // th1f hists
  fOutRatioTH1FHists.resize(fNTH1F);

  // mc err hists 
  fOutRatioMCErrs.resize(fNTH1F);
}

void StackDataMC::InitRatioLines() 
{
  fOutTH1FRatioLines.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fOutTH1FRatioLines[th1f] = new TLine();
  }
}

void StackDataMC::InitOutputCanvPads() 
{
  fOutTH1FCanvases.resize(fNTH1F);
  fOutTH1FStackPads.resize(fNTH1F);
  fOutTH1FRatioPads.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fOutTH1FCanvases[th1f] = new TCanvas(Form("%s_datamc_canv",fTH1FNames[th1f].Data()),"");
    fOutTH1FCanvases[th1f]->cd();
    
    fOutTH1FStackPads[th1f] = new TPad(Form("%s_upad",fTH1FNames[th1f].Data()),"", Config::left_up, Config::bottom_up, Config::right_up, Config::top_up);
    fOutTH1FStackPads[th1f]->SetBottomMargin(0); // Upper and lower plot are joined
    
    fOutTH1FRatioPads[th1f] = new TPad(Form("%s_lpad",fTH1FNames[th1f].Data()), "", Config::left_lp, Config::bottom_lp, Config::right_lp, Config::top_lp);
    fOutTH1FRatioPads[th1f]->SetTopMargin(0);
    fOutTH1FRatioPads[th1f]->SetBottomMargin(0.3);
  }
}

void StackDataMC::InitTH1FNamesAndSubDNames()
{
  // will use the integral of nvtx to derive total yields as no additional cuts are placed on ntvx --> key on name for yields
  
  std::ifstream plotstoread;
  plotstoread.open(Form("%s/%s",Config::outdir.Data(),Config::plotdumpname.Data()),std::ios::in);

  TString plotname; TString subdir;

  while (plotstoread >> plotname >> subdir) 
  {
    fTH1FNames.push_back(plotname);
    fTH1FSubDMap[plotname] = subdir;
  }
  plotstoread.close();

  // store the size of the number of plots
  fNTH1F = fTH1FNames.size();

  if (fTH1FNames.size() == 0) 
  {
    std::cerr << "Somehow, no plots were read in for the stacker ...exiting..." << std::endl;
    exit(1);
  }
}
