// 1000 seconds at 30 Hz
//#define max_events_in_Buffer 30000
// the following is more "suitable" in case a lot of arrays are supposed to
// be kept in memory.... I would maybe suppose that they are not needed,
// and instead a "lot" of histograms could be filled ...
//#define max_events_in_Buffer 303
UInt_t max_events_in_Buffer= 100;

#define REMI_Channels_Max 16
#define REMI_PeaksproChannels_Max 150
#define REMI_Detectors_Max 2
#define REMI_Detectors_Hits_Max 150 // is it reasonable?? Too small
//#define REMI_maxWaveform 40000 // 40us
#define REMI_maxWaveform 20000 // 20us
#define REMI_maxNAME 64

#define VMI_max_cols 640 // ??? it was 8000, it is still too large
#define VMI_max_rows 480 // ??? it was 8000, it is still too large
#define VMI_MAX_IMPACTS 100 //???

TRandom r;
Float_t px,py,pz;
UInt_t Nevents;
//Int_t i;

ULong64_t event_id;

Int_t REMI_nofChannels;

//add channels_t??
Int_t REMI_nofDetectors;
// all of the following were like Double_t REMI_horpos[REMI_Detectors_Max];
Double_t REMI_horpos;
//Short_t REMI_nbrBytes;
Double_t REMI_sampleInterval;
Long_t REMI_nbrSamples; // changed into int32_t now
Double_t REMI_delayTime;
Double_t REMI_trigLevel;
Short_t REMI_trigSlope; // changed into int16_t now
Short_t REMI_trigChannel; // changed into int16_t now
//Long_t REMI_chanCombUsedChannels; // changed into uint32_t now
UInt_t REMI_chanCombUsedChannels; // changed into uint32_t now
Short_t REMI_nbrConvPerChan; // changed into int16_t now

//Int_t REMI_Detector[REMI_Detectors_Max];
//Char_t REMI_Detector[REMI_Detectors_Max][REMI_maxNAME];

Int_t REMI_Detector_nbrOfHits[REMI_Detectors_Max];
Double_t REMI_Detector_Hits_x[REMI_Detectors_Max][REMI_Detectors_Hits_Max];
Double_t REMI_Detector_Hits_y[REMI_Detectors_Max][REMI_Detectors_Hits_Max];
Double_t REMI_Detector_Hits_t[REMI_Detectors_Max][REMI_Detectors_Hits_Max];

//Int_t REMI_Channel[REMI_Channels_Max];
Int_t REMI_Channel_nbrPeaks[REMI_Channels_Max];

Double_t REMI_Channel_vertGain[REMI_Channels_Max];
Short_t REMI_Channel_fullscale[REMI_Channels_Max]; // changed into int16_t now
// changed into int16_t now
//Double_t REMI_Channel_vertOffset[REMI_Channels_Max];
Short_t REMI_Channel_vertOffset[REMI_Channels_Max]; // changed into int16_t now
// changed into int32_t now
//Short_t REMI_Channel_delay[REMI_Channels_Max];
Int_t REMI_Channel_delay[REMI_Channels_Max];
Double_t REMI_Channel_fraction[REMI_Channels_Max];
Double_t REMI_Channel_walk[REMI_Channels_Max];

//Long_t REMI_Channel_Peak[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Double_t REMI_Channel_Peak_time[REMI_Channels_Max][REMI_PeaksproChannels_Max];
//Double_t REMI_Channel_Peak_com[REMI_Channels_Max][REMI_PeaksproChannels_Max];
//Double_t REMI_Channel_Peak_cfd[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Double_t REMI_Channel_Peak_integral[REMI_Channels_Max][REMI_PeaksproChannels_Max];
//Int_t REMI_Channel_Peak_integral[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Double_t REMI_Channel_Peak_height[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Double_t REMI_Channel_Peak_width[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Double_t REMI_Channel_Peak_fwhm[REMI_Channels_Max][REMI_PeaksproChannels_Max];
//Long_t REMI_Channel_Peak_startpos[REMI_Channels_Max][REMI_PeaksproChannels_Max];
//Long_t REMI_Channel_Peak_stoppos[REMI_Channels_Max][REMI_PeaksproChannels_Max];
//Long_t REMI_Channel_Peak_maxpos[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Double_t REMI_Channel_Peak_maximum[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Long_t REMI_Channel_Peak_polarity[REMI_Channels_Max][REMI_PeaksproChannels_Max];
//UShort_t REMI_Channel_Peak_isUsed[REMI_Channels_Max][REMI_PeaksproChannels_Max];
//Bool_t REMI_Channel_Peak_isUsed[REMI_Channels_Max][REMI_PeaksproChannels_Max];

Short_t REMI_Channel_Waveform[REMI_Channels_Max][REMI_maxWaveform];

UShort_t VMI_nof_Impacts;

UShort_t VMI_isFilled;
UInt_t VMI_integral;
UShort_t VMI_maxPixelValue;
UShort_t VMI_columns;
UShort_t VMI_rows;
UShort_t VMI_bitsPerPixel;
UInt_t VMI_offset;
UShort_t VMI_frame[VMI_max_cols][VMI_max_rows];
UShort_t VMI_cutFrame[VMI_max_cols][VMI_max_rows];
UShort_t VMI_coordinatesOfImpact_x[VMI_MAX_IMPACTS];
UShort_t VMI_coordinatesOfImpact_y[VMI_MAX_IMPACTS];

