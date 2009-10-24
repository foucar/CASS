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

//gROOT->cd();
//TFile f("histos1.root","RECREATE");
TTree *T = new TTree("T","circ buffer");

#include "cass_tree.h"
//ClassImp(thisCoordinate)

#include "histo_list.h"
void reset_lastevt_histos();
#include "reset_histos.h"
#include "fill_histos.h"
void   fill_lastevt_histos();
#include <time.h>
time_t rawtime;
struct tm * timeinfo;
char hourmin[12];

//cass::CASSEvent *Theevent;
/*uint64_t od=0;
  cass::CASSEvent *Theevent = new cass::CASSEvent::CASSEvent(od);*/
//cass::CASSEvent::CASSEvent *Theevent = new cass::CASSEvent::CASSEvent(od);

cass::database::Database::Database()
{
  Double_t random;

  //should I add a header to the tree??

  // this is where I am going to start the tree
  T->Branch("Nevents",&Nevents,"Nevents/i");
  T->Branch("px",&px,"px/F");
  T->Branch("py",&py,"py/F");
  //T->Branch("pz",&pz,"pz/F");
  T->Branch("random",&random,"random/D");
  /*if(!TClassTable::GetDict("Event")) {
    gSystem->Load("$ROOTSYS/test/libEvent.so");
    }*/
  /*Event *event = new Event();
    T->Branch("EventBranch","Event",&event,32000,99);*/
  //cass::CASSEvent *event = new cass::CASSEvent::CASSEvent(uint64_t);
  /*  T->Branch("TheCASSevent","cass::CASSEvent::CASSEvent(uint64_t od)",
      &Theevent,32000,99);*/
  // otherwise

  // the eventid
  T->Branch("CASS_id",&event_id,"CASS_id/l");

  //machine quantities
  T->Branch("LCLS_f_11_ENRC",&LCLS_f_11_ENRC,"LCLS_f_11_ENRC/D");
  T->Branch("LCLS_f_12_ENRC",&LCLS_f_12_ENRC,"LCLS_f_12_ENRC/D");
  T->Branch("LCLS_f_21_ENRC",&LCLS_f_21_ENRC,"LCLS_f_21_ENRC/D");
  T->Branch("LCLS_f_22_ENRC",&LCLS_f_22_ENRC,"LCLS_f_22_ENRC/D");

  T->Branch("LCLS_energy",&LCLS_energy,"LCLS_energy/D");
  T->Branch("LCLS_EbeamCharge",&LCLS_EbeamCharge,"LCLS_EbeamCharge/D");
  T->Branch("LCLS_EbeamL3Energy",&LCLS_EbeamL3Energy,"LCLS_EbeamL3Energy/D");
  T->Branch("LCLS_EbeamLTUPosX",&LCLS_EbeamLTUPosX,"LCLS_EbeamLTUPosX/D");
  T->Branch("LCLS_EbeamLTUPosY",&LCLS_EbeamLTUPosY,"LCLS_EbeamLTUPosY/D");
  T->Branch("LCLS_EbeamLTUAngX",&LCLS_EbeamLTUAngX,"LCLS_EbeamLTUAngX/D");
  T->Branch("LCLS_EbeamLTUAngY",&LCLS_EbeamLTUAngY,"LCLS_EbeamLTUAngY/D");

  T->Branch("LCLS_FitTime1",&LCLS_FitTime1,"LCLS_FitTime1/D");
  T->Branch("LCLS_FitTime2",&LCLS_FitTime2,"LCLS_FitTime2/D");
  T->Branch("LCLS_Charge1",&LCLS_Charge1,"LCLS_Charge1/D");
  T->Branch("LCLS_Charge2",&LCLS_Charge2,"LCLS_Charge2/D");

  //REMI
  T->Branch("REMI_nofChannels",&REMI_nofChannels,"REMI_nofChannels/i");

  T->Branch("REMI_horpos",&REMI_horpos,"REMI_horpos/D");
  //T->Branch("REMI_nbrBytes",&REMI_nbrBytes,"REMI_nbrBytes/S");
  T->Branch("REMI_sampleInterval",&REMI_sampleInterval,"REMI_sampleInterval/D");
  T->Branch("REMI_nbrSamples",&REMI_nbrSamples,"REMI_nbrSamples/L");
  T->Branch("REMI_delayTime",&REMI_delayTime,"REMI_delayTime/D");
  T->Branch("REMI_trigLevel",&REMI_trigLevel,"REMI_trigLevel/D");
  T->Branch("REMI_trigSlope",&REMI_trigSlope,"REMI_trigSlope/S");
  T->Branch("REMI_trigChannel",&REMI_trigChannel,"REMI_trigChannel/S");
  T->Branch("REMI_chanCombUsedChannels",&REMI_chanCombUsedChannels,"REMI_chanCombUsedChannels/L");
  T->Branch("REMI_nbrConvPerChan",&REMI_nbrConvPerChan,"REMI_nbrConvPerChan/S");

  T->Branch("REMI_Channel_Waveform",REMI_Channel_Waveform,
	    "REMI_Channel_Waveform[5][10000]/S"); //[REMI_Channels_Max][REMI_maxWaveform 20000]
  // it seems that the following is not really appreciated....
  //  "REMI_Channel_Waveform[REMI_nofChannels][REMI_nbrSamples]/S");

  //T->Branch("REMI_Channel",REMI_Channel,"REMI_Channel[REMI_nofChannels]/i");
  T->Branch("REMI_Channel_nbrPeaks",REMI_Channel_nbrPeaks,"REMI_Channel_nbrPeaks[REMI_nofChannels]/i");

  T->Branch("REMI_Channel_fullscale",REMI_Channel_fullscale,"REMI_Channel_fullscale[REMI_nofChannels]/S");
  T->Branch("REMI_Channel_vertGain",REMI_Channel_vertGain,"REMI_Channel_vertGain[REMI_nofChannels]/D");
  T->Branch("REMI_Channel_vertOffset",REMI_Channel_vertOffset,"REMI_Channel_vertOffset[REMI_nofChannels]/D");
  T->Branch("REMI_Channel_delay",REMI_Channel_delay,"REMI_Channel_delay[REMI_nofChannels]/D");
  T->Branch("REMI_Channel_fraction",REMI_Channel_fraction,"REMI_Channel_fraction[REMI_nofChannels]/D");
  T->Branch("REMI_Channel_walk",REMI_Channel_walk,"REMI_Channel_walk[REMI_nofChannels]/D");

  /*T->Branch("REMI_Channel_Peak_com",REMI_Channel_Peak_com,
     "REMI_Channel_Peak_com[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_cfd",REMI_Channel_Peak_cfd,
  "REMI_Channel_Peak_cfd[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");*/

  T->Branch("REMI_Channel_Peak_integral",REMI_Channel_Peak_integral,
	         "REMI_Channel_Peak_integral[5][2]/D");
	    //     "REMI_Channel_Peak_integral[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/D");

  T->Branch("REMI_Channel_Peak_time",REMI_Channel_Peak_time,
     "REMI_Channel_Peak_time[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/D");

  T->Branch("REMI_Channel_Peak_height",REMI_Channel_Peak_height,
     "REMI_Channel_Peak_height[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/D");
  T->Branch("REMI_Channel_Peak_width",REMI_Channel_Peak_width,
     "REMI_Channel_Peak_width[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/D");
  T->Branch("REMI_Channel_Peak_fwhm",REMI_Channel_Peak_fwhm,
     "REMI_Channel_Peak_fwhm[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/D");
  /*T->Branch("REMI_Channel_Peak_startpos",REMI_Channel_Peak_startpos,
     "REMI_Channel_Peak_startpos[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/L");
  T->Branch("REMI_Channel_Peak_stoppos",REMI_Channel_Peak_stoppos,
     "REMI_Channel_Peak_stoppos[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/L");
  T->Branch("REMI_Channel_Peak_maxpos",REMI_Channel_Peak_maxpos,
     "REMI_Channel_Peak_maxpos[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/L");
  T->Branch("REMI_Channel_Peak_maximum",REMI_Channel_Peak_maximum,
  "REMI_Channel_Peak_maximum[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/D");*/
  T->Branch("REMI_Channel_Peak_polarity",REMI_Channel_Peak_polarity,
     "REMI_Channel_Peak_polarity[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/L");
  /*T->Branch("REMI_Channel_Peak_isUsed",REMI_Channel_Peak_isUsed, // it should be bool
    "REMI_Channel_Peak_isUsed[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");*/

  T->Branch("REMI_nofDetectors",&REMI_nofDetectors,"REMI_nofDetectors/i");
  //T->Branch("REMI_Detector",REMI_Detector,"REMI_Detector[REMI_nofDetectors]/C");
  T->Branch("REMI_Detector_nbrOfHits",REMI_Detector_nbrOfHits,"REMI_Detector_nbrOfHits[REMI_nofDetectors]/i");
  T->Branch("REMI_Detector_Hits_x",REMI_Detector_Hits_x,
     "REMI_Detector_Hits_x[REMI_nofDetectors][REMI_Detector_nbrOfHits[REMI_nofDetectors]]/i");
  T->Branch("REMI_Detector_Hits_y",REMI_Detector_Hits_y,
     "REMI_Detector_Hits_y[REMI_nofDetectors][REMI_Detector_nbrOfHits[REMI_nofDetectors]]/i");
  T->Branch("REMI_Detector_Hits_t",REMI_Detector_Hits_t,
     "REMI_Detector_Hits_t[REMI_nofDetectors][REMI_Detector_nbrOfHits[REMI_nofDetectors]]/i");

  //VMI Pulnix CCD
  T->Branch("VMI_isFilled",&VMI_isFilled,"VMI_isFilled/s");//&cassevent->VMIEvent()->isFilled()
  T->Branch("VMI_integral",&VMI_integral,"VMI_integral/i");
  T->Branch("VMI_maxPixelValue",&VMI_maxPixelValue,"VMI_maxPixelValue/s");
  T->Branch("VMI_columns",&VMI_columns,"VMI_columns/s");
  T->Branch("VMI_rows",&VMI_rows,"VMI_rows/s");
  T->Branch("VMI_bitsPerPixel",&VMI_bitsPerPixel,"VMI_bitsPerPixel/s");
  T->Branch("VMI_offset",&VMI_offset,"VMI_offset/i");
  // the following are vectors..
  T->Branch("VMI_frame",VMI_frame,"VMI_frame[VMI_columns][VMI_rows]/s");
  //T->Branch("VMI_frame",VMI_frame,"VMI_frame/s");
  T->Branch("VMI_cutFrame",VMI_cutFrame,"VMI_cutFrame/s");
  T->Branch("VMI_nof_Impacts",&VMI_nof_Impacts,"VMI_nof_Impacts/s");
  T->Branch("VMI_coordinatesOfImpact_x",VMI_coordinatesOfImpact_x,"VMI_coordinatesOfImpact_x[VMI_nof_Impacts]/s");
  T->Branch("VMI_coordinatesOfImpact_y",VMI_coordinatesOfImpact_y,"VMI_coordinatesOfImpact_y[VMI_nof_Impacts]/s");

  //pnCCD (2)
  // the 2 pnCCD are not allowed to have different setting??
  //T->Branch("pnCCD_num_pixel_arrays",pnCCD_num_pixel_arrays,"pnCCD_num_pixel_arrays[2]/I");
  T->Branch("pnCCD_num_pixel_arrays",&pnCCD_num_pixel_arrays,"pnCCD_num_pixel_arrays/I");
  T->Branch("pnCCD_raw_image_size",pnCCD_raw_image_size,"pnCCD_raw_image_size[pnCCD_num_pixel_arrays]/i");
  T->Branch("pnCCD_corr_image_size",pnCCD_corr_image_size,"pnCCD_corr_image_size[pnCCD_num_pixel_arrays]/i");
  T->Branch("pnCCD_array_x_size",pnCCD_array_x_size,"pnCCD_array_x_size[pnCCD_num_pixel_arrays]/i");
  T->Branch("pnCCD_array_y_size",pnCCD_array_y_size,"pnCCD_array_y_size[pnCCD_num_pixel_arrays]/i");
  T->Branch("pnCCD_max_photons_per_event",pnCCD_max_photons_per_event,"pnCCD_max_photons_per_event[pnCCD_num_pixel_arrays]/i");

  T->Branch("pnCCD_raw_integral",pnCCD_raw_integral,"pnCCD_raw_integral[pnCCD_num_pixel_arrays]/F");
  T->Branch("pnCCD_raw_integral_ROI",pnCCD_raw_integral_ROI,"pnCCD_raw_integral_ROI[pnCCD_num_pixel_arrays]/F");
  T->Branch("pnCCD_corr_integral",pnCCD_corr_integral,"pnCCD_corr_integral[pnCCD_num_pixel_arrays]/F");
  T->Branch("pnCCD_corr_integral_ROI",pnCCD_corr_integral_ROI,"pnCCD_corr_integral_ROI[pnCCD_num_pixel_arrays]/F");

  T->Branch("pnCCD_array_x_size0",&pnCCD_array_x_size0,"pnCCD_array_x_size0/I");
  T->Branch("pnCCD_array_y_size0",&pnCCD_array_y_size0,"pnCCD_array_y_size0/I");
  T->Branch("pnCCD_array_x_size1",&pnCCD_array_x_size1,"pnCCD_array_x_size1/I");
  T->Branch("pnCCD_array_y_size1",&pnCCD_array_y_size1,"pnCCD_array_y_size1/I");

  // actually we may not need the raw version
  T->Branch("pnCCD_raw0",pnCCD_raw0,"pnCCD_raw0[pnCCD_array_x_size0][pnCCD_array_y_size0]/s");
  T->Branch("pnCCD_raw1",pnCCD_raw1,"pnCCD_raw1[pnCCD_array_x_size1][pnCCD_array_y_size1]/s");
  T->Branch("pnCCD_corr0",pnCCD_corr0,"pnCCD_corr0[pnCCD_array_x_size0][pnCCD_array_y_size0]/s");
  T->Branch("pnCCD_corr1",pnCCD_corr1,"pnCCD_corr1[pnCCD_array_x_size1][pnCCD_array_y_size1]/s");

  T->Branch("pnCCD_max_photons_per_event0",&pnCCD_max_photons_per_event0,"pnCCD_max_photons_per_event0/I");
  T->Branch("pnCCD_max_photons_per_event1",&pnCCD_max_photons_per_event1,"pnCCD_max_photons_per_event1/I");

  // all these branches are too large to be used at the same time if the events that
  // we are going to save in memory need to be large

  //I am not allowing all unless MAX_pnCCD_max_photons_per_event small enough...
  //  if(MAX_pnCCD_max_photons_per_event<max_phot_in_Buffer)
  //{
    T->Branch("pnCCD_ph_unrec_x0",     pnCCD_ph_unrec_x0,     "pnCCD_ph_unrec_x0[pnCCD_max_photons_per_event0]/s");
    T->Branch("pnCCD_ph_unrec_y0",     pnCCD_ph_unrec_y0,     "pnCCD_ph_unrec_y0[pnCCD_max_photons_per_event0]/s");
    T->Branch("pnCCD_ph_unrec_amp0",   pnCCD_ph_unrec_amp0,   "pnCCD_ph_unrec_amp0[pnCCD_max_photons_per_event0]/s");
    T->Branch("pnCCD_ph_unrec_energy0",pnCCD_ph_unrec_energy0,"pnCCD_ph_unrec_energy0[pnCCD_max_photons_per_event0]/F");
    T->Branch("pnCCD_ph_unrec_x1",     pnCCD_ph_unrec_x1,     "pnCCD_ph_unrec_x1[pnCCD_max_photons_per_event1]/s");
    T->Branch("pnCCD_ph_unrec_y1",     pnCCD_ph_unrec_y1,     "pnCCD_ph_unrec_y1[pnCCD_max_photons_per_event1]/s");
    T->Branch("pnCCD_ph_unrec_amp1",   pnCCD_ph_unrec_amp1,   "pnCCD_ph_unrec_amp1[pnCCD_max_photons_per_event1]/s");
    T->Branch("pnCCD_ph_unrec_energy1",pnCCD_ph_unrec_energy1,"pnCCD_ph_unrec_energy1[pnCCD_max_photons_per_event1]/F");
    //}
  T->Branch("pnCCD_ph_recom_x0",     pnCCD_ph_recom_x0,     "pnCCD_ph_recom_x0[pnCCD_max_photons_per_event0]/s");
  T->Branch("pnCCD_ph_recom_y0",     pnCCD_ph_recom_y0,     "pnCCD_ph_recom_y0[pnCCD_max_photons_per_event0]/s");
  T->Branch("pnCCD_ph_recom_amp0",   pnCCD_ph_recom_amp0,   "pnCCD_ph_recom_amp0[pnCCD_max_photons_per_event0]/s");
  T->Branch("pnCCD_ph_recom_energy0",pnCCD_ph_recom_energy0,"pnCCD_ph_recom_energy0[pnCCD_max_photons_per_event0]/F");
  //if(MAX_pnCCD_max_photons_per_event<max_phot_in_Buffer_loose)
  //{
    T->Branch("pnCCD_ph_recom_x1",     pnCCD_ph_recom_x1,     "pnCCD_ph_recom_x1[pnCCD_max_photons_per_event1]/s");
    T->Branch("pnCCD_ph_recom_y1",     pnCCD_ph_recom_y1,     "pnCCD_ph_recom_y1[pnCCD_max_photons_per_event1]/s");
    T->Branch("pnCCD_ph_recom_amp1",   pnCCD_ph_recom_amp1,   "pnCCD_ph_recom_amp1[pnCCD_max_photons_per_event1]/s");
    T->Branch("pnCCD_ph_recom_energy1",pnCCD_ph_recom_energy1,"pnCCD_ph_recom_energy1[pnCCD_max_photons_per_event1]/F");
    //}

  //T->Branch();
  //T->Branch();
  // others YAG XFEL intensities...

  T->SetAutoSave();
  T->BranchRef();
  //T->SetCompressionLevel(0);
  T->SetCircular(max_events_in_Buffer);
  printf("Circular buffer allocated with %i events\n",max_events_in_Buffer);

  T->Print();
  Nevents=0;

  // I could also create some default histograms
  // like 1 for each pnCCD: last event, all events since beginning of time

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
  /*UShort_t jj16_u;
    Int_t arraysize;*/
  //printf("I am in\n");

  if(Nevents==0) {
    /*T->SetCircular(max_events_in_Buffer);
    printf("Circular buffer allocated with %i events\n",max_events_in_Buffer);
    T->Print();*/
  }

  //TThread::Lock();

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
  //pz=px*px+py*py;
  random=r.Rndm();

  event_id=cassevent->id();

  cass::MachineData::MachineDataEvent &machinedata = cassevent->MachineDataEvent();
  if(machinedata.isFilled())
  {
    printf("mach full\n");
    LCLS_f_11_ENRC=machinedata.f_11_ENRC();
    LCLS_f_12_ENRC=machinedata.f_12_ENRC();
    LCLS_f_21_ENRC=machinedata.f_21_ENRC();
    LCLS_f_22_ENRC=machinedata.f_22_ENRC();

    LCLS_energy=machinedata.energy();
    LCLS_EbeamCharge=machinedata.EbeamCharge();
    LCLS_EbeamL3Energy=machinedata.EbeamL3Energy();
    LCLS_EbeamLTUPosX=machinedata.EbeamLTUPosX();
    LCLS_EbeamLTUPosY=machinedata.EbeamLTUPosY();
    LCLS_EbeamLTUAngX=machinedata.EbeamLTUAngX();
    LCLS_EbeamLTUAngY=machinedata.EbeamLTUAngY();

    LCLS_FitTime1=machinedata.FitTime1();
    LCLS_FitTime2=machinedata.FitTime2();
    LCLS_Charge1=machinedata.Charge1();
    LCLS_Charge2=machinedata.Charge2();
  }
  else
  {
    printf("mach empty\n");
    LCLS_f_11_ENRC=0.;
    LCLS_f_12_ENRC=0.;
    LCLS_f_21_ENRC=0.;
    LCLS_f_22_ENRC=0.;

    LCLS_energy=0.;
    LCLS_EbeamCharge=0.;
    LCLS_EbeamL3Energy=0.;
    LCLS_EbeamLTUPosX=0.;
    LCLS_EbeamLTUPosY=0.;
    LCLS_EbeamLTUAngX=0.;
    LCLS_EbeamLTUAngY=0.;

    LCLS_FitTime1=0.;
    LCLS_FitTime2=0.;
    LCLS_Charge1=0.;
    LCLS_Charge2=0.;
  }
  printf("he %f %f",LCLS_FitTime1,LCLS_EbeamLTUPosX);

  cass::REMI::REMIEvent &remievent = cassevent->REMIEvent();
  if(!remievent.isFilled())
  {
    REMI_nofChannels=0;
    REMI_nofDetectors=0;
  }
  else
  {
    REMI_nofChannels=remievent.nbrOfChannels();
    REMI_horpos=remievent.horpos();
    //REMI_nbrBytes=remievent.nbrBytes();
    REMI_sampleInterval=remievent.sampleInterval();
    REMI_nbrSamples=remievent.nbrSamples();
    REMI_delayTime=remievent.delayTime();
    REMI_trigLevel=remievent.trigLevel();
    REMI_trigSlope=remievent.trigSlope();
    REMI_trigChannel=remievent.trigChannel();
    REMI_chanCombUsedChannels=remievent.chanCombUsedChannels();
    REMI_nbrConvPerChan=remievent.nbrConvPerChan();

    for(jj=0;jj<REMI_nofChannels;jj++)
    {
      REMI_Channel_nbrPeaks[jj]=remievent.channel(jj).nbrPeaks();
      REMI_Channel_vertGain[jj]=remievent.channel(jj).vertGain();
      REMI_Channel_fullscale[jj]=remievent.channel(jj).fullscale();
      REMI_Channel_vertOffset[jj]=remievent.channel(jj).vertOffset();
      REMI_Channel_delay[jj]=remievent.channel(jj).delay();
      REMI_Channel_fraction[jj]=remievent.channel(jj).fraction();
      REMI_Channel_walk[jj]=remievent.channel(jj).walk();
      REMI_Channel_nbrPeaks[jj]=1;
      for(kk=0;kk<REMI_Channel_nbrPeaks[jj];kk++)
      {
        REMI_Channel_Peak_time[jj][kk]=remievent.channel(jj).peak(kk).time();
        //REMI_Channel_Peak_com[jj][kk]=remievent.channel(jj).peak(kk).com();
        //REMI_Channel_Peak_cfd[jj][kk]=remievent.channel(jj).peak(kk).cfd();
        REMI_Channel_Peak_integral[jj][kk]=remievent.channel(jj).peak(kk).integral();
        //REMI_Channel_Peak_integral[jj][kk]=int(150.*px);
        //REMI_Channel_Peak_integral[jj][kk]=(Double_t)(150.*px);
        //      printf("%f %f ",REMI_Channel_Peak_integral[jj][kk],px);
        //REMI_Channel_Peak_height[jj][kk]=remievent.channel(jj).peak(kk).height();
        REMI_Channel_Peak_width[jj][kk]=remievent.channel(jj).peak(kk).width();
        REMI_Channel_Peak_fwhm[jj][kk]=remievent.channel(jj).peak(kk).fwhm();
        //REMI_Channel_Peak_startpos[jj][kk]=remievent.channel(jj).peak(kk).startpos();
        //REMI_Channel_Peak_stoppos[jj][kk]=remievent.channel(jj).peak(kk).stoppos();
        //REMI_Channel_Peak_maxpos[jj][kk]=remievent.channel(jj).peak(kk).maxpos();
        REMI_Channel_Peak_maximum[jj][kk]=remievent.channel(jj).peak(kk).maximum();
        REMI_Channel_Peak_polarity[jj][kk]=remievent.channel(jj).peak(kk).polarity();
        //REMI_Channel_Peak_isUsed[jj][kk]=(UShort_t)remievent.channel(jj).peak(kk).isUsed();
      }
      /*printf("%i %i \n",remievent.channel(jj).waveformLength(),
	(static_cast<const short*>(remievent.channel(jj).waveform()))[5050]);*/
      memcpy(&REMI_Channel_Waveform[jj][0],
        remievent.channel(jj).waveform(),
	     remievent.channel(jj).waveformLength()*sizeof(REMI_Channel_Waveform[0][0]));
      //REMI_Channel_Waveform[jj][20]=(static_cast<const short*>(remievent.channel(jj).waveform()))[20];
      /*printf("%i \n",REMI_Channel_Waveform[jj][210]);*/
    }
    REMI_nofDetectors=remievent.nbrOfDetectors();
    for(jj=0;jj<REMI_nofDetectors;jj++)
    { 
      //    REMI_Detector[jj]=remievent.detector(jj);
      //strcpy(REMI_Detector[jj][0],remievent.detector(jj).name());
      //memcpy(&REMI_Detector[jj][0],remievent.detector(jj).name(),REMI_maxNAME);
      //printf("%s\n",remievent.detector(jj).name());
      //printf("1%s\n",REMI_Detector[jj]);
      REMI_Detector_nbrOfHits[jj]=remievent.detector(jj).nbrOfHits();
      for(kk=0;kk<REMI_Detector_nbrOfHits[jj];kk++)
      {
        REMI_Detector_Hits_x[jj][kk]=remievent.detector(jj).hit(kk).x();
        REMI_Detector_Hits_y[jj][kk]=remievent.detector(jj).hit(kk).y();
        REMI_Detector_Hits_t[jj][kk]=remievent.detector(jj).hit(kk).t();
      }
    }
  }
  //cassevent->~REMIEvent();

  cass::VMI::VMIEvent &vmievent = cassevent->VMIEvent();
  VMI_isFilled=vmievent.isFilled();
  if(VMI_isFilled)
  {
    VMI_integral=vmievent.integral();
    VMI_maxPixelValue=vmievent.maxPixelValue();
    VMI_columns=vmievent.columns();
    VMI_rows=vmievent.rows();
    VMI_bitsPerPixel=vmievent.bitsPerPixel();
    VMI_offset=vmievent.offset();
    for(jj=0;jj<VMI_columns;jj++)
    {
      for(kk=0;kk<VMI_rows;kk++)
      {
        VMI_frame[kk][jj]=vmievent.frame()[jj*VMI_rows+kk];
        VMI_cutFrame[kk][jj]=vmievent.cutFrame()[jj*VMI_rows+kk];
      }
    }
    //printf("%i\n",&vmievent.frame());
    //std::vector<uint16_t> VMI_frame= new vmievent.frame();
    VMI_nof_Impacts=vmievent.coordinatesOfImpact().size();
    for(jj=0;jj<VMI_nof_Impacts;jj++)
    {
      VMI_coordinatesOfImpact_x[jj]=vmievent.coordinatesOfImpact()[jj].x;
      VMI_coordinatesOfImpact_y[jj]=vmievent.coordinatesOfImpact()[jj].y;
    }
  }
  else
  {
    VMI_integral=0;
    VMI_maxPixelValue=0;
    VMI_columns=0;
    VMI_rows=0;
    VMI_bitsPerPixel=0;
    VMI_offset=-999;
    VMI_nof_Impacts=0;
  }
    //cassevent->~VMIEvent();

  cass::pnCCD::pnCCDEvent &pnccdevent = cassevent->pnCCDEvent();
  // the following could be even done only everytime the configuration changes...
  for(jj=0;jj<MAX_pnCCD;jj++)
  {
    pnCCD_raw_image_size[jj]=0;
    pnCCD_corr_image_size[jj]=0;
    pnCCD_array_x_size[jj]=0;
    pnCCD_array_y_size[jj]=0;
    pnCCD_max_photons_per_event[jj]=0;
  }
  for(jj=0;jj<MAX_pnCCD;jj++)
  {
     pnCCD_raw_integral[jj]=0.;
     pnCCD_raw_integral_ROI[jj]=0.;
     pnCCD_corr_integral[jj]=0.;
     pnCCD_corr_integral_ROI[jj]=0.;
  }
  pnCCD_num_pixel_arrays=pnccdevent.getNumPixArrays();
  // I am not sure of the meaning of the next 2 arrays
  for(jj=0;jj<pnCCD_num_pixel_arrays;jj++)
  {
    pnCCD_array_x_size[jj]=pnccdevent.getArrXSize()[jj];
    pnCCD_array_y_size[jj]=pnccdevent.getArrYSize()[jj];
    pnCCD_max_photons_per_event[jj]=pnccdevent.getMaxPhotPerEvt()[jj];
    //jj16_u=ushort(jj+1);
    //    printf("uu %i ",pnccdevent.rawSignalArrayByteSize(1));

    pnCCD_raw_image_size[jj]=pnccdevent.rawSignalArrayByteSize(jj+1);
    pnCCD_corr_image_size[jj]=pnccdevent.corrSignalArrayByteSize(jj+1);
  }
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

  //arraysize=pnCCD_array_x_size[0]*pnCCD_array_y_size[0]*sizeof(UShort_t);
    //  sizeof(pnccdevent.rawSignalArrayAddr(0));
  //pnCCD_raw=pnccdevent.raw_signal_values;
  if(pnccdevent.rawSignalArrayAddr(1)!=0)
  {
    /*printf("00r %i %i %i %i %i\n", pnccdevent.rawSignalArrayAddr(1),pnCCD_array_x_size[0],pnCCD_array_y_size[0],
      pnccdevent.rawSignalArrayByteSize(1),pnCCD_raw_image_size[0]);*/
    memcpy(&pnCCD_raw0[0][0],pnccdevent.rawSignalArrayAddr(1),pnCCD_raw_image_size[0]);

    /*pnCCD_raw_integral[0]=0.;
      pnCCD_raw_integral_ROI[0]=0.;*/
    //pnCCD_raw0= *pnccdevent.rawSignalArrayAddr(0);
  }  
  //arraysize=pnCCD_array_x_size[1]*pnCCD_array_y_size[1]*sizeof(UShort_t);
    //    sizeof(pnccdevent.rawSignalArrayAddr(1));

  if(pnccdevent.rawSignalArrayAddr(2)!=0)
  {
    /*printf("11r, %i %i ",pnCCD_raw_image_size[1],sizeof(pnccdevent.rawSignalArrayAddr(2)));
      printf("%i %i %i ", pnccdevent.rawSignalArrayAddr(2),pnCCD_array_x_size[1],pnCCD_array_y_size[1]);*/
    memcpy(&pnCCD_raw1[0][0],pnccdevent.rawSignalArrayAddr(2),pnCCD_raw_image_size[1]);

    /*    for(jj_u=0;jj_u<pnCCD_array_y_size[1];jj_u++)
       memcpy(&pnCCD_raw1[0][jj_u],pnccdevent.rawSignalArrayAddr(2)+2*jj_u,pnCCD_raw_image_size[1]);
    */
    /*pnCCD_raw_integral[1]=0.;
      pnCCD_raw_integral_ROI[1]=0.;*/
  }  

  //#ifdef pnCCD
  if(pnccdevent.corrSignalArrayAddr(1)!=0)
  {
    //printf("00c, %i ",arraysize);
    memcpy(&pnCCD_corr0[0][0],pnccdevent.corrSignalArrayAddr(1),pnCCD_corr_image_size[0]);
    /*pnCCD_corr_integral[0]=0.;
      pnCCD_corr_integral_ROI[0]=0.;*/
  }  
  if(pnccdevent.corrSignalArrayAddr(2)!=0)
  {
    //printf("11c, %i \n",arraysize);
    memcpy(&pnCCD_corr1[0][0],pnccdevent.corrSignalArrayAddr(2),pnCCD_corr_image_size[1]);
    /*pnCCD_corr_integral[1]=0.;
      pnCCD_corr_integral_ROI[1]=0.;*/
  }
  
  for(jj=0;jj<pnCCD_max_photons_per_event0;jj++)
  {
    if(pnccdevent.unrecPhotonHitAddr(jj+1)!=0)
    {
      //printf("ph %i ",jj);
      // the following could be much easier if there where the proper functions
      //pnCCD_ph_unrec_x0[jj]=pnccdevent.unrecPhotonHitAddr(jj).x;
      memcpy(&pnCCD_ph_unrec_x0[jj],pnccdevent.unrecPhotonHitAddr(jj+1),sizeof(UShort_t));
      //memcpy(&pnCCD_ph_unrec_y0[jj],pnccdevent.unrecPhotonHitAddr(jj)+sizeof(UShort_t),2);
      //memcpy(&pnCCD_ph_unrec_amp0[jj],pnccdevent.unrecPhotonHitAddr(jj)+2*sizeof(UShort_t),2);
      //memcpy(&pnCCD_ph_unrec_energy0[jj],pnccdevent.unrecPhotonHitAddr(jj)+3*sizeof(UShort_t),4);

      //memcpy(&pnCCD_ph_unrec_x0[jj],pnccdevent.unrecPhotonHitAddr.x[jj],pnCCD_max_photons_per_event[0]);
      //pnCCD_ph_unrec0= *pnccdevent.unrecPhotonHitAddr(0);
    }
    if(pnccdevent.recomPhotonHitAddr(jj+1)!=0)
    {
      //printf(" recomph %i ",jj);
      // the following could be much easier if there where the proper functions
      //pnCCD_ph_unrec_x0[jj]=pnccdevent.unrecPhotonHitAddr(jj).x;
      memcpy(&pnCCD_ph_recom_x0[jj],pnccdevent.recomPhotonHitAddr(jj+1),sizeof(UShort_t));
      //memcpy(&pnCCD_ph_unrec_y0[jj],pnccdevent.unrecPhotonHitAddr(jj)+sizeof(UShort_t),2);
      //memcpy(&pnCCD_ph_unrec_amp0[jj],pnccdevent.unrecPhotonHitAddr(jj)+2*sizeof(UShort_t),2);
      //memcpy(&pnCCD_ph_unrec_energy0[jj],pnccdevent.unrecPhotonHitAddr(jj)+3*sizeof(UShort_t),4);

      //memcpy(&pnCCD_ph_unrec_x0[jj],pnccdevent.unrecPhotonHitAddr.x[jj],pnCCD_max_photons_per_event[0]);
      //pnCCD_ph_unrec0= *pnccdevent.unrecPhotonHitAddr(0);
    }
  }
  //printf("\n");
  //??which/both
  //  pnccd_photon_hit* unrecPhotonHitAddr(uint16_t index);
  //  pnccd_photon_hit* recomPhotonHitAddr(uint16_t index);
  //#endif
  //Theevent=cassevent;

  //  T->Fill();

  //TThread::UnLock();

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
    //T->Draw("px");
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
  //#ifdef RDEBUG
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
#ifdef DEBUG
    T->Draw("pnCCD_raw0>>+h_pnCCD1_lastNevt", c1 && c2);
    T->Draw("pnCCD_array_x_size[0]*.9:pnCCD_array_y_size[0]*.9>>h_pnCCD1_lastNevt", c1 && c2);
    
    //    T->Draw("pnCCD_array_x_size[0]*.9:pnCCD_array_y_size[0]*.9>>h_pnCCD1_lastNevt", "","",lastNevt,start);
    
    /*
    //T->Draw("pnCCD_raw0>>+h_pnCCD1_w_lastNevt", c1 && c3);
    //T->Draw("pnCCD_raw0>>+h_pnCCD1_w_lastNevt", c4);
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
#endif
  }
  //#endif
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

  // maybe if i>max_events_in_Buffer i could save the events to file before
  // overwriting them....
  //#ifdef DEBUG
  UInt_t this1 = max_events_in_Buffer/100*90;
  //TThread::Lock();

  if(Nevents==this1) {
    // I saw a problem if using the circular buffer...
    // after some events the job crashes at Max_buffers*(1+0.1)+1 events
    printf("saving\n");
    //save the histos instead of draw...
    //h_pnCCD1r_lastevt->Draw("Text");
    //h_pnCCD1r_history->Draw("Text");
    TFile f("histos1mach.root","RECREATE");
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
  //#endif

  //cass::CASSEvent::~CASSEvent();

  // I need to delock??... But I did not lock
  //printf("I will delete\n");
  delete cassevent;
  //printf("I will send the nextEvent signal\n");
  emit nextEvent();
}

/*cass::CASSEvent* cass::database::Database::nextEvent()
{
   return 0;
}*/
