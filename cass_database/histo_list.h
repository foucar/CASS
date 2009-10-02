Int_t xbins=1024,ybins=1024;
Float_t xmin=-0.5,xmax=1023.5,ymin=-0.5,ymax=1023.5;
// these could be actually also taken from the runtime configuration
/*xbins=1024;
ybins=1024;
xmin=-0.5;
xmax=1023.5;
ymin=-0.5;
ymax=1023.5;*/
Int_t xy[1024][1024];

// do I need projections...

TH2I *h_pnCCD1_lastevent = new TH2I("h_pnCCD1_lastevent","histo for pnCCD 1, last event", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2_lastevent = new TH2I("h_pnCCD2_lastevent","histo for pnCCD 2, last event",
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD3_lastevent = new TH2I("h_pnCCD3_lastevent","histo for pnCCD 3, last event",
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD4_lastevent = new TH2I("h_pnCCD4_lastevent","histo for pnCCD 4, last event", 
          xbins,xmin,xmax,ybins,ymin,ymax);

TH2I *h_pnCCD1_history = new TH2I("h_pnCCD1_history","histo for pnCCD 1, all history",
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2_history = new TH2I("h_pnCCD2_history","histo for pnCCD 2, all history", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD3_history = new TH2I("h_pnCCD3_history","histo for pnCCD 3, all history", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD4_history = new TH2I("h_pnCCD4_history","histo for pnCCD 4, all history", 
          xbins,xmin,xmax,ybins,ymin,ymax);

// I need also templates to create "runtime" spectra/pnCCD templates
// something like:
TH2I *h_pnCCD1_1event = new TH2I("h_pnCCD1_1event","histo for pnCCD 1, 1 event",
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2_1event = new TH2I("h_pnCCD2_1event","histo for pnCCD 2, 1 event", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD3_1event = new TH2I("h_pnCCD3_1event","histo for pnCCD 3, 1 event", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD4_1event = new TH2I("h_pnCCD4_1event","histo for pnCCD 4, 1 event", 
          xbins,xmin,xmax,ybins,ymin,ymax);

TH2I *h_pnCCD1_lastNevent = new TH2I("h_pnCCD1_lastNevent","histo for pnCCD 1, last N event", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2_lastNevent = new TH2I("h_pnCCD2_lastNevent","histo for pnCCD 2, last N event",
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD3_lastNevent = new TH2I("h_pnCCD3_lastNevent","histo for pnCCD 3, last N event",
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD4_lastNevent = new TH2I("h_pnCCD4_lastNevent","histo for pnCCD 4, last N event", 
          xbins,xmin,xmax,ybins,ymin,ymax);

// we may have sets that "overweight" the last event
TH2I *h_pnCCD1_w_lastNevent = new TH2I("h_pnCCD1_w_lastevent",
          "histo for pnCCD 1, last N event w/ weights", 
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD2_w_lastNevent = new TH2I("h_pnCCD2_w_lastevent",
          "histo for pnCCD 2, last N event w/ weights",
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD3_w_lastNevent = new TH2I("h_pnCCD3_w_lastevent",
          "histo for pnCCD 3, last N event w/ weights",
          xbins,xmin,xmax,ybins,ymin,ymax);
TH2I *h_pnCCD4_w_lastNevent = new TH2I("h_pnCCD4_w_lastevent",
          "histo for pnCCD 4, last N event w/ weights", 
          xbins,xmin,xmax,ybins,ymin,ymax);
