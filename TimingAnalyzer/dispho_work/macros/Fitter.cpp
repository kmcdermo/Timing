// Class include
#include "Fitter.hh"

// ROOT+RooFit includes
#include "TVirtualFitter.h"
#include "RooPlot.h"

Fitter::Fitter(const TString & fitconfig, const TString & outfiletext)
  : fFitConfig(fitconfig), fOutFileText(outfiletext)
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

  // init configuration, set minimizer
  Fitter::SetupDefaultValues();
  Fitter::SetupConfig();
  Fitter::ReadFitConfig();
  Fitter::ReadPlotConfig();
  TVirtualFitter::SetDefaultFitter("Minuit2");
  
  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"RECREATE");

  // setup outtree
  if (fDoFits) Fitter::SetupOutTree();
}

Fitter::~Fitter()
{
  std::cout << "Tidying up in the destructor..." << std::endl;

  delete fConfigPave;

  delete fY;
  delete fYRooBins;

  delete fX;
  delete fXRooBins;

  delete fNPredSign;
  delete fNPredBkgd;
  Fitter::DeleteMap(fFracMap);

  Fitter::DeleteMap(fHistMapY);
  Fitter::DeleteMap(fHistMapX);
  Fitter::DeleteMap(fHistMap2D);

  delete fQCDHistMC_CR;
  delete fGJetsHistMC_CR;
  delete fQCDHistMC_SR;
  delete fGJetsHistMC_SR;
  
  delete fSRFile;
  delete fQCDFile;
  delete fGJetsFile;

  if (fDoFits) delete fOutTree;
  delete fOutFile;
  delete fTDRStyle;
}

void Fitter::DoMain()
{
  std::cout << "In main call..." << std::endl;

  // Get all the variables in place
  Fitter::PrepareCommon();
  
  // Do the fit in 2D
  FitInfo FitInfo2D(RooArgList(*fX,*fY),"2D",TwoD);
  Fitter::PreparePdfs(fHistMap2D,FitInfo2D);
  if (fDoFits) Fitter::MakeFit(FitInfo2D);
  if (fMakeWS) Fitter::ImportToWS(FitInfo2D);
  Fitter::DeleteFitInfo(FitInfo2D);

  // Do the fit in 1D -- X
  FitInfo FitInfoX(RooArgList(*fX),"projX",X);
  Fitter::PreparePdfs(fHistMapX,FitInfoX);
  if (fDoFits) Fitter::MakeFit(FitInfoX);
  if (fMakeWS) Fitter::ImportToWS(FitInfoX);
  Fitter::DeleteFitInfo(FitInfoX);

  // Do the fit in 1D -- Y
  FitInfo FitInfoY(RooArgList(*fY),"projY",Y);
  Fitter::PreparePdfs(fHistMapY,FitInfoY);
  if (fDoFits) Fitter::MakeFit(FitInfoY);
  if (fMakeWS) Fitter::ImportToWS(FitInfoY);
  Fitter::DeleteFitInfo(FitInfoY);

  // Save Output?
  if (fDoFits) Fitter::SaveOutTree();

  // Save MetaData
  Fitter::MakeConfigPave();
}

void Fitter::PrepareCommon()
{
  std::cout << "Preparing common variables and datasets..." << std::endl;

  // Get the input 2D histograms
  Fitter::GetInputHists();

  // Scale data CR to SR via MC SFs
  Fitter::ScaleCRtoSR();

  // Make plots from input hists and dump pre-fit integrals
  Fitter::DumpInputInfo();

  // Project 2D histograms into 1D for later use
  Fitter::Project2DHistTo1D();

  // Get constants/fractions as needed
  Fitter::DeclareCoefficients();

  // Declare variables used in fitting
  Fitter::DeclareXYVars();
}

