// Class include
#include "Fitter.hh"

// ROOT+RooFit includes
#include "TVirtualFitter.h"
#include "RooPlot.h"

Fitter::Fitter(const TString & fitconfig, const TString & miscconfig,
	       const Bool_t savemetadata, const TString & outfiletext)
  : fFitConfig(fitconfig), fMiscConfig(miscconfig), fSaveMetaData(savemetadata), fOutFileText(outfiletext)
{
  std::cout << "Initializing..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(fTDRStyle);

  // init configuration, set minimizer
  Fitter::SetupDefaults();
  Fitter::SetupCommon();
  Fitter::SetupFitConfig();
  Fitter::SetupPlotConfig();
  Fitter::SetupMiscConfig();
  TVirtualFitter::SetDefaultFitter("Minuit2");
  
  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"RECREATE");

  // setup outtree
  if (fDoFits) Fitter::SetupOutTree();
}

Fitter::~Fitter()
{
  std::cout << "Tidying up in the destructor..." << std::endl;

  if (fSaveMetaData) delete fConfigPave;

  delete fY;
  delete fYRooBins;

  delete fX;
  delete fXRooBins;

  Common::DeleteMap(fNPredSignMap);
  delete fNPredBkgd;
  Common::DeleteMap(fFracMap);

  Common::DeleteMap(fHistMapY);
  Common::DeleteMap(fHistMapX);
  Common::DeleteMap(fHistMap2D);

  Common::DeleteMap(fHistMap2DTmp);
  
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
  FitInfo FitInfo2D(RooArgList(*fX,*fY),"2D",FitType::TwoD);
  Fitter::PreparePdfs(fHistMap2D,FitInfo2D);
  if (fDoFits) Fitter::MakeFit(FitInfo2D);
  if (fMakeWS) Fitter::ImportToWS(FitInfo2D);
  Fitter::DeleteFitInfo(FitInfo2D);

  // Do the fit in 1D -- X
  FitInfo FitInfoX(RooArgList(*fX),"1D_projX",FitType::ProjX);
  Fitter::PreparePdfs(fHistMapX,FitInfoX);
  if (fDoFits) Fitter::MakeFit(FitInfoX);
  if (fMakeWS) Fitter::ImportToWS(FitInfoX);
  Fitter::DeleteFitInfo(FitInfoX);

  // Do the fit in 1D -- Y
  FitInfo FitInfoY(RooArgList(*fY),"1D_projY",FitType::ProjY);
  Fitter::PreparePdfs(fHistMapY,FitInfoY);
  if (fDoFits) Fitter::MakeFit(FitInfoY);
  if (fMakeWS) Fitter::ImportToWS(FitInfoY);
  Fitter::DeleteFitInfo(FitInfoY);

  // Save Output?
  if (fDoFits) Fitter::SaveOutTree();

  // Save MetaData
  if (fSaveMetaData) Fitter::MakeConfigPave();
}

void Fitter::PrepareCommon()
{
  std::cout << "Preparing common variables and datasets..." << std::endl;

  // Get the input 2D histograms
  Fitter::GetInputHists();

  // Scale data CR to SR via MC SFs
  Common::GetSRPredFromCRs(fHistMap2D,fHistMap2DTmp,(fXVarBins||fYVarBins),fCRKFMap,fCRXFMap);

  // Check for any negative bins and set them to zero!
  Common::CheckNegativeBins(fHistMap2D);

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

  //////////////
  // GJets CR //
  //////////////

  fGJetsFile = TFile::Open(Form("%s",fGJetsFileName.Data()));
  Common::CheckValidFile(fGJetsFile,fGJetsFileName);
  fGJetsFile->cd();

  Common::SetupCRHists("GJets",fGJetsFile,fHistMap2D,fHistMap2DTmp);

  ////////////
  // QCD CR //
  ////////////

  fQCDFile = TFile::Open(Form("%s",fQCDFileName.Data()));
  Common::CheckValidFile(fQCDFile,fQCDFileName);
  fQCDFile->cd();

  Common::SetupCRHists("QCD",fQCDFile,fHistMap2D,fHistMap2DTmp);

  //////////////
  // SR Hists //
  //////////////
  
  fSRFile = TFile::Open(Form("%s",fSRFileName.Data()));
  Common::CheckValidFile(fSRFile,fSRFileName);
  fSRFile->cd();

  // MC bkgds excluding EWK
  Common::SetupSRMCHists(fSRFile,fHistMap2DTmp);

  // EWK MC
  fHistMap2D[Common::EWKSampleName] = (TH2F*)fSRFile->Get(Form("%s",Common::EWKHistName.Data())); // MC prediction of all other MC backgrounds aside from GJets and QCD

  // Real Data
  if (!fGenData)
  {
    fHistMap2D["Data"] = (TH2F*)fSRFile->Get(Form("%s",Common::HistNameMap["Data"].Data()));
    Common::CheckValidHist(fHistMap2D["Data"],Common::HistNameMap["Data"],fSRFileName);
  }

  // Signal MC
  if (!fBkgdOnly) 
  {
    // use ALL signal samples to start, only one for fitting... from config
    for (const auto & GroupPair : Common::GroupMap)
    {
      const auto & sample = GroupPair.first;
      const auto & group  = GroupPair.second;

      // only load signals!
      if (group != SampleGroup::isSignal) continue;

      // load signals
      fHistMap2D[sample] = (TH2F*)fSRFile->Get(Form("%s",Common::HistNameMap[sample].Data()));
      Common::CheckValidHist(fHistMap2D[sample],Common::HistNameMap[sample],fSRFileName);
    }
  }

  /////////////////
  // Scale Hists //
  /////////////////

  if (fXVarBins || fYVarBins)
  {
    // for real hists
    for (auto & HistPair : fHistMap2D)
    {
      auto & hist = HistPair.second;
      Common::Scale(hist,isUp,fXVarBins,fYVarBins);
    }

    // for temp hists
    for (auto & HistPair : fHistMap2DTmp)
    {
      auto & hist = HistPair.second;
      Common::Scale(hist,isUp,fXVarBins,fYVarBins);
    }
  }
}  

