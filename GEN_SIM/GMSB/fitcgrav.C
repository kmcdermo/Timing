static const std::vector<Color_t> colors = {kRed,kGreen+1,kMagenta,kOrange+1,kGray,kAzure+10,kYellow-7,kViolet-1,kTeal-4,kBlack,kPink-4,kBlue,kOrange+3,kSpring-4,kRed+1,kCyan+2};
  
struct lcg
{
  float cgrav;
  float width;
};

typedef std::vector<lcg> vlcg;

void fitGraph(TGraph *& graph, TF1 *& fit, const int i, const TString & fitname)
{
  Double_t minx,miny,maxx,maxy;
  graph->GetPoint(0,minx,miny);
  graph->GetPoint(graph->GetN()-1,maxx,maxy);
 
  if (fitname.Contains("linear",TString::kExact))
  {
    TFormula form(fitname.Data(),"[0]*x+[1]");
    fit  = new TF1(Form("%s_fit_%i",fitname.Data(),i),fitname.Data(),minx,maxx);
    fit->SetParName(0,"Slope"); 
    fit->SetParameter(0,10.f);
    fit->SetParName(1,"Intercept"); 
    fit->SetParameter(1,0.f);
  }
  else if (fitname.Contains("power",TString::kExact))
  { 
    TFormula form(fitname.Data(),"[0]+[1]*x**[2]");
    fit  = new TF1(Form("%s_fit_%i",fitname.Data(),i),fitname.Data(),minx,maxx);
    fit->SetParName(0,"Intercept"); 
    fit->SetParameter(0,0.f);
    fit->SetParName(1,"Scale0"); 
    fit->SetParameter(1,1.f);
    fit->SetParName(2,"Power0"); 
    fit->SetParameter(2,1.f);
  }
  else 
  {
    std::cerr << "FIT NAME NOT ACCEPTED: " << fitname.Data() << std::endl;
  }

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

  Double_t max = -1;
  i = 0;
  for (auto&& param : params)
  {
    for (Int_t j = 0; j < param.second.size(); j++)
    {
      auto&& point = param.second[j];
      graphs[i]->SetPoint(j,std::sqrt(1.973e-14/point.width),point.cgrav); // 1.973e-34 = hbar*c in GeV * cm
      if (point.cgrav > max) max = point.cgrav;
    }
    i++;
  }

  TCanvas * canv = new TCanvas(); canv->cd();
  TLegend * leg = new TLegend(0.15,0.5,0.42,0.88);
  i = 0;
  for (auto&& param : params)
  {
    graphs[i]->SetMaximum(max*1.05);
    graphs[i]->Draw(i>0?"P same":"AP");
    fitGraph(graphs[i],fits[i],i,"linear");
    leg->AddEntry(graphs[i],Form("#Lambda: %i, m: %4.2f, b: %5.2f",param.first,fits[i]->GetParameter(0),fits[i]->GetParameter(1)),"lp");

    i++;
  }

  leg->Draw("same");
  canv->SaveAs("alphas.png");
  delete leg;

  TGraph * graph = new TGraph(params.size());
  graph->SetTitle("#alpha vs. #Lambda;#Lambda;#alpha");
  graph->SetName("graph_fit");
  graph->SetMarkerColor(kBlack);
  graph->SetLineColor(kBlack);
  graph->SetMarkerStyle(20);

  i = 0;
  for (auto&& param : params)
  {
    graph->SetPoint(i,param.first,fits[i]->GetParameter(0));
    i++;
  }
  graph->Draw("AP");
  i = 0;

  gStyle->SetOptFit(1);
  gStyle->SetStatX(0.5);
  gStyle->SetStatY(0.85);
  TF1 * fit;
  fitGraph(graph,fit,i,"power");

  canv->SaveAs("alpha_fit.png");
  delete fit;
  delete graph;
  delete canv;
}