void Fitter::GetInputHists()
{
  std::cout << "Getting input histograms..." << std::endl;

  // scale up
  const Bool_t isUp = true;

  // GJets CR
  fGJetsFile = TFile::Open(Form("%s",fGJetsFileName.Data()));
  Config::CheckValidFile(fGJetsFile,fGJetsFileName);

  fHistMap2D[GJets] = (TH2F*)fGJetsFile->Get(Form("%s",Config::HistNameMap[Data].Data()));
  fGJetsHistMC_CR   = (TH2F*)fGJetsFile->Get(Form("%s",Config::HistNameMap[GJets].Data()));
  Config::CheckValidTH2F(fHistMap2D[GJets],Config::HistNameMap[Data] ,fGJetsFileName);
  Config::CheckValidTH2F(fGJetsHistMC_CR  ,Config::HistNameMap[GJets],fGJetsFileName);
  Fitter::Scale(fHistMap2D[GJets],isUp);
  Fitter::Scale(fGJetsHistMC_CR,isUp);

  // QCD CR
  fQCDFile = TFile::Open(Form("%s",fQCDFileName.Data()));
  Config::CheckValidFile(fQCDFile,fQCDFileName);

  fHistMap2D[QCD] = (TH2F*)fQCDFile->Get(Form("%s",Config::HistNameMap[Data].Data()));
  fQCDHistMC_CR   = (TH2F*)fQCDFile->Get(Form("%s",Config::HistNameMap[QCD].Data()));
  Config::CheckValidTH2F(fHistMap2D[QCD],Config::HistNameMap[Data],fQCDFileName);
  Config::CheckValidTH2F(fQCDHistMC_CR  ,Config::HistNameMap[QCD] ,fQCDFileName);
  Fitter::Scale(fHistMap2D[QCD],isUp);
  Fitter::Scale(fQCDHistMC_CR,isUp);

  // SR
  fSRFile = TFile::Open(Form("%s",fSRFileName.Data()));
  Config::CheckValidFile(fSRFile,fSRFileName);

  fGJetsHistMC_SR = (TH2F*)fSRFile->Get(Form("%s",Config::HistNameMap[GJets].Data()));
  fQCDHistMC_SR   = (TH2F*)fSRFile->Get(Form("%s",Config::HistNameMap[QCD].Data()));
  Config::CheckValidTH2F(fGJetsHistMC_SR,Config::HistNameMap[GJets],fSRFileName);
  Config::CheckValidTH2F(fQCDHistMC_SR  ,Config::HistNameMap[QCD]  ,fSRFileName);
  Fitter::Scale(fGJetsHistMC_SR,isUp);
  Fitter::Scale(fQCDHistMC_SR,isUp);

  // use signal sample?
  if (!fBkgdOnly) 
  {
    fHistMap2D[GMSB] = (TH2F*)fSRFile->Get(Form("%s",Config::HistNameMap[GMSB].Data()));
    Config::CheckValidTH2F(fHistMap2D[GMSB],Config::HistNameMap[GMSB],fSRFileName);
    Fitter::Scale(fHistMap2D[GMSB],isUp);
  }

  // use real data?
  if (!fGenData)
  {
    fHistMap2D[Data] = (TH2F*)fSRFile->Get(Form("%s",Config::HistNameMap[Data].Data()));
    Config::CheckValidTH2F(fHistMap2D[Data],Config::HistNameMap[Data],fSRFileName);
    Fitter::Scale(fHistMap2D[Data],isUp);
  }
}  

void Fitter::ScaleCRtoSR()
{
  std::cout << "Scaling data hists in CR to SR using MC scale factors..." << std::endl;

  // Create MC SFs
  std::map<SampleType,Float_t> mcSFMap;
  mcSFMap[GJets] = (fGJetsHistMC_SR->Integral())/(fGJetsHistMC_CR->Integral());
  mcSFMap[QCD]   = (fQCDHistMC_SR  ->Integral())/(fQCDHistMC_CR  ->Integral());
  
  // Scale Data CR by MC SFs --> yields SR prediction for each CR
  for (const auto & mcSFPair : mcSFMap)
  {
    const auto & sample = mcSFPair.first;
    const auto & mcsf   = mcSFPair.second;

    fHistMap2D[sample]->Scale(mcsf);
  }
}

void Fitter::DumpInputInfo()
{
  std::cout << "Making quick dump of input histograms..." << std::endl;

  // scale back down temporarily (since already scaled up)
  const Bool_t isUp = false;

  // make tmp canvas
  TCanvas * canv = new TCanvas();

  // GJets integral
  Double_t GJets_Err = 0.;
  const auto GJets_Int = fHistMap2D[GJets]->IntegralAndError(1,fHistMap2D[GJets]->GetXaxis()->GetNbins(),1,fHistMap2D[GJets]->GetYaxis()->GetNbins(),GJets_Err);
  std::cout << "GJets Integral: " << GJets_Int << " +/- " << GJets_Err << std::endl;

  // QCD integral
  Double_t QCD_Err = 0.;
  const Double_t QCD_Int = fHistMap2D[QCD]->IntegralAndError(1,fHistMap2D[QCD]->GetXaxis()->GetNbins(),1,fHistMap2D[QCD]->GetYaxis()->GetNbins(),QCD_Err);
  std::cout << "QCD Integral: " << QCD_Int << " +/- " << QCD_Err << std::endl;

  // Combine Bkgd samples
  auto bkgdHist = (TH2F*)fHistMap2D[GJets]->Clone("BkgdHist");
  bkgdHist->Add(fHistMap2D[QCD]);
  Double_t Bkgd_Err = 0.;
  const auto Bkgd_Int = bkgdHist->IntegralAndError(1,bkgdHist->GetXaxis()->GetNbins(),1,bkgdHist->GetYaxis()->GetNbins(),Bkgd_Err);
  std::cout << "Bkgd Integral: " << Bkgd_Int << " +/- " << Bkgd_Err << std::endl;

  // pretty up and draw bkgd
  Fitter::Scale(bkgdHist,isUp);
  bkgdHist->GetZaxis()->SetTitle("Events/ns/GeV/c");
  bkgdHist->Draw("colz");
  Config::CMSLumi(canv);
  canv->SaveAs("bkgdHist.png");
  delete bkgdHist;
  
  // Clone Signal 
  if (!fBkgdOnly)
  {
    auto signHist = (TH2F*)fHistMap2D[GMSB]->Clone("SignHist");
    Double_t Sign_Err = 0.;
    const auto Sign_Int = signHist->IntegralAndError(1,signHist->GetXaxis()->GetNbins(),1,signHist->GetYaxis()->GetNbins(),Sign_Err);
    std::cout << "Sign Integral: " << Sign_Int << " +/- " << Sign_Err << std::endl;
    
    // pretty up and draw signal
    Fitter::Scale(signHist,isUp);
    signHist->GetZaxis()->SetTitle("Events/ns/GeV/c");
    signHist->Draw("colz");
    Config::CMSLumi(canv);
    canv->SaveAs("signHist.png");
    delete signHist;
  }

  // Data Histogram --> need to blind it if called for
  if (!fGenData)
  {
    auto dataHist = (TH2F*)fHistMap2D[Data]->Clone("DataHist");

    // blinding : FIXME
    for (auto ibinX = 1; ibinX <= dataHist->GetXaxis()->GetNbins(); ibinX++)
    {
      const auto bincenterX = dataHist->GetXaxis()->GetBinCenter(ibinX);
      for (auto ibinY = 1; ibinY <= dataHist->GetYaxis()->GetNbins(); ibinY++)
      {
	const auto bincenterY = dataHist->GetYaxis()->GetBinCenter(ibinY);
	if ((bincenterX > 3) || (bincenterY > 200)) dataHist->SetBinContent(ibinX,ibinY,0);
      }
    }

    Double_t Data_Err = 0.;
    const auto Data_Int = dataHist->IntegralAndError(1,dataHist->GetXaxis()->GetNbins(),1,dataHist->GetYaxis()->GetNbins(),Data_Err);
    std::cout << "Data Integral: " << Data_Int << " +/- " << Data_Err << std::endl;

    // draw and pretty up
    Fitter::Scale(dataHist,isUp);
    dataHist->GetZaxis()->SetTitle("Events/ns/GeV/c");
    dataHist->Draw("colz");
    Config::CMSLumi(canv);
    canv->SaveAs("dataHist.png");
    delete dataHist;
  }

  delete canv;
}