void Fitter::DumpInputInfo()
{
  std::cout << "Making quick dump of input histograms..." << std::endl;
  
  // output file
  std::ofstream outFileDump(fOutFileText+"."+Common::outTextExt,std::ios::trunc);

  // Bkgd indiv samples
  for (const auto & BkgdGroupPair : Common::BkgdGroupMap)
  {
    const auto & sample = BkgdGroupPair.first;
    auto hist = (TH2F*)fHistMap2D[sample]->Clone(Form("%sHist",sample.Data()));
    Fitter::DumpIntegralsAndDraw(hist,outFileDump,sample,false,false);
    delete hist;
  }
  
  // Combine Bkgd samples
  auto bkgdHist = (TH2F*)fHistMap2D[Common::EWKSampleName]->Clone(Form("%sHist",Common::BkgdSampleName.Data()));
  for (const auto & BkgdGroupPair : Common::BkgdGroupMap)
  {
    const auto & sample = BkgdGroupPair.first;
    if (!Common::IsCR(sample)) continue;
    bkgdHist->Add(fHistMap2D[sample]);
  }
  Fitter::DumpIntegralsAndDraw(bkgdHist,outFileDump,Common::BkgdSampleName,false,true);
  if (!fBkgdOnly) Fitter::DumpSignificance(bkgdHist);
  delete bkgdHist;

  // Signal 
  if (!fBkgdOnly)
  {
    for (const auto & sample : fPlotSignalVec)
    {
      const TString signtext = Form("%s_Sign",sample.Data());
      auto signHist = (TH2F*)fHistMap2D[sample]->Clone(Form("%sHist",signtext.Data()));
      Fitter::DumpIntegralsAndDraw(signHist,outFileDump,signtext,false,true);
      delete signHist;
    }
  }

  // Data Histogram --> need to blind it if called for
  if (!fGenData)
  {
    const TString datatext = "Data";
    auto dataHist = (TH2F*)fHistMap2D["Data"]->Clone(Form("%sHist",datatext.Data()));
    Fitter::DumpIntegralsAndDraw(dataHist,outFileDump,datatext,fBlindData,true);
    delete dataHist;
  }
}

void Fitter::DumpIntegralsAndDraw(TH2F *& hist2D, std::ofstream & outFileDump, const TString & text, const Bool_t isBlind, const Bool_t isDraw)
{
  std::cout << "Dumping intregral and drawing for: " << hist2D->GetName() << std::endl;
  
  // get useful bin numbers
  if (isBlind)
  {
    for (const auto & Blind : fBlinds)
    {
      const auto binXlow = hist2D->GetXaxis()->FindBin(Blind.xlow);
      const auto binXup  = hist2D->GetXaxis()->FindBin(Blind.xup);
      const auto binYlow = hist2D->GetYaxis()->FindBin(Blind.ylow);
      const auto binYup  = hist2D->GetYaxis()->FindBin(Blind.yup);
      
      for (auto ibinX = binXlow; ibinX <= binXup; ibinX++) 
      {
	for (auto ibinY = binYlow; ibinY <= binYup; ibinY++) 
	{
	  hist2D->SetBinContent(ibinX,ibinY,0.f);
	  hist2D->SetBinError  (ibinX,ibinY,0.f);
	}
      }
    }
  }

  // reuse integrals
  Double_t hist_int = 0.0;
  Double_t hist_err = 0.0;

  // get integral and error full region
  hist_int = hist2D->IntegralAndError(1,hist2D->GetXaxis()->GetNbins(),1,hist2D->GetYaxis()->GetNbins(),hist_err);
  outFileDump << text.Data() << " Integral: " << hist_int << " +/- " << hist_err << std::endl;

  // get integral and error over signal region (by summing up over each cell in each blinded region)
  Double_t sum_int  = 0.0;
  Double_t sum_err2 = 0.0;
  for (const auto & Blind : fBlinds)
  {
    const auto binXlow = hist2D->GetXaxis()->FindBin(Blind.xlow);
    const auto binXup  = hist2D->GetXaxis()->FindBin(Blind.xup);
    const auto binYlow = hist2D->GetYaxis()->FindBin(Blind.ylow);
    const auto binYup  = hist2D->GetYaxis()->FindBin(Blind.yup);
  
    sum_int  += hist2D->IntegralAndError(binXlow,binXup,binYlow,binYup,hist_err);
    sum_err2 += hist_err*hist_err;
  }
  hist_int = sum_int;
  hist_err = std::sqrt(sum_err2);
  outFileDump << text.Data() << " Integral (In Blinded Region): " << hist_int << " +/- " << hist_err << std::endl;

  // save it to the outfile
  fOutFile->cd();
  hist2D->Write(hist2D->GetName(),TObject::kWriteDelete);
  
  if (isDraw)
  {
    // scale back down temporarily (since already scaled up)
    const Bool_t isUp = false;
    if (fXVarBins || fYVarBins) Common::Scale(hist2D,isUp,fXVarBins,fYVarBins);

    // get new tmp canvas
    auto canv = new TCanvas();
    canv->cd();
    
    // draw TH2 on canv
    hist2D->Draw("colz");
    Common::CMSLumi(canv,10,fEra);
    Common::SaveAs(canv,Form("%sHist_2D",text.Data()));
    delete canv;

    // project in X,Y and draw
    Fitter::DrawProjection(hist2D,text,isBlind,"X");
    Fitter::DrawProjection(hist2D,text,isBlind,"Y");
  }
}

