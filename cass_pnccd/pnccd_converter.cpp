#include "pnccd_converter.h"

#include "pdsdata/xtc/Xtc.hh"
#include "cass_event.h"


void cass::pnCCD::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{
  fileHeaderType  *pnccd_filhdr;
  frameHeaderType *pnccd_frmhdr;


// Check whether the xtc object is contains configuration or event data:
  switch( xtc->contains.id() )
  {
    case (Pds::TypeId::Id_pnCCDconfig) :
//      process(info, (fileHeaderType*) xtc->payload());
      pnccd_filhdr = reinterpret_cast<fileHeaderType*>(xtc->payload());
      break;
    case (Pds::TypeId::Id_pnCCDframe) :
//      process(info, (frameHeaderType*) xtc->payload());
      pnccd_frmhdr = reinterpret_cast<frameHeaderType*>(xtc->payload());
      break;
    default:
      break;

  }


}
