#include "XContaminationDumper.hh"

XContaminationDumper::XContaminationDumper(const TString & infilename, const TString & xcontdumpconfig, const TString & plotconfig,
					   const TString & era, const Bool_t savemetadata, const TString & outfiletext) 
  : fInFileName(infilename), fXContDumpConfig(xcontdumpconfig), fPlotConfig(plotconfig),
    fEra(era), fSaveMetaData(savemetadata), fOutFileText(outfiletext)
{
  std::cout << "Initializing..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // Get main input file
  fInFile = TFile::Open(Form("%s",fInFileName.Data()));
  Common::CheckValidFile(fInFile,fInFileName);

  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(fTDRStyle);

  // setup config
  XContaminationDumper::SetupCommon();
  XContaminationDumper::SetupXContDumpConfig();

  // output root file
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");

  // setup samples, output hists
  XContaminationDumper::SetupSampleVec();
  XContaminationDumper::SetupOutSignalHists();
}

XContaminationDumper::~XContaminationDumper()
{
  std::cout << "Tidying up in destructor..." << std::endl;

  if (fSaveMetaData) delete fConfigPave;

  Common::DeleteMap(fHistMap);

  for (auto & fContPair : fContMap) delete fContPair.second.hist;
  fContMap.clear();

  delete fOutFile;
  delete fTDRStyle;

  delete fInFile;
}

void XContaminationDumper::MakeContaminationDump()
{
  std::cout << "Making contamination dump..." << std::endl;
  
  // Prep contamination dump structs
  XContaminationDumper::PrepContMap();

  // Compute integrals and fractions
  XContaminationDumper::ComputeDumpInfos();

  // Dump into text file
  XContaminationDumper::DumpTextFile();

  // Make signal hists
  XContaminationDumper::FillSignalHists();

  // print signal hists
  XContaminationDumper::PrintSignalHists();

  // Dump meta info
  if (fSaveMetaData) XContaminationDumper::MakeConfigPave();
}

void XContaminationDumper::PrepContMap()
{
  std::cout << "Prepping up contribution struct map..." << std::endl;

  // read in histograms
  for (const auto & sample : fSampleVec)
  {
    fInFile->cd();

    if (Common::IsEWK(sample))
    {
      fContMap[sample] = {(TH2F*)fInFile->Get(Form("%s",Common::EWKHistName.Data()))};
    }
    else if (Common::IsBkgd(sample))
    {
      fContMap[sample] = {(TH2F*)fInFile->Get(Form("%s",Common::BkgdHistName.Data()))};
    }
    else
    {
      fContMap[sample] = {(TH2F*)fInFile->Get(Form("%s",Common::HistNameMap[sample].Data()))};
    }
  }

  // scaling up hists to integral predicted values
  if (fXVarBins || fYVarBins)
  {
    const Bool_t isUp = true;
    for (auto & ContPair : fContMap)
    {
      const auto & sample = ContPair.first;
      auto & info = ContPair.second;

      auto & hist = info.hist;
      Common::Scale(hist,isUp,fXVarBins,fYVarBins);
    }
  }
}

void XContaminationDumper::ComputeDumpInfos()
{
  std::cout << "Computing dump info..." << std::endl;

  // compute intetrals and fractions
  for (const auto & sample : fSampleVec)
  {
    XContaminationDumper::ComputeIntegrals(sample);
    
    // compute fractions for predicted MC
    if ((Common::GroupMap[sample] == SampleGroup::isData) || (Common::IsBkgd(sample))) continue;

    XContaminationDumper::ComputeFractions(sample);
  }
}

