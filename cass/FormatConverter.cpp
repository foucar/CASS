// Copyright (C) 2009 Jochen Küpper

#include <QtCore/QMutexLocker>
#include "FormatConverter.h"


namespace cass {

    // define static members
    FormatConverter *FormatConverter::_instance(0);
    QMutex FormatConverter::_mutex;
    EventQueue *FormatConverter::_eventqueue(0);


    FormatConverter::FormatConverter()
    {
        // create all the necessary individual format converters
    }



    FormatConverter::~FormatConverter()
    {
        // destruct all the individual format converters
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
//        //retrieve the Datagram with given index from the eventqueue//
//        //this will automaticly lock this section of the ringbuffer//
//        Pds::Dgram * datagram = _eventqueue->GetAndLockDatagram(index);
//
//        //if datagram is configuration or an event (L1Accept) then we will iterate through it//
//        if ((datagram->seq.service() == Pds::TransitionId::Configure) ||
//            (datagram->seq.service() == Pds::TransitionId::L1Accept))
//        {
//            //if the datagram is an event than we create a new cass event first//
//            if ()
//            {
//
//                //extract the bunchId from the datagram//
//            }
//
//
//        }

    }

}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
