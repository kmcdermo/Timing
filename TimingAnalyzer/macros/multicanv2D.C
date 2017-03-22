#include "TStyle.h"
#include "TROOT.h"
#include "TFile.h"
#include "TH1F.h"
#include "THStack.h"
#include "TCanvas.h"
#include "TLegend.h"

#include "common/common.C"

#include <vector>
#include <map>
#include <utility>

void multicanv2D()
{
  // set TDR Style (need to force it!)
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  std::vector<TString> histnames = {"photonseedtime_vs_photonpt","photonseedtime_vs_photonHoE","photonseedtime_vs_photonr9","photonseedtime_vs_photonsieie",
				    "photonseedtime_vs_photonsmaj","photonseedtime_vs_photonsmin","photonseedtime_vs_photonalpha","photonseedtime_vs_photonsmin_ov_photonsmaj"};
  std::vector<TString> photons   = {"ph","ph1","phdelay"};
  std::vector<TString> cuts      = {"cuts_phanypt10.0_rhE1.0","cuts_jetpt30.0_njets3_ph1pt50.0_ph1VIDmedium_phanypt10.0_phanyVIDloose_rhE1.0"};
  std::vector<TString> regions   = {"ecalaccept","EBOnly","EEOnly"};

  std::map<TString,std::vector<TString> > ioinfo;
  ioinfo["output/MC/signal/GMSB/photondump/ctau"] = {"100","2000","6000"};
  ioinfo["output/MC/signal/HVDS/photondump/ctau"] = {"100","1000"};
  
  for (std::map<TString,std::vector<TString> >::const_iterator mapiter = ioinfo.begin(); mapiter != ioinfo.end(); ++mapiter)
  {
    for (UInt_t ictau = 0; ictau < mapiter->second.size(); ictau++)
    {
      for (UInt_t iregion = 0; iregion < regions.size(); iregion++)
      {
	for (UInt_t ihistname = 0; ihistname < histnames.size(); ihistname++)
	{
	  // declare canvas
	  TCanvas * canv = new TCanvas("canv","canv",2000,1300); 
	  canv->Divide(photons.size(),cuts.size(),0.0001,0.001);  
	  for (UInt_t icut = 0; icut < cuts.size(); icut++)
          {
	    TFile * file = TFile::Open(Form("%s%s/%s_%s/plots.root",mapiter->first.Data(),mapiter->second[ictau].Data(),cuts[icut].Data(),regions[iregion].Data()));
	    for (UInt_t iphoton = 0; iphoton < photons.size(); iphoton++)
	    {
	      // cd to the right pad
	      Int_t canvid = iphoton + photons.size() * icut + 1; // goddamn ROOT and its fucking horrible index notation
	      canv->cd(canvid);

	      TString histname = histnames[ihistname];
	      histname.ReplaceAll("photon",photons[iphoton]);
	    
	      // get and draw the histogram in question
	      TH1F * hist = (TH1F*)file->Get(histname.Data());
	      hist->Scale(1.f/hist->Integral());
	      hist->Draw("colz");
	    } // end loop over group of photon
	  } // end loop over group of cuts
	  // Rename ecal accept
	  TString region = regions[iregion];
	  if (region.EqualTo("ecalaccept",TString::kExact)) region = "InclusiveECAL";

	  // save and delete the canvas
	  canv->SaveAs(Form("%s%s/%s_%s.png",mapiter->first.Data(),mapiter->second[ictau].Data(),histnames[ihistname].Data(),region.Data()));
	  delete canv;
	} // end loop over histogram names
      } // end loop over ECAL region
    } // end loop over ctau
  } // end loop over distinct signal groups

}