void Fitter::Project2DHistTo1D()
{
  std::cout << "Projecting 2D histograms to 1D..." << std::endl;

  for (auto & HistPair2D : fHistMap2D)
  {
    const auto & sample = HistPair2D.first;
    fHistMapX[sample] = (TH1F*)HistPair2D.second->ProjectionX(Form("%s_projX",Config::HistNameMap[sample].Data()));
    fHistMapY[sample] = (TH1F*)HistPair2D.second->ProjectionY(Form("%s_projY",Config::HistNameMap[sample].Data()));
  }
}

void Fitter::DeclareCoefficients()
{
  std::cout << "Getting integral counts and fractions..." << std::endl;

  // Count up background first
  std::map<SampleType,Float_t> nBkgdMap;
  nBkgdMap[GJets] = fHistMap2D[GJets]->Integral();
  nBkgdMap[QCD]   = fHistMap2D[QCD]  ->Integral();

  fNTotalBkgd = 0.f;
  for (const auto & nBkgdPair : nBkgdMap) fNTotalBkgd += nBkgdPair.second;

  for (auto & nBkgdPair : nBkgdMap)
  {
    const auto & sample = nBkgdPair.first;
    const auto   nbkgd  = nBkgdPair.second;

    const TString name = Form("%s_frac",Config::HistNameMap[sample].Data());
    fFracMap[sample] = new RooRealVar(Form("%s",name.Data()),Form("%s",name.Data()),nbkgd/fNTotalBkgd);
  }

  // Count signal
  fNTotalSign = (fBkgdOnly ? 0.f : fHistMap2D[GMSB]->Integral());

  // make vars for varying extended PDFs
  fNPredBkgd = new RooRealVar("nbkgd","nbkgd",fNTotalBkgd,(fFracLow*fNTotalBkgd),(fFracHigh*fNTotalBkgd));
  fNPredSign = new RooRealVar("nsign","nsign",fNTotalSign,(fFracLow*fNTotalSign),(fFracHigh*fNTotalSign));
}

void Fitter::DeclareXYVars()
{
  std::cout << "Declaring RooFit variables..." << std::endl;

  // Build bins for X
  fXRooBins = new RooBinning(fXBins.front(),fXBins.back(),"X bins");
  for (const auto boundary : fXBins) fXRooBins->addBoundary(boundary);

  // Declare vars and set bins: X
  fX = new RooRealVar("x",Form("%s",fXTitle.Data()),fXBins.front(),fXBins.back());
  fX->setBinning(*fXRooBins);
  
  // Build bins for Y
  fYRooBins = new RooBinning(fYBins.front(),fYBins.back(),"Y bins");
  for (const auto boundary : fYBins) fYRooBins->addBoundary(boundary);

  // Declare vars and set bins: Y
  fY = new RooRealVar("y",Form("%s",fYTitle.Data()),fYBins.front(),fYBins.back());
  fY->setBinning(*fYRooBins);
}

template <typename T>
void Fitter::PreparePdfs(const T & HistMap, FitInfo & fitInfo)
{
  std::cout << "Preparing common pdfs for: " << fitInfo.Text.Data() << std::endl;

  // Declare datasets with input histograms (when using real data as input)
  Fitter::DeclareDatasets(HistMap,fitInfo);

  // Make pdfs from histograms
  Fitter::DeclareSamplePdfs(fitInfo);
}

template <typename T>
void Fitter::DeclareDatasets(const T & HistMap, FitInfo & fitInfo)
{
  std::cout << "Setting datasets for: " << fitInfo.Text.Data() << std::endl;
  
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const auto & hist   = HistPair.second;

    const TString name = Form("%s_RooDataHist_%s",Config::HistNameMap[sample].Data(),fitInfo.Text.Data());
    fitInfo.DataHistMap[sample] = new RooDataHist(Form("%s",name.Data()),Form("%s",name.Data()),fitInfo.ArgList,hist);
  }
}