void XContaminationDumper::ComputeIntegrals(const TString & sample)
{
  std::cout << "Getting integrals for sample: " << sample.Data() << std::endl;

  // get struct + hist
  auto & info = fContMap[sample];
  auto & hist = info.hist;

  // get grand total
  const auto xmax_full = hist->GetXaxis()->GetNbins();
  const auto ymax_full = hist->GetYaxis()->GetNbins();

  info.int_full = hist->IntegralAndError(1,xmax_full,1,ymax_full,info.int_err_full);

  // tmp variables for computing block integrals + errors
  Double_t hist_int = 0., hist_err = 0., tmp_err = 0.;
    
  // do 1D block range first
  for (const auto & block : fBlocks1D)
  {
    // get bin range
    const auto xmin = hist->GetXaxis()->FindBin((fVar1D == Variable::X) ? block.xlow : std::numeric_limits<float>::lowest());
    const auto xmax = hist->GetXaxis()->FindBin((fVar1D == Variable::X) ? block.xup  : std::numeric_limits<float>::max());
    const auto ymin = hist->GetYaxis()->FindBin((fVar1D == Variable::Y) ? block.xlow : std::numeric_limits<float>::lowest());
    const auto ymax = hist->GetYaxis()->FindBin((fVar1D == Variable::Y) ? block.xup  : std::numeric_limits<float>::max());
    
    // get integrals and errors
    hist_int += hist->IntegralAndError(xmin,xmax,ymin,ymax,tmp_err);
    hist_err += tmp_err*tmp_err;
  }
    
  // set info 1D block
  info.int_1D = hist_int;
  info.int_err_1D = std::sqrt(hist_err);
  
  // reset tmp vars
  hist_int = 0., hist_err = 0., tmp_err = 0.;
  
  // now do 2D block
  for (const auto & block : fBlocks2D)
  {
    // get bin range
    const auto xmin = hist->GetXaxis()->FindBin(block.xlow);
    const auto xmax = hist->GetXaxis()->FindBin(block.xup);
    const auto ymin = hist->GetYaxis()->FindBin(block.ylow);
    const auto ymax = hist->GetYaxis()->FindBin(block.yup);
    
    // get integrals and errors
    hist_int += hist->IntegralAndError(xmin,xmax,ymin,ymax,tmp_err);
    hist_err += tmp_err*tmp_err;
  }
  
  // set info 2D block
  info.int_2D = hist_int;
  info.int_err_2D = std::sqrt(hist_err);
}

void XContaminationDumper::ComputeFractions(const TString & sample)
{
  std::cout << "Getting fractions for sample: " << sample.Data() << std::endl;

  // get structs
  const auto & bkgd = fContMap[Common::BkgdSampleName];
  auto & info = fContMap[sample];

  if (Common::GroupMap[sample] == SampleGroup::isBkgd) // Backgrounds, compute ratios for background predicted as denom
  {
    // full
    info.frac_full = info.int_full / bkgd.int_full;
    info.frac_err_full = info.frac_full * Common::hypot(info.int_err_full/info.int_full,bkgd.int_err_full/bkgd.int_full);

    // 1D block
    info.frac_1D = info.int_1D / bkgd.int_1D;
    info.frac_err_1D = info.frac_1D * Common::hypot(info.int_err_1D/info.int_1D,bkgd.int_err_1D/bkgd.int_1D);
    
    // 2D block
    info.frac_2D = info.int_2D / bkgd.int_2D;
    info.frac_err_2D = info.frac_2D * Common::hypot(info.int_err_2D/info.int_2D,bkgd.int_err_2D/bkgd.int_2D);
  }
  else if (Common::GroupMap[sample] == SampleGroup::isSignal) // MC signals, compute ratios for background + signal predicted as denom
  {
    // full
    info.frac_full = info.int_full / (info.int_full + bkgd.int_full);
    info.frac_err_full = Common::hypot(info.int_err_full*bkgd.int_full,bkgd.int_err_full*info.int_full) / std::pow(info.int_full+bkgd.int_full,2);

    // 1D block
    info.frac_1D = info.int_1D / (info.int_1D + bkgd.int_1D);
    info.frac_err_1D = Common::hypot(info.int_err_1D*bkgd.int_1D,bkgd.int_err_1D*info.int_1D) / std::pow(info.int_1D+bkgd.int_1D,2);

    // 2D block
    info.frac_2D = info.int_2D / (info.int_2D + bkgd.int_2D);
    info.frac_err_2D = Common::hypot(info.int_err_2D*bkgd.int_2D,bkgd.int_err_2D*info.int_2D) / std::pow(info.int_2D+bkgd.int_2D,2);
  }
  else
  {
    std::cerr << "How did this happen?? You are using a sample: " << sample.Data() << " that is not background nor signal! Exiting..." << std::endl;
    exit(1);
  }
}

