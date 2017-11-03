#include "../interface/StackMCOnly.hh"

StackMCOnly::StackMCOnly() 
{
  // input data members
  for (const auto & sample : Config::SampleMap)
  {
    if (sample.first.Contains("gmsb") || sample.first.Contains("hvds"))
    {
      fSignalNames.push_back(sample.first);
    }
    else
    {
      fBkgdNames.push_back(sample.first);
    }
  }

  // store for later
  fNSignal = fSignalNames.size();
  fNBkgd   = fBkgdNames.size();

  // output data members
  fOutDir = Form("%s/%i/mconlystacks", Config::outdir.Data(), Config::year); // where to put output stack plots 
  MakeOutDir(fOutDir); // make output directory 
  fOutFile = new TFile(Form("%s/stackmcplots_canvases.root", fOutDir.Data()),"UPDATE"); // make output tfile --> store canvas images here too, for quick editting

  // Read in names of plots to be stacked
  StackMCOnly::InitTH1FNamesAndSubDNames();
  
  // make stack outputs
  MakeSubDirs(fTH1FSubDMap,fOutDir);

  // with all that defined, initialize everything in constructor
  StackMCOnly::OpenInputFiles();
  StackMCOnly::InitInputPlots();
  StackMCOnly::InitOutputPlots();
  StackMCOnly::InitOutputLegends();
  StackMCOnly::InitOutputCanvases();
}

StackMCOnly::~StackMCOnly()
{
  // delete all pointers
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    delete fOutBkgdTH1FHists[th1f];
    delete fOutBkgdTH1FStacks[th1f];
    delete fTH1FLegends[th1f];
    delete fOutTH1FCanvases[th1f];
    
    for (Int_t signal = 0; signal < fNSignal; signal++) 
    {
      delete fInSignalTH1FHists[th1f][signal];
    }
  
    for (Int_t bkgd = 0; bkgd < fNBkgd; bkgd++)
    {
      delete fInBkgdTH1FHists[th1f][bkgd];
    }
  }

  for (Int_t signal = 0; signal < fNSignal; signal++) 
  {
    delete fSignalFiles[signal];
  }

  for (Int_t bkgd = 0; bkgd < fNBkgd; bkgd++) 
  {
    delete fBkgdFiles[bkgd];
  }

  delete fOutFile;
}

void StackMCOnly::DoStacks() 
{
  StackMCOnly::MakeStackMCOnly();
  StackMCOnly::MakeOutputCanvas();
}

void StackMCOnly::MakeStackMCOnly()
{
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++) //th1f hists
  {
    // signal
    for (Int_t signal = 0; signal < fNSignal; signal++) 
    {
      fInSignalTH1FHists[th1f][signal]->Scale(1.f/fInSignalTH1FHists[th1f][signal]->Integral());
      fTH1FLegends[th1f]->AddEntry(fInSignalTH1FHists[th1f][signal],Config::TitleMap[fSignalNames[signal]],"l"); // add data entry to legend
    } 

    // make total hist for errors
    for (Int_t bkgd = 0; bkgd < fNBkgd; bkgd++) 
    {
      if (bkgd == 0) // add first for ratio
      {
	fOutBkgdTH1FHists[th1f] = (TH1F*)fInBkgdTH1FHists[th1f][bkgd]->Clone(Form("%s_clone",fInBkgdTH1FHists[th1f][bkgd]->GetName()));
      }
      else // add first for ratio
      { 
	fOutBkgdTH1FHists[th1f]->Add(fInBkgdTH1FHists[th1f][bkgd]);
      }
    } 

    // scale error hist
    const Double_t bkgdIntegral = fOutBkgdTH1FHists[th1f]->Integral();
    fOutBkgdTH1FHists[th1f]->Scale(1.f/bkgdIntegral);

    // will use the output MC added Hists for plotting uncertainties and add it to legend once
    fOutBkgdTH1FHists[th1f]->SetMarkerSize(0);
    fOutBkgdTH1FHists[th1f]->SetFillStyle(3254);
    fOutBkgdTH1FHists[th1f]->SetFillColor(kGray+3);
    fTH1FLegends[th1f]->AddEntry(fOutBkgdTH1FHists[th1f],"MC Unc.","f");

    // add stacks once renormalized
    for (Int_t bkgd = 0; bkgd < fNBkgd; bkgd++) 
    {
      fInBkgdTH1FHists[th1f][bkgd]->Scale(fInBkgdTH1FHists[th1f][bkgd]->Integral() / bkgdIntegral);
      fOutBkgdTH1FStacks[th1f]->Add(fInBkgdTH1FHists[th1f][bkgd]); // add scaled bkgds to stacks
      fTH1FLegends[th1f]->AddEntry(fInBkgdTH1FHists[th1f][bkgd],Config::TitleMap[fBkgdNames[bkgd]],"f");
    }
  } // end loop over th1f plots
}

