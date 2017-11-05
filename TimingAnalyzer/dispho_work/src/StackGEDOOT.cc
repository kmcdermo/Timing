#include "../interface/StackGEDOOT.hh"

StackGEDOOT::StackGEDOOT(const TString & sample, const Bool_t isMC) : fSample(sample), fIsMC(isMC)  
{
  // I/O dir
  fOutDir = Form("%s/%i/%s/%s", Config::outdir.Data(), Config::year, (fIsMC?"MC":"DATA"), fSample.Data());

  // open input files
  const TString filename = Form("%s/%s", fOutDir.Data(), Config::AnOutName.Data());
  fInFile = TFile::Open(filename.Data());
  CheckValidFile(fInFile,filename);

  // output data members
  fOutFile = new TFile(Form("%s/stackphoplots_canvases.root", fOutDir.Data()),"UPDATE"); // make output tfile --> store canvas images here too, for quick editting

  // Read in names of plots to be stacked
  StackGEDOOT::InitTH1FNamesAndSubDNames();
  // Make subdirs as needed
  StackGEDOOT::InitSubDirs();

  // with all that defined, initialize everything in constructor
  StackGEDOOT::InitInputPlots();
  StackGEDOOT::InitOutputLegends();
  StackGEDOOT::InitRatioPlots();
  StackGEDOOT::InitRatioLines();
  StackGEDOOT::InitOutputCanvPads();
}

StackGEDOOT::~StackGEDOOT()
{
  // delete all pointers
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    delete fOutTH1FRatioPads[th1f];
    delete fOutTH1FStackPads[th1f];
    delete fOutTH1FCanvases[th1f];
    delete fOutTH1FRatioLines[th1f];
    delete fOutRatioTH1FHists[th1f];
    delete fTH1FLegends[th1f];
    delete fInOOTTH1FHists[th1f];
    delete fInGEDTH1FHists[th1f];
  }
  delete fOutFile;
  delete fInFile;
}

void StackGEDOOT::DoStacks()
{
  StackGEDOOT::MakeStackGEDOOT();
  StackGEDOOT::MakeRatioPlots();
  StackGEDOOT::MakeOutputCanvas();
}

void StackGEDOOT::MakeStackGEDOOT()
{
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++) //th1f hists
  {
    const Bool_t isV = fTH1FNames[th1f].Contains("_v_",TString::kExact);

    // GED first
    TString ytitleGED = fInGEDTH1FHists[th1f]->GetYaxis()->GetTitle();
    ytitleGED.ReplaceAll("GED - ","");
    fInGEDTH1FHists[th1f]->GetYaxis()->SetTitle(ytitleGED.Data());
    fInGEDTH1FHists[th1f]->SetLineColor(kRed);
    fInGEDTH1FHists[th1f]->SetMarkerColor(kRed);
    // scale to unity
    if (!isV && fInGEDTH1FHists[th1f]->Integral() > 0) fInGEDTH1FHists[th1f]->Scale(1.f/fInGEDTH1FHists[th1f]->Integral());
    // add to legend
    fTH1FLegends[th1f]->AddEntry(fInGEDTH1FHists[th1f],"GED","epl");

    // OOT first
    TString ytitleOOT = fInOOTTH1FHists[th1f]->GetYaxis()->GetTitle();
    ytitleOOT.ReplaceAll("OOT - ","");
    fInOOTTH1FHists[th1f]->GetYaxis()->SetTitle(ytitleOOT.Data());    
    fInOOTTH1FHists[th1f]->SetLineColor(kBlue);
    fInOOTTH1FHists[th1f]->SetMarkerColor(kBlue);
    // scale to unity
    if (!isV && fInOOTTH1FHists[th1f]->Integral() > 0) fInOOTTH1FHists[th1f]->Scale(1.f/fInOOTTH1FHists[th1f]->Integral());
    // add to legend
    fTH1FLegends[th1f]->AddEntry(fInOOTTH1FHists[th1f],"OOT","epl");
  } // end loop over th1f plots
}

