#include "REMIConverter.h"


void cass::REMI::Converter::operator()(const Pds::Acqiris::DataDescV1& datadesc, cass::REMI::REMIEvent& remievent)const
{
    //first copy the stored configuration into the incoming remievent//
    remievent = _storedEvent;
    //now initialize the rest of the values from the datadescriptor//
    remievent.init(datadesc);
}

void cass::REMI::Converter::operator()(const Pds::Acqiris::ConfigV1& config, cass::REMI::REMIEvent& remievent)
{
    //copy the configuration into the stored event//
    _storedEvent.init(config);
}
