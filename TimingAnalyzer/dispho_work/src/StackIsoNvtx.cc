#include "../interface/StackIsoNvtx.hh"

StackIsoNvtx::StackIsoNvtx(const TString & sample, const Bool_t isMC) : fSample(sample), fIsMC(isMC)  
{
  // I/O dir
  fOutDir = Form("%s/%s/%s",Config::outdir.Data(),(fIsMC?"MC":"DATA"),fSample.Data());

  // open input files
  const TString filename = Form("%s/plots.root",fOutDir.Data());
  fInFile = TFile::Open(filename.Data());
  CheckValidFile(fInFile,filename);

  // output data members
  fOutFile = new TFile(Form("%s/stackisonvtx_canvases.root",fOutDir.Data()),"RECREATE"); // make output tfile --> store canvas images here too, for quick editting

  // Read in names of plots to be stacked
  StackIsoNvtx::InitTH1FNamesAndSubDNames();
  
  // with all that defined, initialize everything in constructor
  StackIsoNvtx::InitInputPlots();
  StackIsoNvtx::InitOutputLegends();
  StackIsoNvtx::InitRatioPlots();
  StackIsoNvtx::InitRatioLines();
  StackIsoNvtx::InitOutputCanvPads();
}

StackIsoNvtx::~StackIsoNvtx()
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

void StackIsoNvtx::DoStacks()
{
  StackIsoNvtx::MakeStackIsoNvtx();
  StackIsoNvtx::MakeRatioPlots();
  StackIsoNvtx::MakeOutputCanvas();
}

void StackIsoNvtx::MakeStackIsoNvtx()
{
  // common denom
  const TString dropGED = "GED ";
  const TString dropOOT = "OOT ";
  const Ssiz_t  lengthGED = dropGED.Length();
  const Ssiz_t  lengthOOT = dropOOT.Length();

  for (Int_t th1f = 0; th1f < fNTH1F; th1f++) //th1f hists
  {
    // GED first
    TString ytitleGED = fInGEDTH1FHists[th1f]->GetYaxis()->GetTitle();
    const Ssiz_t startposGED = ytitleGED.Index(dropGED);
    ytitleGED.Remove(startposGED,lengthGED);
    fInGEDTH1FHists[th1f]->GetYaxis()->SetTitle(ytitleGED.Data());
    fInGEDTH1FHists[th1f]->SetLineColor(kRed);
    fInGEDTH1FHists[th1f]->SetMarkerColor(kRed);
    fTH1FLegends[th1f]->AddEntry(fInGEDTH1FHists[th1f],"GED","epl");

    // OOT first
    TString ytitleOOT = fInOOTTH1FHists[th1f]->GetYaxis()->GetTitle();
    const Ssiz_t startposOOT = ytitleOOT.Index(dropOOT);
    ytitleOOT.Remove(startposOOT,lengthOOT);
    fInOOTTH1FHists[th1f]->GetYaxis()->SetTitle(ytitleOOT.Data());    
    fInOOTTH1FHists[th1f]->SetLineColor(kBlue);
    fInOOTTH1FHists[th1f]->SetMarkerColor(kBlue);
    fTH1FLegends[th1f]->AddEntry(fInOOTTH1FHists[th1f],"OOT","epl");
  } // end loop over th1f plots
}

void StackIsoNvtx::MakeRatioPlots()
{
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  { 
    // ratio value plot
    fOutRatioTH1FHists[th1f] = (TH1F*)fInGEDTH1FHists[th1f]->Clone();
    fOutRatioTH1FHists[th1f]->Add(fInOOTTH1FHists[th1f],-1);  
    fOutRatioTH1FHists[th1f]->Divide(fInGEDTH1FHists[th1f]);  
    fOutRatioTH1FHists[th1f]->GetYaxis()->SetTitle("1-(OOT/GED)");
    fOutRatioTH1FHists[th1f]->SetMinimum(-2.f); // Define Y ..
    fOutRatioTH1FHists[th1f]->SetMaximum( 2.f); // .. range
    fOutRatioTH1FHists[th1f]->SetLineColor(kBlack);
    fOutRatioTH1FHists[th1f]->SetMarkerColor(kBlack);
    fOutRatioTH1FHists[th1f]->SetStats(0);      // No statistics on lower plot
  }
}

void StackIsoNvtx::MakeOutputCanvas() 
{
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    // draw only linear
    StackIsoNvtx::DrawUpperPad(th1f); // upper pad is stack
    StackIsoNvtx::DrawLowerPad(th1f); // lower pad is ratio
    StackIsoNvtx::SaveCanvas(th1f); // now save the canvas
  }
}

