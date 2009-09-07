// Copyright (C) 2009 Jochen Küpper

#include "EventQueue.h"
#include "pdsdata/xtc/Dgram.hh"

cass::EventQueue::EventQueue( QObject *parent):
        QThread(parent)
{
    //initialize the ringbuffer//
    _ringbuffer = new char[(0x700000+1)*4];
    _ringbufferindizes[0] = _ringbuffer;
    _ringbufferindizes[1] = _ringbuffer+(0x700000+1);
    _ringbufferindizes[2] = _ringbuffer+(0x700000+1)*2;
    _ringbufferindizes[3] = _ringbuffer+(0x700000+1)*3;
    _index = 0;
}

void cass::EventQueue::run()
{
    //start the xtcmonitorclient//
    //this eventqueue will subscripe to a partitiontag with name cass//
    runMonitor("cass");
}

void cass::EventQueue::processDgram(Pds::Dgram* datagram)
{
    //check which entry of the ringbuffer is not been working on now//
    bool NotOk=true;
    char * rb;
    while(NotOk)
    {
        rb = _ringbufferindizes[_index];
        if (rb[0])
            _index = (++_index )% 4;
        else
        {
            rb++;
            NotOk = false;
        }
    }
    //copy the datagramm//
    memcpy(rb,datagram,sizeof(Pds::Dgram));
    uint32_t sizeofPayload = datagram->xtc.sizeofPayload();
    memcpy(rb+sizeof(Pds::Dgram),datagram+1,sizeofPayload);

    //tell the world that there is a new datagram available//
    emit nextEvent(_index);
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