#define max_phot_in_Buffer_loose 4096 //16385 = 1024*1024/64+1
#define max_phot_in_Buffer 2048 // 8193= 1024*1024/128+1

#define MAX_pnCCD 2
#define MAX_pnCCD_array_x_size 1024
#define MAX_pnCCD_array_y_size 1024
//#define MAX_pnCCD_max_photons_per_event MAX_pnCCD_array_x_size*MAX_pnCCD_array_y_size
//#define MAX_pnCCD_max_photons_per_event MAX_pnCCD_array_x_size*MAX_pnCCD_array_y_size/16
#define MAX_pnCCD_max_photons_per_event max_phot_in_Buffer

Int_t pnCCD_num_pixel_arrays;
UInt_t pnCCD_array_x_size[MAX_pnCCD];
UInt_t pnCCD_array_y_size[MAX_pnCCD];
UInt_t pnCCD_max_photons_per_event[MAX_pnCCD];
UInt_t pnCCD_raw_image_size[MAX_pnCCD];
UInt_t pnCCD_corr_image_size[MAX_pnCCD];

// or Double_t ??
Float_t pnCCD_raw_integral[MAX_pnCCD];
Float_t pnCCD_raw_integral_ROI[MAX_pnCCD];
Float_t pnCCD_corr_integral[MAX_pnCCD];
Float_t pnCCD_corr_integral_ROI[MAX_pnCCD];

Int_t pnCCD_array_x_size0;
Int_t pnCCD_array_y_size0;
Int_t pnCCD_array_x_size1;
Int_t pnCCD_array_y_size1;
Int_t pnCCD_max_photons_per_event0;
Int_t pnCCD_max_photons_per_event1;

//std::vector<uint16_t> pnCCD_raw[MAX_pnCCD_array_x_size][MAX_pnCCD_array_y_size][2];
//std::vector<uint16_t> pnCCD_corr[MAX_pnCCD_array_x_size][MAX_pnCCD_array_y_size][2];

UShort_t pnCCD_raw0[MAX_pnCCD_array_x_size][MAX_pnCCD_array_y_size];
UShort_t pnCCD_raw1[MAX_pnCCD_array_x_size][MAX_pnCCD_array_y_size];

UShort_t pnCCD_corr0[MAX_pnCCD_array_x_size][MAX_pnCCD_array_y_size];
UShort_t pnCCD_corr1[MAX_pnCCD_array_x_size][MAX_pnCCD_array_y_size];

UShort_t pnCCD_ph_unrec_x0[MAX_pnCCD_max_photons_per_event];
UShort_t pnCCD_ph_unrec_y0[MAX_pnCCD_max_photons_per_event];
UShort_t pnCCD_ph_unrec_amp0[MAX_pnCCD_max_photons_per_event];
Float_t pnCCD_ph_unrec_energy0[MAX_pnCCD_max_photons_per_event];
// index?? frm_idx??

UShort_t pnCCD_ph_unrec_x1[MAX_pnCCD_max_photons_per_event];
UShort_t pnCCD_ph_unrec_y1[MAX_pnCCD_max_photons_per_event];
UShort_t pnCCD_ph_unrec_amp1[MAX_pnCCD_max_photons_per_event];
Float_t pnCCD_ph_unrec_energy1[MAX_pnCCD_max_photons_per_event];

// same for recom_photon_hits_
UShort_t pnCCD_ph_recom_x0[MAX_pnCCD_max_photons_per_event];
UShort_t pnCCD_ph_recom_y0[MAX_pnCCD_max_photons_per_event];
UShort_t pnCCD_ph_recom_amp0[MAX_pnCCD_max_photons_per_event];
Float_t pnCCD_ph_recom_energy0[MAX_pnCCD_max_photons_per_event];

UShort_t pnCCD_ph_recom_x1[MAX_pnCCD_max_photons_per_event];
UShort_t pnCCD_ph_recom_y1[MAX_pnCCD_max_photons_per_event];
UShort_t pnCCD_ph_recom_amp1[MAX_pnCCD_max_photons_per_event];
Float_t pnCCD_ph_recom_energy1[MAX_pnCCD_max_photons_per_event];

//machine quantities
Double_t LCLS_f_11_ENRC;
Double_t LCLS_f_12_ENRC;
Double_t LCLS_f_21_ENRC;
Double_t LCLS_f_22_ENRC;

Double_t LCLS_energy;
Double_t LCLS_EbeamCharge;
Double_t LCLS_EbeamL3Energy;
Double_t LCLS_EbeamLTUPosX;
Double_t LCLS_EbeamLTUPosY;
Double_t LCLS_EbeamLTUAngX;
Double_t LCLS_EbeamLTUAngY;

Double_t LCLS_FitTime1;
Double_t LCLS_FitTime2;
Double_t LCLS_Charge1;
Double_t LCLS_Charge2;