void StackMCOnly::MakeOutputCanvas() 
{
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    // draw first with logy scale
    Bool_t isLogY = true;
    StackMCOnly::DrawCanvas(th1f,isLogY);
    StackMCOnly::SaveCanvas(th1f,isLogY);

    // draw second with lin scale
    isLogY = false;
    StackMCOnly::DrawCanvas(th1f,isLogY);
    StackMCOnly::SaveCanvas(th1f,isLogY);
  }
}

void StackMCOnly::DrawCanvas(const Int_t th1f, const Bool_t isLogY) 
{    
  // pad gymnastics
  fOutTH1FCanvases[th1f]->cd();
  
  // get max/min by comparing added bkgd vs added signal
  const Double_t min = StackMCOnly::GetMinimum(th1f);
  const Double_t max = StackMCOnly::GetMaximum(th1f);

  // set min/max of signals, draw first
  for (Int_t signal = 0; signal < fNSignal; signal++) 
  {
    if (isLogY) 
    { 
      fInSignalTH1FHists[th1f][signal]->SetMinimum(min/1.5);
      fInSignalTH1FHists[th1f][signal]->SetMaximum(max*1.5);
    }
    else 
    {
      fInSignalTH1FHists[th1f][signal]->SetMaximum( max > 0 ? max*1.05 : max/1.05 );      
      fInSignalTH1FHists[th1f][signal]->SetMinimum( min > 0 ? min/1.05 : min*1.05 );
    }
  
    // draw first so labels appear
    fInSignalTH1FHists[th1f][signal]->Draw(signal>0?"L SAME":"L"); 
  }

  // now draw the stacks, and redo axes...
  fOutBkgdTH1FStacks[th1f]->Draw("HIST SAME"); 
  fOutTH1FCanvases[th1f]->RedrawAxis("SAME"); // stack kills axis

  // Draw Bkgd sum total error as well on top of stack --> E2 makes error appear as rectangle
  fOutBkgdTH1FHists[th1f]->Draw("E2 SAME");

  // redraw signals as they disappeared
  for (Int_t signal = 0; signal < fNSignal; signal++) 
  {
    fInSignalTH1FHists[th1f][signal]->Draw("L SAME"); // draw first so labels appear
  }

  // make sure to include the legend!
  fTH1FLegends[th1f]->Draw("SAME");
}

Double_t StackMCOnly::GetMaximum(const Int_t th1f) 
{
  Double_t max = -1e9;

  // check signal first
  for (Int_t signal = 0; signal < fNSignal; signal++) 
  {
    for (Int_t ibin = 1; ibin <= fInSignalTH1FHists[th1f][signal]->GetNbinsX(); ibin++)
    {     
      const Double_t content = fInSignalTH1FHists[th1f][signal]->GetBinContent(ibin);
      if (content > max) max = content;
    }
  }

  // then sum of bkgds
  for (Int_t ibin = 1; ibin <= fOutBkgdTH1FHists[th1f]->GetNbinsX(); ibin++)
  {     
    const Double_t content = fOutBkgdTH1FHists[th1f]->GetBinContent(ibin);
    if (content > max) max = content;
  }

  return max;
}

Double_t StackMCOnly::GetMinimum(const Int_t th1f) 
{
  Double_t min = 1e9;

  // check signal first
  for (Int_t signal = 0; signal < fNSignal; signal++) 
  {
    for (Int_t ibin = 1; ibin <= fInSignalTH1FHists[th1f][signal]->GetNbinsX(); ibin++)
    {     
      const Double_t content = fInSignalTH1FHists[th1f][signal]->GetBinContent(ibin);
      if (content < min && min != 0.) min = content;
    }
  }

  // check individ. bkgds second
  for (Int_t bkgd = 0; bkgd < fNBkgd; bkgd++) 
  {
    for (Int_t ibin = 1; ibin <= fInBkgdTH1FHists[th1f][bkgd]->GetNbinsX(); ibin++)
    {     
      const Double_t content = fInBkgdTH1FHists[th1f][bkgd]->GetBinContent(ibin);
      if (content < min && min != 0.) min = content;
    }
  }

  return min;
}

