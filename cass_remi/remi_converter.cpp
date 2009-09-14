#include "remi_converter.h"
#include "cass_event.h"
#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/TypeId.hh"


void cass::REMI::Converter::operator()(const Pds::Xtc* xtc, cass::CASSEvent* cassevent)
{
    //check whether xtc is a configuration or a event//
    switch (xtc->contains.id())
    {
        //if it is a event then extract all information from the event//
        case (Pds::TypeId::Id_AcqWaveform):
        {
            const Pds::Acqiris::DataDescV1 &datadesc = *reinterpret_cast<const Pds::Acqiris::DataDescV1*>(xtc->payload());
            REMIEvent &remievent = cassevent->REMIEvent();
            //first copy the stored configuration into the incoming remievent//
            remievent = _storedEvent;
            //now initialize the rest of the values from the datadescriptor//
            remievent.init(datadesc);
            break;
        }

        //if it is a configuration then check what kind of configuration
        case (Pds::TypeId::Id_AcqConfig) :
        {
            unsigned version = xtc->contains.version();
            switch (version)
            {
                //if it is the right configuration then initialize the storedevent with the configuration//
                case 1:
                {
                    _storedEvent.init(*reinterpret_cast<const Pds::Acqiris::ConfigV1*>(xtc->payload()));
                }
                break;
                default:
                    printf("Unsupported acqiris configuration version %d\n",version);
                break;
            }
            break;
        }
        default:
            printf("xtc type \"%s\" is not handled by REMIConverter",Pds::TypeId::name(xtc->contains.id()));
        break;
    }
}