void StackIsoNvtx::DrawUpperPad(const Int_t th1f) 
{    
  // pad gymnastics
  fOutTH1FCanvases[th1f]->cd();
  fOutTH1FStackPads[th1f]->Draw(); // draw upper pad   
  fOutTH1FStackPads[th1f]->cd(); // upper pad is current pad
  
  // set maximum by comparing added mc vs added data
  const Float_t min = StackIsoNvtx::GetMinimum(th1f);
  const Float_t max = StackIsoNvtx::GetMaximum(th1f);

  fInGEDTH1FHists[th1f]->SetMaximum( max > 0 ? max*1.05 : max/1.05 );      
  fInGEDTH1FHists[th1f]->SetMinimum( min > 0 ? min/1.05 : min*1.05 );
  
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

Float_t StackIsoNvtx::GetMaximum(const Int_t th1f) 
{
  return (fInGEDTH1FHists[th1f]->GetMaximum() > fInOOTTH1FHists[th1f]->GetMaximum() ? fInGEDTH1FHists[th1f]->GetMaximum() : fInOOTTH1FHists[th1f]->GetMaximum());
}

Float_t StackIsoNvtx::GetMinimum(const Int_t th1f) 
{
  return (fInGEDTH1FHists[th1f]->GetMinimum() < fInOOTTH1FHists[th1f]->GetMinimum() ? fInGEDTH1FHists[th1f]->GetMinimum() : fInOOTTH1FHists[th1f]->GetMinimum());
}

void StackIsoNvtx::DrawLowerPad(const Int_t th1f) 
{    
  // pad gymnastics
  fOutTH1FCanvases[th1f]->cd();   // Go back to the main canvas before defining pad2
  fOutTH1FRatioPads[th1f]->Draw(); // draw lower pad
  fOutTH1FRatioPads[th1f]->cd(); // lower pad is current pad

  // make red line at ratio of 0.0
  StackIsoNvtx::SetLines(th1f);

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

void StackIsoNvtx::SetLines(const Int_t th1f)
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

void StackIsoNvtx::SaveCanvas(const Int_t th1f)
{
  // Go back to the main canvas before saving
  fOutTH1FCanvases[th1f]->cd();    

  // write out Lumi info
  CMSLumi(fOutTH1FCanvases[th1f]); 

  // save image
  fOutTH1FCanvases[th1f]->SaveAs(Form("%s/%s/%s.%s",fOutDir.Data(),fTH1FSubDMap[fTH1FNames[th1f]].Data(),fTH1FNames[th1f].Data(),Config::outtype.Data()));
  
  fOutFile->cd();
  fOutTH1FCanvases[th1f]->Write(Form("%s",fTH1FNames[th1f].Data()));
}


void StackIsoNvtx::InitTH1FNamesAndSubDNames()
{
  // will use the integral of nvtx to derive total yields as no additional cuts are placed on ntvx --> key on name for yields
  
  std::ifstream plotstoread;
  plotstoread.open(Form("%s/%s",Config::outdir.Data(),Config::isonvtxdumpname.Data()),std::ios::in);

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

void StackIsoNvtx::InitInputPlots() 
{
  // common denom
  const TString marker = "_v_nvtx_mean";
  const Ssiz_t  length = marker.Length();
  const TString addged = "_ged"+marker;
  const TString addoot = "_oot"+marker;

  // init input th1f hists
  fInGEDTH1FHists.resize(fNTH1F);
  fInOOTTH1FHists.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    // common to each name
    const Ssiz_t startpos = fTH1FNames[th1f].Index(marker);
    
    // GED First
    TString ged = fTH1FNames[th1f];
    ged.Replace(startpos,length,addged);
    fInGEDTH1FHists[th1f] = (TH1F*)fInFile->Get(Form("%s",ged.Data()));	
    CheckValidTH1F(fInGEDTH1FHists[th1f],ged,fInFile->GetName());

    // OOT Second
    TString oot = fTH1FNames[th1f];
    oot.Replace(startpos,length,addoot);
    fInOOTTH1FHists[th1f] = (TH1F*)fInFile->Get(Form("%s",oot.Data()));	
    CheckValidTH1F(fInOOTTH1FHists[th1f],oot,fInFile->GetName());
  }
}

void StackIsoNvtx::InitOutputLegends() 
{
  fTH1FLegends.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fTH1FLegends[th1f] = new TLegend(0.682,0.7,0.825,0.92);
    fTH1FLegends[th1f]->SetBorderSize(1);
    fTH1FLegends[th1f]->SetLineColor(kBlack);
  }
}

void StackIsoNvtx::InitRatioPlots() 
{
  // th1f hists
  fOutRatioTH1FHists.resize(fNTH1F);
}

void StackIsoNvtx::InitRatioLines() 
{
  fOutTH1FRatioLines.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fOutTH1FRatioLines[th1f] = new TLine();
  }
}

void StackIsoNvtx::InitOutputCanvPads() 
{
  fOutTH1FCanvases.resize(fNTH1F);
  fOutTH1FStackPads.resize(fNTH1F);
  fOutTH1FRatioPads.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fOutTH1FCanvases[th1f] = new TCanvas(Form("%s_canv",fTH1FNames[th1f].Data()),"");
    fOutTH1FCanvases[th1f]->cd();
    
    fOutTH1FStackPads[th1f] = new TPad(Form("%s_upad",fTH1FNames[th1f].Data()),"", Config::left_up, Config::bottom_up, Config::right_up, Config::top_up);
    fOutTH1FStackPads[th1f]->SetBottomMargin(0); // Upper and lower plot are joined
    
    fOutTH1FRatioPads[th1f] = new TPad(Form("%s_lpad",fTH1FNames[th1f].Data()), "", Config::left_lp, Config::bottom_lp, Config::right_lp, Config::top_lp);
    fOutTH1FRatioPads[th1f]->SetTopMargin(0);
    fOutTH1FRatioPads[th1f]->SetBottomMargin(0.3);
  }
}
