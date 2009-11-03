/*
 *  Database.cpp
 *  diode
 *
 *  Created by Nicola Coppola & lutz foucar .
 *  
 */

#include "database.h"
#include "cass_event.h"
//ClassImp(cass::CASSEvent)

#include "machine_event.h"
#include "remi_event.h"
#include "vmi_event.h"
#include "pnccd_event.h"

#include <TROOT.h>
#include <TRandom.h>
#include <TNtuple.h>
#include <TMath.h>
#include <TH2.h>
#include <TFile.h>
#include <TClassTable.h>
#include <TSystem.h>
#include "TDirectory.h"
#include "TProcessID.h"
#include <TCut.h>
#include <TThread.h>
#include <TClass.h>
#include <TDataType.h>
#include <TMapFile.h>

//TFile f("tree_and_histos.root","RECREATE");
TTree *T = new TTree("T","circ buffer");

#include "cass_tree.h"

#include "histo_list.h"
void reset_lastevt_histos();
#include "reset_histos.h"
#include "fill_histos.h"
void   fill_lastevt_histos();
#include <time.h>
time_t rawtime;
struct tm * timeinfo;
char hourmin[12];

//I put the map-file on the shm.... device, but maybe there is a better place..
// it was 2000000000

// reduced version
#include <stdlib.h>
#include <string.h>
#include "map_files_name.h"
char username[10];

TMapFile *mapfile;

#ifndef ROOT_Rtypes
#include <Rtypes.h>
#endif

#ifndef ROOT_TObject
#include <TObject.h>
#endif

ClassImp(cass::MachineData::MachineDataEvent);
ClassImp(cass::REMI::REMIEvent);
ClassImp(cass::VMI::VMIEvent);
ClassImp(cass::pnCCD::pnCCDEvent);

cass::database::Database::Database()
{
  Double_t random;

  sprintf(Tmap_filename,"%s","/dev/shm/test_root_");
  //printf("%s\n",Tmap_filename);
  strcpy(username,"");

  if(getenv("USER")!=NULL)
  {
    sprintf(username,"%s", getenv("USER"));
  }
  else sprintf(username,"%s","nobody");
  
  //  printf("user is %s\n",username);
  if(strcmp(username,"ncoppola")==0)
    strcat(Tmap_filename,"copp.map");
  else if(strcmp(username,"lutz")==0)
    strcat(Tmap_filename,"lutz.map");
  else 
  {  
    printf("unknown user\n");
    strcat(Tmap_filename,"nobody.map\0");
  } 
  printf("TMapFile is in %s\n",Tmap_filename);
  // it was 2000000000
  mapfile = TMapFile::Create(Tmap_filename,"RECREATE", 800000000, "");
  mapfile->Add(T,"T");
  //mapfile->Print();
  //mapfile->ls();

  //should I add a header to the tree??

  // this is where I am going to start the tree
  T->Branch("Nevents",&Nevents,"Nevents/i");
  T->Branch("px",&px,"px/F");
  T->Branch("py",&py,"py/F");
  //T->Branch("pz",&pz,"pz/F");
  T->Branch("random",&random,"random/D");

  // the eventid
  T->Branch("CASS_id",&event_id,"CASS_id/l");

  //machine quantities
  if(!TClassTable::GetDict("cass::MachineData::MachineDataEvent"))
  {
    gSystem->Load("$Dict_LIB/libcass_dictionaries.so");
  }
  else printf("I have got already the MachineData definitions\n");
  cass::MachineData::MachineDataEvent *machinedata = new cass::MachineData::MachineDataEvent();
  T->Branch("MachineEventBranch","cass::MachineData::MachineDataEvent",&machinedata,32000,99);
  //  delete mac_things_event;

  //REMI
  if(!TClassTable::GetDict("cass::REMI::REMIEvent"))
  {
    gSystem->Load("$Dict_LIB/libcass_dictionaries.so");
  }
  else printf("I have got already the REMI definitions\n");
  cass::REMI::REMIEvent *REMIdata = new cass::REMI::REMIEvent();
  T->Branch("REMIEventBranch","cass::REMI::REMIEvent",&REMIdata,32000,99);
  //  delete REMI_things_event;

  //VMI
  if(!TClassTable::GetDict("cass::VMI::VMIEvent"))
  {
    gSystem->Load("$Dict_LIB/libcass_dictionaries.so");
  }
  else printf("I have got already the VMI definitions\n");
  cass::VMI::VMIEvent *VMIdata = new cass::VMI::VMIEvent();
  T->Branch("VMIEventBranch","cass::VMI::VMIEvent",&VMIdata,32000,99);
  //  delete VMI_things_event;

  //pnCCD
  if(!TClassTable::GetDict("cass::pnCCD::pnCCDEvent"))
  {
    gSystem->Load("$Dict_LIB/libcass_dictionaries.so");
  }
  else printf("I have got already the pnCCD definitions\n");
  cass::pnCCD::pnCCDEvent *pnCCDdata = new cass::pnCCD::pnCCDEvent();
  T->Branch("pnCCDEventBranch","cass::pnCCD::pnCCDEvent",&pnCCDdata,32000,99);
  //  delete pnCCD_things_event;

  //pnCCD (2)
  T->Branch("pnCCD_array_x_size0",&pnCCD_array_x_size0,"pnCCD_array_x_size0/I");
  T->Branch("pnCCD_array_y_size0",&pnCCD_array_y_size0,"pnCCD_array_y_size0/I");

  T->Branch("pnCCD_array_x_size1",&pnCCD_array_x_size1,"pnCCD_array_x_size1/I");
  T->Branch("pnCCD_array_y_size1",&pnCCD_array_y_size1,"pnCCD_array_y_size1/I");

  T->Branch("pnCCD_max_photons_per_event0",&pnCCD_max_photons_per_event0,"pnCCD_max_photons_per_event0/I");
  T->Branch("pnCCD_max_photons_per_event1",&pnCCD_max_photons_per_event1,"pnCCD_max_photons_per_event1/I");

  T->SetAutoSave();
  T->BranchRef();
  //T->SetCompressionLevel(0);
  T->SetCircular(max_events_in_Buffer);
  printf("Circular buffer allocated with %i events\n",max_events_in_Buffer);

  T->Print();
  Nevents=0;
}