void StackGEDOOT::MakeRatioPlots()
{
  // vs something : ratio is 1 - OOT/GED == (GED - OOT) / GED
  // else, ratio is OOT/GED

  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  { 
    const Bool_t isV = fTH1FNames[th1f].Contains("_v_",TString::kExact);

    // ratio value plot
    fOutRatioTH1FHists[th1f] = (TH1F*)(isV?fInGEDTH1FHists[th1f]:fInOOTTH1FHists[th1f])->Clone(Form("%s_ratio",fTH1FNames[th1f].Data()));
    if (isV) fOutRatioTH1FHists[th1f]->Add(fInOOTTH1FHists[th1f],-1);  
    fOutRatioTH1FHists[th1f]->Divide(fInGEDTH1FHists[th1f]);

    // Titles
    fOutRatioTH1FHists[th1f]->GetYaxis()->SetTitle((isV?"1-(OOT/GED)":"OOT/GED"));
    const TString drop = (isV?"GED":"OOT");
    const Ssiz_t length = drop.Length();
    TString xtitle = fOutRatioTH1FHists[th1f]->GetXaxis()->GetTitle();
    const Ssiz_t index = xtitle.Index(drop);
    xtitle.Remove(index,length+3); // account for " - "
    fOutRatioTH1FHists[th1f]->GetXaxis()->SetTitle(xtitle.Data());

    // Ranges, colors
    fOutRatioTH1FHists[th1f]->SetMinimum(isV?-2.f:0.5); // Define Y ..
    fOutRatioTH1FHists[th1f]->SetMaximum(isV? 2.f:1.5); // .. range
    fOutRatioTH1FHists[th1f]->SetLineColor(kBlack);
    fOutRatioTH1FHists[th1f]->SetMarkerColor(kBlack);
    fOutRatioTH1FHists[th1f]->SetStats(0);      // No statistics on lower plot
  }
}

void StackGEDOOT::MakeOutputCanvas() 
{
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    const Bool_t isV = fTH1FNames[th1f].Contains("_v_",TString::kExact);

    Bool_t isLogY = true;
    if (!isV)
    {
      // LogY first
      StackGEDOOT::DrawUpperPad(th1f,isLogY); // upper pad is stack
      StackGEDOOT::DrawLowerPad(th1f); // lower pad is ratio
      StackGEDOOT::SaveCanvas(th1f,isLogY); // now save the canvas
    }

    // LinearY second
    isLogY = false;
    StackGEDOOT::DrawUpperPad(th1f,isLogY); // upper pad is stack
    StackGEDOOT::DrawLowerPad(th1f); // lower pad is ratio
    StackGEDOOT::SaveCanvas(th1f,isLogY); // now save the canvas
  }
}

void StackGEDOOT::DrawUpperPad(const Int_t th1f, const Bool_t isLogY) 
{    
  // pad gymnastics
  fOutTH1FCanvases[th1f]->cd();
  fOutTH1FStackPads[th1f]->Draw(); // draw upper pad   
  fOutTH1FStackPads[th1f]->cd(); // upper pad is current pad
  
  // set maximum by comparing added mc vs added data
  const Float_t min = StackGEDOOT::GetMinimum(th1f);
  const Float_t max = StackGEDOOT::GetMaximum(th1f);

  if (isLogY)
  {
    fInGEDTH1FHists[th1f]->SetMaximum( max > 0 ? max*1.5 : max/1.5 );      
    fInGEDTH1FHists[th1f]->SetMinimum( min > 0 ? min/1.5 : 0.00001 );
  }
  else 
  {
    fInGEDTH1FHists[th1f]->SetMaximum( max > 0 ? max*1.05 : max/1.05 );      
    fInGEDTH1FHists[th1f]->SetMinimum( min > 0 ? min/1.05 : min*1.05 );
  }

  // draw hists
  fInGEDTH1FHists[th1f]->Draw("PE"); // draw first so labels appear
  fInOOTTH1FHists[th1f]->Draw("PE SAME"); // draw first so labels appear
  
  // again, have to scale TDR style values by height of upper pad
  fInGEDTH1FHists[th1f]->GetYaxis()->SetLabelSize  (Config::LabelSize / Config::height_up); 
  fInGEDTH1FHists[th1f]->GetYaxis()->SetTitleSize  (Config::TitleSize / Config::height_up);
  fInGEDTH1FHists[th1f]->GetYaxis()->SetTitleOffset(Config::TitleFF * Config::TitleYOffset * Config::height_up);

  // make sure to include the legend!
  fTH1FLegends[th1f]->Draw("SAME"); 
}