void Fitter::DrawProjection(TH2F *& hist2D, const TString & text, const Bool_t isBlind, const TString & proj)
{
  std::cout << "Drawing " << proj.Data() << "-projection for: " << hist2D->GetName() << std::endl;

  // tmp strings
  const TString histname = "tmp_proj"+proj;
  const TString outname  = text+"Hist_proj"+proj;

  // determine which projection
  const Bool_t isX = proj.Contains("X",TString::kExact);
  
  // get hist
  auto hist1D = (TH1F*)( isX ? hist2D->ProjectionX(histname.Data()) : hist2D->ProjectionY(histname.Data()) );

  // blind
  if (isBlind)
  {
    for (const auto & Blind : fBlinds)
    {
      const auto binlow = hist1D->GetXaxis()->FindBin( isX ? Blind.xlow : Blind.ylow );
      const auto binup  = hist1D->GetXaxis()->FindBin( isX ? Blind.xup  : Blind.yup  );
      
      for (auto ibin = binlow; ibin <= binup; ibin++) 
      {
	hist1D->SetBinContent(ibin,0.f);
	hist1D->SetBinError  (ibin,0.f);
      }
    }
  }
  
  // save as both lin and log
  Fitter::SaveHist(hist1D,outname);
  delete hist1D;
}

void Fitter::SaveHist(TH1F *& hist, const TString & text)
{
  // lin first
  Fitter::SaveHist(hist,text,false);
  
  // log second
  Fitter::SaveHist(hist,text,true);
}

void Fitter::SaveHist(TH1F *& hist, const TString & text, const Bool_t isLogY)
{
  // make canvas
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy(isLogY);

  // draw hist
  hist->Draw("ep");
  
  // save it
  Common::CMSLumi(canv,10,fEra);
  Common::SaveAs(canv,text+"_"+(isLogY?"log":"lin"));

  delete canv;
}

void Fitter::DumpSignificance(TH2F *& bkgdHist2D)
{
  std::cout << "Dumping significance in 2D..." << std::endl;

  // first, get the signal histograms into the numerator and denom
  std::vector<TH2F*> significanceVec2D;
  std::vector<TH2F*> splusbDenomVec2D;
  Fitter::ReadInSignalHists(significanceVec2D,"Significance");
  Fitter::ReadInSignalHists(splusbDenomVec2D ,"SPlusB_Denom");
  const auto nSignals = fPlotSignalVec.size();
  
  // compute sqrt(s+b) : add bkgdHist to each signal for denom, compute sqrt of content and errors
  for (auto & hist2D : splusbDenomVec2D)
  {
    hist2D->Add(bkgdHist2D);

    for (auto ibinX = 1; ibinX <= hist2D->GetXaxis()->GetNbins(); ibinX++) 
    {
      for (auto ibinY = 1; ibinY <= hist2D->GetYaxis()->GetNbins(); ibinY++) 
      {
	const auto orig_content = hist2D->GetBinContent(ibinX,ibinY);
	const auto orig_error   = hist2D->GetBinError  (ibinX,ibinY);

	hist2D->SetBinContent(ibinX,ibinY,std::sqrt(orig_content));
	hist2D->SetBinError  (ibinX,ibinY,(orig_error/(2.f*std::sqrt(orig_content)))); // https://en.wikipedia.org/wiki/Propagation_of_uncertainty#Example_formulas
      }
    }
  }
  
  // compute s/sqrt(s+b), knowing that lengths of vectors are the same
  for (auto isignal = 0U; isignal < nSignals; isignal++)
  {
    const auto & denom2D = splusbDenomVec2D [isignal];
    auto       & numer2D = significanceVec2D[isignal];

    numer2D->Divide(denom2D);
  }

  // save and draw 2D, write to file
  for (auto & hist2D : significanceVec2D)
  {
    // save it to the outfile
    fOutFile->cd();
    hist2D->Write(hist2D->GetName(),TObject::kWriteDelete);

    // make canvas
    auto canv = new TCanvas();
  
    // draw TH2 on canv
    hist2D->Draw("colz");
    Common::CMSLumi(canv,10,fEra);
    Common::SaveAs(canv,Form("%s_2D",hist2D->GetName()));

    // delete it
    delete canv;
  }
  
  // project in X,Y and make canvas
  Fitter::DumpSignificance1D(significanceVec2D,"X");
  Fitter::DumpSignificance1D(significanceVec2D,"Y");

  // delete it all!
  for (auto & hist2D : significanceVec2D) delete hist2D;
  for (auto & hist2D : splusbDenomVec2D)  delete hist2D;
}

void Fitter::ReadInSignalHists(std::vector<TH2F*> & hists2D, const TString & text)
{
  // scale the signal, as it turns out the bkgd hist is already scaled up [and NOT blinded] from DumpIntegralAndDraw() 
  const Bool_t isUp = false;

  // loop over signals to plot, clone, and then scale up
  for (const auto & signalname : fPlotSignalVec)
  {
    hists2D.emplace_back((TH2F*)fHistMap2D[signalname]->Clone(Form("%s_%s",signalname.Data(),text.Data())));
   
    auto & hist2D = hists2D.back();
    if (fXVarBins || fYVarBins) Common::Scale(hist2D,isUp,fXVarBins,fYVarBins);
  }
}

