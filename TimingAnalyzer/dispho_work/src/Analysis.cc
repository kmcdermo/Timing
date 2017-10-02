#include "../interface/Analysis.hh"
#include "TROOT.h"

inline Float_t rad2  (const Float_t x, const Float_t y){return x*x + y*y;}
inline Float_t theta (const Float_t r, const Float_t z){return std::atan2(r,z);}
inline Float_t eta   (const Float_t x, const Float_t y, const Float_t z)
{
  return -1.0f*std::log(std::tan(theta(std::sqrt(rad2(x,y)),z)/2.f));
}
inline Float_t rad2  (const Float_t x, const Float_t y, const Float_t z)
{
  return x*x + y*y + z*z;
}
inline Float_t phi   (const Float_t x, const Float_t y){return std::atan2(y,x);}
inline Float_t mphi  (Float_t phi)
{    
  while (phi >= Config::PI) phi -= Config::TWOPI;
  while (phi < -Config::PI) phi += Config::TWOPI;
  return phi;
}
inline Float_t deltaR(const Float_t eta1, const Float_t phi1, const Float_t eta2, const Float_t phi2)
{
  return std::sqrt(rad2(eta2-eta1,mphi(phi1-phi2)));
}
inline Float_t TOF   (const Float_t x,  const Float_t y,  const Float_t z, 
		      const Float_t vx, const Float_t vy, const Float_t vz, const Float_t time)
{
  return time + (std::sqrt(rad2(x,y,z))-std::sqrt(rad2((x-vx),(y-vy),(z-vz))))/Config::sol;
}

Analysis::Analysis(TString sample, Bool_t isMC) : fSample(sample), fIsMC(isMC)
{
  // because root is dumb?
  gROOT->ProcessLine("#include <vector>");

  // Set input
  TString filename = Form("input/%s/%s/%s/%s", Config::year.Data(), (fIsMC?"MC":"DATA"), fSample.Data(), "tree.root");
  fInFile  = TFile::Open(filename.Data());
  CheckValidFile(fInFile,filename);

  TString treename = "tree/tree";
  fInTree = (TTree*)fInFile->Get(treename.Data());
  CheckValidTree(fInTree,treename,filename);
  if (fIsMC)
  {
    if (fSample.Contains("gmsb",TString::kExact)) fIsGMSB = true;
    if (fSample.Contains("hvds",TString::kExact)) fIsHVDS = true;
  }
  Analysis::InitTree();

  // Set Output Stuff
  fOutDir = Form("%s/%s/%s",Config::outdir.Data(), (fIsMC?"MC":"DATA"), fSample.Data());
  MakeOutDir(fOutDir);
  fOutFile = new TFile(Form("%s/plots.root",fOutDir.Data()),"UPDATE");
  fColor = (fIsMC?Config::colorMap[fSample]:kBlack);

  // extra setup for data and MC
  if (fIsMC) 
  { 
    // Get pile-up weights
    TString purwfname = Form("%s/%s/%s",Config::outdir.Data(),Config::pusubdir.Data(),Config::pufilename.Data());
    TFile * purwfile  = TFile::Open(purwfname.Data());
    CheckValidFile(purwfile,purwfname);

    TH1F  * purwplot  = (TH1F*) purwfile->Get(Config::puplotname.Data());
    CheckValidTH1F(purwplot,Config::puplotname.Data(),purwfname);

    for (Int_t i = 1; i <= Config::nbinsvtx; i++)
    {
      fPUweights.push_back(purwplot->GetBinContent(i));
    }
    delete purwplot;
    delete purwfile;
    // end getting pile-up weights

    // set sample xsec + wgtsum
    fXsec   = Config::SampleXsecMap[fSample];
    fWgtsum = Config::SampleWgtsumMap[fSample];
  }  
  else 
  {
    fTH1Dump.open(Form("%s/%s",Config::outdir.Data(),Config::plotdumpname.Data()),std::ios_base::trunc); // do this once, and just do it for data
  }
}

Analysis::~Analysis()
{
  delete fInTree;
  delete fInFile;
  delete fOutFile;
  if (!fIsMC) fTH1Dump.close();
}