void StackMCOnly::SaveCanvas(const Int_t th1f, const Bool_t isLogY)
{
  TString suffix;

  if   (isLogY) {suffix = "log";}
  else          {suffix = "lin";}

  // cd to canvas
  fOutTH1FCanvases[th1f]->cd();
  fOutTH1FCanvases[th1f]->SetLogy(isLogY); 
  CMSLumi(fOutTH1FCanvases[th1f]); // write out Lumi info

  fOutTH1FCanvases[th1f]->SaveAs(Form("%s/%s/%s/%s.%s",fOutDir.Data(),fTH1FSubDMap[fTH1FNames[th1f]].Data(),suffix.Data(),fTH1FNames[th1f].Data(),Config::outtype.Data()));
  fOutFile->cd();
  if (!isLogY) fOutTH1FCanvases[th1f]->Write(Form("%s",fTH1FNames[th1f].Data()));
}

void StackMCOnly::OpenInputFiles() 
{
  // open input files into TFileVec --> signal 
  fSignalFiles.resize(fNSignal);
  for (Int_t signal = 0; signal < fNSignal; signal++) 
  {
    TString signalfile = Form("%s/%i/MC/%s/%s", Config::outdir.Data(), Config::year, fSignalNames[signal].Data(), Config::AnOutName.Data());
    fSignalFiles[signal] = TFile::Open(signalfile.Data());
    CheckValidFile(fSignalFiles[signal],signalfile);
  }

  // open input files into TFileVec --> bkgd 
  fBkgdFiles.resize(fNBkgd);
  for (Int_t bkgd = 0; bkgd < fNBkgd; bkgd++) 
  {
    TString bkgdfile = Form("%s/%i/MC/%s/%s", Config::outdir.Data(), Config::year, fBkgdNames[bkgd].Data(), Config::AnOutName.Data());
    fBkgdFiles[bkgd] = TFile::Open(bkgdfile.Data());
    CheckValidFile(fBkgdFiles[bkgd],bkgdfile);
  }
}

void StackMCOnly::InitInputPlots() 
{
  // init input th1f hists
  fInSignalTH1FHists.resize(fNTH1F);
  fInBkgdTH1FHists.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    // signal first
    fInSignalTH1FHists[th1f].resize(fNSignal); 
    for (Int_t signal = 0; signal < fNSignal; signal++)  // init signal double hists
    {
      fInSignalTH1FHists[th1f][signal] = (TH1F*)fSignalFiles[signal]->Get(Form("%s",fTH1FNames[th1f].Data()));	
      CheckValidTH1F(fInSignalTH1FHists[th1f][signal],fTH1FNames[th1f],fSignalFiles[signal]->GetName());
      fInSignalTH1FHists[th1f][signal]->SetLineColor(Config::ColorMap[fSignalNames[signal]]);
    }

    // bkgd second
    fInBkgdTH1FHists[th1f].resize(fNBkgd); 
    for (Int_t bkgd = 0; bkgd < fNBkgd; bkgd++)  // init bkgd double hists
    {
      fInBkgdTH1FHists[th1f][bkgd] = (TH1F*)fBkgdFiles[bkgd]->Get(Form("%s",fTH1FNames[th1f].Data()));
      CheckValidTH1F(fInBkgdTH1FHists[th1f][bkgd],fTH1FNames[th1f],fBkgdFiles[bkgd]->GetName());
      fInBkgdTH1FHists[th1f][bkgd]->SetFillColor(Config::ColorMap[fBkgdNames[bkgd]]);
      fInBkgdTH1FHists[th1f][bkgd]->SetLineColor(kBlack);
    }
  }
}

void StackMCOnly::InitOutputPlots()
{
  fOutBkgdTH1FHists.resize(fNTH1F); // error hists
  fOutBkgdTH1FStacks.resize(fNTH1F); // stack same size as nHists
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fOutBkgdTH1FStacks[th1f] = new THStack(Form("%s_stack",fTH1FNames[th1f].Data()),"");
  }
}

void StackMCOnly::InitOutputLegends() 
{
  fTH1FLegends.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fTH1FLegends[th1f] = new TLegend(0.682,0.7,0.825,0.92);
    fTH1FLegends[th1f]->SetBorderSize(1);
    fTH1FLegends[th1f]->SetLineColor(kBlack);
  }
}

void StackMCOnly::InitOutputCanvases() 
{
  fOutTH1FCanvases.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fOutTH1FCanvases[th1f] = new TCanvas(Form("%s_signalbkgd_canv",fTH1FNames[th1f].Data()),"");
    fOutTH1FCanvases[th1f]->cd();
  }
}

void StackMCOnly::InitTH1FNamesAndSubDNames()
{
  // will use the integral of nvtx to derive total yields as no additional cuts are placed on ntvx --> key on name for yields
  
  std::ifstream plotstoread;
  plotstoread.open(Form("%s/%s/%s",Config::outdir.Data(),Config::year,Config::plotdumpname.Data()),std::ios::in);

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
