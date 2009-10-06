// Copyright (C) 2009 Jochen Küpper

#include <QtCore/QMutexLocker>
#include "format_converter.h"
#include "remi_converter.h"
#include "vmi_converter.h"
#include "pnccd_converter.h"
#include "xtciterator.h"
#include "event_queue.h"
#include "pdsdata/xtc/Dgram.hh"


namespace cass {

    // define static members
    FormatConverter *FormatConverter::_instance(0);
    QMutex FormatConverter::_mutex;
    EventQueue *FormatConverter::_eventqueue(0);


    FormatConverter::FormatConverter()
    {
        // create all the necessary individual format converters
        _converter[REMI]    = new REMI::Converter();
        _converter[Pulnix]  = new VMI::Converter();
        _converter[pnCCD]   = new pnCCD::Converter();
    }



    FormatConverter::~FormatConverter()
    {
        // destruct all the individual format converters
        for (std::map<Converters, ConversionBackend *>::iterator it=_converter.begin() ; it != _converter.end(); ++it )
            delete (it->second);
    }



    void FormatConverter::destroy()
    {
        QMutexLocker locker(&_mutex);
        delete _instance;
        _instance = 0;
    }


    FormatConverter *FormatConverter::instance(EventQueue* eventqueue)
    {
        QMutexLocker locker(&_mutex);
        _eventqueue = eventqueue;
        if(0 == _instance)
            _instance = new FormatConverter();
        return _instance;
    }



    //this slot is called once the eventqueue has new data available//
    void FormatConverter::processDatagram(uint32_t index)
    {
        //retrieve the Datagram with given index from the eventqueue//
        //this will automaticly lock this section of the ringbuffer//
        Pds::Dgram * datagram = _eventqueue->GetAndLockDatagram(index);

        //if datagram is configuration or an event (L1Accept) then we will iterate through it//
        //otherwise we ignore the datagram//
        if ((datagram->seq.service() == Pds::TransitionId::Configure) ||
            (datagram->seq.service() == Pds::TransitionId::L1Accept))
        {
            CASSEvent *cassevent=0;
            //if the datagram is an event than we create a new cass event first//
            if (datagram->seq.service() == Pds::TransitionId::L1Accept)
            {
                //extract the bunchId from the datagram//
                uint64_t bunchId = datagram->seq.clock().seconds();
                bunchId = (bunchId<<32) + static_cast<uint32_t>(datagram->seq.stamp().fiducials()<<8);

                //create a new cassevent//
                cassevent = new CASSEvent(bunchId);
                //cassevent = database.nextEvent();
            }

            //iterate through the datagram and find the wanted information//
            XtcIterator iter(&(datagram->xtc),_converter,cassevent,0);
            iter.iterate();

            //when the datagram was an event then emit the new CASSEvent//
            if(datagram->seq.service() == Pds::TransitionId::L1Accept)
                emit nextEvent(cassevent);

        }

        //unlock the datagram//
        _eventqueue->UnlockDatagram(index);
    }

}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
