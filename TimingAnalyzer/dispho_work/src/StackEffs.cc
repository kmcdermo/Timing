#include "../interface/StackEffs.hh"

StackEffs::StackEffs() 
{
  // Read in sample names first
  StackEffs::InitSampleNames();

  // output data members
  fOutDir = Form("%s/%i/effstacks", Config::outdir.Data(), Config::year); // where to put output stack plots 
  MakeOutDir(fOutDir); // make output directory 

  // output canvas file
  fOutFile = new TFile(Form("%s/stackeffplots_canvases.root", fOutDir.Data()),"UPDATE"); // make output tfile --> store canvas images here too, for quick editting

  // dump file
  fEffDump.open(Form("%s/efficiency.txt", fOutDir.Data()),std::ios_base::trunc); 

  // Read in names of plots to be stacked
  StackEffs::InitTEffNamesAndSubDNames();
  
  // make stack outputs
  MakeSubDirs(fTEffSubDMap,fOutDir);

  // with all that defined, initialize everything in constructor
  StackEffs::OpenInputFiles();
  StackEffs::InitInputPlots();
  StackEffs::InitOutputPlots();
  StackEffs::InitOutputLegends();
  StackEffs::InitOutputCanvases();
}

StackEffs::~StackEffs()
{
  // delete all pointers
  for (Int_t teff = 0; teff < fNTEff; teff++)
  {
    delete fOutLegends[teff];
    delete fOutCanvases[teff];
    
    for (Int_t sample = 0; sample < fNSample; sample++) 
    {
      delete fInTEffs[teff][sample];
      delete fOutTGAEs[teff][sample];
    }
  }

  for (Int_t sample = 0; sample < fNSample; sample++) 
  {
    delete fSampleFiles[sample];
  }

  fEffDump.close();
  delete fOutFile;
}

void StackEffs::DoStacks() 
{
  StackEffs::MakeStackEffs();
  StackEffs::MakeOutputCanvas();
}

void StackEffs::MakeStackEffs()
{
  for (Int_t teff = 0; teff < fNTEff; teff++) //teff hists
  {
    // store teff name in dump
    const Bool_t dump = fTEffNames[teff].Contains("phi",TString::kExact);
    if (dump) StackEffs::PrintEffHeader(teff);

    for (Int_t sample = 0; sample < fNSample; sample++)
    {
      // setup output tgraph
      const Bool_t isMC = Config::SampleMap[fSampleNames[sample]];
      const Color_t color = (isMC ? Config::ColorMap[fSampleNames[sample]] : kBlack);
      const TString title = (isMC ? Config::TitleMap[fSampleNames[sample]] : "DATA");

      fOutTGAEs[teff][sample]->SetMarkerColor(color);
      fOutTGAEs[teff][sample]->SetLineColor(color);

      // write out total efficiency (phi only)
      if (dump)	StackEffs::PrintTotalEff(teff,sample);
   
      // store legend entry
      fOutLegends[teff]->AddEntry(fOutTGAEs[teff][sample],title.Data(),"epl"); // add data entry to legend
    } // end loop over samples

    if (dump) StackEffs::PrintEffFooter();
  } // end loop over eff names
}

void StackEffs::PrintEffHeader(const Int_t teff)
{
  const TString drop = "effphi";
  const TString swap = "pho";

  TString hname = fTEffNames[teff]; 
  hname.ReplaceAll(drop,swap);

  fEffDump << hname.Data() << std::endl;
}

void StackEffs::PrintTotalEff(const Int_t teff, const Int_t sample)
{
  const Float_t passed = fInTEffs[teff][sample]->GetPassedHistogram()->Integral();
  const Float_t total  = fInTEffs[teff][sample]->GetTotalHistogram()->Integral();
  
  TEfficiency * tmp_eff = new TEfficiency("tmp_eff","tmp_eff",1,0,1);
  tmp_eff->SetTotalEvents(1,total);
  tmp_eff->SetPassedEvents(1,passed);

  const Float_t eff  = tmp_eff->GetEfficiency(1);
  const Float_t elow = tmp_eff->GetEfficiencyErrorLow(1);
  const Float_t eup  = tmp_eff->GetEfficiencyErrorUp(1);

  // dump info into text file
  fEffDump << "   " << fSampleNames[sample].Data() << ": " << eff << ", -" << elow << ", + " << eup << std::endl;

  delete tmp_eff;
}

void StackEffs::PrintEffFooter()
{
  fEffDump << "--------------------" << std::endl;
  fEffDump << std::endl;
}

void StackEffs::MakeOutputCanvas() 
{
  for (Int_t teff = 0; teff < fNTEff; teff++)
  {
    StackEffs::DrawCanvas(teff);
    StackEffs::SaveCanvas(teff);
  }
}

