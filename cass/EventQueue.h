// Copyright (C) 2009 Jochen Küpper

#ifndef CASS_EVENTQUEUE_H
#define CASS_EVENTQUEUE_H

#include <QtCore/QObject>
#include <QThread>
#include <QMutex>

#include "cass.h"
#include "pdsdata/app/XtcMonitorClient.hh"

namespace cass
{

    class CASSSHARED_EXPORT EventQueue : public QThread, Pds::XtcMonitorClient
    {
        Q_OBJECT;
    public:
        EventQueue(QObject *parent=0);
        ~EventQueue();

        void run();
        void processDgram(Pds::Dgram*);

        Pds::Dgram* GetAndLockDatagram(uint32_t index);
        void UnlockDatagram(uint32_t index);

    signals:
        void nextEvent(uint32_t index);

    private:
        char       *_ringbuffer;
        char       *_ringbufferindizes[4];
        QMutex      _mutexes[4];
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