void Fitter::DeclareSamplePdfs(FitInfo & fitInfo)
{
  std::cout << "Setting Sample Pdfs for: " << fitInfo.Text.Data() << std::endl;

  for (const auto & DataHistPair : fitInfo.DataHistMap)
  {
    // build background and signal pdfs
    const auto & sample   = DataHistPair.first;
    const auto & datahist = DataHistPair.second;
    if (Config::GroupMap[sample] == isData) continue;
    
    const TString name = Form("%s_PDF_%s",Config::HistNameMap[sample].Data(),fitInfo.Text.Data());
    fitInfo.HistPdfMap[sample] = new RooHistPdf(Form("%s",name.Data()),Form("%s",name.Data()),fitInfo.ArgList,*datahist);
  }

  // Build Bkgd-Only Pdfs
  const TString bkgdname = Form("Bkgd_PDF_%s",fitInfo.Text.Data());
  fitInfo.BkgdPdf = new RooAddPdf(Form("%s",bkgdname.Data()),Form("%s",bkgdname.Data()),RooArgList(*fitInfo.HistPdfMap.at(GJets),*fitInfo.HistPdfMap.at(QCD)),RooArgList(*fFracMap.at(GJets),*fFracMap.at(QCD)));
}

void Fitter::MakeFit(FitInfo & fitInfo)
{
  std::cout << "Doing full chain of fit for: " << fitInfo.Text.Data() << std::endl;

  // run n fits
  for (auto ifit = 0; ifit < fNFits; ifit++)
  {
    std::cout << "Working on ifit " << ifit << " of " << fNFits << " for: " << fitInfo.Text.Data() << std::endl;

    // Throw random numbers for new nEvents
    if (fGenData) Fitter::ThrowPoisson(fitInfo);

    // Build Model
    Fitter::BuildModel(fitInfo);

    // Construct dataset from model
    if (fGenData) Fitter::GenerateData(fitInfo);
    
    // Fit Model to Data
    Fitter::FitModel(fitInfo);

    // Get Predicted nEvents
    Fitter::GetPredicted(fitInfo);

    // Draw for ntimes
    if (ifit % (fNFits/fNDraw) == 0)
    {
      // Draw fit(s) in 1D
      if (fitInfo.Fit == TwoD)
      {
	Fitter::DrawFit(fX,Form("%i_xfit",ifit),fitInfo);
	Fitter::DrawFit(fY,Form("%i_yfit",ifit),fitInfo);
      }
      else if (fitInfo.Fit == X)
      {
	Fitter::DrawFit(fX,Form("%i_fit",ifit),fitInfo);
      }
      else if (fitInfo.Fit == Y)
      {
	Fitter::DrawFit(fY,Form("%i_fit",ifit),fitInfo);
      }
      else
      {
	std::cerr << "Not sure how, but you provided an incorrect enum for FitType! Exiting..." << std::endl;
	exit(1);
      }
    }

    // Final Bits for fOutTree
    Fitter::FillOutTree(fitInfo);

    // delete dataset and reset pdf
    Fitter::DeleteModel(fitInfo,ifit); 
  }
}

void Fitter::ThrowPoisson(const FitInfo & fitInfo)
{
  std::cout << "Throwing poisson for generating toy data for: " << fitInfo.Text.Data() << std::endl;

  // generate random poisson number from total
  fNPredBkgd->setVal(gRandom->Poisson(fFracGenBkgd*fNTotalBkgd));
  fNPredSign->setVal(gRandom->Poisson(fFracGenSign*fNTotalSign));

  // for fOutTree
  fNGenBkgd = fNPredBkgd->getVal();
  fNGenSign = fNPredSign->getVal();
}

void Fitter::BuildModel(FitInfo & fitInfo)
{
  std::cout << "Build model for: " << fitInfo.Text.Data() << std::endl;

  // Declare strings for naming pdfs
  const TString ebkgdname = Form("Bkgd_ExtPDF_%s",fitInfo.Text.Data());
  const TString esignname = Form("Sign_ExtPDF_%s",fitInfo.Text.Data());
  const TString modelname = Form("Model_PDF_%s",fitInfo.Text.Data());

  // build extended bkgd first
  fitInfo.BkgdExtPdf = new RooExtendPdf(Form("%s",ebkgdname.Data()),Form("%s",ebkgdname.Data()),*fitInfo.BkgdPdf,*fNPredBkgd);

  // use signal?
  if (!fBkgdOnly)
  {
    fitInfo.SignExtPdf = new RooExtendPdf(Form("%s",esignname.Data()),Form("%s",esignname.Data()),*fitInfo.HistPdfMap.at(GMSB),*fNPredSign);
    fitInfo.ModelPdf = new RooAddPdf(Form("%s",modelname.Data()),Form("%s",modelname.Data()),RooArgList(*fitInfo.BkgdExtPdf,*fitInfo.SignExtPdf));
  }
  else
  {
    fitInfo.ModelPdf = new RooAddPdf(Form("%s",modelname.Data()),Form("%s",modelname.Data()),RooArgList(*fitInfo.BkgdExtPdf));
  }
}

void Fitter::GenerateData(FitInfo & fitInfo)
{
  std::cout << "Generating toy data for: " << fitInfo.Text.Data() << std::endl;

  // generate the data and save to "Data" slot in RooDataHist Map
  fitInfo.DataHistMap[Data] = fitInfo.ModelPdf->generateBinned(fitInfo.ArgList,(fNGenBkgd+fNGenSign));

  // Rename to follow conventions so far
  const TString name = Form("%s_RooDataHist_%s",Config::HistNameMap[Data].Data(),fitInfo.Text.Data());
  fitInfo.DataHistMap[Data]->SetName(Form("%s",name.Data()));
}

