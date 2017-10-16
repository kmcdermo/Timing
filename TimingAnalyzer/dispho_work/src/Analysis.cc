#include "../interface/Analysis.hh"
#include "../interface/AnalysisUtils.hh"
#include "TROOT.h"

#include <algorithm>

Analysis::Analysis(const TString & sample, const Bool_t isMC) : fSample(sample), fIsMC(isMC)
{
  // because root is dumb?
  gROOT->ProcessLine("#include <vector>");

  // Set input
  const TString filename = Form("input/%s/%s/%s/%s", Config::year.Data(), (fIsMC?"MC":"DATA"), fSample.Data(), "tree.root");
  fInFile  = TFile::Open(filename.Data());
  CheckValidFile(fInFile,filename);

  const TString treename = "tree/tree";
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
    const TString purwfname = Form("%s/%s/%s",Config::outdir.Data(),Config::pusubdir.Data(),Config::pufilename.Data());
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
    // do this once, and just do it for data
    fTH1Dump.open(Form("%s/%s",Config::outdir.Data(),Config::plotdumpname.Data()),std::ios_base::trunc);
    fTH1PhoDump.open(Form("%s/%s",Config::outdir.Data(),Config::phoplotdumpname.Data()),std::ios_base::trunc);
  }
}

Analysis::~Analysis()
{
  delete fInTree;
  delete fInFile;
  delete fOutFile;
  if (!fIsMC)
  { 
    fTH1Dump.close();
    fTH1PhoDump.close();
  }
}

void Analysis::EventLoop()
{
  // Set up hists first --> first in map is histo name, by design!
  if (Config::doEvStd)    Analysis::SetupEventStandardPlots();
  if (Config::doPhoStd)   Analysis::SetupPhotonStandardPlots();
  if (Config::doIso)      Analysis::SetupIsoPlots();
  if (Config::doIsoNvtx)  Analysis::SetupIsoNvtxPlots();

  // do loop over events, filling histos
  const UInt_t nEntries = (Config::doDemo?Config::demoNum:fInTree->GetEntries());
  for (UInt_t entry = 0; entry < nEntries; entry++)
  {
    fInTree->GetEntry(entry);

    if (Config::dumpStatus) 
    {
      if (entry%Config::nEvCheck == 0 || entry == 0) std::cout << "Processing Entry: " << entry << " out of " << nEntries << std::endl;
    } 

    ////////////////////////////
    //                        // 
    // Determine Event Weight //
    //                        // 
    ////////////////////////////
    Float_t weight = -1.;
    if   (fIsMC) {weight = (fXsec * Config::lumi * genwgt / fWgtsum) * fPUweights[genputrue];}
    else         {weight = 1.0;}

    ////////////////////////////////
    //                            // 
    // Determine how many objects //
    //                            // 
    ////////////////////////////////
    const Int_t Nphotons = std::min(nphotons,Config::nTotalPhotons);
    const Int_t Njets    = std::min(njets,Config::nJets);

    // fill the plots
    if (Config::doEvStd)    Analysis::FillEventStandardPlots(weight);
    if (Config::doPhoStd)   Analysis::FillPhotonStandardPlots(Nphotons,weight);
    if (Config::doIso)      Analysis::FillIsoPlots(Nphotons,weight);
    if (Config::doIsoNvtx)  Analysis::FillIsoNvtxPlots(Nphotons,weight);
  } // end loop over events

   // output hists
  if (Config::doEvStd)    Analysis::OutputEventStandardPlots();
  if (Config::doPhoStd)   Analysis::OutputPhotonStandardPlots();
  if (Config::doIso)      Analysis::OutputIsoPlots();
  if (Config::doIsoNvtx)  Analysis::OutputIsoNvtxPlots();
}

void Analysis::SetupEventStandardPlots()
{
  // event based variables
  const TString dir = "standard";
  const TString ytitle = "Events";

  // nVertices
  stdevTH1Map["nvtx"] = Analysis::MakeTH1Plot("nvtx","",Config::nbinsvtx,0.,Double_t(Config::nbinsvtx),"nVertices",ytitle,stdevTH1SubMap,dir);
  stdevTH2Map["rho_v_nvtx"] = Analysis::MakeTH2Plot("rho_v_nvtx","",Config::nBinsX_iso,0,Config::xhigh_iso,"nVertices",55,0,55,"#rho",stdevTH2SubMap,dir);
}