void Fitter::DumpSignificance1D(std::vector<TH2F*> hists2D, const TString & proj)
{
  std::cout << "Dumping significance in 1D for: " << proj.Data() << std::endl;
  
  // determine which projection to use
  const Bool_t isX = proj.Contains("X",TString::kExact);
  
  // get hists, set colors
  std::vector<TH1F*> hists1D;
  for (auto ihist = 0U; ihist < hists2D.size(); ihist++)
  {
    auto & hist2D = hists2D[ihist];
    const TString histname = Form("%s_proj%s",hist2D->GetName(),proj.Data());

    hists1D.emplace_back( (TH1F*)(isX ? hist2D->ProjectionX(histname.Data()) : hist2D->ProjectionY(histname.Data())) );
    auto & hist1D = hists1D.back();

    hist1D->SetLineColor  (Common::ColorVec[ihist]);
    hist1D->SetMarkerColor(Common::ColorVec[ihist]);
  }
  
  // Draw in log, lin
  Fitter::DrawSignificance1D(hists1D,proj);

  // delete tmp hists
  for (auto & hist1D : hists1D) delete hist1D;
}

void Fitter::DrawSignificance1D(std::vector<TH1F*> & hists1D, const TString & proj)
{
  std::cout << "Drawing significance in 1D for: " << proj.Data() << std::endl;

  // get min, max
  const auto min = Fitter::GetMinimum(hists1D);
  const auto max = Fitter::GetMaximum(hists1D);

  // Draw lin, then log
  Fitter::DrawSignificance1D(hists1D,proj,min,max,false);
  Fitter::DrawSignificance1D(hists1D,proj,min,max,true);
}

Float_t Fitter::GetMinimum(const std::vector<TH1F*> & hists1D)
{
  Float_t min = 1e9;
  for (const auto & hist1D : hists1D)
  {
    for (auto ibin = 1; ibin <= hist1D->GetXaxis()->GetNbins(); ibin++)
    {
      const Float_t tmpmin = hist1D->GetBinContent(ibin);
      if (tmpmin > 0.f && tmpmin < min) min = tmpmin;
    }
  }
  return min;
}

Float_t Fitter::GetMaximum(const std::vector<TH1F*> & hists1D)
{
  Float_t max = -1e9;
  for (const auto & hist1D : hists1D)
  {
    const Float_t tmpmax = hist1D->GetBinContent(hist1D->GetMaximumBin());
    if (tmpmax > max) max = tmpmax;
  }
  return max;
}

void Fitter::DrawSignificance1D(std::vector<TH1F*> & hists1D, const TString & proj,
				const Float_t min, const Float_t max, const Bool_t isLogY)
{
  // make canvas
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy(isLogY);

  // make legend
  auto leg = new TLegend(0.55,0.82,0.825,0.92);
  leg->SetNColumns(2);
  leg->SetBorderSize(1);
  leg->SetLineColor(kBlack);

  // set min, max, and then draw (add to leg)
  for (auto ihist = 0U; ihist < hists1D.size(); ihist++)
  {
    auto & hist1D = hists1D[ihist];
    
    hist1D->SetMinimum( isLogY ? min/1.5f : min/1.05f );
    hist1D->SetMaximum( isLogY ? max*1.5f : max*1.05f );

    // draw
    canv->cd();
    hist1D->Draw(ihist>0?"ep same":"ep");

    // Get label
    auto label = fPlotSignalVec[ihist]; // these are aligned, so we pick out the labels this way
    label.ReplaceAll("GMSB_","");
    label.ReplaceAll("_",",");
    label.ReplaceAll("L","#Lambda:");
    label.ReplaceAll("CTau","c#tau:");

    // add leg
    leg->AddEntry(hist1D,label.Data(),"lep");
  }

  // draw it all
  leg->Draw("same");
  Common::CMSLumi(canv,10,fEra);
  Common::SaveAs(canv,"Significance_proj"+proj+"_"+(isLogY?"log":"lin"));
  
  delete leg;
  delete canv;
}

void Fitter::Project2DHistTo1D()
{
  std::cout << "Projecting 2D histograms to 1D..." << std::endl;

  for (auto & HistPair2D : fHistMap2D)
  {
    const auto & sample = HistPair2D.first;
    fHistMapX[sample] = (TH1F*)HistPair2D.second->ProjectionX(Form("%s_projX",Fitter::GetHistName(sample).Data()));
    fHistMapY[sample] = (TH1F*)HistPair2D.second->ProjectionY(Form("%s_projY",Fitter::GetHistName(sample).Data()));
  }
}

void Fitter::DeclareCoefficients()
{
  std::cout << "Getting integral counts and fractions..." << std::endl;

  // Count up background first
  std::map<TString,Float_t> nBkgdMap;
  fNTotalBkgd = 0.f;
  for (const auto & BkgdGroupPair : Common::BkgdGroupMap)
  {
    const auto & sample = BkgdGroupPair.first;
    auto & nBkgd = nBkgdMap[sample];

    nBkgd = fHistMap2D[sample]->Integral();
    fNTotalBkgd += nBkgd;
  }

  for (const auto & nBkgdPair : nBkgdMap)
  {
    const auto & sample = nBkgdPair.first;
    const auto   nbkgd  = nBkgdPair.second;

    const TString name = Fitter::GetHistName(sample)+"_frac";
    fFracMap[sample] = new RooRealVar(Form("%s",name.Data()),Form("%s",name.Data()),nbkgd/fNTotalBkgd);
  }

  // Scale nBkgd up or down 
  fNTotalBkgd *= fScaleTotalBkgd;

  // make vars for varying extended PDFs
  fNPredBkgd = new RooRealVar("nbkgd","nbkgd",fNTotalBkgd,(fScaleRangeLow*fNTotalBkgd),(fScaleRangeHigh*fNTotalBkgd));

  // Count signal
  for (const auto & GroupPair : Common::GroupMap)
  {
    const auto & sample = GroupPair.first;
    const auto & group  = GroupPair.second;
    
    // only load signals!
    if (group != SampleGroup::isSignal) continue;

    // total predicted for each signal (float)
    fNTotalSignMap[sample] = fHistMap2D[sample]->Integral();

    // scale up or down
    fNTotalSignMap[sample] *= fScaleTotalSign;

    // make vars for varying extended PDFs
    fNPredSignMap[sample] = new RooRealVar(Form("nsign_%s",sample.Data()),Form("nsign_%s",sample.Data()),fNTotalSignMap[sample],
					   (fScaleRangeLow*fNTotalSignMap[sample]),(fScaleRangeHigh*fNTotalSignMap[sample]));
  }
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

    const TString name = Form("%s_RooDataHist_%s",Fitter::GetHistName(sample).Data(),fitInfo.Text.Data());
    fitInfo.DataHistMap[sample] = new RooDataHist(Form("%s",name.Data()),Form("%s",name.Data()),fitInfo.ArgList,hist);
  }
}