cass::database::Database::~Database()
{
  // delete all histos
  delete h_pnCCD1r_lastevt; h_pnCCD1r_lastevt=0;

}

void cass::database::Database::add(cass::CASSEvent* cassevent)
{
  Double_t random;
  Int_t jj,kk;
  UInt_t jj_u;

  if(!cassevent) return;
  //printf("I am in\n");

  /*if(Nevents==0) {
    T->SetCircular(max_events_in_Buffer);
    printf("Circular buffer allocated with %i events\n",max_events_in_Buffer);
    T->Print();
  }*/

  Nevents++;
  //if(Nevents>299) printf("Nevents=%i \n",Nevents);
  // just to have something filled...
#ifdef RDEBUG
  if (int(Nevents)<xbins-1) 
  {
    xy[Nevents][Nevents+1]=2*Nevents;
  }
#endif

  r.Rannor(px,py);
  random=r.Rndm();

  event_id=cassevent->id();

  cass::MachineData::MachineDataEvent *machinedata = &cassevent->MachineDataEvent();
  //if(machinedata.isFilled())
  /*  if(cassevent->MachineDataEvent().isFilled())
      {*/
  T->SetBranchAddress("MachineEventBranch",&machinedata);
    /*  }
  else
  {
    printf("machine data empty, should I reset something??\n");
    }*/

  cass::REMI::REMIEvent *REMIdata = &cassevent->REMIEvent();
  T->SetBranchAddress("REMIEventBranch",&REMIdata);

  cass::VMI::VMIEvent *VMIdata = &cassevent->VMIEvent();
  T->SetBranchAddress("VMIEventBranch",&VMIdata);


  cass::pnCCD::pnCCDEvent *pnCCDdata = &cassevent->pnCCDEvent();
  T->SetBranchAddress("pnCCDEventBranch",&pnCCDdata);

  //#ifdef pnCCD_deb
  cass::pnCCD::pnCCDEvent &pnccdevent = cassevent->pnCCDEvent();
  // the following could be even done only everytime the configuration changes...

  pnCCD_num_pixel_arrays=pnccdevent.getNumPixArrays();

  pnCCD_array_x_size0=0;
  pnCCD_array_y_size0=0;
  pnCCD_array_x_size1=0;
  pnCCD_array_y_size1=0;
  pnCCD_max_photons_per_event0=0;
  pnCCD_max_photons_per_event1=0;
  if(pnCCD_num_pixel_arrays>0)
  {
    pnCCD_array_x_size0=pnccdevent.getArrXSize()[0];
    pnCCD_array_y_size0=pnccdevent.getArrYSize()[0];
    pnCCD_max_photons_per_event0=TMath::Min(int(pnccdevent.getMaxPhotPerEvt()[0]),
         MAX_pnCCD_max_photons_per_event);
    pnCCD_max_photons_per_event0=TMath::Min(pnCCD_max_photons_per_event0,max_phot_in_Buffer_loose);
    if(pnCCD_num_pixel_arrays==2)
    {
      pnCCD_array_x_size1=pnccdevent.getArrXSize()[1];
      pnCCD_array_y_size1=pnccdevent.getArrYSize()[1];
      pnCCD_max_photons_per_event1=TMath::Min(int(pnccdevent.getMaxPhotPerEvt()[1]),
         MAX_pnCCD_max_photons_per_event);
      pnCCD_max_photons_per_event1=TMath::Min(pnCCD_max_photons_per_event1,max_phot_in_Buffer);
    }
  } 

#if DEBUG_pnCCD_raw
  if(pnccdevent.rawSignalArrayAddr(1)!=0)
  {
    size_t idx=0;
    uint16_t* data = (pnccdevent.rawSignalArrayAddr(1));
    for (size_t iy=0;iy<pnCCD_array_y_size0;++iy)
    {
        for (size_t ix=0;ix<pnCCD_array_x_size0;++ix)
	  std::cout <<"m" << data[idx++]<<" "<< pnCCD_raw0[iy][ix];
        std::cout<<std::endl;
    }
  }
#endif


  if ( max_events_in_Buffer>99 && ( Nevents%(max_events_in_Buffer/10) )==0 )
  {
    time(&rawtime);
    timeinfo=localtime(&rawtime);
    strftime(hourmin,11,"%H%M%S",timeinfo);
    printf("done/seen event %i %i %s\n",Nevents,int(event_id), hourmin );
    
    //T->Show(i%max_events_in_Buffer-1);
  }

  /*if(Nevents>1 && (Nevents%max_events_in_Buffer)==0)
  {
    T->Show(Nevents%max_events_in_Buffer-1);
  }*/

  //now fill the history histograms,
  //h_pnCCD1r_history->Fill(float(Nevents%xbins)*xmax,float(Nevents%xbins+1)*ymax,int(xy[Nevents][Nevents+1]));
  //h_pnCCD1r_history->Fill(float(Nevents%ybins)*ymax*2.,float((Nevents%ybins)+1)*ymax*2.,int(2.*xy[Nevents][Nevents+1]));

  // we could "look" for no-hit or for hit to save "showtime"

  // the "last N event" ones need to be clear each time
  //h_pnCCD1r_lastNevt->Reset();
  //h_pnCCD1r_w_lastNevt->Reset();
  //lastNevent=5;
    /*printf("filling histos Nlast start=%i, end=%i and Nevents modulus max_events_in_Buffer %i \n",
      Nevents-6,Nevents,Nevents%max_events_in_Buffer);*/

#ifdef root_local_within_cass
  if(Nevents>lastNevent)
  {
    start=int(Nevents)-lastNevent-1;
    stop=int(Nevents);
//examples to be implemented in diode unless lastNevent>max_events_in_Buffer
// the stdout seems changed by T->Draw() and T->Project() statements
// simple h->Draw() have no such an effect...

    char thisstring[25];
    sprintf(thisstring,"int(Nevents)>=%i",start);
    TCut c1 = thisstring;
    sprintf(thisstring,"int(Nevents)<%i",stop);
    TCut c2 = thisstring;

    sprintf(thisstring,"Nevents<%i",stop-1);
    TCut c3 = thisstring;
    sprintf(thisstring,"%i*(Nevents==%i)",lastNevent,stop);
    TCut c4 = thisstring;

    // the following 3 if are actually not what I want to achieve...
    if ( c1 && c2 )
    {
      //T->Project("h_pnCCD1_lastNevt","pnCCD_array_x_size[0]*.9:pnCCD_array_y_size[0]*.9");
    }

    if ( c1 && c3 )
    {
      //T->Project("h_pnCCD1_w_lastNevt","pnCCD_array_x_size[0]*.9:pnCCD_array_y_size[0]*.9");
    }
    if ( c4 )
    {
      //T->Project("h_pnCCD1_w_lastNevt","pnCCD_array_x_size[0]*.9:pnCCD_array_y_size[0]*.9");
    }

    T->Draw("pnCCD_raw0>>+h_pnCCD1_lastNevt", c1 && c2);
    T->Draw("pnCCD_array_x_size[0]*.9:pnCCD_array_y_size[0]*.9>>h_pnCCD1_lastNevt", c1 && c2);
    
    //    T->Draw("pnCCD_array_x_size[0]*.9:pnCCD_array_y_size[0]*.9>>h_pnCCD1_lastNevt", "","",lastNevt,start);
    
    /*
    T->Draw("pnCCD_raw0>>+h_pnCCD1_w_lastNevt", c1 && c3);
    T->Draw("pnCCD_raw0>>+h_pnCCD1_w_lastNevt", c4);
    T->Draw("pnCCD_array_x_size[0]*.9:pnCCD_array_y_size[0]*.9>>h_pnCCD1_w_lastNevt", c1 && c3);
    T->Draw("pnCCD_array_x_size[0]*.9:pnCCD_array_y_size[0]*.9>>+h_pnCCD1_w_lastNevt", c4);*/
    T->Draw("pnCCD_array_x_size[0]*.9:pnCCD_array_y_size[0]*.9>>h_pnCCD1_w_lastNevt", "","",lastNevent-1,start);
    T->Draw("pnCCD_array_x_size[0]*.9:pnCCD_array_y_size[0]*.9>>+h_pnCCD1_w_lastNevt", "","",1,stop-1);
#endif

#ifdef DEBUG
    for (jj=start;jj<stop;jj++)
    {
      thisevent_mod=int(Nevents%max_events_in_Buffer);
      // the following line overwrites the vars in the root-block....
      //T->GetEntry(thisevent_mod); // or Nevents??
      // I think that I need to use T->Draw()...
      h_pnCCD1r_lastNevt->Fill(float(Nevents)/xmax,float(Nevents+1)/ymax,int(xy));
      // we may have sets that overweight the last event
      // if "#events > 5" divide histo by 5 and add last event... or the other way 
      // multiplying by 5 the last one...
      if(jj<stop-1) {
        h_pnCCD1r_w_lastNevt->Fill(float(Nevents)/xmax,float(Nevents+1)/ymax,int(xy));
      }
      else
      {
        h_pnCCD1r_w_lastNevt->Fill(float(Nevents)/xmax,float(Nevents+1)/ymax,int(xy)*int(lastNevent));
      }
    }
  }
#endif

  // the "last event" ones need to be clear each time
  //reset_lastevt_histos();
  //h_pnCCD1r_lastevt->Reset();
  //fill_lastevt_histos();
  //h_pnCCD1r_lastevt->Fill(float(Nevents)/xmax,float(Nevents+1)/ymax,int(xy));

  //now "remember the entries"
  //GetSumOfWeights()
  // GetSum()-??
  //GetIntegral() < gives error if the integral is zero if called  interactively
  // GetEntries() is for sure not what we want... 
  //h_pnCCD1r_lastevt->GetBinContent(0,0);// is underflow
  //h_pnCCD1r_lastevt->GetBinContent(xbins+1,ybins+1);// is overflow
/*
  pnCCD_raw_integral[0]=h_pnCCD1r_lastevt->GetSumOfWeights();
  pnCCD_raw_integral[1]=h_pnCCD2r_lastevt->GetSumOfWeights();

  pnCCD_raw_integral_ROI[0]=h_pnCCD1r_lastevt_ROI_sm->GetSumOfWeights();
  pnCCD_raw_integral_ROI[1]=h_pnCCD2r_lastevt_ROI_sm->GetSumOfWeights();

  pnCCD_corr_integral[0]=h_pnCCD1c_lastevt->GetSumOfWeights();
  pnCCD_corr_integral[1]=h_pnCCD2c_lastevt->GetSumOfWeights();
  pnCCD_corr_integral_ROI[0]=h_pnCCD1c_lastevt_ROI_sm->GetSumOfWeights();
  pnCCD_corr_integral_ROI[1]=h_pnCCD2c_lastevt_ROI_sm->GetSumOfWeights();
*/
  T->Fill();
  //mapfile.Update(TObject obj = 0);
  if ( max_events_in_Buffer>99 && ( Nevents%(20) )==0 )
  {
    printf("updating mapfile\n");
    mapfile->Update();
  }
  else
  {
    if(Nevents%10==0) mapfile->Update();
  }

  // maybe if i>max_events_in_Buffer i could save the events to file before
  // overwriting them....
#ifdef DEBUG
  UInt_t this1 = max_events_in_Buffer/100*30;
  //TThread::Lock();

  if(Nevents==this1) {
    // I saw a problem if using the circular buffer...
    // after some events the job crashes at Max_buffers*(1+0.1)+1 events
    printf("saving\n");
    //save the histos instead of draw...
    //h_pnCCD1r_lastevt->Draw("Text");
    //h_pnCCD1r_history->Draw("Text");
    TFile f("histos2mach.root","RECREATE");
    //f->cd();
    //h_pnCCD1r_lastevt->Write();
    h_pnCCD1r_history->Write();
    //h_pnCCD1r_lastNevt->Write();
    //h_pnCCD1r_w_lastNevt->Write();
    T->Write();
    f.Close();
    // 1 solution could be to recreate the tree...
    //delete T;
    //TTree *T = new TTree("T","circ buffer");

  }
  // I may have to save some histos to be able to reload them again...
  // maybe this need to be done by diode....
  //TThread::UnLock();
#endif

  // I need to delock??... But I did not lock
  //printf("I will delete\n");
//  delete cassevent;
  //printf("I will send the nextEvent signal\n");
  emit nextEvent();
}

/*cass::CASSEvent* cass::database::Database::nextEvent()
{
   return 0;
}*/
