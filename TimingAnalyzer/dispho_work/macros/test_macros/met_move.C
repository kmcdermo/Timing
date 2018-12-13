#include "met_move.hh"

constexpr Float_t mindR = 0.3;
constexpr Float_t scale_dot = 0.2;

void met_move(const TString & filename, const TString & treename, const TString & textfilename, const TString & outfiletext)
{
  // set style
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);
  Common::SetupEras();

  // get inputs
  auto file = TFile::Open(filename.Data(),"UPDATE");
  Common::CheckValidFile(file,filename);
  
  auto tree = (TTree*)file->Get(treename.Data());
  Common::CheckValidTree(tree,treename,filename);

  // set branches
  Float_t t1pfMETpt = 0.f; TBranch * b_t1pfMETpt = 0; const std::string s_t1pfMETpt = "t1pfMETpt"; tree->SetBranchAddress(s_t1pfMETpt.c_str(), &t1pfMETpt, &b_t1pfMETpt);
  Float_t t1pfMETphi = 0.f; TBranch * b_t1pfMETphi = 0; const std::string s_t1pfMETphi = "t1pfMETphi"; tree->SetBranchAddress(s_t1pfMETphi.c_str(), &t1pfMETphi, &b_t1pfMETphi);
  Int_t nphotons = 0; TBranch * b_nphotons = 0; const std::string s_nphotons = "nphotons"; tree->SetBranchAddress(s_nphotons.c_str(), &nphotons, &b_nphotons);
  
  std::vector<Photon> photonVec(Common::nPhotons);
  for (auto ipho = 0; ipho < Common::nPhotons; ipho++)
  {
    auto & photon = photonVec[ipho];

    tree->SetBranchAddress(Form("pho%s_%i",photon.s_pt.c_str(),ipho), &photon.pt, &photon.b_pt);
    tree->SetBranchAddress(Form("pho%s_%i",photon.s_phi.c_str(),ipho), &photon.phi, &photon.b_phi);
    tree->SetBranchAddress(Form("pho%s_%i",photon.s_eta.c_str(),ipho), &photon.eta, &photon.b_eta);
    tree->SetBranchAddress(Form("pho%s_%i",photon.s_isOOT.c_str(),ipho), &photon.isOOT, &photon.b_isOOT);
  }

  // transients: needed for not double counting
  std::vector<Bool_t> isRemovedVec(Common::nPhotons);

  // output branches
  Float_t hackedMETpt = 0.f; const std::string s_hackedMETpt = "hackedMETpt"; TBranch * b_hackedMETpt = tree->Branch(Form("%s",s_hackedMETpt.c_str()),&hackedMETpt,Form("%s/F",s_hackedMETpt.c_str()));
  Float_t hackedMETphi = 0.f; const std::string s_hackedMETphi = "hackedMETphi"; TBranch * b_hackedMETphi = tree->Branch(Form("%s",s_hackedMETphi.c_str()),&hackedMETphi,Form("%s/F",s_hackedMETphi.c_str()));

  // output hists
  std::vector<HistInfo> Infos = 
  {
    {"h_t1pfMETpt","Old MET","MET [GeV]","Events",50,0,1000.f,"Old MET",kRed},
    {"h_hackedMETpt","New MET","MET [GeV]","Events",50,0,1000.f,"New MET",kBlue}
  };
  std::map<TString,TH1F*> HistMap;
  MakeHists(HistMap,Infos);
  const auto nHists = Infos.size();

  // output textfile and counters
  std::ofstream textfile(textfilename.Data(),std::ios::trunc);
  Int_t nGED = 0;
  Int_t nOOT = 0;
  Int_t nOOT_unmatched = 0;
  Int_t nOOT_matchedGT = 0;
  Int_t nOOT_matchedLT = 0;
  
  const auto nEntries = tree->GetEntries();
  for (auto entry = 0U; entry < nEntries; entry++)
  {
    // dump status check
    if (entry%Common::nEvCheck == 0 || entry == 0) std::cout << "Processing Entry: " << entry << " out of " << nEntries << std::endl;

    // get branches
    b_t1pfMETpt->GetEntry(entry);
    b_t1pfMETphi->GetEntry(entry);
    b_nphotons->GetEntry(entry);

    // nphos used
    const auto nPhotons = std::min(nphotons,Common::nPhotons);

    // get photon branches
    for (auto ipho = 0; ipho < nPhotons; ipho++)
    {
      auto & photon = photonVec[ipho];

      photon.b_pt->GetEntry(entry);
      photon.b_phi->GetEntry(entry);
      photon.b_eta->GetEntry(entry);
      photon.b_isOOT->GetEntry(entry);
    } // end loop over nphotons get entries

    // reset bools
    resetBoolVec(isRemovedVec);

    // set hacked met to input met
    hackedMETpt = t1pfMETpt;
    hackedMETphi = t1pfMETphi;

    // loop over photons, keep highest pt OOT or GED --> sorted in pt, only increment through once in outer
    // + 1 for double loop foward, and then -1 for backward
    // since GED photons already in MET, skip them for outer loop, while check these in inner loop!
    // if object is already matced, skip it, too

    for (auto ipho = 0; ipho < nPhotons; ipho++)
    {
      const auto & photon_i = photonVec[ipho];
      
      if (!photon_i.isOOT)
      { 
	nGED++;
	continue;
      }
      else
      {
	nOOT++;
      }

      // if OOT is completely unmatched, add it to MET
      Bool_t isMatched = false;

      for (auto jpho = ipho+1; jpho < nPhotons; jpho++)
      {
	const auto & photon_j = photonVec[jpho];

	if (photon_j.isOOT) continue;
	if (isRemovedVec[jpho]) continue;

	const auto dR = deltaR(photon_i.phi,photon_i.eta,photon_j.phi,photon_j.eta);
	if (dR < mindR)
	{
	  // compute components
	  const Float_t x = hackedMETpt*std::cos(hackedMETphi) - photon_i.pt*std::cos(photon_i.phi) + photon_j.pt*std::cos(photon_j.phi);
	  const Float_t y = hackedMETpt*std::sin(hackedMETphi) - photon_i.pt*std::sin(photon_i.phi) + photon_j.pt*std::sin(photon_j.phi);
	  
	  // make new met
	  hackedMETpt  = Common::hypot(x,y);
	  hackedMETphi = Common::phi  (x,y);

	  // set bools
	  isRemovedVec[jpho] = true;
	  isMatched = true;

	  // add to counter
	  nOOT_matchedGT++;

	  // only consider the first one matched!
	  break;
	} // end check dR
      } // end loop over j phos

      // backward check for matching --> do NOT add to MET, since OOT photon is lower pt
      if (!isMatched) 
      {
	for (auto jpho = ipho-1; jpho >= 0; jpho--)
        {
	  const auto & photon_j = photonVec[jpho];

	  if (photon_j.isOOT) continue;
	  if (isRemovedVec[jpho]) continue;

	  const auto dR = deltaR(photon_i.phi,photon_i.eta,photon_j.phi,photon_j.eta);
	  if (dR < mindR)
	  {
	    // set bool
	    isMatched = true;
	    
	    // add counter
	    nOOT_matchedLT++;
	    
	    // only consider first match!
	    break;
	  }
	}
      }

      // ootPhoton non-overlapping and by itself
      if (!isMatched)
      {
	// compute components
	const Float_t x = hackedMETpt*std::cos(hackedMETphi) - photon_i.pt*std::cos(photon_i.phi);
	const Float_t y = hackedMETpt*std::sin(hackedMETphi) - photon_i.pt*std::sin(photon_i.phi);
	
	// make new met
	hackedMETpt  = Common::hypot(x,y);
	hackedMETphi = Common::phi  (x,y);

	// add to counter
	nOOT_unmatched++;
      } // end check over is not matched ootPhoton

    } // end loop over i phos

    // store new branches
    b_hackedMETpt->Fill();
    b_hackedMETphi->Fill();
    
    // fill hists
    HistMap["h_t1pfMETpt"]->Fill(t1pfMETpt);
    HistMap["h_hackedMETpt"]->Fill(hackedMETpt);
    
  } // end loop over entries

  // write out tree
  tree->Write(tree->GetName(),TObject::kWriteDelete);

  // write to ouput text file the counters
  textfile << "Events: " << nEntries << std::endl;
  textfile << "nGED Photons: " << nGED << std::endl;
  textfile << "nOOT Photons: " << nOOT << std::endl;
  textfile << "nOOT Photons (Non-overlapping): " << nOOT_unmatched << std::endl;
  textfile << "nOOT Photons (Overlapping and pT > GED): " << nOOT_matchedGT << std::endl;
  textfile << "nOOT Photons (Overlapping and pT < GED): " << nOOT_matchedLT << std::endl;

  // draw some histograms --> quick!
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy();

  // make legend
  auto leg = new TLegend(0.6,0.7,0.8,0.9);
  for (const auto & Info : Infos) leg->AddEntry(HistMap[Info.name],Info.label.Data(),"epl");

  // draw hists + legend
  for (auto ihist = 0; ihist < nHists; ihist++)
  {
    const auto & Info = Infos[ihist];
    auto & hist = HistMap[Info.name];

    hist->SetMarkerSize(tdrStyle->GetMarkerSize()+(0.2f*(1-(ihist/(nHists-1)))));
    hist->Draw(ihist>0?"ep same":"ep");
  }
  leg->Draw("same");

  // save 
  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,outfiletext);

  // delete it all
  delete leg;
  delete canv;
  for (auto & HistPair : HistMap) delete HistPair.second;
  delete tree;
  delete file;
  delete tdrStyle;
}
