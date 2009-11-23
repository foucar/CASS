// Copyright (C) 2009 LMF

#include <iostream>
#include <iomanip>
#include "event_queue.h"
#include "pdsdata/xtc/Dgram.hh"

cass::EventQueue::EventQueue(QObject *parent):
        QThread(parent),
        _index(0),
        _quit(false)
{
    //initialize the ringbuffer//
    _ringbuffer = new char[(_maxdatagramsize)*_maxbufsize];
    for (size_t i=0;i<_maxbufsize;++i)
        _ringbufferindizes[i] = _ringbuffer+(_maxdatagramsize)*i;
}

cass::EventQueue::~EventQueue()
{
    //unlock all mutexes//
    for (size_t i=0;i<_maxbufsize;++i)
        if (_mutexes[i].tryLock())
            _mutexes[i].unlock();
    //delete ringbuffer//
    delete [] _ringbuffer;
}

void cass::EventQueue::run()
{
    //start the xtcmonitorclient//
    //this eventqueue will subscripe to a partitiontag with name cass//
    _quit = false;
    Pds::XtcMonitorClient::run("1_1_AMOx");
}

void cass::EventQueue::end()
{
    _quit=true;
}

int cass::EventQueue::processDgram(Pds::Dgram* datagram)
{
    //check which entry of the ringbuffer is not locked by a receiver//   
    while(!(_mutexes[_index].tryLock()))
        _index = (++_index) % _maxbufsize;

    //remember which index it is and get the corrosponding ringbuffer pointer//
    uint32_t index = _index;
    char * rb = _ringbufferindizes[index];

    //copy the datagramm//
    memcpy(rb,datagram,sizeof(Pds::Dgram));
    uint32_t sizeofPayload = datagram->xtc.sizeofPayload();
    memcpy(rb+sizeof(Pds::Dgram),datagram+1,sizeofPayload);

    //unlock the lock that one can access the new datagram
    //only if it was a L1Accept transition, otherwise keep locked//
    if(datagram->seq.service() == Pds::TransitionId::L1Accept)
        _mutexes[_index].unlock();

    //advance the index such that next time this function is called it will check the next index first//
    _index = (++_index) % _maxbufsize;

    //tell the world that there is a new datagram available//
    emit nextEvent(index);

    //return the quit code//
    return _quit;
}

Pds::Dgram * cass::EventQueue::GetAndLockDatagram(uint32_t index)
{
    Pds::Dgram * datagram = reinterpret_cast<Pds::Dgram*>(_ringbufferindizes[index]);
    //if the datagram is something other than L1Accept, it is already locked//
    //if it is a L1Accept we need to obtain the lock//
    if(datagram->seq.service() == Pds::TransitionId::L1Accept)
        _mutexes[index].lock();
    return datagram;
}

void cass::EventQueue::UnlockDatagram(uint32_t index)
{
    //check whether Mutex is already locked, if so unlock it//
    if(!(_mutexes[index].tryLock()))
        _mutexes[index].unlock();
}

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