void XContaminationDumper::DumpTextFile()
{
  std::cout << "Dumping into text file..." << std::endl;
  
  // main file
  std::ofstream outfile(Form("%s.%s",fOutFileText.Data(),Common::outTextExt.Data()),std::ios_base::trunc);
  outfile << "Sample Full Block1D Block2D" << std::endl;

  // loop over samples
  for (const auto & sample : fSampleVec)
  {
    if (Common::GroupMap[sample] == SampleGroup::isSignal) continue;

    // get info and then dump!
    const auto & info = fContMap[sample];

    if ((Common::GroupMap[sample] == SampleGroup::isData) || (Common::IsBkgd(sample)))
    { 
      outfile << sample.Data() << " "
	      << info.int_full << "+/-" << info.int_err_full << " "
	      << info.int_1D << "+/-" << info.int_err_1D << " "
	      << info.int_2D << "+/-" << info.int_err_2D << std::endl;
    }
    else
    {
      outfile << sample.Data() << " "
	      << info.frac_full << "+/-" << info.frac_err_full << " "
	      << info.frac_1D << "+/-" << info.frac_err_1D << " "
	      << info.frac_2D << "+/-" << info.frac_err_2D << std::endl;
    }
  }
}

void XContaminationDumper::FillSignalHists()
{
  std::cout << "Making output signal hist dumps..." << std::endl;

  // loop over grid and set contents
  for (auto ilambda = 0U; ilambda < fLambdas.size(); ilambda++)
  {
    const auto & lambda = fLambdas[ilambda];
    for (auto ictau = 0U; ictau < fCTaus.size(); ictau++)
    {
      const auto & ctau = fCTaus[ictau];

      // get signal info
      const auto & info = fContMap["GMSB_L"+lambda+"_CTau"+ctau];

      // full
      fHistMap["full_int"] ->SetBinContent(ilambda+1,ictau+1,info.int_full);
      fHistMap["full_int"] ->SetBinError  (ilambda+1,ictau+1,info.int_err_full);
      fHistMap["full_frac"]->SetBinContent(ilambda+1,ictau+1,info.frac_full);
      fHistMap["full_frac"]->SetBinError  (ilambda+1,ictau+1,info.frac_err_full);

      // block1D
      fHistMap["block1D_int"] ->SetBinContent(ilambda+1,ictau+1,info.int_1D);
      fHistMap["block1D_int"] ->SetBinError  (ilambda+1,ictau+1,info.int_err_1D);
      fHistMap["block1D_frac"]->SetBinContent(ilambda+1,ictau+1,info.frac_1D);
      fHistMap["block1D_frac"]->SetBinError  (ilambda+1,ictau+1,info.frac_err_1D);

      // block2D
      fHistMap["block2D_int"] ->SetBinContent(ilambda+1,ictau+1,info.int_2D);
      fHistMap["block2D_int"] ->SetBinError  (ilambda+1,ictau+1,info.int_err_2D);
      fHistMap["block2D_frac"]->SetBinContent(ilambda+1,ictau+1,info.frac_2D);
      fHistMap["block2D_frac"]->SetBinError  (ilambda+1,ictau+1,info.frac_err_2D);
    }
  }

  // write it out!
  fOutFile->cd();
  for (const auto & HistPair : fHistMap)
  {
    const auto & hist = HistPair.second;
    hist->Write(hist->GetName(),TObject::kWriteDelete);
  }
}

void XContaminationDumper::PrintSignalHists()
{
  std::cout << "Printing signal hists..." << std::endl;
  
  for (auto & HistPair : fHistMap)
  {
    const auto & key = HistPair.first;
    auto & hist = HistPair.second;
    
    auto canv = new TCanvas();
    canv->cd();
    if (key.Contains("int",TString::kExact)) canv->SetLogz(1);

    hist->Draw("COLZ TEXTE");

    Common::CMSLumi(canv,0,fEra);
    Common::SaveAs(canv,Form("%s_%s",fOutFileText.Data(),key.Data()));

    delete canv;
  }
}

void XContaminationDumper::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  fOutFile->cd();
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));

  // give grand title
  fConfigPave->AddText("***** XContDumper Config *****");

    // Add era info
  Common::AddEraInfoToPave(fConfigPave,fEra);

  // dump config
  Common::AddTextFromInputConfig(fConfigPave,"XContDump Config",fXContDumpConfig);

  // plot config
  Common::AddTextFromInputConfig(fConfigPave,"Plot Config",fPlotConfig);

  // padding
  Common::AddPaddingToPave(fConfigPave,3);

  // dump in old config
  fConfigPave->AddText("***** 2D Hist Input Config *****");
  fConfigPave->AddText(Form("InFile name: %s",fInFileName.Data()));
  Common::AddTextFromInputPave(fConfigPave,fInFile);

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void XContaminationDumper::SetupCommon()
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupHistNames();

  Common::SetupVarBinsBool("x_bins=",fPlotConfig,fXVarBins);
  Common::SetupVarBinsBool("y_bins=",fPlotConfig,fYVarBins);

  // Append Groups
  Common::GroupMap[Common::EWKSampleName]  = SampleGroup::isBkgd;
  Common::GroupMap[Common::BkgdSampleName] = SampleGroup::isBkgd;
}

