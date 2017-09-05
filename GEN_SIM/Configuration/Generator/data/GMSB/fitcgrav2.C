struct lcg
{
  int lamb;
  float cgrav;
  float width;
};

typedef std::vector<lcg> vlcg;

void fitGraph(TGraph *& graph, TF1 *& fit)
{
  Double_t minx,miny,maxx,maxy;
  graph->GetPoint(0,minx,miny);
  graph->GetPoint(graph->GetN()-1,maxx,maxy);
 
  TFormula form("power","[0]*x**[1]");
  fit  = new TF1("power_fit","power",minx,maxx);
  fit->SetParName(0,"const"); 
  fit->SetParameter(0,1.f);
  fit->SetParName(1,"power"); 
  fit->SetParameter(1,1.f);

  graph->Fit(fit->GetName(),"RB");
}  

void fitcgrav2()
{
  gStyle->SetOptStat(0);

  std::ifstream input("width_cgrav.txt",std::ios_base::in);
  vlcg params;

  float w, cg; int l;
  while(input >> cg >> l >> w)
  {
    //    if (l!=100) continue;
    params.push_back({l,cg,w});
  }

  TGraph * graph = new TGraph(params.size());
  graph->SetTitle("c_{grav}/#sqrt{c#tau} vs. #Lambda;#Lambda;c_{grav}/#sqrt{c#tau}");
  graph->SetMarkerStyle(8);
  graph->SetMarkerSize(0.25);

  Int_t i = 0;
  for (auto&& param : params)
  {
    graph->SetPoint(i,param.lamb,param.cgrav/std::sqrt(1.973e-14/param.width)); // 1.973e-34 = hbar*c in GeV * cm
    i++;
  }

  TCanvas * canv = new TCanvas(); canv->cd();
  graph->Draw("AP");

  gStyle->SetOptFit(1);
  gStyle->SetStatX(0.5);
  gStyle->SetStatY(0.75);
  TF1 * fit;
  fitGraph(graph,fit);

  canv->SaveAs("power_fit.png");
  delete fit;
  delete graph;
  delete canv;
}
