struct lcg
{
  float cgrav;
  float width;
};

typedef std::vector<lcg> vlcg;


void fitGraph(TGraph *& graph, TF1 *& fit)
{
  Double_t minx,miny,maxx,maxy;
  graph->GetPoint(0,minx,miny);
  graph->GetPoint(graph->GetN()-1,maxx,maxy);
 
  TFormula form("linear","[0]*x+[1]");
  fit  = new TF1("linear_fit","linear",minx,maxx);
  fit->SetParName(0,"Slope"); 
  fit->SetParameter(0,1.f);
  fit->SetParName(1,"Intercept"); 
  fit->SetParameter(1,0.f);
  
  graph->Fit(fit->GetName(),"RBQ");
}  


void fitcgrav()
{
  std::ifstream input("width_cgrav.txt",std::ios_base::in);
  std::map<int,vlcg> params;
  
  float w, cg; int l;
  while(input >> cg >> l >> w)
  {
    params[l].push_back({cg,w});
  }

  TGraph * final = new TGraph(params.size());
  Int_t i = 0;
  for (auto&& param : params)
  {
    TGraph * graph = new TGraph(param.second.size());
    for (Int_t j = 0; j < param.second.size(); j++)
    {
      auto&& point = param.second[j];
      graph->SetPoint(j,std::sqrt((1.973e-14/point.width)),point.cgrav); // 1.973e-34 = hbar*c in GeV * cm
    }
    graph->Draw("AP");

    TF1 * fit;
    fitGraph(graph,fit);
    final->SetPoint(i,param.first,fit->GetParameter(0));
    
    delete fit;
    delete graph;

    i++;
  }

  TCanvas * canv = new TCanvas();
  canv->cd();
  final->SetMarkerStyle(20);
  final->Draw("AP");

  TF1 * finalfit;
  fitGraph(final,finalfit);

  canv->SaveAs("alphas.png");
  delete finalfit;
  delete canv;
  delete final;
}
