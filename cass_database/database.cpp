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

//gROOT->cd();
TTree *T = new TTree("T","circ buffer");
#include "cass_tree.h"
//ClassImp(thisCoordinate)

#include "histo_list.h"

//cass::CASSEvent *Theevent;
/*uint64_t od=0;
  cass::CASSEvent *Theevent = new cass::CASSEvent::CASSEvent(od);*/

//cass::CASSEvent::CASSEvent *Theevent = new cass::CASSEvent::CASSEvent(od);

cass::database::Database::Database()
{
  
  Double_t random;

  // this is where I am going to start the tree
  T->Branch("i",&i,"i/i");
  T->Branch("px",&px,"px/F");
  T->Branch("py",&py,"py/F");
  T->Branch("pz",&pz,"pz/F");
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

  //REMI
  T->Branch("REMI_nofChannels",&REMI_nofChannels,"REMI_nofChannels/i");
  T->Branch("REMI_Channel",REMI_Channel,"REMI_Channel[REMI_nofChannels]/i");
  T->Branch("REMI_Channel_nbrPeaks",REMI_Channel_nbrPeaks,"REMI_Channel_nbrPeaks[REMI_nofChannels]/i");
  /*T->Branch("REMI_Channel_Peak",REMI_Channel_Peak,
    "REMI_Channel_Peak[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");*/
  T->Branch("REMI_Channel_Peak_time",REMI_Channel_Peak_time,
     "REMI_Channel_Peak_time[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_com",REMI_Channel_Peak_com,
     "REMI_Channel_Peak_com[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_cfd",REMI_Channel_Peak_cfd,
     "REMI_Channel_Peak_cfd[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_integral",REMI_Channel_Peak_integral,
     "REMI_Channel_Peak_integral[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_height",REMI_Channel_Peak_height,
     "REMI_Channel_Peak_height[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_width",REMI_Channel_Peak_width,
     "REMI_Channel_Peak_width[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_fwhm",REMI_Channel_Peak_fwhm,
     "REMI_Channel_Peak_fwhm[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_startpos",REMI_Channel_Peak_startpos,
     "REMI_Channel_Peak_startpos[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_stoppos",REMI_Channel_Peak_stoppos,
     "REMI_Channel_Peak_stoppos[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_maxpos",REMI_Channel_Peak_maxpos,
     "REMI_Channel_Peak_maxpos[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_maximum",REMI_Channel_Peak_maximum,
     "REMI_Channel_Peak_maximum[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_polarity",REMI_Channel_Peak_polarity,
     "REMI_Channel_Peak_polarity[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_isUsed",REMI_Channel_Peak_isUsed, // it should be bool
	    "REMI_Channel_Peak_isUsed[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i"); 
  
  T->Branch("REMI_nofDetectors",&REMI_nofDetectors,"REMI_nofDetectors/i");
  T->Branch("REMI_Detector",REMI_Detector,"REMI_Detector[REMI_nofDetectors]/i");
  T->Branch("REMI_Detector_nbrOfHits",REMI_Detector_nbrOfHits,"REMI_Detector_nbrOfHits[REMI_nofDetectors]/i");
  T->Branch("REMI_Detector_Hits_x",REMI_Detector_Hits_x,
     "REMI_Detector_Hits_x[REMI_nofDetectors][REMI_Detector_nbrOfHits[REMI_nofDetectors]]/i");
  T->Branch("REMI_Detector_Hits_y",REMI_Detector_Hits_y,
     "REMI_Detector_Hits_y[REMI_nofDetectors][REMI_Detector_nbrOfHits[REMI_nofDetectors]]/i");
  T->Branch("REMI_Detector_Hits_t",REMI_Detector_Hits_t,
     "REMI_Detector_Hits_t[REMI_nofDetectors][REMI_Detector_nbrOfHits[REMI_nofDetectors]]/i");

  T->Branch("REMI_horpos",&REMI_horpos,"REMI_horpos/D");
  T->Branch("REMI_nbrBytes",&REMI_nbrBytes,"REMI_nbrBytes/S");
  T->Branch("REMI_sampleInterval",&REMI_sampleInterval,"REMI_sampleInterval/D");
  T->Branch("REMI_nbrSamples",&REMI_nbrSamples,"REMI_nbrSamples/L");
  T->Branch("REMI_delayTime",&REMI_delayTime,"REMI_delayTime/D");
  T->Branch("REMI_trigLevel",&REMI_trigLevel,"REMI_trigLevel/D");
  T->Branch("REMI_trigSlope",&REMI_trigSlope,"REMI_trigSlope/S");
  T->Branch("REMI_chanCombUsedChannels",&REMI_chanCombUsedChannels,"REMI_chanCombUsedChannels/L");
  T->Branch("REMI_nbrConvPerChan",&REMI_nbrConvPerChan,"REMI_nbrConvPerChan/S");

  //VMI Pulnix CCD
  T->Branch("VMI_integral",&VMI_integral,"VMI_integral/i");
  T->Branch("VMI_maxPixelValue",&VMI_maxPixelValue,"VMI_maxPixelValue/s");
  T->Branch("VMI_columns",&VMI_columns,"VMI_columns/s");
  T->Branch("VMI_rows",&VMI_rows,"VMI_rows/s");
  T->Branch("VMI_bitsPerPixel",&VMI_bitsPerPixel,"VMI_bitsPerPixel/s");
  T->Branch("VMI_offset",&VMI_offset,"VMI_offset/i");
  // the following are vectors..
  //T->Branch("VMI_frame",VMI_frame,"VMI_frame[VMI_columns][VMI_rows]/s");
  //T->Branch("VMI_frame",VMI_frame,"VMI_frame/s");
  //T->Branch("VMI_cutFrame",VMI_cutFrame,"VMI_cutFrame/s");
  //T->Branch("VMI_coordinatesOfImpact_x",VMI_coordinatesOfImpact_x,"VMI_coordinatesOfImpact_x/s");
  //T->Branch("VMI_coordinatesOfImpact_y",VMI_coordinatesOfImpact_y,"VMI_coordinatesOfImpact_y/s");

  //pnCCD (2)
  // the 2 pnCCD are not allowed to have different setting??
  //T->Branch("pnCCD_num_pixel_arrays",pnCCD_num_pixel_arrays,"pnCCD_num_pixel_arrays[2]/I");
  T->Branch("pnCCD_num_pixel_arrays",&pnCCD_num_pixel_arrays,"pnCCD_num_pixel_arrays/I");
  T->Branch("pnCCD_array_x_size",pnCCD_array_x_size,"pnCCD_array_x_size[pnCCD_num_pixel_arrays]/I");
  T->Branch("pnCCD_array_y_size",pnCCD_array_y_size,"pnCCD_array_y_size[pnCCD_num_pixel_arrays]/I");
  T->Branch("pnCCD_max_photons_per_event",pnCCD_max_photons_per_event,"pnCCD_max_photons_per_event[pnCCD_num_pixel_arrays]/I");

  T->Branch("pnCCD_array_x_size0",&pnCCD_array_x_size0,"pnCCD_array_x_size0/I");
  T->Branch("pnCCD_array_y_size0",&pnCCD_array_y_size0,"pnCCD_array_y_size0/I");
  T->Branch("pnCCD_array_x_size1",&pnCCD_array_x_size1,"pnCCD_array_x_size1/I");
  T->Branch("pnCCD_array_y_size1",&pnCCD_array_y_size1,"pnCCD_array_y_size1/I");

  T->Branch("pnCCD_raw0",pnCCD_raw0,"pnCCD_raw0[pnCCD_array_x_size0][pnCCD_array_y_size0]/s");
  T->Branch("pnCCD_raw1",pnCCD_raw1,"pnCCD_raw1[pnCCD_array_x_size1][pnCCD_array_y_size1]/s");
  T->Branch("pnCCD_corr0",pnCCD_corr0,"pnCCD_corr0[pnCCD_array_x_size0][pnCCD_array_y_size0]/s");
  T->Branch("pnCCD_corr1",pnCCD_corr1,"pnCCD_corr1[pnCCD_array_x_size1][pnCCD_array_y_size1]/s");

  T->Branch("pnCCD_max_photons_per_event0",&pnCCD_max_photons_per_event0,"pnCCD_max_photons_per_event0/I");
  T->Branch("pnCCD_max_photons_per_event1",&pnCCD_max_photons_per_event1,"pnCCD_max_photons_per_event1/I");

  // all these branches are too large to be used at the same time if the events that
  // we are going to save in memory need to be large

  //I am not allowing all unless MAX_pnCCD_max_photons_per_event small enough...
  if(MAX_pnCCD_max_photons_per_event<65537) // 1024*1024/16
  {
    T->Branch("pnCCD_ph_unrec_x0",     pnCCD_ph_unrec_x0,     "pnCCD_ph_unrec_x0[pnCCD_max_photons_per_event0]/s");
    T->Branch("pnCCD_ph_unrec_y0",     pnCCD_ph_unrec_y0,     "pnCCD_ph_unrec_y0[pnCCD_max_photons_per_event0]/s");
    T->Branch("pnCCD_ph_unrec_amp0",   pnCCD_ph_unrec_amp0,   "pnCCD_ph_unrec_amp0[pnCCD_max_photons_per_event0]/s");
    T->Branch("pnCCD_ph_unrec_energy0",pnCCD_ph_unrec_energy0,"pnCCD_ph_unrec_energy0[pnCCD_max_photons_per_event0]/F");
    T->Branch("pnCCD_ph_unrec_x1",     pnCCD_ph_unrec_x1,     "pnCCD_ph_unrec_x1[pnCCD_max_photons_per_event1]/s");
    T->Branch("pnCCD_ph_unrec_y1",     pnCCD_ph_unrec_y1,     "pnCCD_ph_unrec_y1[pnCCD_max_photons_per_event1]/s");
    T->Branch("pnCCD_ph_unrec_amp1",   pnCCD_ph_unrec_amp1,   "pnCCD_ph_unrec_amp1[pnCCD_max_photons_per_event1]/s");
    T->Branch("pnCCD_ph_unrec_energy1",pnCCD_ph_unrec_energy1,"pnCCD_ph_unrec_energy1[pnCCD_max_photons_per_event1]/F");
  }
  T->Branch("pnCCD_ph_recom_x0",     pnCCD_ph_recom_x0,     "pnCCD_ph_recom_x0[pnCCD_max_photons_per_event0]/s");
  T->Branch("pnCCD_ph_recom_y0",     pnCCD_ph_recom_y0,     "pnCCD_ph_recom_y0[pnCCD_max_photons_per_event0]/s");
  T->Branch("pnCCD_ph_recom_amp0",   pnCCD_ph_recom_amp0,   "pnCCD_ph_recom_amp0[pnCCD_max_photons_per_event0]/s");
  T->Branch("pnCCD_ph_recom_energy0",pnCCD_ph_recom_energy0,"pnCCD_ph_recom_energy0[pnCCD_max_photons_per_event0]/F");
  if(MAX_pnCCD_max_photons_per_event<131073) // 1024*1024/8
  {
    T->Branch("pnCCD_ph_recom_x1",     pnCCD_ph_recom_x1,     "pnCCD_ph_recom_x1[pnCCD_max_photons_per_event1]/s");
    T->Branch("pnCCD_ph_recom_y1",     pnCCD_ph_recom_y1,     "pnCCD_ph_recom_y1[pnCCD_max_photons_per_event1]/s");
    T->Branch("pnCCD_ph_recom_amp1",   pnCCD_ph_recom_amp1,   "pnCCD_ph_recom_amp1[pnCCD_max_photons_per_event1]/s");
    T->Branch("pnCCD_ph_recom_energy1",pnCCD_ph_recom_energy1,"pnCCD_ph_recom_energy1[pnCCD_max_photons_per_event1]/F");
  }
  //T->Branch();
  //T->Branch();
  // others YAG XFEL intensities...

  T->SetCircular(max_events_in_Buffer);
  printf("Circular buffer allocated with %i events\n",max_events_in_Buffer);

  T->Print();
  i=0;

  // I could also create some default histograms
  // like 1 for each pnCCD: last event, all events since beginning of time

}

