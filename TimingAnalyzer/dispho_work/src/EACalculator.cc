#include "../interface/EACalculator.hh"

EACalculator::EACalculator(const TString & sample, const Bool_t isMC) : fSample(sample), fIsMC(isMC)
{
  // I/O dir
  fOutDir = Form("%s/%s/%s",Config::outdir.Data(),(fIsMC?"MC":"DATA"),fSample.Data());

  // open input files
  const TString filename = Form("%s/plots.root",fOutDir.Data());
  fInFile = TFile::Open(filename.Data());
  CheckValidFile(fInFile,filename);

  // output data members
  fOutFile = new TFile(Form("%s/effarea_canvases.root",fOutDir.Data()),"UPDATE"); // make output tfile --> store canvas images here too, for quick editting
  fEAFile.open(Form("%s/%s",fOutDir.Data(),Config::eadumpname.Data()),std::ios_base::trunc);

  // Read in names of plots to be stacked
  EACalculator::InitTH1FNamesAndSubDNames();
  // Make subdirs as needed
  EACalculator::InitSubDirs();

  // with all that defined, initialize everything in constructor
  EACalculator::InitInputPlots();
  EACalculator::InitFits();
  EACalculator::InitOutputCanvs();
}

EACalculator::~EACalculator()
{
  fEAFile.close();

  // delete all pointers
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    delete fOutTH1FCanvases[th1f];
    delete fOutTH1FTF1s[th1f];
    delete fInTH1FHists[th1f];
  }
  delete fOutRhoCanvas;
  delete fOutRhoTF1;
  delete fInRhoHist;

  delete fOutFile;
  delete fInFile;
}

void EACalculator::ExtractEA()
{
  // First compute the slope of rho vs nvtx
  fInRhoHist->Fit(Form("%s_%s_fit",fRhoName.Data(),Config::formname.Data()),"RQ");
  
  // get the slope of the rho iso line
  const Double_t rho_slope = fOutRhoTF1->GetParameter(1);

  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    // Then compute slope of pho iso vs nvtx.
    fInTH1FHists[th1f]->Fit(Form("%s_%s_fit",fTH1FNames[th1f].Data(),Config::formname.Data()),"RQ");

    // get the slope of the pho iso line
    const Double_t pho_slope = fOutTH1FTF1s[th1f]->GetParameter(1);

    // compute the effective area
    const Double_t iso_ea = pho_slope / rho_slope;

    fEAFile << fTH1FNames[th1f].Data() << " " << iso_ea << std::endl;
  }

  // Then finally output the canvases for safekeeping
  EACalculator::OutputFitCanvases();
}

void EACalculator::OutputFitCanvases()
{
  // cd to out file
  fOutFile->cd();

  // do pho iso plots first
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fOutTH1FCanvases[th1f]->cd();
    fInTH1FHists[th1f]->Draw("ep");
    CMSLumi(fOutTH1FCanvases[th1f]);
    fOutTH1FCanvases[th1f]->Write(fOutTH1FCanvases[th1f]->GetName(),TObject::kWriteDelete);
    fOutTH1FCanvases[th1f]->SaveAs(Form("%s/%s/%s/lin/%s.%s",fOutDir.Data(),fTH1FSubDMap[fTH1FNames[th1f]].Data(),
					Config::easubdir.Data(),fTH1FNames[th1f].Data(),Config::outtype.Data()));
  }

  // do rho plots
  fOutRhoCanvas->cd();
  fInRhoHist->Draw("ep");
  CMSLumi(fOutRhoCanvas);
  fOutRhoCanvas->Write(fOutRhoCanvas->GetName(),TObject::kWriteDelete);
  fOutRhoCanvas->SaveAs(Form("%s/%s/%s/lin/%s.%s",fOutDir.Data(),fRhoSubD.Data(),
			     Config::easubdir.Data(),fRhoName.Data(),Config::outtype.Data()));
}

void EACalculator::InitTH1FNamesAndSubDNames()
{
  std::ifstream plotstoread;
  plotstoread.open(Form("%s/%s",Config::outdir.Data(),Config::plotdumpname.Data()),std::ios::in);

  TString plotname; TString subdir;

  const TString rho   = "rho";
  const TString pho   = "pho";
  const TString vnvtx = "_v_nvtx";
  while (plotstoread >> plotname >> subdir) 
  {
    if      (plotname.Contains(pho,TString::kExact) && plotname.Contains(vnvtx,TString::kExact))
    {
      fTH1FNames.push_back(plotname);
      fTH1FSubDMap[plotname] = subdir;
    }
    else if (plotname.Contains(rho,TString::kExact) && plotname.Contains(vnvtx,TString::kExact))
    {
      fRhoName = plotname;
      fRhoSubD = subdir;
    }
  }
  plotstoread.close();

  // store the size of the number of plots
  fNTH1F = fTH1FNames.size();

  if (fTH1FNames.size() == 0 || fRhoName == "") 
  {
    std::cerr << "Somehow, no plots were read in for the EA calculator ...exiting..." << std::endl;
    exit(1);
  }
}

void EACalculator::InitSubDirs()
{
  MakeSubDirs(fTH1FSubDMap,fOutDir,Config::easubdir);
  MakeSubDir(fRhoSubD,fOutDir,Config::easubdir);
}

void EACalculator::InitInputPlots() 
{
  // init input th1f hists
  fInTH1FHists.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fInTH1FHists[th1f] = (TH1F*)fInFile->Get(Form("%s",fTH1FNames[th1f].Data()));	
    CheckValidTH1F(fInTH1FHists[th1f],fTH1FNames[th1f],fInFile->GetName());
  }

  // Get Rho Plot
  fInRhoHist = (TH1F*)fInFile->Get(Form("%s",fRhoName.Data()));	
  CheckValidTH1F(fInRhoHist,fRhoName,fInFile->GetName());
}

void EACalculator::InitFits()
{
  // photons first
  fOutTH1FTF1s.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fOutTH1FTF1s[th1f] = new TF1(Form("%s_%s_fit",fTH1FNames[th1f].Data(),Config::formname.Data()),
				 Config::formname.Data(),Config::xmin_ea,Config::xmax_ea);
    fOutTH1FTF1s[th1f]->SetParName(0,"intercept");
    fOutTH1FTF1s[th1f]->SetParameter(0,0.);
    fOutTH1FTF1s[th1f]->SetParName(1,"slope");
    fOutTH1FTF1s[th1f]->SetParameter(1,1.);
  }

  // then rho
  fOutRhoTF1 = new TF1(Form("%s_%s_fit",fRhoName.Data(),Config::formname.Data()),
		       Config::formname.Data(),Config::xmin_ea,Config::xmax_ea);
  fOutRhoTF1->SetParName(0,"intercept");
  fOutRhoTF1->SetParameter(0,0.);
  fOutRhoTF1->SetParName(1,"slope");
  fOutRhoTF1->SetParameter(1,1.);
}

void EACalculator::InitOutputCanvs() 
{
  fOutFile->cd();

  fOutTH1FCanvases.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fOutTH1FCanvases[th1f] = new TCanvas(Form("%s_ea_canv",fTH1FNames[th1f].Data()),"");
    fOutTH1FCanvases[th1f]->cd();
  }

  fOutRhoCanvas = new TCanvas(Form("%s_ea_canv",fRhoName.Data()),"");
  fOutRhoCanvas->cd();
}
