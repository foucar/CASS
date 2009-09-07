// Copyright (C) 2009 Jochen Küpper

#ifndef CASS_EVENTQUEUE_H
#define CASS_EVENTQUEUE_H

#include <QtCore/QObject>
#include <QThread>

#include "cass.h"
#include "pdsdata/app/XtcMonitorClient.hh"

namespace cass
{

    class CASSSHARED_EXPORT EventQueue : public QThread, Pds::XtcMonitorClient
    {
        Q_OBJECT;
    public:
        EventQueue(QObject *parent=0);

        void run();
        void processDgram(Pds::Dgram*);

        Pds::Dgram* Datagram(uint32_t index)    {_ringbufferindizes[index][0]=1;return reinterpret_cast<Pds::Dgram*>(_ringbufferindizes[index]+1);}
        void doneWithDatagram(uint32_t index)   {_ringbufferindizes[index][0]=0;}

    signals:
        void nextEvent(uint32_t index);

    private:
        char       *_ringbuffer;
        char       *_ringbufferindizes[4];
        uint32_t    _index;
    };

}//end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
