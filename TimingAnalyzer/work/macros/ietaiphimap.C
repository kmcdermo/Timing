#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"

#include <iostream>
#include <vector>
#include <map>
#include <fstream>

struct epid
{
public:
  epid() {}
  Int_t i1_; // iphi (EB) or ix (EE)
  Int_t i2_; // ieta (EB) or iy (EE)
};

typedef std::map<Int_t,epid> epidMap;

void getMap(epidMap & epids)
{
  //  initialize map of detector ids of iphi/ieta
  std::ifstream inputids;
  inputids.open("config/detids.txt",std::ios::in);
  Int_t ID;
  Int_t i1, i2;
  TString name;
  while (inputids >> ID >> i1 >> i2 >> name){
    epids[ID]       = epid();
    epids[ID].i1_   = i1;
    epids[ID].i2_   = i2;
  }
  inputids.close();
}

void ietaiphimap()
{
  // initialize ecal map
  epidMap epids;
  getMap(epids);

  // initialize tree and file + options
  TFile * file = TFile::Open("input/DATA/doubleeg/skimmedtree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");

  TString el = "el1";

  // initialize branches !!!!!!!!!!! ALWAYS SET VECTOR OF BRANCHES TO ZERO OR SEG FAULTS BECAUSE ROOT IS PIECE OF %^&*!
  std::vector<Float_t> * rhtimes = 0; tree->SetBranchAddress(Form("%srhtimes",el.Data()), &rhtimes);
  std::vector<Float_t> * rhEs = 0;    tree->SetBranchAddress(Form("%srhEs",el.Data()), &rhEs);
  std::vector<Int_t>   * rhids = 0;   tree->SetBranchAddress(Form("%srhids",el.Data()), &rhids);
  Int_t nrh;        tree->SetBranchAddress(Form("%snrh",el.Data()),&nrh);
  Float_t scE;      tree->SetBranchAddress(Form("%sscE",el.Data()), &scE);
  Float_t seedtime; tree->SetBranchAddress(Form("%sseedtime",el.Data()), &seedtime);
  Float_t seedE;    tree->SetBranchAddress(Form("%sseedE",el.Data()), &seedE);

  // output histograms
  UInt_t entry = 601; //49, 601, 833, 1213, 1675, 1809, 1857, 1979, 2308
  tree->GetEntry(entry);

  Int_t minphi = 1000, maxphi = -1;
  Int_t mineta = 1000, maxeta = -1;
  Float_t maxE = -1.f;
  for (Int_t rh = 0; rh < nrh; rh++)
  {
    const Float_t tmpE = (*rhEs)[rh];    

    if (tmpE < 1) continue;

    const Int_t detid = (*rhids)[rh]; 
    const Int_t tmpphi = epids[detid].i1_; 
    const Int_t tmpeta = epids[detid].i2_;

    if (tmpphi > maxphi) maxphi = tmpphi;
    if (tmpphi < minphi) minphi = tmpphi;
    if (tmpeta > maxeta) maxeta = tmpeta;
    if (tmpeta < mineta) mineta = tmpeta;

    if (tmpE > maxE) maxE = tmpE;
  }

  std::cout << "maxE: " << maxE << " seedE: " << seedE << std::endl;
  std::cout << "minphi: " << minphi << " maxphi: " << maxphi << std::endl;
  std::cout << "mineta: " << mineta << " maxeta: " << maxeta << std::endl;

  TH2F * h_map = new TH2F("h_map","Energy Deposit in ECAL (GeV)",maxeta-mineta+1,mineta,maxeta+1,maxphi-minphi+1,minphi,maxphi+1);
  h_map->SetXTitle("i_{#eta}"); h_map->SetYTitle("i_{#phi}"); h_map->SetMinimum(0.0);
  for (Int_t xbin = 1; xbin <= h_map->GetXaxis()->GetNbins(); xbin++)
  {
    Int_t label = h_map->GetXaxis()->GetBinLowEdge(xbin);
    h_map->GetXaxis()->SetBinLabel(xbin,Form("%i",label));
  }
  for (Int_t ybin = 1; ybin <= h_map->GetYaxis()->GetNbins(); ybin++)
  {
    Int_t label = h_map->GetYaxis()->GetBinLowEdge(ybin);
    h_map->GetYaxis()->SetBinLabel(ybin,Form("%i",label));
  }

  std::ofstream output;
  output.open("scdump.txt",std::ios_base::app);
  output << "Event: " << entry << std::endl;
  output << "----------------" << std::endl;
  for (Int_t rh = 0; rh < nrh; rh++)
  {
    const Float_t tmpE = (*rhEs)[rh];    

    if (tmpE < 1) continue;

    const Int_t detid = (*rhids)[rh]; 
    const Int_t tmpphi = epids[detid].i1_; 
    const Int_t tmpeta = epids[detid].i2_;

    h_map->Fill(tmpeta,tmpphi,tmpE);

    output << "Detid " << detid << " ieta " << tmpeta << " iphi " << tmpphi << " energy " << tmpE << " time " << (*rhtimes)[rh] << std::endl;
  }
  output << "----------------" << std::endl << std::endl;
  output.close();
  
  h_map->Draw("colz");
}
