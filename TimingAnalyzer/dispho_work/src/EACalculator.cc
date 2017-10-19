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
  fOutFile = new TFile(Form("%s/effarea_canvases.root",fOutDir.Data()),"RECREATE"); // make output tfile --> store canvas images here too, for quick editting
  fEAFile.open(Form("%s/%s",fOutDir.Data(),Config::eadumpname.Data()),std::ios_base::trunc);

  // Read in names of plots to be stacked
  StackGEDOOT::InitTH1FNamesAndSubDNames();
  // Make subdirs as needed
  StackGEDOOT::InitSubDirs();

  // with all that defined, initialize everything in constructor
  StackGEDOOT::InitInputPlots();
  StackGEDOOT::InitFits();
  StackGEDOOT::InitOutputCanvs();
}

EACalculator::~EACalculator()
{
  fEAfile.close();

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
  
}

void StackGEDOOT::InitTH1FNamesAndSubDNames()
{
  // will use the integral of nvtx to derive total yields as no additional cuts are placed on ntvx --> key on name for yields
  
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

  if (fTH1FNames.size() == 0) 
  {
    std::cerr << "Somehow, no plots were read in for the stacker ...exiting..." << std::endl;
    exit(1);
  }
}

void StackGEDOOT::InitSubDirs()
{
  MakeSubDirs(fTH1FSubDMap,fOutDir,Config::easubdir);
  MakeSubDir(fRhoSubD,fOutDir,Config::easubdir);
}

void StackGEDOOT::InitInputPlots() 
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

void StackGEDOOT::InitOutputCanvPads() 
{
  fOutTH1FCanvases.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fOutTH1FCanvases[th1f] = new TCanvas(Form("%s_canv",fTH1FNames[th1f].Data()),"");
    fOutTH1FCanvases[th1f]->cd();
  }

  fOutRhoCanvas = new TCanvas(Form("%s_canv",fRhoName.Data()),"");
  fOutRhoCanvas->cd();
}
