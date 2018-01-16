#include "TreePlotter2D.hh"

TreePlotter2D::TreePlotter2D(const TString & text, const TString & commoncut, const TString & inconfig) 
  : fText(text), fCommonCut(commoncut), fInConfig(inconfig) 
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

  // setup hists
  TreePlotter2D::InitConfig();
  TreePlotter2D::ReadInConfig();
  TreePlotter2D::SetupHists();

  // output root file
  fOutFile = TFile::Open(Form("%s.root",fText.Data()),"UPDATE");
}

TreePlotter2D::~TreePlotter2D()
{
  delete RatioMCErrs;
  delete RatioHist;
  delete BkgdHist;
  delete fOutFile;
  for (auto & HistPair : HistMap) delete HistPair.second;
  delete fTDRStyle;
}

void TreePlotter2D::MakePlot()
{
  // Fill Hists from TTrees
  TreePlotter2D::MakeHistFromTrees();

  // Make Bkgd Output
  TreePlotter2D::MakeBkgdOutput();

  // Make Ratio Output
  TreePlotter2D::MakeRatioOutput();
}

void TreePlotter2D::MakeHistFromTrees()
{
  for (const auto & SamplePair : Config::SampleMap)
  {
    // Init
    const auto & input  = SamplePair.first;
    const auto & sample = SamplePair.second;
    const Bool_t isMC = (sample != Data);
    std::cout << "Working on " << (isMC?"MC":"DATA") << " sample: " << input.Data() << std::endl;

    // Get File
    const TString filename = Form("%s/%s/%s/%s",Config::eosDir.Data(),Config::baseDir.Data(),input.Data(),Config::tupleFileName.Data());
    TFile * file = TFile::Open(Form("%s",filename.Data()));
    Config::CheckValidFile(file,filename);
    file->cd();
	
    // Get TTree
    TTree * tree = (TTree*)file->Get(Form("%s",Config::disphotreename.Data()));
    Config::CheckValidTree(tree,Config::disphotreename,filename);

    // Make temp hist
    TString histname = Config::ReplaceSlashWithUndescore(input);
    TH2F * hist = TreePlotter2D::SetupHist(Form("%s_Hist",histname.Data()));
    
    // Fill from tree
    tree->Draw(Form("%s:%s>>%s",fYVar.Data(),fXVar.Data(),hist->GetName()),Form("(%s) * (%s)",Config::CutMap[sample].Data(),Config::WeightString(isMC).Data()),"goff");
    
    // Add to main hists
    HistMap[sample]->Add(hist);

    // save to output file
    fOutFile->cd();
    hist->Write(hist->GetName(),TObject::kWriteDelete);

    // delete everything
    delete hist;
    delete tree;
    delete file;
  }

  // save totals to output file
  fOutFile->cd();
  for (const auto & HistPair : HistMap)
  { 
    const auto & hist = HistPair.second;
    hist->Write(hist->GetName(),TObject::kWriteDelete);
  }
}

void TreePlotter2D::MakeBkgdOutput()
{
  std::cout << "Making Bkgd Output..." << std::endl;

  // Make Total Bkgd Hist: for ratio and error plotting
  BkgdHist = TreePlotter2D::SetupHist("Bkgd_Hist");
  BkgdHist->Add(HistMap[GJets]);
  BkgdHist->Add(HistMap[QCD]);
  BkgdHist->SetMarkerSize(0);
  BkgdHist->SetFillStyle(3254);
  BkgdHist->SetFillColor(kGray+3);

  // save to output file
  fOutFile->cd();
  BkgdHist->Write(BkgdHist->GetName(),TObject::kWriteDelete);
}

void TreePlotter2D::MakeRatioOutput()
{
  std::cout << "Making Ratio Output..." << std::endl;

  // ratio value plot
  RatioHist = TreePlotter2D::SetupHist("Ratio_Hist");
  RatioHist->Add(HistMap[Data]);
  RatioHist->Divide(BkgdHist);  
  RatioHist->SetStats(0);      // No statistics on lower plot
  
  // ratio MC error plot
  RatioMCErrs = TreePlotter2D::SetupHist("Ratio_MCErrs");
  RatioMCErrs->Add(BkgdHist);
  RatioMCErrs->Divide(BkgdHist);

  // save to output file
  fOutFile->cd();
  RatioHist->Write(RatioHist->GetName(),TObject::kWriteDelete);
  RatioMCErrs->Write(RatioMCErrs->GetName(),TObject::kWriteDelete);
  
  // save to output file
  fOutFile->cd();
}

void TreePlotter2D::InitConfig()
{
  Config::SetupSamples();
  Config::SetupCuts(fCommonCut);
}

void TreePlotter2D::ReadInConfig()
{
  std::fstream infile;
  infile.open(Form("%s",fInConfig.Data()),std::ios::in);

  TString tmp;

  Int_t counter = 0;
  Bool_t readFloats = false;

  while (infile >> tmp)
  {
    if (counter == 0)
    {
      fXVar = tmp;
      counter++;
    }
    else if (counter == 1)
    {
      fYVar = tmp;
      counter++;
    }
    else if (counter == 2) 
    {
      fName = Config::ReplaceXXX(tmp);
      counter++;
    }
    else if (counter == 3) 
    {
      fName = Config::ReplaceXXX(tmp);
      counter++;
    }
    else if (counter == 4 && tmp == "" && !readFloats)
    {
      readFloats = true;
    }
    else if (counter == 4 && readFloats)
    {
      fXBins.push_back(tmp.Atof());
    }
    else if (counter == 4 && tmp == "" && readFloats)
    {
      readFloats = false;
      counter++;
    }
    else if (counter == 5)
    {
      fXTitle = Config::ReplaceXXX(tmp);
      counter++;
    }
    else if (counter == 6 && tmp == "" && !readFloats)
    {
      readFloats = true;
    }
    else if (counter == 6 && readFloats)
    {
      fYBins.push_back(tmp.Atof());
    }
    else if (counter == 6 && tmp == "" && readFloats)
    {
      readFloats = false;
      counter++;
    }
    else if (counter == 7)
    {
      fYTitle = Config::ReplaceXXX(tmp);
      counter++;
    }
    else 
    {
      std::cerr << "Aye... your config is messed up, try again!" << std::endl;
      exit(1);
    }
  }

  infile.close();
}

void TreePlotter2D::SetupHists()
{
  HistMap[QCD]   = SetupHist("QCD_Hist");
  HistMap[GJets] = SetupHist("GJets_Hist");
  HistMap[GMSB]  = SetupHist("GMSB_Hist");
  HistMap[Data]  = SetupHist("Data_Hist");
}

TH2F * TreePlotter2D::SetupHist(const TString & name)
{
  const Double_t * xbins = &fXBins[0];
  const Double_t * ybins = &fYBins[0];

  TH2F * hist = new TH2F(name.Data(),fTitle.Data(),fBinsX.size()-1,xbins,fBinsY.size()-1,ybins);
  hist->GetXaxis()->SetTitle(fXTitle.Data());
  hist->GetYaxis()->SetTitle(fYTitle.Data());
  hist->Sumw2();
  
  return hist;
}
