#include "../interface/PtCalculator.hh"

PtCalculator::PtCalculator(const TString & sample, const Bool_t isMC) : fSample(sample), fIsMC(isMC)
{
  // I/O dir
  fOutDir = Form("%s/%i/%s/%s",Config::outdir.Data(), Config::year, (fIsMC?"MC":"DATA"), fSample.Data());

  // open input files
  const TString filename = Form("%s/%s", fOutDir.Data(), Config::AnOutName.Data());
  fInFile = TFile::Open(filename.Data());
  CheckValidFile(fInFile,filename);

  // output data members
  fOutFile = new TFile(Form("%s/ptscale_canvases.root",fOutDir.Data()),"UPDATE"); // make output tfile --> store canvas images here too, for quick editting
  fPtFile.open(Form("%s/%s", fOutDir.Data(), Config::ptdumpname.Data()),std::ios_base::trunc);

  // Read in names of plots to be stacked
  PtCalculator::InitTH1FNamesAndSubDNames();
  // Make subdirs as needed
  PtCalculator::InitSubDirs();

  // with all that defined, initialize everything in constructor
  PtCalculator::InitInputPlots();
  PtCalculator::InitFits();
  PtCalculator::InitOutputCanvs();
}

PtCalculator::~PtCalculator()
{
  fPtFile.close();

  // delete all pointers
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    delete fOutTH1FCanvases[th1f];
    delete fOutTH1FTF1s[th1f];
    delete fInTH1FHists[th1f];
  }

  delete fOutFile;
  delete fInFile;
}

void PtCalculator::ExtractPtScaling()
{
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    // compute slope of pho iso vs pt
    fInTH1FHists[th1f]->Fit(Form("%s_%s_fit",fTH1FNames[th1f].Data(),Config::ptformname.Data()),"RQ");

    // save the slope
    fPtFile << fTH1FNames[th1f].Data() << " " << fOutTH1FTF1s[th1f]->GetParameter(1) << std::endl;
  }

  // Then finally output the canvases for safekeeping
  PtCalculator::OutputFitCanvases();
}

void PtCalculator::OutputFitCanvases()
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
					Config::ptsubdir.Data(),fTH1FNames[th1f].Data(),Config::outtype.Data()));
  }
}

void PtCalculator::InitTH1FNamesAndSubDNames()
{
  std::ifstream plotstoread;
  plotstoread.open(Form("%s/%i/%s",Config::outdir.Data(),Config::year,Config::plotdumpname.Data()),std::ios::in);

  TString plotname; TString subdir;

  const TString pho = "pho";
  const TString pt  = "_v_pt_quant";
  while (plotstoread >> plotname >> subdir) 
  {
    if      (plotname.Contains(pho,TString::kExact) && plotname.Contains(pt,TString::kExact))
    {
      fTH1FNames.push_back(plotname);
      fTH1FSubDMap[plotname] = subdir;
    }
  }
  plotstoread.close();

  // store the size of the number of plots
  fNTH1F = fTH1FNames.size();

  if (fTH1FNames.size() == 0) 
  {
    std::cerr << "Somehow, no plots were read in for the Pt scaling calculator ...exiting..." << std::endl;
    exit(1);
  }
}

void PtCalculator::InitSubDirs()
{
  MakeSubDirs(fTH1FSubDMap,fOutDir,Config::ptsubdir);
}

void PtCalculator::InitInputPlots() 
{
  // init input th1f hists
  fInTH1FHists.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fInTH1FHists[th1f] = (TH1F*)fInFile->Get(Form("%s",fTH1FNames[th1f].Data()));	
    CheckValidTH1F(fInTH1FHists[th1f],fTH1FNames[th1f],fInFile->GetName());
  }
}

void PtCalculator::InitFits()
{
  fOutTH1FTF1s.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fOutTH1FTF1s[th1f] = new TF1(Form("%s_%s_fit",fTH1FNames[th1f].Data(),Config::ptformname.Data()),
				 Config::ptformname.Data(),Config::xmin_pt,Config::xmax_pt);
    fOutTH1FTF1s[th1f]->SetParName(0,"intercept");
    fOutTH1FTF1s[th1f]->SetParameter(0,0.);
    fOutTH1FTF1s[th1f]->SetParName(1,"slope");
    fOutTH1FTF1s[th1f]->SetParameter(1,1.);
  }
}

void PtCalculator::InitOutputCanvs() 
{
  fOutFile->cd();

  fOutTH1FCanvases.resize(fNTH1F);
  for (Int_t th1f = 0; th1f < fNTH1F; th1f++)
  {
    fOutTH1FCanvases[th1f] = new TCanvas(Form("%s_ptscale_canv",fTH1FNames[th1f].Data()),"");
    fOutTH1FCanvases[th1f]->cd();
  }
}