void Fitter::FitModel(FitInfo & fitInfo)
{
  std::cout << "Fit model for: " << fitInfo.Text.Data() << std::endl;

  // initialize norms before each fit, i.e. don't cheat!
  fNPredBkgd->setVal(((fFracLow*fNTotalBkgd)+(fFracHigh*fNTotalBkgd))/2.f);
  fNPredSign->setVal(((fFracLow*fNTotalSign)+(fFracHigh*fNTotalSign))/2.f);

  // perform the fit!
  fitInfo.ModelPdf->fitTo(*fitInfo.DataHistMap.at(Data),RooFit::SumW2Error(true));
}

void Fitter::GetPredicted(FitInfo & fitInfo)
{
  std::cout << "Get predicted number of events for: " << fitInfo.Text.Data() << std::endl;

  fNFitBkgd = fNPredBkgd->getVal();
  fNFitBkgdErr = fNPredBkgd->getError();
  fNFitSign = fNPredSign->getVal();
  fNFitSignErr = fNPredSign->getError();
}

void Fitter::DrawFit(RooRealVar *& var, const TString & title, const FitInfo & fitInfo)
{
  std::cout << "Draw fits projected into 1D for: " << fitInfo.Text.Data() << std::endl;
  
  // which variable?
  const TString varname = var->GetName();
  const Bool_t isX = varname.EqualTo("x",TString::kExact);
  const Bool_t isY = varname.EqualTo("y",TString::kExact);

  // rescale as needed
  const Bool_t rescale = (isX && fXVarBins) || (isY && fYVarBins);
  const Bool_t isUp    = false;

  // Draw 1D frame -- temporary
  auto plot = var->frame();
  
  // Data Hist Info
  auto & datahist = fitInfo.DataHistMap.at(Data);
  const TString tmpname = Form("%s_TmpPlot",datahist->GetName());

  // Blind the data!
  if      (isX) datahist->plotOn(plot,RooFit::Name(Form("%s",tmpname.Data())),RooFit::Cut(Form("%s",fXCut.Data())),RooFit::DrawOption("PZ"),RooFit::DataError(RooAbsData::SumW2));
  else if (isY) datahist->plotOn(plot,RooFit::Name(Form("%s",tmpname.Data())),RooFit::Cut(Form("%s",fYCut.Data())),RooFit::DrawOption("PZ"),RooFit::DataError(RooAbsData::SumW2));
  else 
  {
    std::cerr << "How did this happen?? Specified a variable that is not predefined... exiting.." << std::endl;
    exit(1);
  }

  // Get and Set TGraph of Data
  auto tmpDataGraph = (TGraphAsymmErrors*)plot->findObject(Form("%s",tmpname.Data()));
  auto dataGraph    = (TGraphAsymmErrors*)tmpDataGraph->Clone(Form("%s_Graph",datahist->GetName()));

  if (rescale)
  {
    const auto & bins = (isX ? fXBins : fYBins);
    Fitter::Scale(dataGraph,bins,isUp);
  }
  
  // delete plotOn --> will not use to draw, also deletes tmpDataGraph
  delete plot;

  // Setup PDF fit plot
  TH1F * modelHist;
  TH1F * bkgdHist;
  TH1F * signHist = nullptr;

  // Set names of plots
  const TString modelname = Form("%s_Fit_Hist",fitInfo.ModelPdf  ->GetName());
  const TString bkgdname  = Form("%s_Fit_Hist",fitInfo.BkgdExtPdf->GetName());
  const TString signname  = ((!fBkgdOnly) ? Form("%s_Fit_Hist",fitInfo.SignExtPdf->GetName()) : "");

  // Get the plots
  if      ((fitInfo.Fit == X) || (fitInfo.Fit == Y))
  {
    // create histograms straight from pdfs (norms already taken care of!)
    modelHist = (TH1F*)fitInfo.ModelPdf  ->createHistogram(Form("%s",modelname.Data()),*var);
    bkgdHist  = (TH1F*)fitInfo.BkgdExtPdf->createHistogram(Form("%s",bkgdname .Data()),*var);
    if (!fBkgdOnly) signHist = (TH1F*)fitInfo.SignExtPdf->createHistogram(Form("%s",signname.Data()),*var);
  }
  else if (fitInfo.Fit == TwoD)
  {
    // get other var
    auto & projvar = (isX ? fY : fX);

    // Project out in other variables first
    RooAbsPdf * model1D = fitInfo.ModelPdf  ->createProjection(*projvar);
    RooAbsPdf * bkgd1D  = fitInfo.BkgdExtPdf->createProjection(*projvar);
    RooAbsPdf * sign1D  = nullptr;
    if (!fBkgdOnly) sign1D = fitInfo.SignExtPdf->createProjection(*projvar);

    // Now make the histograms
    modelHist = (TH1F*)model1D->createHistogram(Form("%s",modelname.Data()),*var);
    bkgdHist  = (TH1F*)bkgd1D ->createHistogram(Form("%s",bkgdname .Data()),*var);
    if (!fBkgdOnly) signHist = (TH1F*)sign1D->createHistogram(Form("%s",signname.Data()),*var);

    // Rescale (down) as necessary
    if (rescale)
    {
      Fitter::Scale(modelHist,isUp);
      Fitter::Scale(bkgdHist,isUp);
      if (!fBkgdOnly) Fitter::Scale(signHist,isUp);
    }

    // Normalize
    const auto nFitTotal = fNFitBkgd + fNFitSign;
    modelHist->Scale(rescale ? nFitTotal/modelHist->Integral("widths") : nFitTotal);
    bkgdHist ->Scale(rescale ? fNFitBkgd/bkgdHist ->Integral("widths") : fNFitBkgd);
    if (!fBkgdOnly) signHist->Scale(rescale ? fNFitSign/signHist->Integral("widths") : fNFitSign);
     
    // delete projections
    if (!fBkgdOnly) delete sign1D;
    delete bkgd1D;
    delete model1D;
  }
  else
  {
    std::cerr << "How did this happen?? Specified a fit that is not predefined... exiting.." << std::endl;
    exit(1);
  }

  // decorate and draw PDFs
  modelHist->SetLineColor(kBlue);
  bkgdHist ->SetLineColor(kRed);
  if (!fBkgdOnly) signHist->SetLineColor(kGreen);
  modelHist->SetLineWidth(2);

  // Get canvas and draw
  auto canv = new TCanvas();
  canv->cd();

  // draw PDF first
  modelHist->Draw("hist");
  bkgdHist ->Draw("hist same");
  if (!fBkgdOnly) signHist->Draw("hist same");
  dataGraph->Draw("PZ same");

  // add legend
  auto leg = new TLegend(0.682,0.7,0.825,0.92);
  leg->AddEntry(dataGraph,"Toy Data","epl");
  leg->AddEntry(modelHist,"Norm. Fit","l");
  leg->AddEntry(bkgdHist,"Norm. Bkgd","l");
  if (!fBkgdOnly) leg->AddEntry(signHist,"Norm. Sign","l");
  leg->Draw("same");

  // get min/max
  const auto min = Fitter::GetMinimum(dataGraph,bkgdHist,signHist);
  const auto max = Fitter::GetMaximum(dataGraph,modelHist);

  // make the range nice and save (LIN)
  modelHist->SetMinimum(min/1.05);
  modelHist->SetMaximum(max*1.05);
  canv->SetLogy(false);
  Config::CMSLumi(canv);
  canv->SaveAs(Form("%s_%s_lin.png",title.Data(),fitInfo.Text.Data()));

  // make the range nice and save (LOG)
  modelHist->SetMinimum(min/1.5);
  modelHist->SetMaximum(max*1.5);
  canv->SetLogy(true);
  Config::CMSLumi(canv);
  canv->SaveAs(Form("%s_%s_log.png",title.Data(),fitInfo.Text.Data()));

  // delete the rest
  delete leg;
  delete canv;
  if (!fBkgdOnly) delete signHist;
  delete bkgdHist;
  delete modelHist;
  delete dataGraph;
}