Float_t StackGEDOOT::GetMaximum(const Int_t th1f) 
{
  return (fInGEDTH1FHists[th1f]->GetMaximum() > fInOOTTH1FHists[th1f]->GetMaximum() ? fInGEDTH1FHists[th1f]->GetMaximum() : fInOOTTH1FHists[th1f]->GetMaximum());
}

Float_t StackGEDOOT::GetMinimum(const Int_t th1f) 
{
  // GED First
  Float_t gedmin  = 1e9;
  Bool_t newgedmin = false;
  for (Int_t ibin = 1; ibin <= fInGEDTH1FHists[th1f]->GetNbinsX(); ibin++)
  {
    const Float_t tmpmin = fInGEDTH1FHists[th1f]->GetBinContent(ibin);
    if ((tmpmin < gedmin) && (tmpmin != 0))
    {
      gedmin    = tmpmin;
      newgedmin = true;
    }
  }

  // OOT Second
  Float_t ootmin  = 1e9;
  Bool_t newootmin = false;
  for (Int_t ibin = 1; ibin <= fInOOTTH1FHists[th1f]->GetNbinsX(); ibin++)
  {
    const Float_t tmpmin = fInOOTTH1FHists[th1f]->GetBinContent(ibin);
    if ((tmpmin < ootmin) && (tmpmin != 0))
    {
      ootmin    = tmpmin;
      newootmin = true;
    }
  }
  
  Float_t min = 0.f;
  if (newgedmin || newootmin)
  {
    if      ( newgedmin &&  newootmin) min = (gedmin < ootmin ? gedmin : ootmin);
    else if ( newgedmin && !newootmin) min = gedmin; 
    else if (!newgedmin &&  newootmin) min = ootmin; 
  }
  return min;
}

void StackGEDOOT::DrawLowerPad(const Int_t th1f) 
{    
  // pad gymnastics
  fOutTH1FCanvases[th1f]->cd();   // Go back to the main canvas before defining pad2
  fOutTH1FRatioPads[th1f]->Draw(); // draw lower pad
  fOutTH1FRatioPads[th1f]->cd(); // lower pad is current pad

  // make red line at ratio of 0.0
  StackGEDOOT::SetLines(th1f);

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
}

void StackGEDOOT::SetLines(const Int_t th1f)
{
  const Bool_t isV = fTH1FNames[th1f].Contains("_v_",TString::kExact);

  // have line held at ratio of 1.0 or 0.0 over whole x range
  fOutTH1FRatioLines[th1f]->SetX1(fOutRatioTH1FHists[th1f]->GetXaxis()->GetXmin());
  fOutTH1FRatioLines[th1f]->SetX2(fOutRatioTH1FHists[th1f]->GetXaxis()->GetXmax());
  fOutTH1FRatioLines[th1f]->SetY1(isV?0.f:1.f);
  fOutTH1FRatioLines[th1f]->SetY2(isV?0.f:1.f);

  // customize appearance
  fOutTH1FRatioLines[th1f]->SetLineColor(kRed);
  fOutTH1FRatioLines[th1f]->SetLineWidth(2);
}

void StackGEDOOT::SaveCanvas(const Int_t th1f, const Bool_t isLogY)
{
  // cd to upper pad to make it log or not
  fOutTH1FStackPads[th1f]->cd(); // upper pad is current pad
  fOutTH1FStackPads[th1f]->SetLogy(isLogY); //  set logy on this pad

  // Go back to the main canvas before saving
  fOutTH1FCanvases[th1f]->cd();    

  // write out Lumi info
  CMSLumi(fOutTH1FCanvases[th1f]); 

  // save image
  fOutTH1FCanvases[th1f]->SaveAs(Form("%s/%s/%s/%s/%s.%s",fOutDir.Data(),fTH1FSubDMap[fTH1FNames[th1f]].Data(),
				      Config::phosubdir.Data(),(isLogY?"log":"lin"),fTH1FNames[th1f].Data(),Config::outtype.Data()));
  
  fOutFile->cd();
  fOutTH1FCanvases[th1f]->Write(Form("%s",fTH1FNames[th1f].Data()),TObject::kWriteDelete);
}

