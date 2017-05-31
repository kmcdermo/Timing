#include <map>
#include <fstream>
#include <iostream>
#include <vector>
#include <array>
#include <utility>

#include "TString.h"

typedef std::map<TString,std::array<Float_t,2> > sizemap;
typedef sizemap::iterator sizemapiter;

typedef std::pair<TString,std::array<Float_t,2> > sizepair;
typedef std::vector<sizepair> sizevec;
typedef sizevec::iterator sizeveciter;

inline bool sortByComp(const sizepair & pair1, const sizepair & pair2)
{
  return (pair1.second[1] > pair2.second[1]);  
}

void readInMap(TString filename, sizemap & inmap);

void compareSize()
{
  TString type = "aod";

  sizemap promptmap, ootmap;
  sizevec diffvec, onlyvec;

  readInMap(Form("prompt_eventsize_%s.txt",type.Data()),promptmap);
  readInMap(Form("oot_eventsize_%s.txt"   ,type.Data()),ootmap);

  for (sizemapiter mapiter = ootmap.begin(); mapiter != ootmap.end(); ++mapiter)
  {
    TString name = mapiter->first.Data();
    if (promptmap.count(name))
    {
      std::array<Float_t, 2> tmparr = {ootmap[name][0]-promptmap[name][0],ootmap[name][1]-promptmap[name][1]};
      sizepair tmppair = std::make_pair(name,tmparr);

      diffvec.push_back(tmppair);
    }
    else
    {
      std::array<Float_t, 2> tmparr = {ootmap[name][0],ootmap[name][1]};
      sizepair tmppair = std::make_pair(name,tmparr);

      onlyvec.push_back(tmppair);
    }
  }

  // Output for diff in file
  std::sort(diffvec.begin(),diffvec.end(),sortByComp);
  std::ofstream outdiff;
  outdiff.open(Form("outdiff_%s.txt",type.Data()),std::ios_base::trunc);
  Float_t diff_uncomp_tot = 0.f, diff_comp_tot = 0.f;
  for (sizeveciter veciter = diffvec.begin(); veciter != diffvec.end(); ++veciter)
  {
    outdiff << veciter->first.Data() << " " << veciter->second[0] << " " << veciter->second[1] << std::endl;
    diff_uncomp_tot += veciter->second[0];
    diff_comp_tot   += veciter->second[1];
  }
  outdiff << std::endl;
  outdiff << "Total " << diff_uncomp_tot << " " << diff_comp_tot << std::endl;
  outdiff.close();

  // Output for collections only in OOT
  std::sort(onlyvec.begin(),onlyvec.end(),sortByComp);
  std::ofstream outonly;
  outonly.open(Form("outonly_%s.txt",type.Data()),std::ios_base::trunc);
  Float_t only_uncomp_tot = 0.f, only_comp_tot = 0.f;
  for (sizeveciter veciter = onlyvec.begin(); veciter != onlyvec.end(); ++veciter)
  {
    outonly << veciter->first.Data() << " " << veciter->second[0] << " " << veciter->second[1] << std::endl;
    only_uncomp_tot += veciter->second[0];
    only_comp_tot   += veciter->second[1];
  }
  outonly << std::endl;
  outonly << "Total " << only_uncomp_tot << " " << only_comp_tot << std::endl;
  outonly.close();
}  

void readInMap(TString filename, sizemap & inmap)
{
  std::ifstream input;
  input.open(filename.Data(),std::ios::in);

  TString name;
  Float_t uncomp, comp; 

  while (input >> name >> uncomp >> comp)
  {
    inmap[name] = {uncomp,comp};
  }
  
  input.close();
}
