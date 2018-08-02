#include "Common.cpp+"

#include "TStyle.h"
#include "TMarker.h"
#include "TColor.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TCanvas.h"
#include "TLegend.h"

#include <vector>

struct File
{
  File() {}
  File(const TString & label, const TString & filename, const Color_t & color)
    : label(label), filename(filename), color(color) {}
  
  TString label;
  TString filename;
  Color_t color;
  TFile * file;
};

struct Graph
{
  Graph() {}
  Graph(const TString & label, const TString & graphname, const Marker_t & marker)
    : label(label), graphname(graphname), marker(marker) {}
  
  TString label;
  TString graphname;
  Marker_t marker;
};

struct SigEff
{
  SigEff() {}
  SigEff(const TString & label, TGraphAsymmErrors * graph, const Color_t & color, const Marker_t & marker)
    : label(label), graph(graph), color(color), marker(marker) {}
  
  TString label;
  TGraphAsymmErrors * graph;
  Color_t color;
  Marker_t marker;
};

void overplot_sigeff()
{
  // set style
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);

  // set file names
  std::vector<File> files = {{"Orig","orig_sig_eff.root",Common::ColorVec[0]},
			     {"DEG","deg_sig_eff.root",Common::ColorVec[1]},
			     {"SPH200","sph200_sig_eff.root",Common::ColorVec[2]}};

  // set up graph names
  std::vector<Graph> graphs = {{"0.1","GMSB_CTau0p1cm_Graph",kFullCircle},
			       {"1200","GMSB_CTau1200cm_Graph",kFullSquare}};

  // read in files, graphs
  std::vector<SigEff> sigeffs;
  for (auto & file : files)
  {
    file.file = TFile::Open(file.filename.Data());
    for (auto & graph : graphs)
    {
      sigeffs.emplace_back(file.label+", c#tau: "+graph.label+" cm",(TGraphAsymmErrors*)file.file->Get(graph.graphname.Data()),file.color,graph.marker);
    }
  }
    
  // new the canvas
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy(1);

  // new the legend
  auto leg = new TLegend(0.55,0.2,0.82,0.42);
  leg->SetBorderSize(1);
  leg->SetLineColor(kBlack);

  // draw on same canvas
  for (auto i = 0U; i < sigeffs.size(); i++)
  {
    auto & sigeff = sigeffs[i];

    sigeff.graph->SetLineColor(sigeff.color);
    sigeff.graph->SetMarkerColor(sigeff.color);
    sigeff.graph->SetMarkerStyle(sigeff.marker);
    sigeff.graph->GetHistogram()->GetYaxis()->SetRangeUser(1e-3f,1.f);

    sigeff.graph->Draw(i>0?"PZ SAME":"APZ");
    leg->AddEntry(sigeff.graph,sigeff.label.Data(),"LEP");
  }

  // final touches
  leg->Draw("SAME");
  Common::CMSLumi(canv,0);
  Common::SaveAs(canv,"overplot_sigeff");
 
  // delete it all
  delete leg;
  delete canv;

  for (auto & sigeff : sigeffs) delete sigeff.graph;
  for (auto & file : files) delete file.file;
  
  delete tdrStyle;
}