void StackGEDOOT::InitTH1FNamesAndSubDNames()
{
  // will use the integral of nvtx to derive total yields as no additional cuts are placed on ntvx --> key on name for yields
  
  std::ifstream plotstoread;
  plotstoread.open(Form("%s/%i/%s",Config::outdir.Data(),Config::year,Config::phoplotdumpname.Data()),std::ios::in);

  TString plotname; TString subdir;

  const TString drop = "_GED"; // use bare name in subdir
  while (plotstoread >> plotname >> subdir) 
  {
    fTH1FNames.push_back(plotname);
    plotname.ReplaceAll(drop,"");
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

void StackGEDOOT::InitSubDirs()
{
  MakeSubDirs(fTH1FSubDMap,fOutDir,Config::phosubdir);
}

void StackGEDOOT::InitInputPlots() 
{
  // GED as the base
  const TString drop = "GED";
  const TString swap = "OOT";

  // init input th1f hists
  fInGEDTH1FHists.resize(fNTH1F);
  fInOOTTH1FHists.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    // GED First
    fInGEDTH1FHists[th1f] = (TH1F*)fInFile->Get(Form("%s",fTH1FNames[th1f].Data()));	
    CheckValidTH1F(fInGEDTH1FHists[th1f],fTH1FNames[th1f],fInFile->GetName());

    // Replace GED with OOT...
    fTH1FNames[th1f].ReplaceAll(drop,swap);

    // OOT Second
    fInOOTTH1FHists[th1f] = (TH1F*)fInFile->Get(Form("%s",fTH1FNames[th1f].Data()));	
    CheckValidTH1F(fInOOTTH1FHists[th1f],fTH1FNames[th1f],fInFile->GetName());

    // Finally, drop OOT from name
    fTH1FNames[th1f].ReplaceAll("_"+swap,""); // remove trailing "_"
  }
}

void StackGEDOOT::InitOutputLegends() 
{
  fTH1FLegends.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fTH1FLegends[th1f] = new TLegend(0.75,0.8,0.83,0.92);
    fTH1FLegends[th1f]->SetBorderSize(1);
    fTH1FLegends[th1f]->SetLineColor(kBlack);
  }
}

void StackGEDOOT::InitRatioPlots() 
{
  // th1f hists
  fOutRatioTH1FHists.resize(fNTH1F);
}

void StackGEDOOT::InitRatioLines() 
{
  fOutTH1FRatioLines.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fOutTH1FRatioLines[th1f] = new TLine();
  }
}

void StackGEDOOT::InitOutputCanvPads() 
{
  fOutTH1FCanvases.resize(fNTH1F);
  fOutTH1FStackPads.resize(fNTH1F);
  fOutTH1FRatioPads.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fOutTH1FCanvases[th1f] = new TCanvas(Form("%s_gedoot_canv",fTH1FNames[th1f].Data()),"");
    fOutTH1FCanvases[th1f]->cd();
    
    fOutTH1FStackPads[th1f] = new TPad(Form("%s_upad",fTH1FNames[th1f].Data()),"", Config::left_up, Config::bottom_up, Config::right_up, Config::top_up);
    fOutTH1FStackPads[th1f]->SetBottomMargin(0); // Upper and lower plot are joined
    
    fOutTH1FRatioPads[th1f] = new TPad(Form("%s_lpad",fTH1FNames[th1f].Data()), "", Config::left_lp, Config::bottom_lp, Config::right_lp, Config::top_lp);
    fOutTH1FRatioPads[th1f]->SetTopMargin(0);
    fOutTH1FRatioPads[th1f]->SetBottomMargin(0.3);
  }
}
