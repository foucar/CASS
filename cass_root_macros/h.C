#include <TNtuple.h>
#include <TH1.h>
Int_t entries;
Int_t entry;
Int_t jj,kk;
#include <iostream>
#include "cass_database/cass_tree.h"

//TTree *T=(TTree*)n->Get("T");
//gROOT->Reset();
//TFile f("tree_and_histos001.root");
//TTree *T=(TTree*)f.Get("T");

void h(){

  //TH1I *h1 = new TH1I("h1","raw",10000,0.,10001.);
  //TH1I *h3 = new TH1I("h3","raw",10000,0.,10001.);
  //TH1F *h2 = new TH1F("h2","rescaled",10000,0.,10001.);
entries=(Int_t)T->GetEntries();
T->SetBranchAddress("Nevents",&Nevents);

//delete gROOT->GetListOfCanvases()->FindObject("c1");
//TCanvas *c1 = new TCanvas("checks", "roses", 1000, 500);
//c1->Divide(1,2);

Short_t wave;
Short_t wavelong[10000];

cass::MachineData::MachineDataEvent::EpicsDataMap::iterator ito;

printf("entries read %i\n",entries);
for(entry=0;entry<entries;entry++)
{
  T->GetEntry(entry);

  //  printf("%i %i\n",entry,Nevents);
  printf("mach %i %f %i\n",entry,MachineEventBranch->f_11_ENRC(),MachineEventBranch->EpicsData().size());

  for( ito=MachineEventBranch->EpicsData().begin();
          ito!=MachineEventBranch->EpicsData().end();ito++)
  { 
    //    if((*ito).second != 0)
    std::cout << (*ito).first << " ulla " << (*ito).second << std::endl;
  }
  //return;
}
}