void Analysis::EventLoop()
{
  // Set up hists first --> first in map is histo name, by design!
  if (Config::doStandard) Analysis::SetupStandardPlots();

  // do loop over events, filling histos
  for (UInt_t entry = 0; entry < (!Config::doDemo?fInTree->GetEntries():Config::demoNum); entry++)
  {
    fInTree->GetEntry(entry);

    if (Config::dumpStatus) 
    {
      if (entry%Config::nEvCheck == 0 || entry == 0) std::cout << "Processing Entry: " << entry << " out of " << fInTree->GetEntries() << std::endl;
    } 

    ////////////////////////////
    //                        // 
    // Determine Event Weight //
    //                        // 
    ////////////////////////////
    Float_t weight = -1.;
    if   (fIsMC) {weight = (fXsec * Config::lumi * genwgt / fWgtsum) * fPUweights[genputrue];}
    else         {weight = 1.0;}

    // fill the plots
    if (Config::doStandard) Analysis::FillStandardPlots(weight);
  } // end loop over events

   // output hists
  if (Config::doStandard) Analysis::OutputStandardPlots();
}

void Analysis::SetupStandardPlots()
{
  // event based variables
  standardTH1Map["nvtx"] = Analysis::MakeTH1Plot("nvtx","",Config::nbinsvtx,0.,Double_t(Config::nbinsvtx),"nVertices","Events",standardTH1SubMap,"standard/vtx");
  
  // proof of concept
  for (Int_t ipho = 0; ipho < Config::nPhotons; ipho++)
  {
    //EB
    standardTH1Map[Form("phopt_%i_EB",ipho)] = 
      Analysis::MakeTH1Plot(Form("phopt_%i_EB",ipho),"",50,0.,1000.f,Form("Photon %i p_{T} (EB)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phochgiso_%i_EB",ipho)] = 
      Analysis::MakeTH1Plot(Form("phochgiso_%i_EB",ipho),"",50,0.,20.f,Form("Photon %i PF Charged Hadron Iso (EB)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phoneuiso_%i_EB",ipho)] = 
      Analysis::MakeTH1Plot(Form("phoneuiso_%i_EB",ipho),"",50,0.,20.f,Form("Photon %i PF Neutral Hadron Iso (EB)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phophoiso_%i_EB",ipho)] = 
      Analysis::MakeTH1Plot(Form("phophoiso_%i_EB",ipho),"",50,0.,20.f,Form("Photon %i PF Photon Iso (EB)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phoecaliso_%i_EB",ipho)] = 
      Analysis::MakeTH1Plot(Form("phoecaliso_%i_EB",ipho),"",50,0.,20.f,Form("Photon %i PFCluser ECAL Iso (EB)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phohcaliso_%i_EB",ipho)] = 
      Analysis::MakeTH1Plot(Form("phohcaliso_%i_EB",ipho),"",50,0.,20.f,Form("Photon %i PFCluser HCAL Iso (EB)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("photrkiso_%i_EB",ipho)] = 
      Analysis::MakeTH1Plot(Form("photrkiso_%i_EB",ipho),"",50,0.,20.f,Form("Photon %i Track Iso (EB)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phochgiso_zoom_%i_EB",ipho)] = 
      Analysis::MakeTH1Plot(Form("phochgiso_zoom_%i_EB",ipho),"",50,0.,1.f,Form("Photon %i PF Charged Hadron Iso (EB)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phoneuiso_zoom_%i_EB",ipho)] = 
      Analysis::MakeTH1Plot(Form("phoneuiso_zoom_%i_EB",ipho),"",50,0.,1.f,Form("Photon %i PF Neutral Hadron Iso (EB)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phophoiso_zoom_%i_EB",ipho)] = 
      Analysis::MakeTH1Plot(Form("phophoiso_zoom_%i_EB",ipho),"",50,0.,1.f,Form("Photon %i PF Photon Iso (EB)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phoecaliso_zoom_%i_EB",ipho)] = 
      Analysis::MakeTH1Plot(Form("phoecaliso_zoom_%i_EB",ipho),"",50,0.,1.f,Form("Photon %i PFCluser ECAL Iso (EB)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phohcaliso_zoom_%i_EB",ipho)] = 
      Analysis::MakeTH1Plot(Form("phohcaliso_zoom_%i_EB",ipho),"",50,0.,1.f,Form("Photon %i PFCluser HCAL Iso (EB)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("photrkiso_zoom_%i_EB",ipho)] = 
      Analysis::MakeTH1Plot(Form("photrkiso_zoom_%i_EB",ipho),"",50,0.,1.f,Form("Photon %i Track Iso (EB)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phoseedtime_%i_EB",ipho)] = 
      Analysis::MakeTH1Plot(Form("phoseedtime_%i_EB",ipho),"",50,-25.f,25.f,Form("Photon %i Seed Time [ns] (EB)",ipho),"Events",standardTH1SubMap,"standard/pho");

    //EE
    standardTH1Map[Form("phopt_%i_EE",ipho)] = 
      Analysis::MakeTH1Plot(Form("phopt_%i_EE",ipho),"",50,0.,1000.f,Form("Photon %i p_{T} (EE)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phochgiso_%i_EE",ipho)] = 
      Analysis::MakeTH1Plot(Form("phochgiso_%i_EE",ipho),"",50,0.,20.f,Form("Photon %i PF Charged Hadron Iso (EE)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phoneuiso_%i_EE",ipho)] = 
      Analysis::MakeTH1Plot(Form("phoneuiso_%i_EE",ipho),"",50,0.,20.f,Form("Photon %i PF Neutral Hadron Iso (EE)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phophoiso_%i_EE",ipho)] = 
      Analysis::MakeTH1Plot(Form("phophoiso_%i_EE",ipho),"",50,0.,20.f,Form("Photon %i PF Photon Iso (EE)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phoecaliso_%i_EE",ipho)] = 
      Analysis::MakeTH1Plot(Form("phoecaliso_%i_EE",ipho),"",50,0.,20.f,Form("Photon %i PFCluser ECAL Iso (EE)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phohcaliso_%i_EE",ipho)] = 
      Analysis::MakeTH1Plot(Form("phohcaliso_%i_EE",ipho),"",50,0.,20.f,Form("Photon %i PFCluser HCAL Iso (EE)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("photrkiso_%i_EE",ipho)] = 
      Analysis::MakeTH1Plot(Form("photrkiso_%i_EE",ipho),"",50,0.,20.f,Form("Photon %i Track Iso (EE)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phochgiso_zoom_%i_EE",ipho)] = 
      Analysis::MakeTH1Plot(Form("phochgiso_zoom_%i_EE",ipho),"",50,0.,1.f,Form("Photon %i PF Charged Hadron Iso (EE)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phoneuiso_zoom_%i_EE",ipho)] = 
      Analysis::MakeTH1Plot(Form("phoneuiso_zoom_%i_EE",ipho),"",50,0.,1.f,Form("Photon %i PF Neutral Hadron Iso (EE)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phophoiso_zoom_%i_EE",ipho)] = 
      Analysis::MakeTH1Plot(Form("phophoiso_zoom_%i_EE",ipho),"",50,0.,1.f,Form("Photon %i PF Photon Iso (EE)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phoecaliso_zoom_%i_EE",ipho)] = 
      Analysis::MakeTH1Plot(Form("phoecaliso_zoom_%i_EE",ipho),"",50,0.,1.f,Form("Photon %i PFCluser ECAL Iso (EE)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phohcaliso_zoom_%i_EE",ipho)] = 
      Analysis::MakeTH1Plot(Form("phohcaliso_zoom_%i_EE",ipho),"",50,0.,1.f,Form("Photon %i PFCluser HCAL Iso (EE)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("photrkiso_zoom_%i_EE",ipho)] = 
      Analysis::MakeTH1Plot(Form("photrkiso_zoom_%i_EE",ipho),"",50,0.,1.f,Form("Photon %i Track Iso (EE)",ipho),"Events",standardTH1SubMap,"standard/pho");
    standardTH1Map[Form("phoseedtime_%i_EE",ipho)] = 
      Analysis::MakeTH1Plot(Form("phoseedtime_%i_EE",ipho),"",50,-25.f,25.f,Form("Photon %i Seed Time [ns] (EE)",ipho),"Events",standardTH1SubMap,"standard/pho");
  }
}

void Analysis::FillStandardPlots(const Float_t weight)
{
  // standard "validation" and Z mass plots
  standardTH1Map["nvtx"]->Fill(nvtx,weight);

  const Int_t nPhos = std::min(nphotons,Config::nPhotons);
  for (Int_t ipho = 0; ipho < nPhos; ipho++)
  {
    const auto & pho = phos[ipho];
    if (pho.isEB)
    {
      standardTH1Map[Form("phopt_%i_EB",ipho)]->Fill(pho.pt,weight);

      if (!pho.isOOT)
      {
	standardTH1Map[Form("phochgiso_%i_EB",ipho)]->Fill(pho.ChgHadIso,weight);
	standardTH1Map[Form("phoneuiso_%i_EB",ipho)]->Fill(pho.NeuHadIso,weight);
	standardTH1Map[Form("phophoiso_%i_EB",ipho)]->Fill(pho.PhoIso,weight);
	standardTH1Map[Form("phochgiso_zoom_%i_EB",ipho)]->Fill(pho.ChgHadIso,weight);
	standardTH1Map[Form("phoneuiso_zoom_%i_EB",ipho)]->Fill(pho.NeuHadIso,weight);
	standardTH1Map[Form("phophoiso_zoom_%i_EB",ipho)]->Fill(pho.PhoIso,weight);
      }
      standardTH1Map[Form("phoecaliso_%i_EB",ipho)]->Fill(pho.EcalPFClIso,weight);
      standardTH1Map[Form("phohcaliso_%i_EB",ipho)]->Fill(pho.HcalPFClIso,weight);
      standardTH1Map[Form("photrkiso_%i_EB",ipho)]->Fill(pho.TrkIso,weight);
      standardTH1Map[Form("phoecaliso_zoom_%i_EB",ipho)]->Fill(pho.EcalPFClIso,weight);
      standardTH1Map[Form("phohcaliso_zoom_%i_EB",ipho)]->Fill(pho.HcalPFClIso,weight);
      standardTH1Map[Form("photrkiso_zoom_%i_EB",ipho)]->Fill(pho.TrkIso,weight);

      if (pho.seed >= 0)
      { 
	standardTH1Map[Form("phoseedtime_%i_EB",ipho)]->Fill((*rhtime)[pho.seed],weight);
      }
    }
    else 
    {
      standardTH1Map[Form("phopt_%i_EE",ipho)]->Fill(pho.pt,weight);

      if (!pho.isOOT)
      {
	standardTH1Map[Form("phochgiso_%i_EE",ipho)]->Fill(pho.ChgHadIso,weight);
	standardTH1Map[Form("phoneuiso_%i_EE",ipho)]->Fill(pho.NeuHadIso,weight);
	standardTH1Map[Form("phophoiso_%i_EE",ipho)]->Fill(pho.PhoIso,weight);
	standardTH1Map[Form("phochgiso_zoom_%i_EE",ipho)]->Fill(pho.ChgHadIso,weight);
	standardTH1Map[Form("phoneuiso_zoom_%i_EE",ipho)]->Fill(pho.NeuHadIso,weight);
	standardTH1Map[Form("phophoiso_zoom_%i_EE",ipho)]->Fill(pho.PhoIso,weight);
      }
      standardTH1Map[Form("phoecaliso_%i_EE",ipho)]->Fill(pho.EcalPFClIso,weight);
      standardTH1Map[Form("phohcaliso_%i_EE",ipho)]->Fill(pho.HcalPFClIso,weight);
      standardTH1Map[Form("photrkiso_%i_EE",ipho)]->Fill(pho.TrkIso,weight);
      standardTH1Map[Form("phoecaliso_zoom_%i_EE",ipho)]->Fill(pho.EcalPFClIso,weight);
      standardTH1Map[Form("phohcaliso_zoom_%i_EE",ipho)]->Fill(pho.HcalPFClIso,weight);
      standardTH1Map[Form("photrkiso_zoom_%i_EE",ipho)]->Fill(pho.TrkIso,weight);

      if (pho.seed >= 0)
      { 
	standardTH1Map[Form("phoseedtime_%i_EE",ipho)]->Fill((*rhtime)[pho.seed],weight);
      }
    }
  }
}

void Analysis::OutputStandardPlots() 
{
  MakeSubDirs(standardTH1SubMap,fOutDir);
  Analysis::SaveTH1s(standardTH1Map,standardTH1SubMap);
  if (!fIsMC) Analysis::DumpTH1Names(standardTH1Map,standardTH1SubMap);
  Analysis::DeleteTH1s(standardTH1Map);
}

TH1F * Analysis::MakeTH1Plot(TString hname, TString htitle, Int_t nbins, Double_t xlow, Double_t xhigh,
			     TString xtitle, TString ytitle, TStrMap& subdirmap, TString subdir) 
{
  TH1F * hist = new TH1F(hname.Data(),htitle.Data(),nbins,xlow,xhigh);
  hist->SetLineColor(kBlack);
  if (fIsMC)
  {
    hist->SetFillColor(fColor);
    hist->SetMarkerColor(fColor);
  }
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->GetYaxis()->SetTitleOffset(hist->GetYaxis()->GetTitleOffset() * Config::TitleFF);
  hist->Sumw2();

  // cheat a bit and set subdir map here
  subdirmap[hname] = subdir;
  
  return hist;
}

TH2F * Analysis::MakeTH2Plot(TString hname, TString htitle, const DblVec& vxbins, Int_t nbinsy, Double_t ylow, 
			     Double_t yhigh, TString xtitle, TString ytitle, TStrMap& subdirmap, TString subdir) 
{
  // need to convert vectors into arrays per ROOT
  const Double_t * axbins = &vxbins[0]; // https://stackoverflow.com/questions/2923272/how-to-convert-vector-to-array-c

  TH2F * hist = new TH2F(hname.Data(),htitle.Data(),vxbins.size()-1,axbins,nbinsy,ylow,yhigh);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  // cheat a bit and set subdir map here
  subdirmap[hname] = subdir;
  
  return hist;
}

void Analysis::SaveTH1s(TH1Map & th1map, TStrMap & subdirmap) 
{
  fOutFile->cd();

  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); ++mapiter) 
  { 
    // save to output file
    mapiter->second->Write(mapiter->second->GetName(),TObject::kWriteDelete); // map is map["hist name",TH1D*]

    if (Config::saveHists)
    {
      // now draw onto canvas to save as png
      TCanvas * canv = new TCanvas("canv","canv");
      canv->cd();
      mapiter->second->Draw( fIsMC ? "HIST" : "PE" );
      
      // first save as linear, then log
      canv->SetLogy(0);
      CMSLumi(canv);
      canv->SaveAs(Form("%s/%s/lin/%s.%s",fOutDir.Data(),subdirmap[mapiter->first].Data(),mapiter->first.Data(),Config::outtype.Data()));

      canv->SetLogy(1);
      CMSLumi(canv);
      canv->SaveAs(Form("%s/%s/log/%s.%s",fOutDir.Data(),subdirmap[mapiter->first].Data(),mapiter->first.Data(),Config::outtype.Data()));
    
      delete canv;
    }
  }
}

void Analysis::SaveTH2s(TH2Map & th2map, TStrMap & subdirmap) 
{
  fOutFile->cd();

  TCanvas * canv = new TCanvas("canv","canv");
  for (TH2MapIter mapiter = th2map.begin(); mapiter != th2map.end(); ++mapiter) 
  { 
    mapiter->second->Write(mapiter->second->GetName(),TObject::kWriteDelete);; // map is map["hist name",TH1D*]

    // now draw onto canvas to save as png
    canv->cd();
    mapiter->second->Draw("colz");
    
    // only save as linear
    canv->SetLogy(0);
    CMSLumi(canv);
    canv->SaveAs(Form("%s/%s/%s_2D.%s",fOutDir.Data(),subdirmap[mapiter->first].Data(),mapiter->first.Data(),Config::outtype.Data()));
  }

  delete canv;
}

void Analysis::DumpTH1Names(TH1Map & th1map, TStrMap & subdirmap) 
{
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); ++mapiter) 
  { 
    fTH1Dump << mapiter->first.Data()  << " " <<  subdirmap[mapiter->first].Data() << std::endl;
  }
}

void Analysis::DeleteTH1s(TH1Map & th1map)
{
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); ++mapiter) 
  { 
    delete (mapiter->second);
  }
  th1map.clear();
}

void Analysis::DeleteTH2s(TH2Map & th2map) 
{
  for (TH2MapIter mapiter = th2map.begin(); mapiter != th2map.end(); ++mapiter) 
  { 
    delete (mapiter->second);
  }
  th2map.clear();
}

void Analysis::InitTree() 
{
  Analysis::InitStructs();
  Analysis::InitBranchVecs();
  Analysis::InitBranches();
}

void Analysis::InitStructs()
{
  if (fIsMC)
  {
    if (fIsGMSB)
    {
      gmsbs.clear(); 
      gmsbs.resize(Config::nGMSBs);
    }
    if (fIsHVDS)
    {
      hvdss.clear(); 
      hvdss.resize(Config::nHVDSs);
    }
  }

  jets.clear();
  jets.resize(Config::nJets);

  phos.clear();
  phos.resize(Config::nPhotons);
}

void Analysis::InitBranchVecs()
{
  rheta = 0;
  rhphi = 0;
  rhE = 0;
  rhtime = 0;
  rhOOT = 0;
  rhID = 0;

  for (Int_t ipho = 0; ipho < Config::nPhotons; ipho++) 
  {
    phos[ipho].recHits = 0;
  }  
}

void Analysis::InitBranches()
{
  if (fIsMC)
  {
    fInTree->SetBranchAddress("genwgt", &genwgt, &b_genwgt);
    fInTree->SetBranchAddress("genpuobs", &genpuobs, &b_genpuobs);
    fInTree->SetBranchAddress("genputrue", &genputrue, &b_genputrue);
    
    if (fIsGMSB)
    {
      fInTree->SetBranchAddress("nNeutoPhGr", &nNeutoPhGr, &b_nNeutoPhGr);
      for (Int_t igmsb = 0; igmsb < Config::nGMSBs; igmsb++) 
      {
	auto & gmsb = gmsbs[igmsb]; 
	fInTree->SetBranchAddress(Form("genNmass_%i",igmsb), &gmsb.genNmass, &gmsb.b_genNmass);
	fInTree->SetBranchAddress(Form("genNE_%i",igmsb), &gmsb.genNE, &gmsb.b_genNE);
	fInTree->SetBranchAddress(Form("genNpt_%i",igmsb), &gmsb.genNpt, &gmsb.b_genNpt);
	fInTree->SetBranchAddress(Form("genNphi_%i",igmsb), &gmsb.genNphi, &gmsb.b_genNphi);
	fInTree->SetBranchAddress(Form("genNeta_%i",igmsb), &gmsb.genNeta, &gmsb.b_genNeta);
	fInTree->SetBranchAddress(Form("genNprodvx_%i",igmsb), &gmsb.genNprodvx, &gmsb.b_genNprodvx);
	fInTree->SetBranchAddress(Form("genNprodvy_%i",igmsb), &gmsb.genNprodvy, &gmsb.b_genNprodvy);
	fInTree->SetBranchAddress(Form("genNprodvz_%i",igmsb), &gmsb.genNprodvz, &gmsb.b_genNprodvz);
	fInTree->SetBranchAddress(Form("genNdecayvx_%i",igmsb), &gmsb.genNdecayvx, &gmsb.b_genNdecayvx);
	fInTree->SetBranchAddress(Form("genNdecayvy_%i",igmsb), &gmsb.genNdecayvy, &gmsb.b_genNdecayvy);
	fInTree->SetBranchAddress(Form("genNdecayvz_%i",igmsb), &gmsb.genNdecayvz, &gmsb.b_genNdecayvz);
	fInTree->SetBranchAddress(Form("genphE_%i",igmsb), &gmsb.genphE, &gmsb.b_genphE);
	fInTree->SetBranchAddress(Form("genphpt_%i",igmsb), &gmsb.genphpt, &gmsb.b_genphpt);
	fInTree->SetBranchAddress(Form("genphphi_%i",igmsb), &gmsb.genphphi, &gmsb.b_genphphi);
	fInTree->SetBranchAddress(Form("genpheta_%i",igmsb), &gmsb.genpheta, &gmsb.b_genpheta);
	fInTree->SetBranchAddress(Form("genphmatch_%i",igmsb), &gmsb.genphmatch, &gmsb.b_genphmatch);
	fInTree->SetBranchAddress(Form("gengrmass_%i",igmsb), &gmsb.gengrmass, &gmsb.b_gengrmass);
	fInTree->SetBranchAddress(Form("gengrE_%i",igmsb), &gmsb.gengrE, &gmsb.b_gengrE);
	fInTree->SetBranchAddress(Form("gengrpt_%i",igmsb), &gmsb.gengrpt, &gmsb.b_gengrpt);
	fInTree->SetBranchAddress(Form("gengrphi_%i",igmsb), &gmsb.gengrphi, &gmsb.b_gengrphi);
	fInTree->SetBranchAddress(Form("gengreta_%i",igmsb), &gmsb.gengreta, &gmsb.b_gengreta);
      } // end loop over neutralinos
    } // end block over gmsb

    if (fIsHVDS)
    {
      fInTree->SetBranchAddress("nvPions", &nvPions, &b_nvPions);
      for (Int_t ihvds = 0; ihvds < Config::nHVDSs; ihvds++) 
      {
	auto & hvds = hvdss[ihvds]; 
	fInTree->SetBranchAddress(Form("genvPionmass_%i",ihvds), &hvds.genvPionmass, &hvds.b_genvPionmass);
	fInTree->SetBranchAddress(Form("genvPionE_%i",ihvds), &hvds.genvPionE, &hvds.b_genvPionE);
	fInTree->SetBranchAddress(Form("genvPionpt_%i",ihvds), &hvds.genvPionpt, &hvds.b_genvPionpt);
	fInTree->SetBranchAddress(Form("genvPionphi_%i",ihvds), &hvds.genvPionphi, &hvds.b_genvPionphi);
	fInTree->SetBranchAddress(Form("genvPioneta_%i",ihvds), &hvds.genvPioneta, &hvds.b_genvPioneta);
	fInTree->SetBranchAddress(Form("genvPionprodvx_%i",ihvds), &hvds.genvPionprodvx, &hvds.b_genvPionprodvx);
	fInTree->SetBranchAddress(Form("genvPionprodvy_%i",ihvds), &hvds.genvPionprodvy, &hvds.b_genvPionprodvy);
	fInTree->SetBranchAddress(Form("genvPionprodvz_%i",ihvds), &hvds.genvPionprodvz, &hvds.b_genvPionprodvz);
	fInTree->SetBranchAddress(Form("genvPiondecayvx_%i",ihvds), &hvds.genvPiondecayvx, &hvds.b_genvPiondecayvx);
	fInTree->SetBranchAddress(Form("genvPiondecayvy_%i",ihvds), &hvds.genvPiondecayvy, &hvds.b_genvPiondecayvy);
	fInTree->SetBranchAddress(Form("genvPiondecayvz_%i",ihvds), &hvds.genvPiondecayvz, &hvds.b_genvPiondecayvz);
	fInTree->SetBranchAddress(Form("genHVph0E_%i",ihvds), &hvds.genHVph0E, &hvds.b_genHVph0E);
	fInTree->SetBranchAddress(Form("genHVph0pt_%i",ihvds), &hvds.genHVph0pt, &hvds.b_genHVph0pt);
	fInTree->SetBranchAddress(Form("genHVph0phi_%i",ihvds), &hvds.genHVph0phi, &hvds.b_genHVph0phi);
	fInTree->SetBranchAddress(Form("genHVph0eta_%i",ihvds), &hvds.genHVph0eta, &hvds.b_genHVph0eta);
	fInTree->SetBranchAddress(Form("genHVph0match_%i",ihvds), &hvds.genHVph0match, &hvds.b_genHVph0match);
	fInTree->SetBranchAddress(Form("genHVph1E_%i",ihvds), &hvds.genHVph1E, &hvds.b_genHVph1E);
	fInTree->SetBranchAddress(Form("genHVph1pt_%i",ihvds), &hvds.genHVph1pt, &hvds.b_genHVph1pt);
	fInTree->SetBranchAddress(Form("genHVph1phi_%i",ihvds), &hvds.genHVph1phi, &hvds.b_genHVph1phi);
	fInTree->SetBranchAddress(Form("genHVph1eta_%i",ihvds), &hvds.genHVph1eta, &hvds.b_genHVph1eta);
	fInTree->SetBranchAddress(Form("genHVph1match_%i",ihvds), &hvds.genHVph1match, &hvds.b_genHVph1match);
      } // end loop over nvpions 
    } // end block over hvds
  } // end block over isMC

  fInTree->SetBranchAddress("run", &run, &b_run);
  fInTree->SetBranchAddress("lumi", &lumi, &b_lumi);
  fInTree->SetBranchAddress("event", &event, &b_event);
  fInTree->SetBranchAddress("hltDisPho", &hltDisPho, &b_hltDisPho);
  fInTree->SetBranchAddress("nvtx", &nvtx, &b_nvtx);
  fInTree->SetBranchAddress("vtxX", &vtxX, &b_vtxX);
  fInTree->SetBranchAddress("vtxY", &vtxY, &b_vtxY);
  fInTree->SetBranchAddress("vtxZ", &vtxZ, &b_vtxZ);
  fInTree->SetBranchAddress("rho", &rho, &b_rho);
  fInTree->SetBranchAddress("t1pfMETpt", &t1pfMETpt, &b_t1pfMETpt);
  fInTree->SetBranchAddress("t1pfMETphi", &t1pfMETphi, &b_t1pfMETphi);
  fInTree->SetBranchAddress("t1pfMETsumEt", &t1pfMETsumEt, &b_t1pfMETsumEt);
  fInTree->SetBranchAddress("jetHT", &jetHT, &b_jetHT);

  fInTree->SetBranchAddress("njets", &njets, &b_njets);
  for (Int_t ijet = 0; ijet < Config::nJets; ijet++) 
  {
    auto & jet = jets[ijet];
    fInTree->SetBranchAddress(Form("jetE_%i",ijet), &jet.E, &jet.b_E);    
    fInTree->SetBranchAddress(Form("jetpt_%i",ijet), &jet.pt, &jet.b_pt);    
    fInTree->SetBranchAddress(Form("jetphi_%i",ijet), &jet.phi, &jet.b_phi);    
    fInTree->SetBranchAddress(Form("jeteta_%i",ijet), &jet.eta, &jet.b_eta);    
  }

  fInTree->SetBranchAddress("nrechits", &nrechits, &b_nrechits);
  fInTree->SetBranchAddress("rheta", &rheta, &b_rheta);
  fInTree->SetBranchAddress("rhphi", &rhphi, &b_rhphi);
  fInTree->SetBranchAddress("rhE", &rhE, &b_rhE);
  fInTree->SetBranchAddress("rhtime", &rhtime, &b_rhtime);
  fInTree->SetBranchAddress("rhOOT", &rhOOT, &b_rhOOT);
  fInTree->SetBranchAddress("rhID", &rhID, &b_rhID);

  fInTree->SetBranchAddress("nphotons", &nphotons, &b_nphotons);
  for (Int_t ipho = 0; ipho < Config::nPhotons; ipho++) 
  {
    auto & pho = phos[ipho];
    fInTree->SetBranchAddress(Form("phoE_%i",ipho), &pho.E, &pho.b_E);
    fInTree->SetBranchAddress(Form("phopt_%i",ipho), &pho.pt, &pho.b_pt);
    fInTree->SetBranchAddress(Form("phoeta_%i",ipho), &pho.eta, &pho.b_eta);
    fInTree->SetBranchAddress(Form("phophi_%i",ipho), &pho.phi, &pho.b_phi);
    fInTree->SetBranchAddress(Form("phoscE_%i",ipho), &pho.scE, &pho.b_scE);
    fInTree->SetBranchAddress(Form("phosceta_%i",ipho), &pho.sceta, &pho.b_sceta);
    fInTree->SetBranchAddress(Form("phoscphi_%i",ipho), &pho.scphi, &pho.b_scphi);
    fInTree->SetBranchAddress(Form("phoHadTowOE_%i",ipho), &pho.HadTowOE, &pho.b_HadTowOE);
    fInTree->SetBranchAddress(Form("phoHadronOE_%i",ipho), &pho.HadronOE, &pho.b_HadronOE);
    fInTree->SetBranchAddress(Form("phor9_%i",ipho), &pho.r9, &pho.b_r9);
    fInTree->SetBranchAddress(Form("phofullr9_%i",ipho), &pho.fullr9, &pho.b_fullr9);
    fInTree->SetBranchAddress(Form("phoChgHadIso_%i",ipho), &pho.ChgHadIso, &pho.b_ChgHadIso);
    fInTree->SetBranchAddress(Form("phoNeuHadIso_%i",ipho), &pho.NeuHadIso, &pho.b_NeuHadIso);
    fInTree->SetBranchAddress(Form("phoPhoIso_%i",ipho), &pho.PhoIso, &pho.b_PhoIso);
    fInTree->SetBranchAddress(Form("phoEcalPFClIso_%i",ipho), &pho.EcalPFClIso, &pho.b_EcalPFClIso);
    fInTree->SetBranchAddress(Form("phoHcalPFClIso_%i",ipho), &pho.HcalPFClIso, &pho.b_HcalPFClIso);
    fInTree->SetBranchAddress(Form("phoTrkIso_%i",ipho), &pho.TrkIso, &pho.b_TrkIso);
    fInTree->SetBranchAddress(Form("phosmaj_%i",ipho), &pho.smaj, &pho.b_smaj);
    fInTree->SetBranchAddress(Form("phosmin_%i",ipho), &pho.smin, &pho.b_smin);
    fInTree->SetBranchAddress(Form("phoalpha_%i",ipho), &pho.alpha, &pho.b_alpha);
    fInTree->SetBranchAddress(Form("phoseed_%i",ipho), &pho.seed, &pho.b_seed);
    fInTree->SetBranchAddress(Form("phorecHits_%i",ipho), &pho.recHits, &pho.b_recHits);
    fInTree->SetBranchAddress(Form("phoisOOT_%i",ipho), &pho.isOOT, &pho.b_isOOT);
    fInTree->SetBranchAddress(Form("phoisEB_%i",ipho), &pho.isEB, &pho.b_isEB);
    fInTree->SetBranchAddress(Form("phoisHLT_%i",ipho), &pho.isHLT, &pho.b_isHLT);
    fInTree->SetBranchAddress(Form("phoID_%i",ipho), &pho.ID, &pho.b_ID);
  }
}
