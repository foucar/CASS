#include "pnccd_converter.h"

#include "pdsdata/xtc/Xtc.hh"
#include "cass_event.h"


void cass::pnCCD::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{
// Check whether the xtc object is contains configuration or event data:

/*
  switch( xtc->contains.id() )
  {
    case (TypeId::Id_pnCCDconfig) :
      process(info, (fileHeaderType*) xtc->payload());
      break;
    case (TypeId::Id_pnCCDframe) :
      process(info, (frameHeaderType*) xtc->payload());
      break;
    default:
      break;

  }
*/

}
