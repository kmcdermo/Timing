#include "Common.cpp+"

// ROOT includes
#include "TStyle.h"
#include "TString.h"
#include "TFile.h"
#include "TH2F.h"
#include "TH1F.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TLegend.h"

// c++ includes
#include <iostream>
#include <string>
#include <map>
#include <vector>

////////////////
// Containers //
////////////////

struct ConfigStruct
{
  ConfigStruct () {}

  TString x_title;
  TString y_title;
  std::vector<Double_t> xbin_boundaries;
  std::vector<Double_t> ybin_boundaries;
};

struct SignifStruct
{
  SignifStruct () {}
  SignifStruct (const Double_t signif, const Double_t xbin_boundary, const Double_t ybin_boundary)
    : signif(signif), xbin_boundary(xbin_boundary), ybin_boundary(ybin_boundary) {}
  
  Double_t signif;
  Double_t xbin_boundary;
  Double_t ybin_boundary;
};

/////////////////////////
// Config Sub-routines //
/////////////////////////

void SetupCommon();
void ReadConfig(const TString & signif_config, ConfigStruct & config);
void PrepBoundaries(const std::string & str, const std::string & delim, std::vector<Double_t> & boundaries);

///////////////////////
// Boundary Plotting //
///////////////////////

void countBoundaries(const ConfigStruct & config, const std::map<TString,SignifStruct> & SignifMap, 
		     const TString & signif_outtext, TFile * outfile);
void projectHist(TH2F * hist2D, const TString & s_hist, const TString & proj, 
		 const TString & signif_outtext, TFile * outfile);
void saveHist(TH1F * hist1D, const TString & s_hist1D, const TString & signif_outtext, const Bool_t isLogy);

/////////////////////
// Signif Plotting //
/////////////////////

inline Double_t ctau(TString signalsubgroup);
void savePlot(TCanvas * canv, TLegend * leg, std::vector<TGraph*> & graphs,
	      const UInt_t nsub, const TString & signif_outtext, const Bool_t isLogy);

////////////////
// Main macro //
////////////////