void Fitter::DeclareSamplePdfs(FitInfo & fitInfo)
{
  std::cout << "Setting Sample Pdfs for: " << fitInfo.Text.Data() << std::endl;

  // Make a Pdf for each signal and background
  for (const auto & DataHistPair : fitInfo.DataHistMap)
  {
    // build background and signal pdfs
    const auto & sample   = DataHistPair.first;
    const auto & datahist = DataHistPair.second;

    if (Fitter::IsData(sample)) continue;
    
    const TString name = Form("%s_PDF_%s",Fitter::GetHistName(sample).Data(),fitInfo.Text.Data());
    fitInfo.HistPdfMap[sample] = new RooHistPdf(Form("%s",name.Data()),Form("%s",name.Data()),fitInfo.ArgList,*datahist);
  }

  // Build Sum of Bkgd-Only Pdf
  RooArgList BkgdPdfList;
  RooArgList BkgdFracList;
  for (const auto & BkgdGroupPair : Common::BkgdGroupMap)
  {
    const auto & sample = BkgdGroupPair.first;

    BkgdPdfList.add(*fitInfo.HistPdfMap.at(sample));
    BkgdFracList.add(*fFracMap.at(sample));
  }

  const TString bkgdname = Form("Bkgd_PDF_%s",fitInfo.Text.Data());
  fitInfo.BkgdPdf = new RooAddPdf(Form("%s",bkgdname.Data()),Form("%s",bkgdname.Data()),BkgdPdfList,BkgdFracList);
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
      if (fitInfo.Fit == FitType::TwoD)
      {
	Fitter::DrawFit(fX,Form("%i_xfit",ifit),fitInfo);
	Fitter::DrawFit(fY,Form("%i_yfit",ifit),fitInfo);
      }
      else if (fitInfo.Fit == FitType::ProjX)
      {
	Fitter::DrawFit(fX,Form("%i_fit",ifit),fitInfo);
      }
      else if (fitInfo.Fit == FitType::ProjY)
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
  fNPredBkgd->setVal(gRandom->Poisson(fScaleGenBkgd*fNTotalBkgd));
  fNPredSignMap[fSignalSample]->setVal(gRandom->Poisson(fScaleGenSign*fNTotalSignMap[fSignalSample]));

  // for fOutTree
  fNGenBkgd = fNPredBkgd->getVal();
  fNGenSign = fNPredSignMap[fSignalSample]->getVal();
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
    fitInfo.SignExtPdf = new RooExtendPdf(Form("%s",esignname.Data()),Form("%s",esignname.Data()),*fitInfo.HistPdfMap.at(fSignalSample),*fNPredSignMap.at(fSignalSample));
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
  fitInfo.DataHistMap["Data"] = fitInfo.ModelPdf->generateBinned(fitInfo.ArgList,(fNGenBkgd+fNGenSign));

  // Rename to follow conventions so far
  const TString name = Form("%s_RooDataHist_%s",Common::HistNameMap["Data"].Data(),fitInfo.Text.Data());
  fitInfo.DataHistMap["Data"]->SetName(Form("%s",name.Data()));
}

void Fitter::FitModel(FitInfo & fitInfo)
{
  std::cout << "Fit model for: " << fitInfo.Text.Data() << std::endl;

  // initialize norms before each fit, i.e. don't cheat!
  fNPredBkgd->setVal(((fScaleRangeLow*fNTotalBkgd)+(fScaleRangeHigh*fNTotalBkgd))/2.f);
  fNPredSignMap[fSignalSample]->setVal(((fScaleRangeLow*fNTotalSignMap[fSignalSample])+(fScaleRangeHigh*fNTotalSignMap[fSignalSample]))/2.f);

  // perform the fit!
  fitInfo.ModelPdf->fitTo(*fitInfo.DataHistMap.at("Data"),RooFit::SumW2Error(true));
}

