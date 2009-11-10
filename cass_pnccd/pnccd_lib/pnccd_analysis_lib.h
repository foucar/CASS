// pnccd_analysis_lib.h : Nils Kimmel 2009
// pnCCD analysis functionality that provides the analysis
// algorithms of Xonline

#ifndef PNCCD_ANALYSIS_LIB_H
#define PNCCD_ANALYSIS_LIB_H

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#ifndef ROOT_TObject
#include "TObject.h"
#endif

// Include as defined in C99. This should thus hopefully work
// on all systems which support this standard:
#include <vector>
//#include "pdsdata/pnCCD/fformat.h"
//#include <inttypes.h>
#include <string.h>
#include <stdint.h>

#include "pnccd_photon_hit.h"
#include "pnccd_event.h"

//#include "cass_pnccd.h"
