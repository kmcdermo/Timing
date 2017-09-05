static const std::vector<Color_t> colors = {kRed+1,kGreen+1,kMagenta,kOrange+1,kAzure+10,kBlack,kYellow-7,kViolet-1,kYellow+3};
  
struct lcg
{
  float cgrav;
  float width;
};

typedef std::vector<lcg> vlcg;

void fitGraph(TGraph *& graph, TF1 *& fit, const int i)
{
  Double_t minx,miny,maxx,maxy;
  graph->GetPoint(0,minx,miny);
  graph->GetPoint(graph->GetN()-1,maxx,maxy);
 
  TFormula form("linear","[0]*x+[1]");
  fit  = new TF1(Form("linear_fit_%i",i),"linear",minx,maxx);
  fit->SetParName(0,"Slope"); 
  fit->SetParameter(0,10.f);
  fit->SetParName(1,"Intercept"); 
  fit->SetParameter(1,0.f);
  fit->SetLineColor(colors[i]);

  graph->Fit(fit->GetName(),"RBQ");
}  

void fitcgrav()
{
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);

  std::ifstream input("width_cgrav.txt",std::ios_base::in);
  std::map<int,vlcg> params;

  float w, cg; int l;
  while(input >> cg >> l >> w)
  {
    params[l].push_back({cg,w});
  }

  Int_t i = 0;
  std::vector<TGraph*> graphs(params.size());
  std::vector<TF1 *> fits(params.size());
  for (auto&& param : params)
  {
    graphs[i] = new TGraph(param.second.size());
    graphs[i]->SetTitle("c_{grav} vs. #sqrt{c#tau};#sqrt{c#tau};c_{grav}");
    graphs[i]->SetName(Form("graph_%i",param.first));
    graphs[i]->SetMarkerColor(colors[i]);
    graphs[i]->SetLineColor(colors[i]);
    graphs[i]->SetMarkerStyle(20);
    i++; 
  }

  TCanvas * canv = new TCanvas(); canv->cd();
  TLegend * leg = new TLegend(0.6,0.2,0.85,0.45);
  i = 0;
  for (auto&& param : params)
  {
    for (Int_t j = 0; j < param.second.size(); j++)
    {
      auto&& point = param.second[j];
      graphs[i]->SetPoint(j,std::sqrt(1.973e-14/point.width),point.cgrav); // 1.973e-34 = hbar*c in GeV * cm
    }

    graphs[i]->Draw(i>0?"P same":"AP");
    fitGraph(graphs[i],fits[i],i);
    leg->AddEntry(graphs[i],Form("#Lambda: %i, m: %4.2f, b: %5.2f",param.first,fits[i]->GetParameter(0),fits[i]->GetParameter(1)),"lp");

    i++;
  }

  leg->Draw("same");
  canv->SaveAs("alphas.png");
  delete leg;

  TGraph * graph = new TGraph(params.size());
  graph->SetTitle("#alpha vs. #sqrt{#Lambda};#sqrt{#Lambda};#alpha");
  graph->SetName("graph_fit");
  graph->SetMarkerColor(colors[i]);
  graph->SetLineColor(colors[i]);
  graph->SetMarkerStyle(20);

  i = 0;
  for (auto&& param : params)
  {
    graph->SetPoint(i,std::sqrt(param.first),fits[i]->GetParameter(0));
    i++;
  }
  graph->Draw("AP");

  gStyle->SetOptFit(1);
  TF1 * fit;
  fitGraph(graph,fit,i);

  canv->SaveAs("alpha_fit.png");
  delete fit;
  delete graph;
  delete canv;
}