void Fitter::GetPredicted(FitInfo & fitInfo)
{
  std::cout << "Get predicted number of events for: " << fitInfo.Text.Data() << std::endl;

  fNFitBkgd = fNPredBkgd->getVal();
  fNFitBkgdErr = fNPredBkgd->getError();
  fNFitSign = fNPredSignMap[fSignalSample]->getVal();
  fNFitSignErr = fNPredSignMap[fSignalSample]->getError();
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
  auto & datahist = fitInfo.DataHistMap.at("Data");
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
    Common::Scale(dataGraph,bins,isUp);
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
  if      ((fitInfo.Fit == FitType::ProjX) || (fitInfo.Fit == FitType::ProjY))
  {
    // create histograms straight from pdfs (norms already taken care of!)
    modelHist = (TH1F*)fitInfo.ModelPdf  ->createHistogram(Form("%s",modelname.Data()),*var);
    bkgdHist  = (TH1F*)fitInfo.BkgdExtPdf->createHistogram(Form("%s",bkgdname .Data()),*var);
    if (!fBkgdOnly) signHist = (TH1F*)fitInfo.SignExtPdf->createHistogram(Form("%s",signname.Data()),*var);
  }
  else if (fitInfo.Fit == FitType::TwoD)
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
      Common::Scale(modelHist,isUp);
      Common::Scale(bkgdHist,isUp);
      if (!fBkgdOnly) Common::Scale(signHist,isUp);
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
  Common::CMSLumi(canv,10,fEra);
  Common::SaveAs(canv,Form("%s_%s_lin",title.Data(),fitInfo.Text.Data()));

  // make the range nice and save (LOG)
  modelHist->SetMinimum(min/1.5);
  modelHist->SetMaximum(max*1.5);
  canv->SetLogy(true);
  Common::CMSLumi(canv,10,fEra);
  Common::SaveAs(canv,Form("%s_%s_log",title.Data(),fitInfo.Text.Data()));

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

  if (fGenData && (ifit != (fNFits - 1))) delete fitInfo.DataHistMap["Data"];
}

void Fitter::ImportToWS(FitInfo & fitInfo)
{
  std::cout << "Make workspace for " << fitInfo.Text.Data() << std::endl;

  // make new workspace
  auto workspace = new RooWorkspace(Form("workspace_%s",fitInfo.Text.Data()),Form("workspace_%s",fitInfo.Text.Data()));

  // give meaningful names first, then change objects
  const TString bkgdname = Form("Bkgd_PDF_%s",fitInfo.Text.Data());
  fitInfo.BkgdPdf->SetName(Form("%s",bkgdname.Data()));
  fNPredBkgd->SetName(Form("%s_norm",bkgdname.Data()));

  // Change names of relative fractions of PDFs
  std::map<TString,TString> fracNameMap;
  for (auto & FracPair : fFracMap)
  {
    const auto & sample = FracPair.first;
    auto       & frac   = FracPair.second;
 
    auto & fracName = fracNameMap[sample];
    fracName = frac->GetName();

    frac->SetName(Form("%s_%s",fracName.Data(),fitInfo.Text.Data()));
  }

  // Set bkgd to generic expectation
  fNPredBkgd->setVal(fNTotalBkgd);

  // Set up signals...
  for (const auto & GroupPair : Common::GroupMap)
  {
    const auto & sample = GroupPair.first;
    const auto & group  = GroupPair.second;
    
    // only load signals!
    if (group != SampleGroup::isSignal) continue;
  
    const TString signname = Form("%s_PDF_%s",sample.Data(),fitInfo.Text.Data());
    fitInfo.HistPdfMap[sample]->SetName(Form("%s",signname.Data()));
    
    fNPredSignMap[sample]->setVal(fNTotalSignMap[sample]);
    fNPredSignMap[sample]->setConstant(true);
    fNPredSignMap[sample]->SetName(Form("%s_norm",signname.Data()));
  }

  // make sanity plots
  if (fDumpWS) Fitter::DumpWS(fitInfo,"pre");

  // import into workspace
  workspace->import(*fitInfo.DataHistMap.at("Data"));
  workspace->import(*fitInfo.BkgdPdf);
  workspace->import(*fNPredBkgd);

  for (const auto & GroupPair : Common::GroupMap)
  {
    const auto & sample = GroupPair.first;
    const auto & group  = GroupPair.second;
    
    // only load signals!
    if (group != SampleGroup::isSignal) continue;
  
    workspace->import(*fitInfo.HistPdfMap[sample]);
    workspace->import(*fNPredSignMap[sample]);
  }

  // make sanity plots
  if (fDumpWS) Fitter::DumpWS(fitInfo,"post");

  // write it out
  fOutFile->cd();
  workspace->Write(workspace->GetName(),TObject::kWriteDelete);

  // now delete it!
  delete workspace;

  // rename now that it is safe
  for (auto & FracPair : fFracMap)
  {
    const auto & sample = FracPair.first;
    auto       & frac   = FracPair.second;

    frac->SetName(fracNameMap[sample].Data());
  }
}

