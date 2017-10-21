static const std::vector<Color_t> colors = {kRed,kGreen+1,kMagenta,kOrange+1,kGray,kAzure+10,kYellow-7,kViolet-1,kTeal-4,kBlack,kPink-4,kBlue,kOrange+3,kSpring-4,kRed+1,kCyan+2};

struct ctd
{
  float ctau;
  float diff;
};
typedef std::vector<ctd> ctdvec;
typedef std::map<int,ctdvec> ictdvm;

void plotdiffs(const TString & lambdas)
{
  gStyle->SetOptStat(0);

  ictdvm params;
  std::ifstream infile(lambdas.Data(),std::ios_base::in);
  TString lambda;
  while (infile >> lambda) 
  {
    std::ifstream input(Form("Diffs/Lambda%sTeV_diffs.txt",lambda.Data()),std::ios_base::in);

    float ctau, diff; int l = lambda.Atoi();
    while(input >> ctau >> diff)
    {
      params[l].push_back({ctau,diff*100.f});
    }
    input.close();
  }
  infile.close();

  Int_t i = 0;
  std::vector<TGraph*> graphs(params.size());
  for (auto&& param : params)
  {
    graphs[i] = new TGraph(param.second.size());
    graphs[i]->SetTitle("Percent Diff. vs.c#tau;c#tau;Percent Diff.");
    graphs[i]->SetName(Form("graph_%i",param.first));
    graphs[i]->SetMarkerColor(colors[i]);
    graphs[i]->SetLineColor(colors[i]);
    graphs[i]->SetMarkerStyle(20);
    i++; 
  }

  Double_t min =  1e9;
  Double_t max = -1e9;
  i = 0;
  for (auto&& param : params)
  {
    for (Int_t j = 0; j < param.second.size(); j++)
    {
      auto&& point = param.second[j];
      graphs[i]->SetPoint(j,point.ctau,point.diff);
      if (point.diff < min) min = point.diff;
      if (point.diff > max) max = point.diff;
    }
    i++;
  }

  TCanvas * canv = new TCanvas(); canv->cd();
  TLegend * leg = new TLegend(0.9,0.5,1.0,1.0);
  //  leg->SetNColumns(2);
  i = 0;
  for (auto&& param : params)
  {
    graphs[i]->SetMinimum(min*1.1);
    graphs[i]->SetMaximum(max*1.1);
    graphs[i]->Draw(i>0?"P same":"AP");
    leg->AddEntry(graphs[i],Form("#Lambda: %i TeV",param.first),"lp");

    i++;
  }

  leg->Draw("same");
  canv->SaveAs("diffs.png");
  delete leg;
  delete canv;
  
  for (auto & graph : graphs)
  {
    delete graph;
  }
}