void Analysis::SetupPhotonStandardPlots()
{
  // Photons
  const TString ytitle = "Events";
  const TString dir = "photon/standard";
  for (Int_t ipho = 0; ipho < Config::nPhotons; ipho++)
  {
    for (const auto & region : Config::regions)
    {
      for (const auto & split : Config::splits)
      {
	const TString name  = Form("%i_%s_%s",ipho,region.Data(),split.Data());
	const TString title = Form("%s - %s",split.Data(),region.Data());

	stdphoTH1Map[Form("phopt_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("phopt_%s",name.Data()),"",50,0.,1000.f,Form("Photon %i p_{T} (%s)",ipho,title.Data()),ytitle,stdphoTH1SubMap,dir);
	stdphoTH1Map[Form("phophi_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("phophi_%s",name.Data()),"",32,-Config::PI,Config::PI,Form("Photon %i #phi (%s)",ipho,title.Data()),ytitle,stdphoTH1SubMap,dir);
	stdphoTH1Map[Form("phoeta_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("phoeta_%s",name.Data()),"",50,-5.f,5.f,Form("Photon %i #eta (%s)",ipho,title.Data()),ytitle,stdphoTH1SubMap,dir);
	stdphoTH1Map[Form("phohoe_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("phohoe_%s",name.Data()),"",50,0.f,0.2,Form("Photon %i H/E (%s)",ipho,title.Data()),ytitle,stdphoTH1SubMap,dir);
	stdphoTH1Map[Form("phor9_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("phor9_%s",name.Data()),"",50,0.f,1.2f,Form("Photon %i R_{9} (%s)",ipho,title.Data()),ytitle,stdphoTH1SubMap,dir);
	stdphoTH1Map[Form("phosieie_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("phosieie_%s",name.Data()),"",50,0.f,0.1,Form("Photon %i #sigma_{i#eta i#eta} (%s)",ipho,title.Data()),ytitle,stdphoTH1SubMap,dir);
	stdphoTH1Map[Form("phosieip_%s",name.Data())] =
	  Analysis::MakeTH1Plot(Form("phosieip_%s",name.Data()),"",50,0.f,0.1,Form("Photon %i #sigma_{i#eta i#phi} (%s)",ipho,title.Data()),ytitle,stdphoTH1SubMap,dir);
	stdphoTH1Map[Form("phosipip_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("phosipip_%s",name.Data()),"",50,0.f,0.1,Form("Photon %i #sigma_{i#phi i#phi} (%s)",ipho,title.Data()),ytitle,stdphoTH1SubMap,dir);
	stdphoTH1Map[Form("phosmaj_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("phosmaj_%s",name.Data()),"",50,0.f,1.f,Form("Photon %i S_{Major} (%s)",ipho,title.Data()),ytitle,stdphoTH1SubMap,dir);
	stdphoTH1Map[Form("phosmin_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("phosmin_%s",name.Data()),"",50,0.f,1.f,Form("Photon %i S_{Minor} (%s)",ipho,title.Data()),ytitle,stdphoTH1SubMap,dir);
	stdphoTH1Map[Form("phoseedtime_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("phoseedtime_%s",name.Data()),"",50,-25.f,25.f,Form("Photon %i Seed Time [ns] (%s)",ipho,title.Data()),ytitle,stdphoTH1SubMap,dir);
      } // end loop over split by type or inclusive
    } // end loop over regions
  } // end loop over nphotons
}

void Analysis::SetupIsoPlots()
{
  // Photon Isolation
  const TString ytitle = "Events";
  const TString dir = "photon/iso";
  for (Int_t ipho = 0; ipho < Config::nPhotons; ipho++)
  {
    for (const auto & region : Config::regions)
    {
      for (const auto & split : Config::splits)
      {
	const TString name  = Form("%i_%s_%s",ipho,region.Data(),split.Data());
	const TString title = Form("%s - %s",split.Data(),region.Data());

	isoTH1Map[Form("phochgiso_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("phochgiso_%s",name.Data()),"",50,0.,20.f,Form("Photon %i PF Charged Hadron Iso (%s)",ipho,title.Data()),ytitle,isoTH1SubMap,dir);
	isoTH1Map[Form("phoneuiso_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("phoneuiso_%s",name.Data()),"",50,0.,20.f,Form("Photon %i PF Neutral Hadron Iso (%s)",ipho,title.Data()),ytitle,isoTH1SubMap,dir);
	isoTH1Map[Form("phophoiso_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("phophoiso_%s",name.Data()),"",50,0.,20.f,Form("Photon %i PF Photon Iso (%s)",ipho,title.Data()),ytitle,isoTH1SubMap,dir);
	isoTH1Map[Form("phoecaliso_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("phoecaliso_%s",name.Data()),"",50,0.,20.f,Form("Photon %i PFCluser ECAL Iso (%s)",ipho,title.Data()),ytitle,isoTH1SubMap,dir);
	isoTH1Map[Form("phohcaliso_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("phohcaliso_%s",name.Data()),"",50,0.,20.f,Form("Photon %i PFCluser HCAL Iso (%s)",ipho,title.Data()),ytitle,isoTH1SubMap,dir);
	isoTH1Map[Form("photrkiso_%s",name.Data())] = 
	  Analysis::MakeTH1Plot(Form("photrkiso_%s",name.Data()),"",50,0.,20.f,Form("Photon %i Track Iso (%s)",ipho,title.Data()),ytitle,isoTH1SubMap,dir);
      } // end loop over split by type or inclusive
    } // end loop over regions
  } // end loop over nphotons
}

void Analysis::SetupIsoNvtxPlots()
{
  // Photon Isolation vs nVertices
  const TString xtitle = "nVertices";
  const TString dir = "photon/iso_v_nvtx";
  for (Int_t ipho = 0; ipho < Config::nPhotons; ipho++)
  {
    for (const auto & region : Config::regions)
    {
      for (const auto & split : Config::splits)
      {
	const TString name  = Form("%i_%s_%s_v_nvtx",ipho,region.Data(),split.Data());
	const TString title = Form("%s - %s",split.Data(),region.Data());

	isonvtxTH2Map[Form("phochgiso_%s",name.Data())] = 
	  Analysis::MakeTH2Plot(Form("phochgiso_%s",name.Data()),"",Config::nBinsX_iso,0,Config::xhigh_iso,xtitle,50,0.,20.f,Form("Photon %i PF Charged Hadron Iso (%s)",ipho,title.Data()),isonvtxTH2SubMap,dir);
	isonvtxTH2Map[Form("phoneuiso_%s",name.Data())] = 
	  Analysis::MakeTH2Plot(Form("phoneuiso_%s",name.Data()),"",Config::nBinsX_iso,0,Config::xhigh_iso,xtitle,50,0.,20.f,Form("Photon %i PF Neutral Hadron Iso (%s)",ipho,title.Data()),isonvtxTH2SubMap,dir);
	isonvtxTH2Map[Form("phophoiso_%s",name.Data())] = 
	  Analysis::MakeTH2Plot(Form("phophoiso_%s",name.Data()),"",Config::nBinsX_iso,0,Config::xhigh_iso,xtitle,50,0.,20.f,Form("Photon %i PF Photon Iso (%s)",ipho,title.Data()),isonvtxTH2SubMap,dir);
	isonvtxTH2Map[Form("phoecaliso_%s",name.Data())] = 
	  Analysis::MakeTH2Plot(Form("phoecaliso_%s",name.Data()),"",Config::nBinsX_iso,0,Config::xhigh_iso,xtitle,50,0.,20.f,Form("Photon %i PFCluser ECAL Iso (%s)",ipho,title.Data()),isonvtxTH2SubMap,dir);
	isonvtxTH2Map[Form("phohcaliso_%s",name.Data())] = 
	  Analysis::MakeTH2Plot(Form("phohcaliso_%s",name.Data()),"",Config::nBinsX_iso,0,Config::xhigh_iso,xtitle,50,0.,20.f,Form("Photon %i PFCluser HCAL Iso (%s)",ipho,title.Data()),isonvtxTH2SubMap,dir);
	isonvtxTH2Map[Form("photrkiso_%s",name.Data())] = 
	  Analysis::MakeTH2Plot(Form("photrkiso_%s",name.Data()),"",Config::nBinsX_iso,0,Config::xhigh_iso,xtitle,50,0.,20.f,Form("Photon %i Track Iso (%s)",ipho,title.Data()),isonvtxTH2SubMap,dir);
      } // end loop over split by type or inclusive
    } // end loop over regions
  } // end loop over nphotons
}

void Analysis::FillEventStandardPlots(const Float_t weight)
{
  stdevTH1Map["nvtx"]->Fill(nvtx,weight);
  stdevTH2Map["rho_v_nvtx"]->Fill(nvtx,rho,weight);
}

void Analysis::FillPhotonStandardPlots(const Int_t Nphotons, const Float_t weight)
{
  Int_t iged = 0, ioot = 0;
  for (Int_t ipho = 0; ipho < Nphotons; ipho++)
  {
    const auto & pho = phos[ipho];
    const Int_t iPho = (!pho.isOOT ? iged++ : ioot++);
    const TString name = Form("%i_%s_%s", (Config::splitPho ? iPho : ipho), (pho.isEB ? "EB" : "EE"), (!pho.isOOT ? "GED" : "OOT"));

    stdphoTH1Map[Form("phopt_%s",name.Data())]->Fill(pho.pt,weight);
    stdphoTH1Map[Form("phophi_%s",name.Data())]->Fill(pho.phi,weight);
    stdphoTH1Map[Form("phoeta_%s",name.Data())]->Fill(pho.eta,weight);
    stdphoTH1Map[Form("phohoe_%s",name.Data())]->Fill(pho.HoE,weight);
    stdphoTH1Map[Form("phor9_%s",name.Data())]->Fill(pho.r9,weight); 
    stdphoTH1Map[Form("phosieie_%s",name.Data())]->Fill(pho.sieie,weight);
    stdphoTH1Map[Form("phosieip_%s",name.Data())]->Fill(pho.sipip,weight);
    stdphoTH1Map[Form("phosipip_%s",name.Data())]->Fill(pho.sieip,weight);
    stdphoTH1Map[Form("phosmaj_%s",name.Data())]->Fill(pho.smaj,weight);
    stdphoTH1Map[Form("phosmin_%s",name.Data())]->Fill(pho.smin,weight);

    if (pho.seed >= 0)
    { 
      stdphoTH1Map[Form("phoseedtime_%s",name.Data())]->Fill((*rhtime)[pho.seed],weight);
    } // end check over seed
  } // end loop over nphotons
}

void Analysis::FillIsoPlots(const Int_t Nphotons, const Float_t weight)
{
  Int_t iged = 0, ioot = 0;
  for (Int_t ipho = 0; ipho < Nphotons; ipho++)
  {
    const auto & pho = phos[ipho];
    const Int_t iPho = (!pho.isOOT ? iged++ : ioot++);
    const TString name = Form("%i_%s_%s", (Config::splitPho ? iPho : ipho), (pho.isEB ? "EB" : "EE"), (!pho.isOOT ? "GED" : "OOT"));

    const float chgHadIso = (Config::pfIsoEA ? std::max(pho.ChgHadIso - rho * GetChargedHadronEA(pho.sceta),0.f) : pho.ChgHadIso);
    const float neuHadIso = (Config::pfIsoEA ? std::max(pho.NeuHadIso - rho * GetNeutralHadronEA(pho.sceta),0.f) : pho.NeuHadIso);
    const float phoIso    = (Config::pfIsoEA ? std::max(pho.PhoIso    - rho * GetGammaEA        (pho.sceta),0.f) : pho.PhoIso);

    isoTH1Map[Form("phochgiso_%s",name.Data())]->Fill(chgHadIso,weight);
    isoTH1Map[Form("phoneuiso_%s",name.Data())]->Fill(neuHadIso,weight);
    isoTH1Map[Form("phophoiso_%s",name.Data())]->Fill(phoIso,weight);
    isoTH1Map[Form("phoecaliso_%s",name.Data())]->Fill(pho.EcalPFClIso,weight);
    isoTH1Map[Form("phohcaliso_%s",name.Data())]->Fill(pho.HcalPFClIso,weight);
    isoTH1Map[Form("photrkiso_%s",name.Data())]->Fill(pho.TrkIso,weight);
  } // end loop over photons
}

void Analysis::FillIsoNvtxPlots(const Int_t Nphotons, const Float_t weight)
{
  Int_t iged = 0, ioot = 0;
  for (Int_t ipho = 0; ipho < Nphotons; ipho++)
  {
    const auto & pho = phos[ipho];
    const Int_t iPho = (!pho.isOOT ? iged++ : ioot++);
    const TString name = Form("%i_%s_%s_v_nvtx", (Config::splitPho ? iPho : ipho), (pho.isEB ? "EB" : "EE"), (!pho.isOOT ? "GED" : "OOT"));

    const float chgHadIso = (Config::pfIsoEA ? std::max(pho.ChgHadIso - rho * GetChargedHadronEA(pho.sceta),0.f) : pho.ChgHadIso);
    const float neuHadIso = (Config::pfIsoEA ? std::max(pho.NeuHadIso - rho * GetNeutralHadronEA(pho.sceta),0.f) : pho.NeuHadIso);
    const float phoIso    = (Config::pfIsoEA ? std::max(pho.PhoIso    - rho * GetGammaEA        (pho.sceta),0.f) : pho.PhoIso);

    isonvtxTH2Map[Form("phochgiso_%s",name.Data())]->Fill(nvtx,chgHadIso,weight);
    isonvtxTH2Map[Form("phoneuiso_%s",name.Data())]->Fill(nvtx,neuHadIso,weight);
    isonvtxTH2Map[Form("phophoiso_%s",name.Data())]->Fill(nvtx,phoIso,weight);
    isonvtxTH2Map[Form("phoecaliso_%s",name.Data())]->Fill(nvtx,pho.EcalPFClIso,weight);
    isonvtxTH2Map[Form("phohcaliso_%s",name.Data())]->Fill(nvtx,pho.HcalPFClIso,weight);
    isonvtxTH2Map[Form("photrkiso_%s",name.Data())]->Fill(nvtx,pho.TrkIso,weight);    
  } // end loop over nphotons
}      

void Analysis::OutputEventStandardPlots() 
{
  MakeSubDirs(stdevTH1SubMap,fOutDir);
  Analysis::SaveTH1s(stdevTH1Map,stdevTH1SubMap);
  if (!fIsMC) Analysis::DumpTH1Names(stdevTH1Map,stdevTH1SubMap);
  Analysis::DeleteTH1s(stdevTH1Map);

  Analysis::SaveTH2s(stdevTH2Map,stdevTH2SubMap);
  for (TH2MapIter mapiter = stdevTH2Map.begin(); mapiter != stdevTH2Map.end(); ++mapiter)
  {
    const TString name = mapiter->first;
    Analysis::Make1DIsoPlots(mapiter->second,stdevTH2SubMap[name],name);
  }
  Analysis::DeleteTH2s(stdevTH2Map);
}

void Analysis::OutputPhotonStandardPlots() 
{
  MakeSubDirs(stdphoTH1SubMap,fOutDir);
  Analysis::MakeInclusiveTH1s(stdphoTH1Map,stdphoTH1SubMap);  
  Analysis::SaveTH1s(stdphoTH1Map,stdphoTH1SubMap);
  if (!fIsMC)
  { 
    Analysis::DumpTH1Names(stdphoTH1Map,stdphoTH1SubMap);
    Analysis::DumpTH1PhoNames(stdphoTH1Map,stdphoTH1SubMap);
  }
  Analysis::DeleteTH1s(stdphoTH1Map);
}

void Analysis::OutputIsoPlots() 
{
  MakeSubDirs(isoTH1SubMap,fOutDir);
  Analysis::MakeInclusiveTH1s(isoTH1Map,isoTH1SubMap);
  Analysis::SaveTH1s(isoTH1Map,isoTH1SubMap);
  if (!fIsMC)
  { 
    Analysis::DumpTH1Names(isoTH1Map,isoTH1SubMap);
    Analysis::DumpTH1PhoNames(isoTH1Map,isoTH1SubMap);
  }
  Analysis::DeleteTH1s(isoTH1Map);
}

void Analysis::OutputIsoNvtxPlots() 
{
  MakeSubDirs(isonvtxTH2SubMap,fOutDir);
  Analysis::MakeInclusiveTH2s(isonvtxTH2Map,isonvtxTH2SubMap);
  Analysis::SaveTH2s(isonvtxTH2Map,isonvtxTH2SubMap);
  for (TH2MapIter mapiter = isonvtxTH2Map.begin(); mapiter != isonvtxTH2Map.end(); ++mapiter)
  {
    const TString name = mapiter->first;
    Analysis::Make1DIsoPlots(mapiter->second,isonvtxTH2SubMap[name],name);
  }
  Analysis::DeleteTH2s(isonvtxTH2Map);
}

void Analysis::MakeInclusiveTH1s(TH1Map & th1map, TStrMap & subdirmap)
{
  Analysis::MakeInclusiveNphoTH1s(th1map,subdirmap);
  Analysis::MakeInclusiveSplitTH1s(th1map,subdirmap);
  //  Analysis::MakeInclusiveRegionTH1s(th1map,subdirmap);
}

void Analysis::MakeInclusiveNphoTH1s(TH1Map & th1map, TStrMap & subdirmap) 
{
  // Use leading photon as base (i.e. pho0)
  const TString drop = "0"; 
  TStrVec names;
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); ++mapiter) 
  {
    TString name = mapiter->second->GetName();
    if (name.Contains(drop,TString::kExact)) names.emplace_back(name);
  }

  const Ssiz_t length = drop.Length();
  for (auto & name : names)
  {
    // prep declaration of inclusive hist
    TString hname = name; 
    const Ssiz_t hnamepos = hname.Index(drop);
    hname.Remove(hnamepos-1,length+1); // account for "_0"
    
    TString xtitle = th1map[name]->GetXaxis()->GetTitle();
    const Ssiz_t xtitlepos = xtitle.Index(drop);

    xtitle.Remove(xtitlepos-1,length+1); // account for " 0"
    xtitle.ReplaceAll("Photon","All Photons");

    // make inclusive hist
    th1map[hname] = Analysis::MakeTH1Plot(hname,"",th1map[name]->GetNbinsX(),th1map[name]->GetXaxis()->GetXmin(),th1map[name]->GetXaxis()->GetXmax(),
					  xtitle,th1map[name]->GetYaxis()->GetTitle(),subdirmap,subdirmap[name]);

    TString tmpdrop = drop;
    for (Int_t ipho = 0; ipho < Config::nPhotons; ipho++)
    {
      const TString swap = Form("%i",ipho);
      name.ReplaceAll(tmpdrop,swap);
      th1map[hname]->Add(th1map[name]);
      tmpdrop = swap;
    }
  }
}

void Analysis::MakeInclusiveSplitTH1s(TH1Map & th1map, TStrMap & subdirmap) 
{
  // Use GED photons as the base
  const TString drop = "GED";
  TStrVec names;
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); ++mapiter) 
  {
    TString name = mapiter->second->GetName();
    if (name.Contains(drop,TString::kExact)) names.emplace_back(name);
  }

  // loop over names and adjust them accordingly
  const Ssiz_t length = drop.Length();
  const TString swap = "OOT";
  for (auto & name : names)
  {
    // prep declaration of inclusive hist
    TString hname = name; 
    const Ssiz_t hnamepos = hname.Index(drop);
    hname.Remove(hnamepos-1,length+1); // account for "_GED"
    
    TString xtitle = th1map[name]->GetXaxis()->GetTitle();
    const Ssiz_t xtitlepos = xtitle.Index(drop);
    xtitle.Remove(xtitlepos,length+3); // account for "GED - "

    // make inclusive hist
    th1map[hname] = Analysis::MakeTH1Plot(hname,"",th1map[name]->GetNbinsX(),th1map[name]->GetXaxis()->GetXmin(),th1map[name]->GetXaxis()->GetXmax(),
					  xtitle,th1map[name]->GetYaxis()->GetTitle(),subdirmap,subdirmap[name]);

    // add GED first
    th1map[hname]->Add(th1map[name]);

    // add OOT second
    name.ReplaceAll(drop,swap);
    th1map[hname]->Add(th1map[name]);
  }
}

void Analysis::MakeInclusiveRegionTH1s(TH1Map & th1map, TStrMap & subdirmap) 
{
  // Use EB as the base
  const TString drop = "EB";
  TStrVec names;
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); ++mapiter) 
  {
    TString name = mapiter->second->GetName();
    if (name.Contains(drop,TString::kExact)) names.emplace_back(name);
  }

  // loop over names and adjust them accordingly
  const Ssiz_t length = drop.Length();
  const TString swap = "EE";
  for (auto & name : names)
  {
    // prep declaration of inclusive hist
    TString hname = name; 
    const Ssiz_t hnamepos = hname.Index(drop);
    hname.Remove(hnamepos-1,length+1); // account for "EB"
    
    TString xtitle = th1map[name]->GetXaxis()->GetTitle();
    const Ssiz_t xtitlepos = xtitle.Index(drop);
    xtitle.Remove(xtitlepos-1,length+2); // account for "(EB)"

    // make inclusive hist
    th1map[hname] = Analysis::MakeTH1Plot(hname,"",th1map[name]->GetNbinsX(),th1map[name]->GetXaxis()->GetXmin(),th1map[name]->GetXaxis()->GetXmax(),
					  xtitle,th1map[name]->GetYaxis()->GetTitle(),subdirmap,subdirmap[name]);

    // add EB first
    th1map[hname]->Add(th1map[name]);

    // add EE second
    name.ReplaceAll(drop,swap);
    th1map[hname]->Add(th1map[name]);
  }
}

void Analysis::MakeInclusiveTH2s(TH2Map & th2map, TStrMap & subdirmap)
{
  Analysis::MakeInclusiveNphoTH2s(th2map,subdirmap);
  Analysis::MakeInclusiveSplitTH2s(th2map,subdirmap);
  //  Analysis::MakeInclusiveRegionTH2s(th2map,subdirmap);
}

void Analysis::MakeInclusiveNphoTH2s(TH2Map & th2map, TStrMap & subdirmap) 
{
  // Use leading photon as base (i.e. pho0)
  const TString drop = "0"; 
  TStrVec names;
  for (TH2MapIter mapiter = th2map.begin(); mapiter != th2map.end(); ++mapiter) 
  {
    TString name = mapiter->second->GetName();
    if (name.Contains(drop,TString::kExact)) names.emplace_back(name);
  }

  const Ssiz_t length = drop.Length();
  for (auto & name : names)
  {
    // prep declaration of inclusive hist
    TString hname = name; 
    const Ssiz_t hnamepos = hname.Index(drop);
    hname.Remove(hnamepos-1,length+1); // account for "_0"
    
    TString ytitle = th2map[name]->GetYaxis()->GetTitle();
    const Ssiz_t ytitlepos = ytitle.Index(drop);
    ytitle.Remove(ytitlepos-1,length+1); // account for " 0"
    ytitle.ReplaceAll("Photon","All Photons");

    // make inclusive hist
    th2map[hname] = Analysis::MakeTH2Plot(hname,"",th2map[name]->GetNbinsX(),th2map[name]->GetXaxis()->GetXmin(),th2map[name]->GetXaxis()->GetXmax(),
					  th2map[name]->GetXaxis()->GetTitle(),th2map[name]->GetNbinsY(),th2map[name]->GetYaxis()->GetXmin(),
					  th2map[name]->GetYaxis()->GetXmax(),ytitle,subdirmap,subdirmap[name]);

    TString tmpdrop = drop;
    for (Int_t ipho = 0; ipho < Config::nPhotons; ipho++)
    {
      const TString swap = Form("%i",ipho);
      name.ReplaceAll(tmpdrop,swap);
      th2map[hname]->Add(th2map[name]);
      tmpdrop = swap;
    }
  }
}

void Analysis::MakeInclusiveSplitTH2s(TH2Map & th2map, TStrMap & subdirmap) 
{
  // Use GED photons as the base
  const TString drop = "GED";
  TStrVec names;
  for (TH2MapIter mapiter = th2map.begin(); mapiter != th2map.end(); ++mapiter) 
  {
    TString name = mapiter->second->GetName();
    if (name.Contains(drop,TString::kExact)) names.emplace_back(name);
  }

  // loop over names and adjust them accordingly
  const Ssiz_t length = drop.Length();
  const TString swap = "OOT";
  for (auto & name : names)
  {
    // prep declaration of inclusive hist
    TString hname = name; 
    const Ssiz_t hnamepos = hname.Index(drop);
    hname.Remove(hnamepos-1,length+1); // account for "_GED"
    
    TString ytitle = th2map[name]->GetYaxis()->GetTitle();
    const Ssiz_t ytitlepos = ytitle.Index(drop);
    ytitle.Remove(ytitlepos,length+3); // account for "GED - "

    // make inclusive hist
    th2map[hname] = Analysis::MakeTH2Plot(hname,"",th2map[name]->GetNbinsX(),th2map[name]->GetXaxis()->GetXmin(),th2map[name]->GetXaxis()->GetXmax(),
					  th2map[name]->GetXaxis()->GetTitle(),th2map[name]->GetNbinsY(),th2map[name]->GetYaxis()->GetXmin(),
					  th2map[name]->GetYaxis()->GetXmax(),ytitle,subdirmap,subdirmap[name]);

    // add GED first
    th2map[hname]->Add(th2map[name]);

    // add OOT second
    name.ReplaceAll(drop,swap);
    th2map[hname]->Add(th2map[name]);
  }
}

void Analysis::MakeInclusiveRegionTH2s(TH2Map & th2map, TStrMap & subdirmap) 
{
  // Use EB as the base
  const TString drop = "EB";
  TStrVec names;
  for (TH2MapIter mapiter = th2map.begin(); mapiter != th2map.end(); ++mapiter) 
  {
    TString name = mapiter->second->GetName();
    if (name.Contains(drop,TString::kExact)) names.emplace_back(name);
  }

  // loop over names and adjust them accordingly
  const Ssiz_t length = drop.Length();
  const TString swap = "EE";
  for (auto & name : names)
  {
    // prep declaration of inclusive hist
    TString hname = name; 
    const Ssiz_t hnamepos = hname.Index(drop);
    hname.Remove(hnamepos-1,length+1); // account for "EB"
    
    TString ytitle = th2map[name]->GetYaxis()->GetTitle();
    const Ssiz_t ytitlepos = ytitle.Index(drop);
    ytitle.Remove(ytitlepos-1,length+2); // account for "(EB)"

    // make inclusive hist
    th2map[hname] = Analysis::MakeTH2Plot(hname,"",th2map[name]->GetNbinsX(),th2map[name]->GetXaxis()->GetXmin(),th2map[name]->GetXaxis()->GetXmax(),
					  th2map[name]->GetXaxis()->GetTitle(),th2map[name]->GetNbinsY(),th2map[name]->GetYaxis()->GetXmin(),
					  th2map[name]->GetYaxis()->GetXmax(),ytitle,subdirmap,subdirmap[name]);

    // add EB first
    th2map[hname]->Add(th2map[name]);

    // add EE second
    name.ReplaceAll(drop,swap);
    th2map[hname]->Add(th2map[name]);
  }
}

void Analysis::Make1DIsoPlots(const TH2F * hist2d, const TString & subdir2d, const TString & name)
{
  TH1Map th1dmap; TStrMap th1dsubmap; TStrIntMap th1dbinmap;
  Analysis::Project2Dto1D(hist2d,subdir2d,th1dmap,th1dsubmap,th1dbinmap);
  if (Config::useMean)
  {
    Analysis::ProduceMeanHist(hist2d,subdir2d,th1dmap,th1dbinmap);
  }
  else 
  {
    Analysis::ProduceQuantile(hist2d,subdir2d,th1dmap,th1dbinmap);
  }
  if (Config::saveTempHists) 
  {
    Analysis::SaveTH1s(th1dmap,th1dsubmap);
    if (!fIsMC)
    {
      Analysis::DumpTH1Names(th1dmap,th1dsubmap);
      Analysis::DumpTH1PhoNames(th1dmap,th1dsubmap);
    }
  }
  Analysis::DeleteTH1s(th1dmap);
}

void Analysis::Project2Dto1D(const TH2F * hist2d, const TString & subdir2d, TH1Map & th1dmap, TStrMap & subdir1dmap, TStrIntMap & th1dbinmap) 
{
  const TString  basename = hist2d->GetName();
  const Int_t    nBinsX   = hist2d->GetNbinsX();
  const TString  xtitle   = hist2d->GetXaxis()->GetTitle();
  const Int_t    nBinsY   = hist2d->GetNbinsY();
  const Double_t ylow     = hist2d->GetYaxis()->GetXmin();
  const Double_t yhigh    = hist2d->GetYaxis()->GetXmax();
  const TString  ytitle   = hist2d->GetYaxis()->GetTitle();

  // loop over all x bins to project out
  for (Int_t ibinx = 1; ibinx <= nBinsX; ibinx++)
  {  
    // if no bins are filled, then continue to next plot
    Bool_t isFilled = false;
    for (Int_t ibiny = 0; ibiny <= nBinsY + 1; ibiny++) 
    {
      if (hist2d->GetBinContent(ibinx,ibiny) > 0) {isFilled = true; break;}
    }
    if (!isFilled) continue;

    const Double_t xlow  = hist2d->GetXaxis()->GetBinLowEdge(ibinx); 
    const Double_t xhigh = hist2d->GetXaxis()->GetBinUpEdge(ibinx);

    TString histname = "";
    // First create each histogram
    const Int_t ixlow  = Int_t(xlow); 
    const Int_t ixhigh = Int_t(xhigh); 
    histname = Form("%s_%i_%i_bin%i",basename.Data(),ixlow,ixhigh,ibinx);
    th1dmap[histname.Data()] = Analysis::MakeTH1Plot(histname.Data(),"",nBinsY,ylow,yhigh,
						     Form("%s in %s bin: %i to %i",ytitle.Data(),xtitle.Data(),ixlow,ixhigh),"Events",subdir1dmap,subdir2d);

    th1dbinmap[histname.Data()] = ibinx; // universal pairing

    // then fill corresponding bins from y
    for (Int_t ibiny = 0; ibiny <= nBinsY + 1; ibiny++) 
    {
      th1dmap[histname.Data()]->SetBinContent(ibiny,hist2d->GetBinContent(ibinx,ibiny));
      th1dmap[histname.Data()]->SetBinError(ibiny,hist2d->GetBinError(ibinx,ibiny)); 
    }
  }
}

void Analysis::ProduceQuantile(const TH2F * hist2d, const TString & subdir2d, TH1Map & th1dmap, TStrIntMap & th1dbinmap) 
{
  // initialize new mean/sigma histograms
  TH1F * outhist_quant = Analysis::MakeTH1PlotFromTH2(hist2d,Form("%s_quant_%4.2f",hist2d->GetName(),Config::quantProb),
						      Form("%i%% Quantile of %s",Int_t(100*Config::quantProb),hist2d->GetYaxis()->GetTitle()));

  // use this to store runs that by themselves produce bad fits
  for (TH1MapIter mapiter = th1dmap.begin(); mapiter != th1dmap.end(); ++mapiter) 
  { 
    const Int_t ibin = th1dbinmap[mapiter->first]; // returns which bin each th1 corresponds to one the new plot

    // Perform quantile analysis
    Float_t x = 0.f, dx_dn = 0.f, dx_up = 0.f;
    Analysis::GetQuantileX(mapiter->second,x,dx_dn,dx_up);

    outhist_quant->SetBinContent(ibin,x);
    outhist_quant->SetBinError(ibin,(dx_dn>dx_up?dx_dn:dx_up)); // really should set with tgraphasymmerrors
  } // end loop over th1s

  Analysis::SaveProjectedTH1(outhist_quant,subdir2d);

  delete outhist_quant;
}

void Analysis::ProduceMeanHist(const TH2F * hist2d, const TString & subdir2d, TH1Map & th1dmap, TStrIntMap & th1dbinmap) 
{
  // initialize new mean/sigma histograms
  TH1F * outhist_mean = Analysis::MakeTH1PlotFromTH2(hist2d,Form("%s_mean",hist2d->GetName()),Form("Mean of %s",hist2d->GetYaxis()->GetTitle()));

  // use this to store runs that by themselves produce bad fits
  for (TH1MapIter mapiter = th1dmap.begin(); mapiter != th1dmap.end(); ++mapiter) 
  { 
    const Int_t ibin = th1dbinmap[mapiter->first]; // returns which bin each th1 corresponds to one the new plot
    outhist_mean->SetBinContent(ibin,mapiter->second->GetMean());
    outhist_mean->SetBinError(ibin,mapiter->second->GetMeanError());
  } // end loop over th1s

  Analysis::SaveProjectedTH1(outhist_mean,subdir2d);

  delete outhist_mean;
}

void Analysis::GetQuantileX(const TH1F * hist, Float_t & x, Float_t & dx_dn, Float_t & dx_up)
{
  const Float_t integral = hist->Integral();
  const Int_t   nBinsX   = hist->GetNbinsX();
  
  std::vector<Float_t> eff(nBinsX), eff_err(nBinsX), centers(nBinsX);
  
  Float_t sum = 0.f; 
  for (Int_t ibin = 1; ibin <= nBinsX; ibin++)
  { 
    sum += hist->GetBinContent(ibin); 
    centers[ibin-1] = hist->GetXaxis()->GetBinCenter(ibin);

    if (sum != 0.f && integral != 0.f)
    { 
      eff    [ibin-1] = sum/integral;
      eff_err[ibin-1] = std::sqrt(eff[ibin-1]*(1.f-eff[ibin-1])/integral); 
    }
    else
    {
      eff    [ibin-1] = 0.f;
      eff_err[ibin-1] = 0.f; 
    }
  }

  for (Int_t i = 0; i < nBinsX; i++)
  {
    if (eff[i] > Config::quantProb) 
    {
      x = centers[i];
      break;
    }
  }

  dx_dn = Analysis::FluctuateX(eff,eff_err,centers,x,false);
  dx_up = Analysis::FluctuateX(eff,eff_err,centers,x,true);
}

Float_t Analysis::FluctuateX(const FltVec & eff, const FltVec & eff_err, const FltVec & centers, const Float_t x, const Bool_t isUp)
{
  const Int_t nEffs = eff.size();
  std::vector<Float_t> eff_v(nEffs); // v = variation
  
  Int_t i_v = -1;
  for (Int_t i = 0; i < nEffs; i++)
  {
    eff_v[i] = eff[i] + ((isUp)?1.f:-1.f) * eff_err[i];
    if (eff_v[i] > Config::quantProb) 
    {
      i_v = i;
      break;
    }
  }

  Float_t x_v = 0.f;
  if (i_v != -1)
  {
    if (eff_v[i_v] > 0.f && eff_v[i_v-1] > 0.f && (centers[i_v]-centers[i_v-1]) > 0.f)
    {
      const Float_t slope = (eff_v[i_v]-eff_v[i_v-1])/(centers[i_v]-centers[i_v-1]);
      if (slope > 0.f)
      {
	const Float_t intercept = eff_v[i_v]-slope*centers[i_v];
	x_v = (Config::quantProb-intercept)/slope;
      }
    }
  }

  return ((i_v != -1) ? std::abs(x-x_v) : 0.f);
}
   
TH1F * Analysis::MakeTH1Plot(const TString & hname, const TString & htitle, const Int_t nbinsx, Double_t xlow, Double_t xhigh,
			     const TString & xtitle, const TString & ytitle, TStrMap& subdirmap, const TString & subdir) 
{
  TH1F * hist = new TH1F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh);
  hist->SetLineColor(kBlack);
  hist->SetMarkerColor(fColor);
  if (fIsMC) hist->SetFillColor(fColor);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->GetYaxis()->SetTitleOffset(hist->GetYaxis()->GetTitleOffset() * Config::TitleFF);
  hist->Sumw2();

  // cheat a bit and set subdir map here
  subdirmap[hname] = subdir;
  
  return hist;
}

TH1F * Analysis::MakeTH1PlotFromTH2(const TH2F * hist2d, const TString & name, const TString & ytitle)
{
  TH1F * hist = new TH1F(name.Data(),"",hist2d->GetNbinsX(),hist2d->GetXaxis()->GetXmin(),hist2d->GetXaxis()->GetXmax());
  hist->GetXaxis()->SetTitle(hist2d->GetXaxis()->GetTitle());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->SetLineColor(fColor);
  hist->SetMarkerColor(fColor);
  hist->GetYaxis()->SetTitleOffset(hist->GetYaxis()->GetTitleOffset() * Config::TitleFF);
  hist->Sumw2();
  
  return hist;
}

TH2F * Analysis::MakeTH2Plot(const TString & hname, const TString & htitle, const Int_t nbinsx, const Double_t xlow, const Double_t xhigh, const TString & xtitle,
			     const Int_t nbinsy, const Double_t ylow, const Double_t yhigh, const TString & ytitle, TStrMap& subdirmap, const TString & subdir) 
{
  TH2F * hist = new TH2F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh,nbinsy,ylow,yhigh);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  // cheat a bit and set subdir map here
  subdirmap[hname] = subdir;
  
  return hist;
}

TH2F * Analysis::MakeTH2Plot(const TString & hname, const TString & htitle, const DblVec& vxbins, const TString & xtitle, 
			     const Int_t nbinsy, const Double_t ylow, const Double_t yhigh, const TString & ytitle, TStrMap& subdirmap, const TString & subdir) 
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
  
  TCanvas * canv = new TCanvas("canv","canv");
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); ++mapiter) 
  { 
    // save to output file
    mapiter->second->Write(mapiter->second->GetName(),TObject::kWriteDelete); // map is map["hist name",TH1D*]

    if (Config::saveHists)
    {
      // now draw onto canvas to save as png
      canv->cd();
      mapiter->second->Draw( fIsMC ? "HIST" : "PE" );
      
      // first save as logY, then linearY
      canv->SetLogy(1);
      CMSLumi(canv);
      canv->SaveAs(Form("%s/%s/log/%s.%s",fOutDir.Data(),subdirmap[mapiter->first].Data(),mapiter->first.Data(),Config::outtype.Data()));
    
      canv->SetLogy(0);
      CMSLumi(canv);
      canv->SaveAs(Form("%s/%s/lin/%s.%s",fOutDir.Data(),subdirmap[mapiter->first].Data(),mapiter->first.Data(),Config::outtype.Data()));
    } // end check on save hists
  } // end loop over hists

  delete canv;
}

void Analysis::SaveProjectedTH1(TH1F * hist, const TString & subdir2d)
{
  const TString name = hist->GetName();

  // write output hist to file
  fOutFile->cd();
  hist->Write(hist->GetName(),TObject::kWriteDelete);
  if (!fIsMC)
  {
    fTH1Dump << name.Data() << " " << subdir2d.Data() << std::endl;
    if (name.Contains("GED",TString::kExact)) fTH1PhoDump << name.Data() << " " << subdir2d.Data() << std::endl;
  }

  if (Config::saveHists)
  {
    // save log/lin of each plot
    TCanvas * canv = new TCanvas("canv","canv");
    canv->cd();
    canv->SetLogy(0);
    
    hist->Draw("PE");
    CMSLumi(canv);
    canv->SaveAs(Form("%s/%s/lin/%s.%s",fOutDir.Data(),subdir2d.Data(),hist->GetName(),Config::outtype.Data()));
    
    delete canv;
  }
}

void Analysis::SaveTH2s(TH2Map & th2map, TStrMap & subdirmap) 
{
  fOutFile->cd();

  TCanvas * canv = new TCanvas("canv","canv");
  for (TH2MapIter mapiter = th2map.begin(); mapiter != th2map.end(); ++mapiter) 
  { 
    mapiter->second->Write(mapiter->second->GetName(),TObject::kWriteDelete);; // map is map["hist name",TH1D*]

    if (Config::saveHists)
    {
      // now draw onto canvas to save as png
      canv->cd();
      mapiter->second->Draw("colz");
      
      // only save as linear
      canv->SetLogy(0);
      CMSLumi(canv);
      canv->SaveAs(Form("%s/%s/%s_2D.%s",fOutDir.Data(),subdirmap[mapiter->first].Data(),mapiter->first.Data(),Config::outtype.Data()));
    } // end check on save hists
  } // end loop over hists

  delete canv;
}

void Analysis::DumpTH1Names(TH1Map & th1map, TStrMap & subdirmap) 
{
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); ++mapiter) 
  { 
    fTH1Dump << mapiter->first.Data() << " " << subdirmap[mapiter->first].Data() << std::endl;
  }
}

void Analysis::DumpTH1PhoNames(TH1Map & th1map, TStrMap & subdirmap) 
{
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); ++mapiter) 
  { 
    if (mapiter->first.Contains("GED")) fTH1PhoDump << mapiter->first.Data() << " " << subdirmap[mapiter->first].Data() << std::endl;
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
  phos.resize(Config::nTotalPhotons);
}

void Analysis::InitBranchVecs()
{
  rheta = 0;
  rhphi = 0;
  rhE = 0;
  rhtime = 0;
  rhOOT = 0;
  rhID = 0;

  for (Int_t ipho = 0; ipho < Config::nTotalPhotons; ipho++) 
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
  for (Int_t ipho = 0; ipho < Config::nTotalPhotons; ipho++) 
  {
    auto & pho = phos[ipho];
    fInTree->SetBranchAddress(Form("phoE_%i",ipho), &pho.E, &pho.b_E);
    fInTree->SetBranchAddress(Form("phopt_%i",ipho), &pho.pt, &pho.b_pt);
    fInTree->SetBranchAddress(Form("phoeta_%i",ipho), &pho.eta, &pho.b_eta);
    fInTree->SetBranchAddress(Form("phophi_%i",ipho), &pho.phi, &pho.b_phi);
    fInTree->SetBranchAddress(Form("phoscE_%i",ipho), &pho.scE, &pho.b_scE);
    fInTree->SetBranchAddress(Form("phosceta_%i",ipho), &pho.sceta, &pho.b_sceta);
    fInTree->SetBranchAddress(Form("phoscphi_%i",ipho), &pho.scphi, &pho.b_scphi);
    fInTree->SetBranchAddress(Form("phoHoE_%i",ipho), &pho.HoE, &pho.b_HoE);
    fInTree->SetBranchAddress(Form("phor9_%i",ipho), &pho.r9, &pho.b_r9);
    fInTree->SetBranchAddress(Form("phoChgHadIso_%i",ipho), &pho.ChgHadIso, &pho.b_ChgHadIso);
    fInTree->SetBranchAddress(Form("phoNeuHadIso_%i",ipho), &pho.NeuHadIso, &pho.b_NeuHadIso);
    fInTree->SetBranchAddress(Form("phoPhoIso_%i",ipho), &pho.PhoIso, &pho.b_PhoIso);
    fInTree->SetBranchAddress(Form("phoEcalPFClIso_%i",ipho), &pho.EcalPFClIso, &pho.b_EcalPFClIso);
    fInTree->SetBranchAddress(Form("phoHcalPFClIso_%i",ipho), &pho.HcalPFClIso, &pho.b_HcalPFClIso);
    fInTree->SetBranchAddress(Form("phoTrkIso_%i",ipho), &pho.TrkIso, &pho.b_TrkIso);
    fInTree->SetBranchAddress(Form("phosieie_%i",ipho), &pho.sieie, &pho.b_sieie);
    fInTree->SetBranchAddress(Form("phosipip_%i",ipho), &pho.sipip, &pho.b_sipip);
    fInTree->SetBranchAddress(Form("phosieip_%i",ipho), &pho.sieip, &pho.b_sieip);
    fInTree->SetBranchAddress(Form("phosmaj_%i",ipho), &pho.smaj, &pho.b_smaj);
    fInTree->SetBranchAddress(Form("phosmin_%i",ipho), &pho.smin, &pho.b_smin);
    fInTree->SetBranchAddress(Form("phoalpha_%i",ipho), &pho.alpha, &pho.b_alpha);
    fInTree->SetBranchAddress(Form("phoseed_%i",ipho), &pho.seed, &pho.b_seed);
    fInTree->SetBranchAddress(Form("phorecHits_%i",ipho), &pho.recHits, &pho.b_recHits);
    fInTree->SetBranchAddress(Form("phoisOOT_%i",ipho), &pho.isOOT, &pho.b_isOOT);
    fInTree->SetBranchAddress(Form("phoisEB_%i",ipho), &pho.isEB, &pho.b_isEB);
    fInTree->SetBranchAddress(Form("phoisHLT_%i",ipho), &pho.isHLT, &pho.b_isHLT);
    fInTree->SetBranchAddress(Form("phoisTrk_%i",ipho), &pho.isTrk, &pho.b_isTrk);
    fInTree->SetBranchAddress(Form("phoID_%i",ipho), &pho.ID, &pho.b_ID);
  }
}