void Fitter::DumpWS(const FitInfo & fitInfo, const TString & label)
{
  TCanvas * canv = new TCanvas();
  RooPlot * frame = nullptr;
  TH1F * hist = nullptr;
  if (fitInfo.Fit == FitType::TwoD || fitInfo.Fit == FitType::ProjX)
  {
    frame = fX->frame();
    fitInfo.BkgdPdf->plotOn(frame);
    frame->Draw();
    Common::SaveAs(canv,Form("%simport_x_plotOn_%s",label.Data(),fitInfo.Text.Data()));

    hist = (TH1F*)fitInfo.BkgdPdf->createHistogram(Form("bkgdhist_%sx",label.Data()),*fX,RooFit::Binning(fX->getBinning()));
    hist->Draw();
    Common::SaveAs(canv,Form("%simport_x_createHist_%s",label.Data(),fitInfo.Text.Data()));

    delete hist;
    delete frame;
  }
  
  if (fitInfo.Fit == FitType::TwoD || fitInfo.Fit == FitType::ProjY)
  {
    frame = fY->frame();
    fitInfo.BkgdPdf->plotOn(frame);
    frame->Draw();
    Common::SaveAs(canv,Form("%simport_y_plotOn_%s",label.Data(),fitInfo.Text.Data()));

    hist = (TH1F*)fitInfo.BkgdPdf->createHistogram(Form("bkgdhist_%sy",label.Data()),*fY,RooFit::Binning(fY->getBinning()));
    hist->Draw();
    Common::SaveAs(canv,Form("%simport_y_createHist_%s",label.Data(),fitInfo.Text.Data()));

    delete hist;
    delete frame;
  }
  
  if (fitInfo.Fit == FitType::TwoD)
  {
    auto xpdf = fitInfo.BkgdPdf->createProjection(*fY);

    frame = fX->frame();
    xpdf->plotOn(frame);
    frame->Draw();
    Common::SaveAs(canv,Form("%simport_x_plotOn_2D_projX",label.Data()));
    delete frame;

    auto xhist = (TH1F*)xpdf->createHistogram("xhist",*fX);                                                      
    xhist->Draw();
    Common::SaveAs(canv,Form("%simport_x_createHist_2D_projX",label.Data()));
    delete xhist;

    delete xpdf;
    
    auto ypdf = fitInfo.BkgdPdf->createProjection(*fX);

    frame = fY->frame();
    ypdf->plotOn(frame);
    frame->Draw();
    Common::SaveAs(canv,Form("%simport_y_plotOn_2D_projY",label.Data()));
    delete frame;

    auto yhist = (TH1F*)ypdf->createHistogram("yhist",*fY);                                                      
    yhist->Draw();
    Common::SaveAs(canv,Form("%simport_y_createHist_2D_projY",label.Data()));
    delete yhist;

    delete ypdf;

    auto hist2d = (TH2F*)fitInfo.BkgdPdf->createHistogram(Form("bkgdhist_%s2d",label.Data()),*fX,RooFit::Binning(fX->getBinning()),RooFit::YVar(*fY,RooFit::Binning(fY->getBinning())));
    hist2d->Draw("colz");
    Common::SaveAs(canv,Form("%simport_xy_createHist_%s",label.Data(),fitInfo.Text.Data()));
    delete hist2d;
  }

  delete canv;
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

  // create the pave, copying in old info
  fOutFile->cd();
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));

  // give grand title
  fConfigPave->AddText("***** Fitter Config *****");

    // Add era info
  Common::AddEraInfoToPave(fConfigPave,fEra);

  // fit config
  Common::AddTextFromInputConfig(fConfigPave,"Fit Config",fFitConfig);

  // plot config
  Common::AddTextFromInputConfig(fConfigPave,"Plot Config",fPlotConfig);

  // store last bits of misc info
  Common::AddTextFromInputConfig(fConfigPave,"Miscellaneous Config",fMiscConfig);

  // padding
  Common::AddPaddingToPave(fConfigPave,3);

  // dump in old config
  fConfigPave->AddText("***** GJets CR Config *****");
  Common::AddTextFromInputPave(fConfigPave,fGJetsFile);

  fConfigPave->AddText("***** QCD CR Config *****");
  Common::AddTextFromInputPave(fConfigPave,fQCDFile);

  fConfigPave->AddText("***** SR Config *****");
  Common::AddTextFromInputPave(fConfigPave,fSRFile);

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void Fitter::SetupDefaults()
{
  std::cout << "Setting defaults..." << std::endl;
  
  fBkgdOnly = false;
  fGenData  = false;
  fDoFits   = false;
  fMakeWS   = false;
  fDumpWS   = false;

  fNFits = 1;
  fNDraw = 100;
  fScaleTotalBkgd = 1;
  fScaleTotalSign = 1;
  fScaleRangeLow = -100;
  fScaleRangeHigh = 100;
  fScaleGenBkgd = 1;
  fScaleGenSign = 1;

  fBlindData = false;
}

void Fitter::SetupCommon()
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupBkgdGroups();
  Common::SetupHistNames();
  Common::SetupBkgdHistNames();
}