void plotSignificances(const TString & signif_list, const TString & signif_config, const TString & signif_outtext)
{
  // set style
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);

  // init config
  ConfigStruct config;

  // setup config
  SetupCommon();
  ReadConfig(signif_config,config);

  // make output file
  auto outfile = TFile::Open(signif_outtext+".root","RECREATE");

  // dump sig info into map
  std::map<TString,SignifStruct> SignifMap;

  // read list into map
  std::ifstream infile(signif_list.Data(),std::ios::in);
  TString sample, infilename;
  Double_t significance, xbin_boundary, ybin_boundary;

  while (infile >> sample >> significance >> xbin_boundary >> ybin_boundary >> infilename)
  {
    SignifMap[sample] = {significance,xbin_boundary,ybin_boundary};
  }

  // make quick hists of counting boundaries
  countBoundaries(config,SignifMap,signif_outtext,outfile);

  // get signals vec
  std::vector<TString> signalsubgroups;
  for (const auto & SignalSubGroupPair : Common::SignalSubGroupMap)
  {
    signalsubgroups.emplace_back(SignalSubGroupPair.first);
  }
  std::sort(signalsubgroups.begin(),signalsubgroups.end(),
	    [](const auto & signalsubgroup1, const auto & signalsubgroup2)
	    {
	      const auto ctau_signalsubgroup1 = ctau(signalsubgroup1);
	      const auto ctau_signalsubgroup2 = ctau(signalsubgroup2);
	      return (ctau_signalsubgroup1 < ctau_signalsubgroup2);
	    });

  // cache size of signal subgroups
  const auto nsub = signalsubgroups.size();

  // draw stuff
  auto canv = new TCanvas();
  canv->cd();
  
  auto leg = new TLegend(0.6,0.75,0.8,0.92);
  leg->SetBorderSize(1);
  leg->SetLineColor(kBlack);

  // make graphs
  std::vector<TGraph*> graphs(nsub);
  for (auto isub = 0U; isub < nsub; isub++)
  {
    const auto & subgroup = signalsubgroups[isub];
    const auto & signals  = Common::SignalSubGroupMap[subgroup];
    const auto nsig       = signals.size();
    const auto color      = Common::SignalSubGroupColorMap[subgroup].color;
    auto & graph          = graphs[isub];
    
    graph = new TGraph(nsig);
    graph->SetName (subgroup+"_"+signif_outtext);
    graph->SetTitle(subgroup+" "+signif_outtext);
    graph->SetLineColor  (color);
    graph->SetMarkerColor(color);

    // fill it
    for (auto isig = 0U; isig < nsig; isig++)
    {
      const auto & signal = signals[isig];

      const TString s_lambda = "_L";
      auto i_lambda = signal.Index(s_lambda);
      auto l_lambda = s_lambda.Length();
      
      const TString s_ctau = "_CTau";
      auto i_ctau = signal.Index(s_ctau);
    
      const TString lambda(signal(i_lambda+l_lambda,i_ctau-i_lambda-l_lambda));

      if (SignifMap.count(signal))
      {
	graph->SetPoint(isig,lambda.Atoi(),SignifMap.at(signal).signif);
      }
    }

    // draw it
    graph->Draw(isub>0?"P SAME":"AP");
    graph->GetXaxis()->SetTitle("#Lambda");
    graph->GetYaxis()->SetTitle("Significance");

    // add to legend
    auto label = subgroup;
    label.ReplaceAll("GMSB_CTau","c#tau: ");
    label.ReplaceAll("p",".");
    leg->AddEntry(graph,label.Data(),"p");

    // save it
    outfile->cd();
    graph->Write(graph->GetName(),TObject::kWriteDelete);
  }

  // log plot
  savePlot(canv,leg,graphs,nsub,signif_outtext,true);

  // linear plot
  savePlot(canv,leg,graphs,nsub,signif_outtext,false);

  // save it
  outfile->cd();
  leg->Write(leg->GetName(),TObject::kWriteDelete);
  canv->Write(canv->GetName(),TObject::kWriteDelete);

  // delete it all
  for (auto & graph : graphs) delete graph;
  delete leg;
  delete canv;
  delete outfile;
  delete tdrStyle;
}

/////////////////////////
// Config Sub-routines //
/////////////////////////

void SetupCommon()
{
  Common::SetupEras();
  Common::SetupSignalSamples();
  Common::SetupSignalGroups();
  Common::SetupSignalSubGroups();
  Common::SetupSignalSubGroupColors();
}

void PrepBoundaries(const std::string & str, const std::string & delim, std::vector<Double_t> & boundaries)
{
  std::ifstream boundary_infile(Common::RemoveDelim(str,delim).c_str(),std::ios::in);
  Double_t boundary;
  
  while (boundary_infile >> boundary)
  {
    boundaries.emplace_back(boundary);
  }
}

void ReadConfig(const TString & signif_config, ConfigStruct & config)
{
  std::ifstream config_infile(signif_config.Data(),std::ios::in);
  std::string str;

  while (std::getline(config_infile,str))
  {
    if (str == "") continue;
    else if (str.find("x_title=") != std::string::npos)
    {
      config.x_title = Common::RemoveDelim(str,"x_title=");
    }
    else if (str.find("y_title=") != std::string::npos)
    {
      config.y_title = Common::RemoveDelim(str,"y_title=");
    }
    else if (str.find("xbin_boundaries=") != std::string::npos)
    {
      PrepBoundaries(str,"xbin_boundaries=",config.xbin_boundaries);
    }
    else if (str.find("ybin_boundaries=") != std::string::npos)
    {
      PrepBoundaries(str,"ybin_boundaries=",config.ybin_boundaries);
    }
    else
    {
      std::cerr << "Signif config is messed up: " << str.c_str() << " is not a valid setting. Exiting..." << std::endl;
      exit(1);
    }
  }
}

