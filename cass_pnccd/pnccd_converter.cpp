#include "pnccd_converter.h"

#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/pnCCD/FrameV1.hh"
#include "cass_event.h"


cass::pnCCD::Converter::Converter()
{
    //this converter should react on acqiris config and waveform//
    _types.push_back(Pds::TypeId::Id_pnCCDconfig);
    _types.push_back(Pds::TypeId::Id_pnCCDframe);
}


void cass::pnCCD::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{
//  fileHeaderType  *pnccd_filhdr;
//  frameHeaderType *pnccd_frmhdr;

// Check whether the xtc object id contains configuration or event data:
  switch( xtc->contains.id() )
  {
  case (Pds::TypeId::Id_pnCCDconfig) :
//    process(info, (fileHeaderType*) xtc->payload());
//      pnccd_filhdr = reinterpret_cast<fileHeaderType*>(xtc->payload());
       // Initialize the stored event with the file header information:
//      _storedEvent.init(pnccd_filhdr,1);
      //just store the config
      _pnccdConfig = (*reinterpret_cast<const Pds::PNCCD::ConfigV1*>(xtc->payload()));
      break;
  case (Pds::TypeId::Id_pnCCDframe) :
      {
          // Get a reference to the pnCCDEvent in the argument address:
          pnCCDEvent      &pnccd_evt = cassevent->pnCCDEvent();
          //      process(info, (frameHeaderType*) xtc->payload());
//          pnccd_frmhdr = reinterpret_cast<frameHeaderType*>(xtc->payload());
          Pds::PNCCD::FrameV1* frame = reinterpret_cast<Pds::PNCCD::FrameV1*>(xtc->payload());
          // Copy the stored information into the pnCCDEvent:

          // the following statement will also copy the pointers of the stored
          // event, which are deleted at the end of the processing of the event
          // and they are not available anymore...
//          pnccd_evt = _storedEvent;
          // Initialize the event in the argument with the xtc payload data:
//          pnccd_evt.init(pnccd_frmhdr,1);
          pnccd_evt.init(_pnccdConfig,frame);
      }
      break;
  default:
      break;
  }


}
