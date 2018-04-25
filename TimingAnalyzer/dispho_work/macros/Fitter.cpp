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
  Fitter::SetupDefaultBools();
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
  delete fX;

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

  // Project 2D histograms into 1D for later use
  Fitter::Project2DHistTo1D();

  // Get constants/fractions as needed
  Fitter::GetCoefficients();

  // Declare variables used in fitting
  Fitter::DeclareXYVars();
}

void Fitter::GetInputHists()
{
  std::cout << "Getting input histograms..." << std::endl;

  // GJets CR
  fGJetsFile = TFile::Open(Form("%s",fGJetsFileName.Data()));
  Config::CheckValidFile(fGJetsFile,fGJetsFileName);

  fHistMap2D[GJets] = (TH2F*)fGJetsFile->Get(Form("%s",Config::HistNameMap[Data].Data()));
  fGJetsHistMC_CR   = (TH2F*)fGJetsFile->Get(Form("%s",Config::HistNameMap[GJets].Data()));
  Config::CheckValidTH2F(fHistMap2D[GJets],Config::HistNameMap[Data] ,fGJetsFileName);
  Config::CheckValidTH2F(fGJetsHistMC_CR  ,Config::HistNameMap[GJets],fGJetsFileName);
  Fitter::ScaleUp(fHistMap2D[GJets]);
  Fitter::ScaleUp(fGJetsHistMC_CR);

  // QCD CR
  fQCDFile = TFile::Open(Form("%s",fQCDFileName.Data()));
  Config::CheckValidFile(fQCDFile,fQCDFileName);

  fHistMap2D[QCD] = (TH2F*)fQCDFile->Get(Form("%s",Config::HistNameMap[Data].Data()));
  fQCDHistMC_CR   = (TH2F*)fQCDFile->Get(Form("%s",Config::HistNameMap[QCD].Data()));
  Config::CheckValidTH2F(fHistMap2D[QCD],Config::HistNameMap[Data],fQCDFileName);
  Config::CheckValidTH2F(fQCDHistMC_CR  ,Config::HistNameMap[QCD] ,fQCDFileName);
  Fitter::ScaleUp(fHistMap2D[QCD]);
  Fitter::ScaleUp(fQCDHistMC_CR);

  // SR
  fSRFile = TFile::Open(Form("%s",fSRFileName.Data()));
  Config::CheckValidFile(fSRFile,fSRFileName);

  fGJetsHistMC_SR = (TH2F*)fSRFile->Get(Form("%s",Config::HistNameMap[GJets].Data()));
  fQCDHistMC_SR   = (TH2F*)fSRFile->Get(Form("%s",Config::HistNameMap[QCD].Data()));
  Config::CheckValidTH2F(fGJetsHistMC_SR,Config::HistNameMap[GJets],fSRFileName);
  Config::CheckValidTH2F(fQCDHistMC_SR  ,Config::HistNameMap[QCD]  ,fSRFileName);
  Fitter::ScaleUp(fGJetsHistMC_SR);
  Fitter::ScaleUp(fQCDHistMC_SR);

  // use signal sample?
  if (!fBkgdOnly) 
  {
    fHistMap2D[GMSB] = (TH2F*)fSRFile->Get(Form("%s",Config::HistNameMap[GMSB].Data()));
    Config::CheckValidTH2F(fHistMap2D[GMSB],Config::HistNameMap[GMSB],fSRFileName);
    Fitter::ScaleUp(fHistMap2D[GMSB]);
  }

  // use real data?
  if (!fGenData)
  {
    fHistMap2D[Data] = (TH2F*)fSRFile->Get(Form("%s",Config::HistNameMap[Data].Data()));
    Config::CheckValidTH2F(fHistMap2D[Data],Config::HistNameMap[Data],fSRFileName);
    Fitter::ScaleUp(fHistMap2D[Data]);
  }
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

void Fitter::GetCoefficients()
{
  std::cout << "Getting integral counts and fractions..." << std::endl;

  // Count up background first
  std::map<SampleType,Float_t> nBkgdMap;
  nBkgdMap[GJets] = ((fHistMap2D[GJets]->Integral())*(fGJetsHistMC_SR->Integral()))/(fGJetsHistMC_CR->Integral());
  nBkgdMap[QCD]   = ((fHistMap2D[QCD]  ->Integral())*(fQCDHistMC_SR  ->Integral()))/(fQCDHistMC_CR  ->Integral());
 
  fNBkgdTotal = 0.f;
  for (const auto & nBkgdPair : nBkgdMap) fNBkgdTotal += nBkgdPair.second;

  for (auto & nBkgdPair : nBkgdMap)
  {
    const auto & sample = nBkgdPair.first;
    const auto   nbkgd  = nBkgdPair.second;

    const TString name = Form("%s_frac",Config::HistNameMap[sample].Data());
    fFracMap[sample] = new RooRealVar(Form("%s",name.Data()),Form("%s",name.Data()),nbkgd/fNBkgdTotal);
  }

  // Count signal
  fNSignTotal = (fBkgdOnly ? 0.f : fHistMap2D[GMSB]->Integral());

  // make vars for varying extended PDFs
  fNPredBkgd = new RooRealVar("nbkgd","nbkgd",fNBkgdTotal,(fFracLow*fNBkgdTotal),(fFracHigh*fNBkgdTotal));
  fNPredSign = new RooRealVar("nsign","nsign",fNSignTotal,(fFracLow*fNSignTotal),(fFracHigh*fNSignTotal));
}

void Fitter::DeclareXYVars()
{
  std::cout << "Declaring RooFit variables..." << std::endl;

  fX = new RooRealVar("x",Form("%s",fXTitle.Data()),fXBins.front(),fXBins.back());
  fY = new RooRealVar("y",Form("%s",fYTitle.Data()),fYBins.front(),fYBins.back());
}

template <typename T>
void Fitter::PreparePdfs(const T & HistMap, FitInfo & fitInfo)
{
  std::cout << "Preparing common pdfs for: " << fitInfo.Text.Data() << std::endl;

  // Declare datasets with input histograms (when using real data as input)
  Fitter::DeclareDatasets(HistMap,fitInfo);

  // Make pdfs from histograms
  Fitter::MakeSamplePdfs(fitInfo);
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

void Fitter::MakeSamplePdfs(FitInfo & fitInfo)
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
    Fitter::FillOutTree();

    // delete dataset and reset pdf
    Fitter::DeleteModel(fitInfo); 
  }
}

