void reset_lastevt_histos()
{

h_pnCCD1r_lastevt->Reset();
h_pnCCD2r_lastevt->Reset();
/*h_pnCCD3r_lastevt->Reset();
h_pnCCD4r_lastevt->Reset();*/

// this will contain the "last" image that is supposedly background
h_pnCCD1r_lastevt_bkg->Reset();
h_pnCCD2r_lastevt_bkg->Reset();

// this will contain the "last" image w/o beam ROI (small cone)
h_pnCCD1r_lastevt_ROI_sm->Reset();
h_pnCCD2r_lastevt_ROI_sm->Reset();
// this will contain the "last" image w/o beam ROI (large cone)
h_pnCCD1r_lastevt_ROI_lg->Reset();
h_pnCCD2r_lastevt_ROI_lg->Reset();

// this will contain the "last" image w/o beam ROI (triangolar area)
h_pnCCD1r_lastevt_ROI_tri->Reset();
h_pnCCD2r_lastevt_ROI_tri->Reset();
// this will contain the "last" image with  weight=ln(x)
h_pnCCD1r_lastevt_ln->Reset();
h_pnCCD2r_lastevt_ln->Reset();
// this will contain the "last" image with  weight=x**0.25
h_pnCCD1r_lastevt_x025->Reset();
h_pnCCD2r_lastevt_x025->Reset();
// this will contain the "last" image with  weight=x**0.50
h_pnCCD1r_lastevt_x050->Reset();
h_pnCCD2r_lastevt_x050->Reset();

h_pnCCD1c_lastevt->Reset();
h_pnCCD2c_lastevt->Reset();

// this will contain the "last" image that is supposedly background
h_pnCCD1c_lastevt_bkg->Reset();
h_pnCCD2c_lastevt_bkg->Reset();

// this will contain the "last" image w/o beam ROI (small cone)
h_pnCCD1c_lastevt_ROI_sm->Reset();
h_pnCCD2c_lastevt_ROI_sm->Reset();
// this will contain the "last" image w/o beam ROI (large cone)
h_pnCCD1c_lastevt_ROI_lg->Reset();
h_pnCCD2c_lastevt_ROI_lg->Reset();

// this will contain the "last" image w/o beam ROI (triangolar area)
h_pnCCD1c_lastevt_ROI_tri->Reset();
h_pnCCD2c_lastevt_ROI_tri->Reset();
// this will contain the "last" image with  weight=ln(x)
h_pnCCD1c_lastevt_ln->Reset();
h_pnCCD2c_lastevt_ln->Reset();
// this will contain the "last" image with  weight=x**0.25
h_pnCCD1c_lastevt_x025->Reset();
h_pnCCD2c_lastevt_x025->Reset();
// this will contain the "last" image with  weight=x**0.50
h_pnCCD1c_lastevt_x050->Reset();
h_pnCCD2c_lastevt_x050->Reset();

}
