#define REMI_Channels_Max 20
#define REMI_PeaksproChannels_Max 20
#define REMI_Detectors_Max 20
#define REMI_Detectors_Hits_Max 100 // is it reasonable?? Too small

#define VMI_Channels_Max 20
#define VMI_Detectors_Max 20
#define VMI_max_cols 8000 // ???
#define VMI_max_rows 8000 // ???

TRandom r;
Float_t px,py,pz;
Int_t i;

ULong64_t event_id;

Int_t REMI_nofChannels;
Int_t REMI_Channel[REMI_Channels_Max];
Int_t REMI_Channel_nbrPeaks[REMI_Channels_Max];
//Long_t REMI_Channel_Peak[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Double_t REMI_Channel_Peak_time[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Double_t REMI_Channel_Peak_com[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Double_t REMI_Channel_Peak_cfd[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Double_t REMI_Channel_Peak_integral[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Double_t REMI_Channel_Peak_height[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Double_t REMI_Channel_Peak_width[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Double_t REMI_Channel_Peak_fwhm[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Long_t REMI_Channel_Peak_startpos[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Long_t REMI_Channel_Peak_stoppos[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Long_t REMI_Channel_Peak_maxpos[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Double_t REMI_Channel_Peak_maximum[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Long_t REMI_Channel_Peak_polarity[REMI_Channels_Max][REMI_PeaksproChannels_Max];
Bool_t REMI_Channel_Peak_isUsed[REMI_Channels_Max][REMI_PeaksproChannels_Max];

//add channels_t??
Int_t REMI_nofDetectors;
Int_t REMI_Detector[REMI_Detectors_Max];
Int_t REMI_Detector_nbrOfHits[REMI_Detectors_Max];
Double_t REMI_Detector_Hits_x[REMI_Detectors_Max][REMI_Detectors_Hits_Max];
Double_t REMI_Detector_Hits_y[REMI_Detectors_Max][REMI_Detectors_Hits_Max];
Double_t REMI_Detector_Hits_t[REMI_Detectors_Max][REMI_Detectors_Hits_Max];
Double_t REMI_horpos[REMI_Detectors_Max];
Short_t REMI_nbrBytes[REMI_Detectors_Max];
Double_t REMI_sampleInterval[REMI_Detectors_Max];
Long_t REMI_nbrSamples[REMI_Detectors_Max];
Double_t REMI_delayTime[REMI_Detectors_Max];
Double_t REMI_trigLevel[REMI_Detectors_Max];
Short_t REMI_trigSlope[REMI_Detectors_Max];
Long_t REMI_chanCombUsedChannels[REMI_Detectors_Max];
Short_t REMI_nbrConvPerChan[REMI_Detectors_Max];

UInt_t VMI_integral;
UShort_t VMI_maxPixelValue;
UShort_t VMI_columns;
UShort_t VMI_rows;
UShort_t VMI_bitsPerPixel;
UInt_t VMI_offset;
UShort_t VMI_frame[VMI_max_cols][VMI_max_rows];
UShort_t VMI_cutFrame[VMI_max_cols][VMI_max_rows];
//std::vector<uint16_t> VMI_frame;
//std::vector<uint16_t> VMI_cutFrame;

/*class thisCoordinate
{
public:
   thisCoordinate(uint16_t X,uint16_t Y):x(X),y(Y){}
   ~thisCoordinate(){}
   uint16_t x;
   uint16_t y;

   ClassDef(thisCoordinate,1)
};*/

//std::vector<thisCoordinate> VMI_coordinatesOfImpact;
std::vector<uint16_t> VMI_coordinatesOfImpact_x;
std::vector<uint16_t> VMI_coordinatesOfImpact_y;

#define MAX_pnCCD 2
#define MAX_pnCCD_array_x_size 1024
#define MAX_pnCCD_array_y_size 1024

//Int_t pnCCD_num_pixel_arrays[2];
Int_t pnCCD_num_pixel_arrays;
Int_t pnCCD_array_x_size[2];
Int_t pnCCD_array_y_size[2];
Int_t pnCCD_max_photons_per_event[2];

Int_t pnCCD_array_x_size0;
Int_t pnCCD_array_y_size0;
Int_t pnCCD_array_x_size1;
Int_t pnCCD_array_y_size1;

//std::vector<uint16_t> pnCCD_raw[MAX_pnCCD_array_x_size][MAX_pnCCD_array_y_size][2];
//std::vector<uint16_t> pnCCD_corr[MAX_pnCCD_array_x_size][MAX_pnCCD_array_y_size][2];

UShort_t pnCCD_raw0[MAX_pnCCD_array_x_size][MAX_pnCCD_array_y_size];
UShort_t pnCCD_raw1[MAX_pnCCD_array_x_size][MAX_pnCCD_array_y_size];

UShort_t pnCCD_corr0[MAX_pnCCD_array_x_size][MAX_pnCCD_array_y_size];
UShort_t pnCCD_corr1[MAX_pnCCD_array_x_size][MAX_pnCCD_array_y_size];