void Fitter::ThrowPoisson(const FitInfo & fitInfo)
{
  std::cout << "Throwing poisson for generating toy data for: " << fitInfo.Text.Data() << std::endl;

  // generate random poisson number from total
  fNPredBkgd->setVal(gRandom->Poisson(fFracGen*fNBkgdTotal));
  fNPredSign->setVal(gRandom->Poisson(fFracGen*fNSignTotal));

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
    fitInfo.SignExPdf = new RooExtendPdf(Form("%s",esignname.Data()),Form("%s",esignname.Data()),*fitInfo.HistPdfMap.at(GMSB),*fNPredSign);
    fitInfo.ModelPdf = new RooAddPdf(Form("%s",modelname.Data()),Form("%s",modelname.Data()),RooArgList(*fitInfo.BkgdExtPdf,*fitInfo.SignExtPdf));
  }
  else
  {
    fitInfo.ModelPdf = new RooAddPdf(Form("%s",modelname.Data()),Form("%s",modelname.Data()),RooArgList(*fitInfo.BkgdExtPdf));
  }
}

void Fitter::GenerateData(FitInfo & fitInfo)
{
  std::cout << "Generating Asimov data for: " << fitInfo.Text.Data() << std::endl;

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
  fNPredBkgd->setVal(((fFracLow*fNBkgdTotal)+(fFracHigh*fNBkgdTotal))/2.f);
  fNPredSign->setVal(((fFracLow*fNSignTotal)+(fFracHigh*fNSignTotal))/2.f);

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
    Fitter::ScaleDown(dataGraph,bins);
  }
  
  // delete plotOn --> will not use to draw, also deletes tmpDataGraph
  delete plot;

  // Setup PDF fit plot
  TH1F * modelHist;
  TH1F * bkgdHist;
  TH1F * signHist;

  // Set names of plots
  const TString modelname = Form("%s_Fit_Hist",fitInfo.ModelPdf  ->GetName());
  const TString bkgdname  = Form("%s_Fit_Hist",fitInfo.BkgdExtPdf->GetName());
  const TString signname  = Form("%s_Fit_Hist",fitInfo.SignExtPdf->GetName());

  // Get the plots
  if      ((fitInfo.Fit == X) || (fitInfo.Fit == Y))
  {
    // create histograms straight from pdfs
    modelHist = (TH1F*)fitInfo.ModelPdf  ->createHistogram(Form("%s",modelname.Data()),*var);
    bkgdHist  = (TH1F*)fitInfo.BkgdExtPdf->createHistogram(Form("%s",bkgdname .Data()),*var);
    signHist  = (TH1F*)fitInfo.SignExtPdf->createHistogram(Form("%s",signname .Data()),*var);

    // scale to actual value after the fit
    bkgdHist->Scale(fNFitBkgd/bkgdHist->Integral("widths"));
    signHist->Scale(fNFitSign/signHist->Integral("widths"));
  }
  else if (fitInfo.Fit == TwoD)
  {
    // get other var
    auto & projvar = (isX ? fY : fX);

    // Project out in other variables first
    RooAbsPdf * model1D = fitInfo.ModelPdf->createProjection(*projvar);
    RooAbsPdf * bkgd1D  = fitInfo.BkgdEPdf->createProjection(*projvar);
    RooAbsPdf * sign1D  = fitInfo.SignEPdf->createProjection(*projvar);

    // Now make the histograms
    modelHist = (TH1F*)model1D->createHistogram(Form("%s",modelname.Data()),*var);
    bkgdHist  = (TH1F*)bkgd1D ->createHistogram(Form("%s",bkgdname .Data()),*var);
    signHist  = (TH1F*)sign1D ->createHistogram(Form("%s",signname .Data()),*var);

    // Rescale as necessary
    if (rescale)
    {
      Fitter::ScaleDown(modelHist);
      Fitter::ScaleDown(bkgdHist);
      Fitter::ScaleDown(signHist);
    }

    // Normalize
    const auto nFitTotal = fNFitBkgd + fNFitSign;
    modelHist->Scale(rescale ? nFitTotal/modelHist->Integral("widths") : nFitTotal);
    bkgdHist ->Scale(rescale ? fNFitBkgd/modelHist->Integral("widths") : fNFitBkgd);
    modelHist->Scale(rescale ? fNFitSign/modelHist->Integral("widths") : fNFitSign);

    // delete projections
    delete sign1D;
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
  signHist ->SetLineColor(kGreen);
  modelHist->SetLineWidth(2);

  // Get canvas and draw
  auto canv = new TCanvas();
  canv->cd();

  // draw PDF first
  modelHist->Draw("hist");
  bkgdHist ->Draw("hist same");
  signHist ->Draw("hist same");
  dataGraph->Draw("PZ same");

  // add legend
  auto leg = new TLegend(0.682,0.7,0.825,0.92);
  leg->AddEntry(dataGraph,"Toy Data","epl");
  leg->AddEntry(modelHist,"Norm. Fit","l");
  leg->AddEntry(bkgdHist,"Norm. Bkgd","l");
  leg->AddEntry(signHist,"Norm. Sign","l");
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
  delete signHist;
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

void Fitter::DeleteModel(FitInfo & fitInfo)
{
  std::cout << "Deleting model info for: " << fitInfo.Text.Data() << std::endl;      

  delete fitInfo.BkgdExtPdf;
  delete fitInfo.SignExtPdf;
  delete fitInfo.ModelPdf;

  if (fGenData) delete fitInfo.DataHistMap[Data];
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
  fitInfo.SignPdf->SetName(Form("%s",signname.Data()));
  fNPredBkgd->SetName(Form("%s_norm",bkgdname.Data()));
  fNPredSign->SetName(Form("%s_norm",signname.Data()));

  // Set values to generic expectationss
  fNPredBkgd->setVal(fNBkgdTotal);
  fNPredSign->setVal(fNSignTotal);

  // import into workspace
  workspace->import(*fitInfo.BkgdPdf);
  workspace->import(*fNPredBkgd);
  workspace->import(*fitInfo.SignPdf);
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
  std::cout << "Setting up config..." << std::endl;

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
  fFracGen = 1;
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
    else if (str.find("frac_gen=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"frac_gen=");
      fFracGen = std::atof(str.c_str());
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
	     (str.find("y_labels=") != std::string::npos))
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
  fOutTree->Branch("nBkgdTotal",&fNBkgdTotal);
  fOutTree->Branch("nSignTotal",&fNSignTotal);
  fOutTree->Branch("fracGen",&fFracGen);
  fOutTree->Branch("nGenBkgd",&fNGenBkgd);
  fOutTree->Branch("nGenSign",&fNGenSign);
  fOutTree->Branch("nFitBkgd",&fNFitBkgd);
  fOutTree->Branch("nFitBkgdErr",&fNFitBkgdErr);
  fOutTree->Branch("nFitSign",&fNFitSign);
  fOutTree->Branch("nFitSignErr",&fNFitSignErr);
  fOutTree->Branch("fitID",&fFitID);
}

void Fitter::ScaleUp(TH2F *& hist)
{
  std::cout << "Scaling up hist: " << hist->GetName() << std::endl;

  for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
  {
    const auto binwidthX = hist->GetXaxis()->GetBinWidth(ibinX);
    for (auto ibinY = 1; ibinY <= hist->GetYaxis()->GetNbins(); ibinY++)
    {
      const auto binwidthY = hist->GetYaxis()->GetBinWidth(ibinY);

      auto multiplier = 1.f;      
      if (fXVarBins) multiplier *= binwidthX;
      if (fYVarBins) multiplier *= binwidthY;

      hist->SetBinContent(ibinX,ibinY,(hist->GetBinContent(ibinX,ibinY))*multiplier);
      hist->SetBinError  (ibinX,ibinY,(hist->GetBinError  (ibinX,ibinY))*multiplier);
    }
  }
}

void Fitter::ScaleDown(TGraphAsymmErrors *& graph, const std::vector<Double_t> & bins)
{
  std::cout << "Scaling down graph: " << graph->GetName() << std::endl;

  for (UInt_t i = 0; i < bins.size()-1; i++)
  {
    const auto divisor = bins[i+1]-bins[i];
    
    Double_t xval,yval;
    graph->GetPoint(i,xval,yval);
    graph->SetPoint(i,xval,yval/divisor);
    
    auto yerrl = graph->GetErrorYlow (i);
    auto yerrh = graph->GetErrorYhigh(i);
    graph->SetPointEYlow (i,yerrl/divisor);
    graph->SetPointEYhigh(i,yerrh/divisor);
  }
}

void Fitter::ScaleDown(TH1F *& hist)
{
  std::cout << "Scaling down hist: " << graph->GetName() << std::endl;

  for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
  {
    const auto divisor = hist->GetXaxis()->GetBinWidth(ibinX);
    
    hist->SetBinContent(ibinX,(hist->GetBinContent(ibinX))/divisor);
    hist->SetBinError  (ibinX,(hist->GetBinError  (ibinX))/divisor);
  }
}

Float_t Fitter::GetMinimum(TGraphAsymmErrors *& graph, TH1F *& hist1, TH1F *& hist1)
{
  std::cout << "Getting minimum for plot..." << std::endl;

  auto min = 1e9;

  // need to loop through to check bin != 0
  for (auto bin = 1; bin <= hist1->GetNbinsX(); bin++)
  {
    const auto tmpmin = hist1->GetBinContent(bin);
    if ((tmpmin < min) && (tmpmin > 0)) min = tmpmin;
  }

  // need to loop through to check bin != 0
  for (auto bin = 1; bin <= hist2->GetNbinsX(); bin++)
  {
    const auto tmpmin = hist2->GetBinContent(bin);
    if ((tmpmin < min) && (tmpmin > 0)) min = tmpmin;
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
