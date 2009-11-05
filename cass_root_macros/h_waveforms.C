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

void h_waveforms(){

TH1I *h1 = new TH1I("h1","raw",10000,0.,10001.);
TH1I *h3 = new TH1I("h3","raw",10000,0.,10001.);
TH1F *h2 = new TH1F("h2","rescaled",10000,0.,10001.);
entries=(Int_t)T->GetEntries();
T->SetBranchAddress("Nevents",&Nevents);

delete gROOT->GetListOfCanvases()->FindObject("c1");
TCanvas *c1 = new TCanvas("checks", "roses", 1000, 500);
c1->Divide(1,2);

Short_t wave;
Short_t wavelong[10000];

cass::MachineData::MachineDataEvent::EpicsDataMap::iterator ito;

printf("entries read %i\n",entries);
for(entry=0;entry<entries;entry++)
{
  T->GetEntry(entry);

  //  printf("%i %i\n",entry,Nevents);

  for(Int_t kk=0;kk<REMIEventBranch->nbrOfChannels();kk++)
  {
    //printf("I am in %i %i\n",REMIEventBranch->channel(kk).nbrPeaks(),kk);
    /*if(REMIEventBranch->channel(kk).nbrPeaks() ==0)
      {*/
      h1->Reset();
      h3->Reset();
      printf("I am in event %i vert-gain = %f %i\n",entry,REMIEventBranch->channel(kk).vertGain(),
             REMIEventBranch->channel(kk).fullscale());
      memcpy(wavelong,REMIEventBranch->channel(kk).waveform(),
	       REMIEventBranch->channel(kk).waveformLength()
	     *sizeof(REMIEventBranch->nbrBytes()));
      for(jj=0;jj<REMIEventBranch->channel(kk).waveformLength();jj++)
      {
        memcpy(&wave,REMIEventBranch->channel(kk).waveform()+
               jj*sizeof(int16_t), sizeof(int16_t));
	       //	       sizeof(REMIEventBranch->channel(kk).waveform()));
        h1->Fill(Float_t(jj)+0.5, wave);
        h3->Fill(Float_t(jj)+0.5, wavelong[jj]);
        if(wave!=wavelong[jj])printf("error %i %i\n",wave,wavelong[jj]);
#ifdef uu
        h2->Fill(Float_t(jj)+0.5,Float_t(REMI_Channel_Waveform[kk][jj])*REMI_Channel_vertGain[kk]-Float_t(REMI_Channel_vertOffset[kk]) );
#endif
      }
      c1->cd(1);
      h1->Draw();
      c1->Update();
      c1->cd(2);
      h3->Draw();
      c1->Update();
      //return;
      /*}*/
  }
}
}