void StackEffs::DrawCanvas(const Int_t teff)
{    
  // pad gymnastics
  fOutCanvases[teff]->cd();

  // set min/max
  for (Int_t sample = 0; sample < fNSample; sample++) 
  {
    fOutTGAEs[teff][sample]->SetMinimum(0.f);
    fOutTGAEs[teff][sample]->SetMaximum(1.5);

    // draw first so labels appear
    fOutTGAEs[teff][sample]->Draw(sample>0?"P SAME":"AP"); 
  }

  // make sure to include the legend!
  fOutLegends[teff]->Draw("SAME");
}

void StackEffs::SaveCanvas(const Int_t teff)
{
  // cd to canvas
  fOutCanvases[teff]->cd();
  CMSLumi(fOutCanvases[teff]); // write out Lumi info

  fOutCanvases[teff]->SaveAs(Form("%s/%s/lin/%s.%s",fOutDir.Data(),fTEffSubDMap[fTEffNames[teff]].Data(),fTEffNames[teff].Data(),Config::outtype.Data()));
  fOutFile->cd();
  fOutCanvases[teff]->Write(Form("%s",fTEffNames[teff].Data()));
}

void StackEffs::OpenInputFiles() 
{
  // open input files into TFileVec --> sample 
  fSampleFiles.resize(fNSample);
  for (Int_t sample = 0; sample < fNSample; sample++) 
  {
    const Bool_t isMC = Config::SampleMap[fSampleNames[sample]];
    const TString samplefile = Form("%s/%i/%s/%s/%s", Config::outdir.Data(), Config::year, (isMC?"MC":"DATA"), fSampleNames[sample].Data(), Config::AnOutName.Data());

    fSampleFiles[sample] = TFile::Open(samplefile.Data());
    CheckValidFile(fSampleFiles[sample],samplefile);
  }
}

void StackEffs::InitInputPlots() 
{
  // init input teff hists
  fInTEffs.resize(fNTEff);
  for (Int_t teff = 0; teff < fNTEff; teff++)
  {
    // signal first
    fInTEffs[teff].resize(fNSample); 
    for (Int_t sample = 0; sample < fNSample; sample++)  // init sample double hists
    {
      fInTEffs[teff][sample] = (TEfficiency*)fSampleFiles[sample]->Get(Form("%s",fTEffNames[teff].Data()));	
      CheckValidTEff(fInTEffs[teff][sample],fTEffNames[teff],fSampleFiles[sample]->GetName());
    }
  }
}

void StackEffs::InitOutputPlots()
{
  fOutTGAEs.resize(fNTEff);
  for (Int_t teff = 0; teff < fNTEff; teff++)
  {
    fOutTGAEs[teff].resize(fNSample); 
    for (Int_t sample = 0; sample < fNSample; sample++)  // init sample double hists
    {
      fOutTGAEs[teff][sample] = fInTEffs[teff][sample]->CreateGraph();
      fOutTGAEs[teff][sample]->SetName(Form("%s_graph",fTEffNames[teff].Data()));
    }
  }
}

void StackEffs::InitOutputLegends() 
{
  fOutLegends.resize(fNTEff);
  for (Int_t teff = 0; teff < fNTEff; teff++)
  {
    fOutLegends[teff] = new TLegend(0.682,0.7,0.825,0.92);
    fOutLegends[teff]->SetBorderSize(1);
    fOutLegends[teff]->SetLineColor(kBlack);
  }
}

void StackEffs::InitOutputCanvases() 
{
  fOutCanvases.resize(fNTEff);
  for (Int_t teff = 0; teff < fNTEff; teff++)
  {
    fOutCanvases[teff] = new TCanvas(Form("%s_geff_canv",fTEffNames[teff].Data()),"");
    fOutCanvases[teff]->cd();
  }
}

void StackEffs::InitSampleNames()
{
  for (const auto & sample : Config::SampleMap)
  {
    if (!sample.second)
    { 
      fSampleNames.push_back(sample.first);
    }
  }

  for (const auto & sample : Config::SampleMap)
  {
    if (sample.second && (sample.first.Contains("gmsb") || sample.first.Contains("hvds"))) 
    {
      fSampleNames.push_back(sample.first);
    } 
  }

  for (const auto & sample : Config::SampleMap)
  {
    if (sample.second && !(sample.first.Contains("gmsb") || sample.first.Contains("hvds"))) 
    {
      fSampleNames.push_back(sample.first);
    } 
  }
  
  // for use later
  fNSample = fSampleNames.size();
}

void StackEffs::InitTEffNamesAndSubDNames()
{
  // will use the integral of nvtx to derive total yields as no additional cuts are placed on ntvx --> key on name for yields
  
  std::ifstream plotstoread;
  plotstoread.open(Form("%s/%i/%s",Config::outdir.Data(),Config::year,Config::effdumpname.Data()),std::ios::in);

  TString plotname; TString subdir;

  while (plotstoread >> plotname >> subdir) 
  {
    fTEffNames.push_back(plotname);
    fTEffSubDMap[plotname] = subdir;
  }
  plotstoread.close();

  // store the size of the number of plots
  fNTEff = fTEffNames.size();

  if (fTEffNames.size() == 0) 
  {
    std::cerr << "Somehow, no plots were read in for the stacker ...exiting..." << std::endl;
    exit(1);
  }
}
