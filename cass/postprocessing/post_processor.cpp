#include "post_processor.h"
#include "cass_event.h"
#include "pnccd_event.h"
#include "machine_event.h"
#include "remi_event.h"
#include "vmi_event.h"


void cass::PostProcessor::postProcess(cass::CASSEvent &cassevent)
{
  cassevent.pnCCDEvent().detectors().size();
  
  size_t sizeofFrame = cassevent.pnCCDEvent().detectors()[0].correctedFrame().size();
  cassevent.id();
}
