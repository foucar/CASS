//maybe the pnCCD could be shrunk in size...
// to help timing...

// the following could be set at init-time or config-time...
Int_t xbins=MAX_pnCCD_array_x_size,ybins=MAX_pnCCD_array_y_size;
Float_t xmin=-0.5,xmax=1023.5,ymin=-0.5,ymax=1023.5;
// these could be actually also taken from the runtime configuration
/*xbins=1024;
ybins=1024;
xmin=-0.5;
xmax=1023.5;
ymin=-0.5;
ymax=1023.5;*/
Int_t xy[MAX_pnCCD_array_x_size][MAX_pnCCD_array_x_size];

UInt_t lastNevent;

Int_t start;
Int_t stop;
Int_t thisevent_mod;

Int_t entries_bins=1000;
Float_t entries_min=0.;
Float_t entries_max=65535.*100.; // 68718428160==65535*1024**2
Float_t log_entries_min=6.0 ;//1048576
Float_t log_entries_max=10.9 ;// 68718428160==65535*1024**2

// do I need projections...

// CCD1r is ccd1 raw, CCD1c is ccd1 corr....
TH2I *h_pnCCD1r_lastevt = new TH2I("h_pnCCD1r_lastevt","histo pnCCD1, last event raw", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2r_lastevt = new TH2I("h_pnCCD2r_lastevt","histo pnCCD2, last event raw",
          xbins,xmin,xmax,ybins,ymin,ymax);
/*TH2I *h_pnCCD3r_lastevt = new TH2I("h_pnCCD3r_lastevt","histo pnCCD3, last event",
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD4r_lastevt = new TH2I("h_pnCCD4r_lastevt","histo pnCCD4, last event", 
          xbins,xmin,xmax,ybins,ymin,ymax);*/

// this will contain the "last" image that is supposedly background
TH2I *h_pnCCD1r_lastevt_bkg = new TH2I("h_pnCCD1r_lastevt_bkg","histo pnCCD1, last event raw, background", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2r_lastevt_bkg = new TH2I("h_pnCCD2r_lastevt_bkg","histo pnCCD2, last event raw, background",
          xbins,xmin,xmax,ybins,ymin,ymax);

// this will contain the "last" image w/o beam ROI (small cone)
TH2I *h_pnCCD1r_lastevt_ROI_sm = new TH2I("h_pnCCD1r_lastevt_ROI_sm","histo pnCCD1 w/o beam ROI (small cone), last event raw", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2r_lastevt_ROI_sm = new TH2I("h_pnCCD2r_lastevt_ROI_sm","histo pnCCD2 w/o beam ROI (small cone), last event raw",
          xbins,xmin,xmax,ybins,ymin,ymax);
// this will contain the "last" image w/o beam ROI (large cone)
TH2I *h_pnCCD1r_lastevt_ROI_lg = new TH2I("h_pnCCD1r_lastevt_ROI_lg","histo pnCCD1 w/o beam ROI (large cone), last event raw", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2r_lastevt_ROI_lg = new TH2I("h_pnCCD2r_lastevt_ROI_lg","histo pnCCD2 w/o beam ROI (large cone), last event raw",
          xbins,xmin,xmax,ybins,ymin,ymax);

// this will contain the "last" image w/o beam ROI (triangolar area)
TH2I *h_pnCCD1r_lastevt_ROI_tri = new TH2I("h_pnCCD1r_lastevt_ROI_tri","histo pnCCD1 w/o beam ROI (triangolar area), last event raw", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2r_lastevt_ROI_tri = new TH2I("h_pnCCD2r_lastevt_ROI_tri","histo pnCCD2 w/o beam ROI (triangolar area), last event raw",
          xbins,xmin,xmax,ybins,ymin,ymax);

// this will contain the "last" image with  weight=ln(x)
TH2I *h_pnCCD1r_lastevt_ln = new TH2I("h_pnCCD1r_lastevt_ln","histo pnCCD1 weight=ln(x), last event raw", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2r_lastevt_ln = new TH2I("h_pnCCD2r_lastevt_ln","histo pnCCD2 weight=ln(x), last event raw",
          xbins,xmin,xmax,ybins,ymin,ymax);
// this will contain the "last" image with  weight=x**0.25
TH2I *h_pnCCD1r_lastevt_x025 = new TH2I("h_pnCCD1r_lastevt_x025","histo pnCCD1 weight=x**0.25, last event raw", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2r_lastevt_x025 = new TH2I("h_pnCCD2r_lastevt_x025","histo pnCCD2 weight=x**0.25, last event raw",
          xbins,xmin,xmax,ybins,ymin,ymax);
// this will contain the "last" image with  weight=x**0.50
TH2I *h_pnCCD1r_lastevt_x050 = new TH2I("h_pnCCD1r_lastevt_x050","histo pnCCD1 weight=x**0.50, last event raw", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2r_lastevt_x050 = new TH2I("h_pnCCD2r_lastevt_x050","histo pnCCD2 weight=x**0.50, last event raw",
          xbins,xmin,xmax,ybins,ymin,ymax);

// decide for the next 2*8 if 1F is the proper thing to fill
// this will contain the d(N)/Entries vs Entries
TH1F *h_pnCCD1r_entries = new TH1F("h_pnCCD1r_entries","histo pnCCD 1, dN/ vs entries raw", 
          entries_bins,entries_min,entries_max);
TH1F *h_pnCCD2r_entries = new TH1F("h_pnCCD2r_entries","histo pnCCD 2, dN/ vs entries raw",
          entries_bins,entries_min,entries_max);

// this will contain the d(N)/Entries vs log(Entries)
TH1F *h_pnCCD1r_logentries = new TH1F("h_pnCCD1r_logentries","histo pnCCD 1, dN/ vs log(entries) raw", 
          entries_bins,log_entries_min,log_entries_max);
TH1F *h_pnCCD2r_logentries = new TH1F("h_pnCCD2r_logentries","histo pnCCD 2, dN/ vs log(entries) raw",
          entries_bins,log_entries_min,log_entries_max);

// this will contain the d(N)/Entries vs Entries ROI
TH1F *h_pnCCD1r_entries_ROb = new TH1F("h_pnCCD1r_entries_ROb","histo pnCCD1, dN/ vs entries w/o RObeam raw", 
          entries_bins,entries_min,entries_max);
TH1F *h_pnCCD2r_entries_ROb = new TH1F("h_pnCCD2r_entries_ROb","histo pnCCD2, dN/ vs entries w/o RObeam raw",
          entries_bins,entries_min,entries_max);
// this will contain the d(N)/Entries vs log(Entries) ROI
TH1F *h_pnCCD1r_logentries_ROb = new TH1F("h_pnCCD1r_logentries_ROb","histo pnCCD 1, dN/ vs log(entries) w/o RObeam raw", 
          entries_bins,log_entries_min,log_entries_max);
TH1F *h_pnCCD2r_logentries_ROb = new TH1F("h_pnCCD2r_logentries_ROb","histo pnCCD 2, dN/ vs log(entries) w/o RObeam raw",
          entries_bins,log_entries_min,log_entries_max);

// this will contain the Entries vs time
// this will contain the Entries vs time ROI

// CCD1r is ccd1 raw, CCD1c is ccd1 corr....
TH2I *h_pnCCD1c_lastevt = new TH2I("h_pnCCD1c_lastevt","histo pnCCD1, last event corr", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2c_lastevt = new TH2I("h_pnCCD2c_lastevt","histo pnCCD2, last event corr",
          xbins,xmin,xmax,ybins,ymin,ymax);
/*TH2I *h_pnCCD3c_lastevt = new TH2I("h_pnCCD3c_lastevt","histo pnCCD3, last event",
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD4c_lastevt = new TH2I("h_pnCCD4c_lastevt","histo pnCCD4, last event", 
          xbins,xmin,xmax,ybins,ymin,ymax);*/

// this will contain the "last" image that is supposedly background
TH2I *h_pnCCD1c_lastevt_bkg = new TH2I("h_pnCCD1c_lastevt_bkg","histo pnCCD1, last event corr, background", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2c_lastevt_bkg = new TH2I("h_pnCCD2c_lastevt_bkg","histo pnCCD2, last event corr, background",
          xbins,xmin,xmax,ybins,ymin,ymax);

// this will contain the "last" image w/o beam ROI (small cone)
TH2I *h_pnCCD1c_lastevt_ROI_sm = new TH2I("h_pnCCD1c_lastevt_ROI_sm","histo pnCCD1 w/o beam ROI (small cone), last event corr", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2c_lastevt_ROI_sm = new TH2I("h_pnCCD2c_lastevt_ROI_sm","histo pnCCD2 w/o beam ROI (small cone), last event corr",
          xbins,xmin,xmax,ybins,ymin,ymax);
// this will contain the "last" image w/o beam ROI (large cone)
TH2I *h_pnCCD1c_lastevt_ROI_lg = new TH2I("h_pnCCD1c_lastevt_ROI_lg","histo pnCCD1 w/o beam ROI (large cone), last event corr", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2c_lastevt_ROI_lg = new TH2I("h_pnCCD2c_lastevt_ROI_lg","histo pnCCD2 w/o beam ROI (large cone), last event corr",
          xbins,xmin,xmax,ybins,ymin,ymax);

// this will contain the "last" image w/o beam ROI (triangolar area)
TH2I *h_pnCCD1c_lastevt_ROI_tri = new TH2I("h_pnCCD1c_lastevt_ROI_tri","histo pnCCD1 w/o beam ROI (triangolar area), last event corr", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2c_lastevt_ROI_tri = new TH2I("h_pnCCD2c_lastevt_ROI_tri","histo pnCCD2 w/o beam ROI (triangolar area), last event corr",
          xbins,xmin,xmax,ybins,ymin,ymax);

// this will contain the "last" image with  weight=ln(x)
TH2I *h_pnCCD1c_lastevt_ln = new TH2I("h_pnCCD1c_lastevt_ln","histo pnCCD1 weight=ln(x), last event corr", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2c_lastevt_ln = new TH2I("h_pnCCD2c_lastevt_ln","histo pnCCD2 weight=ln(x), last event corr",
          xbins,xmin,xmax,ybins,ymin,ymax);
// this will contain the "last" image with  weight=x**0.25
TH2I *h_pnCCD1c_lastevt_x025 = new TH2I("h_pnCCD1c_lastevt_x025","histo pnCCD1 weight=x**0.25, last event corr", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2c_lastevt_x025 = new TH2I("h_pnCCD2c_lastevt_x025","histo pnCCD2 weight=x**0.25, last event corr",
          xbins,xmin,xmax,ybins,ymin,ymax);
// this will contain the "last" image with  weight=x**0.50
TH2I *h_pnCCD1c_lastevt_x050 = new TH2I("h_pnCCD1c_lastevt_x050","histo pnCCD1 weight=x**0.50, last event corr", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2c_lastevt_x050 = new TH2I("h_pnCCD2c_lastevt_x050","histo pnCCD2 weight=x**0.50, last event corr",
          xbins,xmin,xmax,ybins,ymin,ymax);

// decide for the next 2*8 if 1F is the proper thing to fill
// this will contain the d(N)/Entries vs Entries
TH1F *h_pnCCD1c_entries = new TH1F("h_pnCCD1c_entries","histo pnCCD 1, dN/ vs entries corr", 
          entries_bins,entries_min,entries_max);
TH1F *h_pnCCD2c_entries = new TH1F("h_pnCCD2c_entries","histo pnCCD 2, dN/ vs entries corr",
          entries_bins,entries_min,entries_max);

// this will contain the d(N)/Entries vs log(Entries)
TH1F *h_pnCCD1c_logentries = new TH1F("h_pnCCD1c_logentries","histo pnCCD 1, dN/ vs log(entries) corr", 
          entries_bins,log_entries_min,log_entries_max);
TH1F *h_pnCCD2c_logentries = new TH1F("h_pnCCD2c_logentries","histo pnCCD 2, dN/ vs log(entries) corr",
          entries_bins,log_entries_min,log_entries_max);

// this will contain the d(N)/Entries vs Entries ROI
TH1F *h_pnCCD1c_entries_ROb = new TH1F("h_pnCCD1c_entries_ROb","histo pnCCD1, dN/ vs entries w/o RObeam corr", 
          entries_bins,entries_min,entries_max);
TH1F *h_pnCCD2c_entries_ROb = new TH1F("h_pnCCD2c_entries_ROb","histo pnCCD2, dN/ vs entries w/o RObeam corr",
          entries_bins,entries_min,entries_max);
// this will contain the d(N)/Entries vs log(Entries) ROI
TH1F *h_pnCCD1c_logentries_ROb = new TH1F("h_pnCCD1c_logentries_ROb","histo pnCCD 1, dN/ vs log(entries) w/o RObeam corr", 
          entries_bins,log_entries_min,log_entries_max);
TH1F *h_pnCCD2c_logentries_ROb = new TH1F("h_pnCCD2c_logentries_ROb","histo pnCCD 2, dN/ vs log(entries) w/o RObeam corr",
          entries_bins,log_entries_min,log_entries_max);

// this will contain the Entries vs time
// this will contain the Entries vs time ROI

TH2I *h_pnCCD1r_history = new TH2I("h_pnCCD1r_history","histo pnCCD1, all history raw",
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2r_history = new TH2I("h_pnCCD2r_history","histo pnCCD2, all history raw", 
          xbins,xmin,xmax,ybins,ymin,ymax);
/*TH2I *h_pnCCD3r_history = new TH2I("h_pnCCD3r_history","histo pnCCD3, all history raw", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD4r_history = new TH2I("h_pnCCD4r_history","histo pnCCD4, all history raw", 
xbins,xmin,xmax,ybins,ymin,ymax);*/

// I need also templates to create "runtime" spectra/pnCCD templates
// something like:
TH2I *h_pnCCD1r_1evt = new TH2I("h_pnCCD1r_1evt","histo pnCCD1, 1 event raw",
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2r_1evt = new TH2I("h_pnCCD2r_1evt","histo pnCCD2, 1 event raw", 
          xbins,xmin,xmax,ybins,ymin,ymax);
/*TH2I *h_pnCCD3r_1evt = new TH2I("h_pnCCD3r_1evt","histo pnCCD3, 1 event raw", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD4r_1evt = new TH2I("h_pnCCD4r_1evt","histo pnCCD4, 1 event raw", 
          xbins,xmin,xmax,ybins,ymin,ymax);*/

TH2I *h_pnCCD1r_lastNevt = new TH2I("h_pnCCD1r_lastNevt","histo pnCCD1, last N event raw", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2r_lastNevt = new TH2I("h_pnCCD2r_lastNevt","histo pnCCD2, last N event raw",
          xbins,xmin,xmax,ybins,ymin,ymax);
/*TH2I *h_pnCCD3r_lastNevt = new TH2I("h_pnCCD3r_lastNevt","histo pnCCD3, last N event raw",
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD4r_lastNevt = new TH2I("h_pnCCD4r_lastNevt","histo pnCCD4, last N event raw", 
xbins,xmin,xmax,ybins,ymin,ymax);*/

// we may have sets that "overweight" the last event
TH2I *h_pnCCD1r_w_lastNevt = new TH2I("h_pnCCD1r_w_lastevt",
          "histo pnCCD1, last N event w/ weights raw", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2r_w_lastNevt = new TH2I("h_pnCCD2r_w_lastevt",
          "histo pnCCD2, last N event w/ weights raw",
          xbins,xmin,xmax,ybins,ymin,ymax);
/*TH2I *h_pnCCD3r_w_lastNevt = new TH2I("h_pnCCD3r_w_lastevt",
          "histo pnCCD3, last N event w/ weights raw",
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD4r_w_lastNevt = new TH2I("h_pnCCD4r_w_lastevt",
          "histo pnCCD4, last N event w/ weights raw", 
          xbins,xmin,xmax,ybins,ymin,ymax);*/

// only the energies...
