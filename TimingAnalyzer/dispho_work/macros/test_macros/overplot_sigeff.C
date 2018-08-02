#include "Common.cpp+"

#include "TStyle.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TCanvas.h"
#include "TLegend.h"

#include <vector>

struct File
{
  File() {}
  File(const TString & label, const TString & filename) 
    : label(label), filename(filename) {}
  
  TString label;
  TString filename;
  TFile * file;
};

struct Graph
{
  Graph() {}
  Graph(const TString & label, const TString & graphname)
    : label(label), graphname(graphname) {}
  
  TString label;
  TString graphname;
};

struct SigEff
{
  SigEff() {}
  SigEff(const TString & label, const TGraphAsymmErrors *& graph) 
    : label(label), graph(graph) {}
  
  TString label;
  TGraphAsymmErrors * graph;
};

void overplot_sigeff()
{
  // set file names
  std::vector<File> files = {{"Orig","orig_sig_eff.root"},{"DEG","deg_sig_eff.root"},{"SPH200","sph200_sig_eff.root"}};

  // set up graph names
  std::vector<Graph> graphs = {{"0.1","GMSB_CTau0p1cm_Graph"},{"1200","GMSB_CTau1200cm_Graph"}}; 

  // read in files, graphs
  std::vector<SigEff> sigeffs;
  for (auto & file : files)
  {
    file.file = TFile::Open(file.filename.Data());
    for (auto & graph : graphs)
    {
      sigeffs.emplace_back(file.label+", c#tau: "+graph.label+"cm",(TGraphAsymmErrors*)file.file->Get(graph.graphname.Data()));
    }
  }
    
  // new the canvas
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy(1);

  // new the legend
  auto leg = new TLegend(0.2,0.75,0.4,0.92);
  leg->SetBorderSize(1);
  leg->SetLineColor(kBlack);

  // draw on same canvas
  for (auto i = 0U; i < sigeffs.size(); i++)
  {
    auto & sigeff = sigeffs[i];
    
    sigeff.graph->GetHistogram()->GetYaxis()->SetRangeUser(1e-3f,1.f);
    sigeff.graph->Draw(i>0?"P SAME":"AP");
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
}