void Fitter::SetupFitConfig()
{
  std::cout << "Reading fit config..." << std::endl;

  std::ifstream infile(Form("%s",fFitConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("CR_GJets_in=") != std::string::npos)
    {
      fGJetsFileName = Common::RemoveDelim(str,"CR_GJets_in=");
    }
    else if (str.find("CR_QCD_in=") != std::string::npos)
    {
      fQCDFileName = Common::RemoveDelim(str,"CR_QCD_in=");
    }
    else if (str.find("SR_in=") != std::string::npos)
    {
      fSRFileName = Common::RemoveDelim(str,"SR_in=");
    }
    else if (str.find("plot_config=") != std::string::npos)
    {
      fPlotConfig = Common::RemoveDelim(str,"plot_config=");
    }
    else if (str.find("era=") != std::string::npos)
    {
      fEra = Common::RemoveDelim(str,"era=");
    }
    else if (str.find("bkgd_only=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"bkgd_only=");
      Common::SetupBool(str,fBkgdOnly);
    }
    else if (str.find("scale_total_bkgd=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"scale_total_bkgd=");
      fScaleTotalBkgd = std::atof(str.c_str());
    }
    else if (str.find("scale_total_sign=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"scale_total_sign=");
      fScaleTotalSign = std::atof(str.c_str());
    }
    else if (str.find("scale_range_low=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"scale_range_low=");
      fScaleRangeLow = std::atof(str.c_str());
    }
    else if (str.find("scale_range_high=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"scale_range_high=");
      fScaleRangeHigh = std::atof(str.c_str());
    }
    else if (str.find("gen_data=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"gen_data=");
      Common::SetupBool(str,fGenData);
    }
    else if (str.find("scale_gen_bkgd=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"scale_gen_bkgd=");
      fScaleGenBkgd = std::atof(str.c_str());
    }
    else if (str.find("scale_gen_sign=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"scale_gen_sign=");
      fScaleGenSign = std::atof(str.c_str());
    }
    else if (str.find("make_ws=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"make_ws=");
      Common::SetupBool(str,fMakeWS);
    }
    else if (str.find("dump_ws=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"dump_ws=");
      Common::SetupBool(str,fDumpWS);
    }
    else if (str.find("do_fits=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"do_fits=");
      Common::SetupBool(str,fDoFits);
    }
    else if (str.find("n_fits=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"n_fits=");
      fNFits = std::atoi(str.c_str());
    }
    else if (str.find("n_draw=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"n_draw=");
      fNDraw = std::atoi(str.c_str());
    }
    else if (str.find("x_cut=") != std::string::npos)
    {
      fXCut = Common::RemoveDelim(str,"x_cut=");
    }
    else if (str.find("y_cut=") != std::string::npos)
    {
      fYCut = Common::RemoveDelim(str,"y_cut=");
    }
    else 
    {
      std::cerr << "Aye... your fit config is messed up, try again! Offending line: " << str.c_str() << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void Fitter::SetupPlotConfig()
{
  std::cout << "Reading plot config..." << std::endl;

  std::ifstream infile(Form("%s",fPlotConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("x_title=") != std::string::npos)
    {
      fXTitle = Common::RemoveDelim(str,"x_title=");
    }
    else if (str.find("x_bins=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_bins=");
      Common::SetupBins(str,fXBins,fXVarBins);
    }
    else if (str.find("y_title=") != std::string::npos)
    {
      fYTitle = Common::RemoveDelim(str,"y_title=");
    }
    else if (str.find("y_bins=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"y_bins=");
      Common::SetupBins(str,fYBins,fYVarBins);
    }
    else if (str.find("blinding=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"blinding=");
      Common::SetupBlockRanges(str,fBlinds);
    }
    else if ((str.find("plot_title=") != std::string::npos) ||
	     (str.find("x_var=") != std::string::npos)      ||
	     (str.find("x_var_data=") != std::string::npos) ||
	     (str.find("x_var_bkgd=") != std::string::npos) ||
	     (str.find("x_var_sign=") != std::string::npos) ||
	     (str.find("x_labels=") != std::string::npos)   ||
	     (str.find("y_var=") != std::string::npos)      ||
	     (str.find("y_var_data=") != std::string::npos) ||
	     (str.find("y_var_bkgd=") != std::string::npos) ||
	     (str.find("y_var_sign=") != std::string::npos) ||
	     (str.find("y_labels=") != std::string::npos)   ||
	     (str.find("z_title=") != std::string::npos))
    {
      continue;
    }
    else 
    {
      std::cerr << "Aye... your plot config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void Fitter::SetupMiscConfig()
{
  std::cout << "Reading miscellaneous plot config..." << std::endl;

  std::ifstream infile(Form("%s",fMiscConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("signal_to_model=") != std::string::npos) 
    {
      str = Common::RemoveDelim(str,"signal_to_model=");
      std::vector<TString> tempvec;
      Common::SetupWhichSignals(str,tempvec);

      // know that only one signal is specified for model building+fitting!
      fSignalSample = tempvec[0];
    }
    else if (str.find("signals_to_plot=") != std::string::npos) 
    {
      str = Common::RemoveDelim(str,"signals_to_plot=");
      Common::SetupWhichSignals(str,fPlotSignalVec);
    }
    else if (str.find("scale_mc_to_data=") != std::string::npos) 
    {
      std::cout << "scale_mc_to_data not currently implemented in Fitter, skipping..." << std::endl;
    }
    else if (str.find("scale_to_unity=") != std::string::npos)
    {
      std::cout << "scale_to_unity not currently implemented in Fitter, skipping..." << std::endl;
    }
    else if (str.find("blind_data=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"blind_data=");
      Common::SetupBool(str,fBlindData);
    }
    else if (str.find("signals_only=") != std::string::npos)
    {
      std::cout << "signals_only not currently implemented in Fitter, skipping..." << std::endl;
    }
    else 
    {
      std::cerr << "Aye... your miscellaneous plot config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
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
  fOutTree->Branch("scaleTotalBkgd",&fScaleTotalBkgd);
  fOutTree->Branch("scaleTotalSign",&fScaleTotalSign);
  fOutTree->Branch("nTotalBkgd",&fNTotalBkgd);
  fOutTree->Branch("nTotalSign",&fNTotalSignMap[fSignalSample]);
  fOutTree->Branch("scaleGenBkgd",&fScaleGenBkgd);
  fOutTree->Branch("scaleGenSign",&fScaleGenSign);
  fOutTree->Branch("nGenBkgd",&fNGenBkgd);
  fOutTree->Branch("nGenSign",&fNGenSign);
  fOutTree->Branch("scaleRangeLow",&fScaleRangeLow);
  fOutTree->Branch("scaleRangeHigh",&fScaleRangeHigh);
  fOutTree->Branch("nFitBkgd",&fNFitBkgd);
  fOutTree->Branch("nFitBkgdErr",&fNFitBkgdErr);
  fOutTree->Branch("nFitSign",&fNFitSign);
  fOutTree->Branch("nFitSignErr",&fNFitSignErr);
  fOutTree->Branch("fitID",&fFitID);
}

Bool_t Fitter::IsData(const TString & sample)
{
  if   (Common::IsEWK(sample)) return false;
  else                         return (Common::GroupMap[sample] == SampleGroup::isData);
}

TString Fitter::GetHistName(const TString & sample)
{
  TString name;
  
  if   (!Common::IsEWK(sample)) name = Common::HistNameMap[sample];
  else                          name = Common::EWKHistName;
  
  return name;
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
  
  Common::DeleteMap(fitInfo.DataHistMap);
  Common::DeleteMap(fitInfo.HistPdfMap);

  delete fitInfo.BkgdPdf;
}
