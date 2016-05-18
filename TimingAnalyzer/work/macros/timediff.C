void timediff(){

  TFile * file = TFile::Open("../input/tree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");
  
  tree->Draw("el1time-el2time>>h1","nelectrons==2&&zeemass>76.&&zeemass<106.");
  //  h1->Draw();
}