void XContaminationDumper::SetupXContDumpConfig()
{
  std::cout << "Reading dumper config..." << std::endl;

  std::ifstream infile(Form("%s",fXContDumpConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("1D_var=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"1D_var=");
      Common::SetWhichVar(str,fVar1D);
    }
    else if (str.find("1D_range=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"1D_range=");
      Common::SetupBlockRanges(str,fBlocks1D);
    }
    else if (str.find("2D_range=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"2D_range=");
      Common::SetupBlockRanges(str,fBlocks2D);
    }
    else if (str.find("lambdas=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"lambdas=");
      XContaminationDumper::SetupLambdas(str);
    }
    else if (str.find("ctaus=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"ctaus=");
      XContaminationDumper::SetupCTaus(str);
    }
    else 
    {
      std::cerr << "Aye... your dump config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void XContaminationDumper::SetupLambdas(const std::string & str)
{
  std::stringstream ss(str);
  std::string lambda;
  while (ss >> lambda)
  {
    fLambdas.emplace_back(lambda);
  }
}

void XContaminationDumper::SetupCTaus(const std::string & str)
{
  std::stringstream ss(str);
  std::string ctau;
  while (ss >> ctau)
  {
    fCTaus.emplace_back(ctau);
  }
}

void XContaminationDumper::SetupSampleVec()
{
  std::cout << "Settting up sample vec..." << std::endl;

  // data + bkgd mc
  fSampleVec = {"Data",Common::BkgdSampleName,"QCD","GJets",Common::EWKSampleName};
  
  // signal mc
  for (const auto & lambda : fLambdas)
  {
    for (const auto & ctau : fCTaus)
    {
      fSampleVec.emplace_back("GMSB_L"+lambda+"_CTau"+ctau);
    }
  }
}

void XContaminationDumper::SetupOutSignalHists()
{
  std::cout << "Settting up signal out hists..." << std::endl;

  // integral counts
  fHistMap["full_int"] = XContaminationDumper::MakeSignalHist("num_signal_full","nSignal Full",true);
  fHistMap["block1D_int"] = XContaminationDumper::MakeSignalHist("num_signal_block1D","nSignal 1D Block",true);
  fHistMap["block2D_int"] = XContaminationDumper::MakeSignalHist("num_signal_block2D","nSignal 2D Block",true);

  // fractional S/S+B
  fHistMap["full_frac"] = XContaminationDumper::MakeSignalHist("sig_over_sig_plus_bkgd_full","S/S+B Full",false);
  fHistMap["block1D_frac"] = XContaminationDumper::MakeSignalHist("sig_over_sig_plus_bkgd_block1D","S/S+B 1D Block",false);
  fHistMap["block2D_frac"] = XContaminationDumper::MakeSignalHist("sig_over_sig_plus_bkgd_block2D","S/S+B 2D Block",false);
}

TH2F * XContaminationDumper::MakeSignalHist(const TString & name, const TString & title, const Bool_t isIntegral)
{
  fOutFile->cd();

  auto hist = new TH2F(name.Data(),title.Data(),fLambdas.size(),0,fLambdas.size(),fCTaus.size(),0,fCTaus.size());
  hist->SetMarkerSize(1.8f);
  hist->Sumw2();

  hist->GetXaxis()->SetTitle("#Lambda [TeV]");
  hist->GetYaxis()->SetTitle("c#tau [ns]");

  for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
  {
    hist->GetXaxis()->SetBinLabel(ibinX,fLambdas[ibinX-1]);
  }

  for (auto ibinY = 1; ibinY <= hist->GetYaxis()->GetNbins(); ibinY++)
  {
    hist->GetYaxis()->SetBinLabel(ibinY,fCTaus[ibinY-1]);
  }

  if (isIntegral)
  {
    hist->GetZaxis()->SetTitle("nSignal Events");
    hist->GetZaxis()->SetRangeUser(1e-5,1e5);
  }
  else // fractions
  {
    hist->GetZaxis()->SetTitle("S/S+B");
    hist->GetZaxis()->SetRangeUser(0.f,1.f);
  }

  return hist;
}