///////////////////////
// Boundary Plotting //
///////////////////////

void countBoundaries(const ConfigStruct & config, const std::map<TString,SignifStruct> & SignifMap,
		     const TString & signif_outtext, TFile * outfile)
{
  // make 2D hist, then project it
  outfile->cd();
 
  const TString s_hist = "Boundaries";
  const TString s_hist2D = s_hist+"2D";
  auto hist2D = new TH2F(s_hist2D.Data(),s_hist2D+";"+config.x_title+";"+config.y_title+";",
			 config.xbin_boundaries.size()-1,&config.xbin_boundaries[0],
			 config.ybin_boundaries.size()-1,&config.ybin_boundaries[0]);
  hist2D->Sumw2();

  // fill 2d hist
  for (const auto & SignifPair : SignifMap)
  {
    const auto & SignifInfo = SignifPair.second;
    hist2D->Fill(SignifInfo.xbin_boundary,SignifInfo.ybin_boundary);
  }

  // plot it (scale by bin widths, make canv, save to output file)
  Common::Scale(hist2D,false,true,true);
  
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogz(1);
  
  hist2D->Draw("colz");
  Common::CMSLumi(canv,0,"Full");

  for (const auto & extension : Common::extensions)
  {
    canv->SaveAs(signif_outtext+"_"+s_hist2D+"."+extension);
  }

  outfile->cd();
  hist2D->Write(hist2D->GetName(),TObject::kWriteDelete);
 
  // delete canv
  delete canv;
 
  // project in X, then Y, plot and save
  Common::Scale(hist2D,true,true,true); // scale back up first

  projectHist(hist2D,s_hist,"X",signif_outtext,outfile);
  projectHist(hist2D,s_hist,"Y",signif_outtext,outfile);

  // delete the rest
  delete hist2D;
}

void projectHist(TH2F * hist2D, const TString & s_hist, const TString & proj, 
		 const TString & signif_outtext, TFile * outfile)
{
  const TString s_hist1D = s_hist+proj;
  const auto isX = proj.EqualTo("X",TString::kExact);

  auto hist1D = (TH1F*)( isX ? hist2D->ProjectionX(s_hist1D.Data()) : hist2D->ProjectionY(s_hist1D.Data()) );
  Common::Scale(hist1D,false);
  
  outfile->cd();
  hist1D->Write(hist1D->GetName(),TObject::kWriteDelete);
  
  // save it lin and log
  saveHist(hist1D,s_hist1D,signif_outtext,false);
  saveHist(hist1D,s_hist1D,signif_outtext,true);

  delete hist1D;
}

void saveHist(TH1F * hist1D, const TString & s_hist1D, const TString & signif_outtext, const Bool_t isLogy)
{
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy(isLogy);

  hist1D->Draw("ep");
  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,signif_outtext+"_"+s_hist1D+"_"+(isLogy?"log":"lin"));

  // delete when done
  delete canv;
}

/////////////////////
// Signif Plotting //
/////////////////////

inline Double_t ctau(TString signalsubgroup)
{
  signalsubgroup.ReplaceAll("GMSB_CTau","");
  signalsubgroup.ReplaceAll("cm","");
  signalsubgroup.ReplaceAll("p",".");

  return signalsubgroup.Atof();
}

void savePlot(TCanvas * canv, TLegend * leg, std::vector<TGraph*> & graphs,
	      const UInt_t nsub, const TString & signif_outtext, const Bool_t isLogy)
{
  canv->SetLogy(isLogy);
  for (auto isub = 0; isub < nsub; isub++)
  {
    auto & graph = graphs[isub];
    graph->GetYaxis()->SetRangeUser((isLogy?1e-3:0),(isLogy?1.5:1.05));
    graph->Draw(isub>0?"P SAME":"AP");
    canv->Update();
  }
  leg->Draw("SAME");
  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,signif_outtext+"_"+(isLogy?"log":"lin"));
}