void Fitter::FillOutTree(const FitInfo & fitInfo)
{
  std::cout << "Fill last bits for outtree for: " << fitInfo.Text.Data() << std::endl;

  fFitID = fitInfo.Text.Data();
  fOutTree->Fill();
}

void Fitter::DeleteModel(FitInfo & fitInfo, const Int_t ifit)
{
  std::cout << "Deleting model info for: " << fitInfo.Text.Data() << std::endl;      

  delete fitInfo.BkgdExtPdf;
  if (!fBkgdOnly) delete fitInfo.SignExtPdf;
  delete fitInfo.ModelPdf;

  if (fGenData && (ifit != (fNFits - 1))) delete fitInfo.DataHistMap[Data];
}

void Fitter::ImportToWS(FitInfo & fitInfo)
{
  std::cout << "Make workspace for " << fitInfo.Text.Data() << std::endl;

  // make new workspace
  auto workspace = new RooWorkspace(Form("workspace_%s",fitInfo.Text.Data()),Form("workspace_%s",fitInfo.Text.Data()));

  // give meaningful names first
  const TString bkgdname = "BkgdPDF";
  const TString signname = "SignPDF";

  // change names as needed
  fitInfo.BkgdPdf->SetName(Form("%s",bkgdname.Data()));
  fitInfo.HistPdfMap[GMSB]->SetName(Form("%s",signname.Data()));
  fNPredBkgd->SetName(Form("%s_norm",bkgdname.Data()));
  fNPredSign->SetName(Form("%s_norm",signname.Data()));

  // Set values to generic expectationss
  fNPredBkgd->setVal(fNTotalBkgd);
  fNPredSign->setVal(fNTotalSign);

  // import into workspace
  workspace->import(*fitInfo.BkgdPdf);
  workspace->import(*fNPredBkgd);
  workspace->import(*fitInfo.HistPdfMap[GMSB]);
  workspace->import(*fNPredSign);
  workspace->import(*fitInfo.DataHistMap.at(Data));

  // write it out
  fOutFile->cd();
  workspace->Write(workspace->GetName(),TObject::kWriteDelete);

  // now delete it!
  delete workspace;
}

void Fitter::SaveOutTree()
{
  std::cout << "Writing fOutTree..." << std::endl;

  fOutFile->cd();
  fOutTree->Write(fOutTree->GetName(),TObject::kWriteDelete);
}