cass::database::Database::~Database()
{
  // delete all histos
  delete h_pnCCD1_lastevent; h_pnCCD1_lastevent=0;

}

void cass::database::Database::add(cass::CASSEvent* cassevent)
{
  Double_t random;
  Int_t jj,kk;

  if(i==0) {
    /*T->SetCircular(max_events_in_Buffer);
    printf("Circular buffer allocated with %i events\n",max_events_in_Buffer);
    T->Print();*/
  }

  i++;
  // just have something filled...
  if (i<xbins-1) 
  {
    xy[i][i+1]=2*i;
  }

  r.Rannor(px,py);
  pz=px*px+py*py;
  random=r.Rndm();

  event_id=cassevent->id();

  cass::REMI::REMIEvent &remievent = cassevent->REMIEvent();
  REMI_nofChannels=remievent.nbrOfChannels();
  for(jj=0;jj<REMI_nofChannels;jj++)
  {
    REMI_Channel_nbrPeaks[jj]=remievent.channel(jj).nbrPeaks();
    for(kk=0;kk<REMI_Channel_nbrPeaks[jj];kk++)
    {
      REMI_Channel_Peak_time[jj][kk]=remievent.channel(jj).peak(kk).time();
      REMI_Channel_Peak_com[jj][kk]=remievent.channel(jj).peak(kk).com();
      REMI_Channel_Peak_cfd[jj][kk]=remievent.channel(jj).peak(kk).cfd();
      REMI_Channel_Peak_integral[jj][kk]=remievent.channel(jj).peak(kk).integral();
      REMI_Channel_Peak_height[jj][kk]=remievent.channel(jj).peak(kk).height();
      REMI_Channel_Peak_width[jj][kk]=remievent.channel(jj).peak(kk).width();
      REMI_Channel_Peak_fwhm[jj][kk]=remievent.channel(jj).peak(kk).fwhm();
      REMI_Channel_Peak_startpos[jj][kk]=remievent.channel(jj).peak(kk).startpos();
      REMI_Channel_Peak_stoppos[jj][kk]=remievent.channel(jj).peak(kk).stoppos();
      REMI_Channel_Peak_maxpos[jj][kk]=remievent.channel(jj).peak(kk).maxpos();
      REMI_Channel_Peak_maximum[jj][kk]=remievent.channel(jj).peak(kk).maximum();
      REMI_Channel_Peak_polarity[jj][kk]=remievent.channel(jj).peak(kk).polarity();
    }
  }
  REMI_nofDetectors=remievent.nbrOfDetectors();
  for(jj=0;jj<REMI_nofDetectors;jj++)
  { 
    //    REMI_Detector[jj]=remievent.detector(jj);
    REMI_Detector_nbrOfHits[jj]=remievent.detector(jj).nbrOfHits();
    for(kk=0;kk<REMI_Detector_nbrOfHits[jj];kk++)
    {
      REMI_Detector_Hits_x[jj][kk]=remievent.detector(jj).hit(kk).x();
      REMI_Detector_Hits_y[jj][kk]=remievent.detector(jj).hit(kk).y();
      REMI_Detector_Hits_t[jj][kk]=remievent.detector(jj).hit(kk).t();
    }
  }

  REMI_horpos=remievent.horpos();
  REMI_nbrBytes=remievent.nbrBytes();
  REMI_sampleInterval=remievent.sampleInterval();
  REMI_nbrSamples=remievent.nbrSamples();
  REMI_delayTime=remievent.delayTime();
  REMI_trigLevel=remievent.trigLevel();
  REMI_trigSlope=remievent.trigSlope();
  REMI_chanCombUsedChannels=remievent.chanCombUsedChannels();
  REMI_nbrConvPerChan=remievent.nbrConvPerChan();
  //cassevent->~REMIEvent();

  cass::VMI::VMIEvent &vmievent = cassevent->VMIEvent();
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
      //      VMI_frame[kk][jj]=vmievent.frame()[jj*VMI_rows+kk];
    }
  }
  //printf("%i\n",&vmievent.frame());
  //std::vector<uint16_t> VMI_frame= new vmievent.frame();
  //VMI_cutFrame=vmievent.cutFrame();
  //VMI_coordinatesOfImpact[0]=vmievent.coordinatesOfImpact();
  //cassevent->~VMIEvent();

  cass::pnCCD::pnCCDEvent &pnccdevent = cassevent->pnCCDEvent();
  // the following could be even done only everytime the configuration changes...
  for(jj=0;jj<MAX_pnCCD;jj++)
  {
    pnCCD_array_x_size[jj]=0;
    pnCCD_array_y_size[jj]=0;
    pnCCD_max_photons_per_event[jj]=0;
  }
  pnCCD_num_pixel_arrays=pnccdevent.getNumPixArrays();
  // I am not sure of the meaning of the next 2 arrays
  for(jj=0;jj<pnCCD_num_pixel_arrays;jj++)
  {
    pnCCD_array_x_size[jj]=pnccdevent.getArrXSize()[jj];
    pnCCD_array_y_size[jj]=pnccdevent.getArrYSize()[jj];
    pnCCD_max_photons_per_event[jj]=pnccdevent.getMaxPhotPerEvt()[jj];
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
    pnCCD_max_photons_per_event0=TMath::Min(int(pnccdevent.getMaxPhotPerEvt()[0]),MAX_pnCCD_max_photons_per_event);
    if(pnCCD_num_pixel_arrays==2)
    {
      pnCCD_array_x_size1=pnccdevent.getArrXSize()[1];
      pnCCD_array_y_size1=pnccdevent.getArrYSize()[1];
      pnCCD_max_photons_per_event1=TMath::Min(int(pnccdevent.getMaxPhotPerEvt()[1]),MAX_pnCCD_max_photons_per_event);
    }
  } 
  //pnCCD_raw=pnccdevent.raw_signal_values;
  if(pnccdevent.rawSignalArrayAddr(0)!=0)
    printf("%i %i %i\n", pnccdevent.rawSignalArrayAddr(0),pnCCD_array_x_size[0],pnCCD_array_x_size[1]);
  if(pnccdevent.rawSignalArrayAddr(0)!=0)
  {
    printf("oh 000r");
    //memcpy(&pnCCD_raw0[0][0],pnccdevent.rawSignalArrayAddr(0),pnCCD_array_x_size[0]*pnCCD_array_y_size[0]);
    //pnCCD_raw0= *pnccdevent.rawSignalArrayAddr(0);
  }  
  if(pnccdevent.rawSignalArrayAddr(1)!=0)
  {
    printf("oh 111r");
    //memcpy(&pnCCD_raw1[0][0],pnccdevent.rawSignalArrayAddr(1),pnCCD_array_x_size[1]*pnCCD_array_y_size[1]);
  }  

  if(pnccdevent.corrSignalArrayAddr(0)!=0)
  {
    printf("oh 000c");
    //memcpy(&pnCCD_corr0[0][0],pnccdevent.corrSignalArrayAddr(0),pnCCD_array_x_size[0]*pnCCD_array_y_size[0]);
  }  
  if(pnccdevent.corrSignalArrayAddr(1)!=0)
  {
    printf("oh 111c");
    //memcpy(&pnCCD_corr1[0][0],pnccdevent.corrSignalArrayAddr(1),pnCCD_array_x_size[1]*pnCCD_array_y_size[1]);
  }
  /*
  if(MAX_pnCCD_max_photons_per_event<131073) // 1024*1024/8
  {}
  if(pnccdevent.unrecPhotonHitAddr.x[0]!=0)
  {
    printf("oh 000ph");
    memcpy(&pnCCD_ph_unrec_x0[0],pnccdevent.unrecPhotonHitAddr.x[0],pnCCD_max_photons_per_event[0]);
    //pnCCD_ph_unrec0= *pnccdevent.unrecPhotonHitAddr(0);
    }*/
  //??which/both
  //  pnccd_photon_hit* unrecPhotonHitAddr(uint16_t index);
  //  pnccd_photon_hit* recomPhotonHitAddr(uint16_t index);

  //Theevent=cassevent;
  T->Fill();

  if ( max_events_in_Buffer>100 && i>1 && (i%(max_events_in_Buffer/10))==0 )
  {
    printf("done/seen event %i %i\n",i,int(event_id));
    //T->Show(i%max_events_in_Buffer-1);
  }

  if(i>1 && (i%max_events_in_Buffer)==0)
  {
    //T->Draw("px");
    T->Show(i%max_events_in_Buffer-1);
  }

  //now fill the history histograms,
  h_pnCCD1_history->Fill(float(i),float(i+1),int(xy));

  // we may have sets that overweight the last event
  // if "#events > 5" divide histo by 5 and add last event...

  // the "last event" ones need to be clear each time
  h_pnCCD1_lastevent->Reset();
  h_pnCCD1_lastevent->Fill(float(i),float(i+1),int(xy));

  // maybe if i>max_events_in_Buffer i could save the events to file before
  // overwriting them....
#ifdef DEBUG
  if(i==max_events_in_Buffer) {
    // I saw a problem if using the circular buffer...
    // after some events the job crashes at Max_buffers*(1+0.1)+1 events
    printf("saving\n");
    //save the histos instead of draw...
    //h_pnCCD1_lastevent->Draw("Text");
    //h_pnCCD1_history->Draw("Text");
    TFile f("histos.root","RECREATE");
    h_pnCCD1_lastevent->Write();
    h_pnCCD1_history->Write();
    T->Write();
    f.Close();
  }
  // I may have to save some histos to be able to reload them again...
  // maybe this need to be done by diode....
#endif

  //cass::CASSEvent::~CASSEvent();

  // I need to delock??... But I did not lock
  //emit nextEvent();
}

cass::CASSEvent* cass::database::Database::nextEvent()
{
    return 0;
}
