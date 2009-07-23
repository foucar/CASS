#include "REMIConverter.h"


void cass::REMI::Converter::operator()(const Pds::Acqiris::ConfigV1& config, const Pds::Acqiris::DataDescV1& datadesc, REMIEvent& remievent)
{
    remievent.CreateEventFromLCLSData(config, datadesc);
}