void Fitter::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave
  fConfigPave = new TPaveText();
  fConfigPave->SetName("Config");
  std::string str; // tmp string
  
  // plot config first
  fConfigPave->AddText("Plot Config");
  std::ifstream plotfile(Form("%s",fPlotConfig.Data()),std::ios::in);
  while (std::getline(plotfile,str))
  {
    fConfigPave->AddText(str.c_str());
  }

  // fit config second
  fConfigPave->AddText("Fit Config");
  std::ifstream fitfile(Form("%s",fFitConfig.Data()),std::ios::in);
  while (std::getline(fitfile,str))
  {
    fConfigPave->AddText(str.c_str());
  }

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void Fitter::SetupDefaultValues()
{
  std::cout << "Setting defaults..." << std::endl;
  
  fBkgdOnly = false;
  fGenData  = false;
  fDoFits   = false;
  fMakeWS   = false;

  fNFits = 1;
  fNDraw = 100;
  fFracGenBkgd = 1;
  fFracGenSign = 1;
  fFracLow = 0;
  fFracHigh = 100;
}

void Fitter::SetupConfig()
{
  std::cout << "Setting up config..." << std::endl;

  Config::SetupGroups();
  Config::SetupHistNames();
}

void Fitter::ReadFitConfig()
{
  std::cout << "Reading fit config..." << std::endl;

  std::ifstream infile(Form("%s",fFitConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("CR_GJets_in=") != std::string::npos)
    {
      fGJetsFileName = Config::RemoveDelim(str,"CR_GJets_in=");
    }
    else if (str.find("CR_QCD_in=") != std::string::npos)
    {
      fQCDFileName = Config::RemoveDelim(str,"CR_QCD_in=");
    }
    else if (str.find("SR_in=") != std::string::npos)
    {
      fSRFileName = Config::RemoveDelim(str,"SR_in=");
    }
    else if (str.find("plot_config=") != std::string::npos)
    {
      fPlotConfig = Config::RemoveDelim(str,"plot_config=");
    }
    else if (str.find("frac_low=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"frac_low=");
      fFracLow = std::atof(str.c_str());
    }
    else if (str.find("frac_high=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"frac_high=");
      fFracHigh = std::atof(str.c_str());
    }
    else if (str.find("x_cut=") != std::string::npos)
    {
      fXCut = Config::RemoveDelim(str,"x_cut=");
    }
    else if (str.find("y_cut=") != std::string::npos)
    {
      fYCut = Config::RemoveDelim(str,"y_cut=");
    }
    else if (str.find("bkgd_only=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"bkgd_only=");
      Config::SetupBool(str,fBkgdOnly);
    }
    else if (str.find("gen_data=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"gen_data=");
      Config::SetupBool(str,fGenData);
    }
    else if (str.find("frac_gen_bkgd=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"frac_gen_bkgd=");
      fFracGenBkgd = std::atof(str.c_str());
    }
    else if (str.find("frac_gen_sign=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"frac_gen_sign=");
      fFracGenSign = std::atof(str.c_str());
    }
    else if (str.find("make_ws=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"make_ws=");
      Config::SetupBool(str,fMakeWS);
    }
    else if (str.find("do_fits=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"do_fits=");
      Config::SetupBool(str,fDoFits);
    }
    else if (str.find("n_fits=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"n_fits=");
      fNFits = std::atoi(str.c_str());
    }
    else if (str.find("n_draw=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"n_draw=");
      fNDraw = std::atoi(str.c_str());
    }
    else 
    {
      std::cerr << "Aye... your fit config is messed up, try again! Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void Fitter::ReadPlotConfig()
{
  std::cout << "Reading plot config..." << std::endl;

  std::ifstream infile(Form("%s",fPlotConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("x_title=") != std::string::npos)
    {
      fXTitle = Config::RemoveDelim(str,"x_title=");
    }
    else if (str.find("x_bins=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"x_bins=");
      Config::SetupBins(str,fXBins,fXVarBins);
    }
    else if (str.find("y_title=") != std::string::npos)
    {
      fYTitle = Config::RemoveDelim(str,"y_title=");
    }
    else if (str.find("y_bins=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"y_bins=");
      Config::SetupBins(str,fYBins,fYVarBins);
    }
    else if ((str.find("plot_title=") != std::string::npos) ||
	     (str.find("x_var=") != std::string::npos)      ||
	     (str.find("x_labels=") != std::string::npos)   ||
	     (str.find("y_var=") != std::string::npos)      ||
	     (str.find("y_labels=") != std::string::npos)   ||
	     (str.find("z_title=") != std::string::npos))
    {
      continue;
    }
    else 
    {
      std::cerr << "Aye... your plot config is messed up, try again!" << std::endl;
      exit(1);
    }
  }
}

void Fitter::SetupOutTree()
{
  std::cout << "Setting up outtree..." << std::endl;

  fOutFile->cd();
  fOutTree = new TTree("exptree","Experiment Tree");
  
  fOutTree->Branch("nNFits",fNFits);
  fOutTree->Branch("nTotalBkgd",&fNTotalBkgd);
  fOutTree->Branch("nTotalSign",&fNTotalSign);
  fOutTree->Branch("fracGenBkgd",&fFracGenBkgd);
  fOutTree->Branch("fracGenSign",&fFracGenSign);
  fOutTree->Branch("nGenBkgd",&fNGenBkgd);
  fOutTree->Branch("nGenSign",&fNGenSign);
  fOutTree->Branch("nFitBkgd",&fNFitBkgd);
  fOutTree->Branch("nFitBkgdErr",&fNFitBkgdErr);
  fOutTree->Branch("nFitSign",&fNFitSign);
  fOutTree->Branch("nFitSignErr",&fNFitSignErr);
  fOutTree->Branch("fitID",&fFitID);
}

void Fitter::Scale(TH2F *& hist, const Bool_t isUp)
{
  std::cout << "Scaling " << (isUp?"up":"down") << " hist: " << hist->GetName() << std::endl;

  for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
  {
    const auto binwidthX = hist->GetXaxis()->GetBinWidth(ibinX);
    for (auto ibinY = 1; ibinY <= hist->GetYaxis()->GetNbins(); ibinY++)
    {
      const auto binwidthY = hist->GetYaxis()->GetBinWidth(ibinY);

      // get multiplier/divisor
      auto multiplier = 1.f;      
      if (fXVarBins) multiplier *= binwidthX;
      if (fYVarBins) multiplier *= binwidthY;

      // get content/error
      auto content = hist->GetBinContent(ibinX,ibinY);
      auto error   = hist->GetBinError  (ibinX,ibinY);

      // scale it
      if (isUp)
      {
	content *= multiplier;
	error   *= multiplier;
      }
      else
      {
	content /= multiplier;
	error   /= multiplier;
      }

      // set new contents
      hist->SetBinContent(ibinX,ibinY,content);
      hist->SetBinError  (ibinX,ibinY,error);
    }
  }
}

void Fitter::Scale(TGraphAsymmErrors *& graph, const std::vector<Double_t> & bins, const Bool_t isUp)
{
  std::cout << "Scaling " << (isUp?"up":"down") << " graph: " << graph->GetName() << std::endl;

  for (UInt_t i = 0; i < bins.size()-1; i++)
  {
    // get width
    const auto multiplier = bins[i+1]-bins[i];
    
    // get contents + errors
    Double_t xval,yval;
    graph->GetPoint(i,xval,yval);
    auto yerrl = graph->GetErrorYlow (i);
    auto yerrh = graph->GetErrorYhigh(i);

    // scale up or down
    if (isUp)
    {
      yval  *= multiplier;
      yerrl *= multiplier;
      yerrh *= multiplier;
    }
    else
    {
      yval  /= multiplier;
      yerrl /= multiplier;
      yerrh /= multiplier;
    }

    // set point with new values
    graph->SetPoint(i,xval,yval);
    graph->SetPointEYlow (i,yerrl);
    graph->SetPointEYhigh(i,yerrh);
  }
}

void Fitter::Scale(TH1F *& hist, const Bool_t isUp)
{
  std::cout << "Scaling " << (isUp?"up":"down") << " hist: " << hist->GetName() << std::endl;

  for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
  {
    // get width
    const auto multiplier = hist->GetXaxis()->GetBinWidth(ibinX);
    
    // get content and errors
    auto content = hist->GetBinContent(ibinX);
    auto error   = hist->GetBinError  (ibinX);

    // scale up or down
    if (isUp)
    {
      content *= multiplier;
      error   *= multiplier;
    }
    else
    {
      content /= multiplier;
      error   /= multiplier;
    }

    // set values
    hist->SetBinContent(ibinX,content);
    hist->SetBinError  (ibinX,error);
  }
}

Float_t Fitter::GetMinimum(TGraphAsymmErrors *& graph, TH1F *& hist1, TH1F *& hist2)
{
  std::cout << "Getting minimum for plot..." << std::endl;

  auto min = 1e9;

  // need to loop through to check bin != 0
  if (hist1 != (TH1F*)NULL)
  {
    for (auto bin = 1; bin <= hist1->GetNbinsX(); bin++)
    {
      const auto tmpmin = hist1->GetBinContent(bin);
      if ((tmpmin < min) && (tmpmin > 0)) min = tmpmin;
    }
  }

  // need to loop through to check bin != 0
  if (hist2 != (TH1F*)NULL)
  {
    for (auto bin = 1; bin <= hist2->GetNbinsX(); bin++)
    {
      const auto tmpmin = hist2->GetBinContent(bin);
      if ((tmpmin < min) && (tmpmin > 0)) min = tmpmin;
    }
  }

  // need to loop through to check point yval != 0
  for (auto point = 0; point <= graph->GetN(); point++)
  {
    Double_t xtmpmin,ytmpmin;
    graph->GetPoint(point,xtmpmin,ytmpmin);
    if ((ytmpmin < min) && (ytmpmin > 0)) min = ytmpmin;
  }

  return min;
}

Float_t Fitter::GetMaximum(TGraphAsymmErrors *& graph, TH1F *& hist)
{
  std::cout << "Getting maximum for plot..." << std::endl;

  const auto histmax  = hist->GetBinContent(hist->GetMaximumBin());
  const auto graphmax = graph->GetMaximum();
  return (histmax > graphmax ? histmax : graphmax);
}

void Fitter::DeleteFitInfo(FitInfo & fitInfo)
{
  std::cout << "Deleting remaining bits of fitInfo: " << fitInfo.Text.Data() << std::endl;
  
  Fitter::DeleteMap(fitInfo.DataHistMap);
  Fitter::DeleteMap(fitInfo.HistPdfMap);

  delete fitInfo.BkgdPdf;
}

template <typename T>
void Fitter::DeleteMap(T & Map)
{
  for (auto & Pair : Map) delete Pair.second;
  Map.clear();			  
}
