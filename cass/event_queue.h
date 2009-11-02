// Copyright (C) 2009 Jochen Küpper

#ifndef CASS_EVENTQUEUE_H
#define CASS_EVENTQUEUE_H

#include <QtCore/QObject>
#include <QThread>
#include <QMutex>

#include "cass.h"
#include "pdsdata/app/XtcMonitorClient.hh"

//#define _maxbufsize 4
//#define _maxbufsize 1
//#define _maxdatagramsize 0x800000

namespace cass
{

    class CASSSHARED_EXPORT EventQueue : public QThread, Pds::XtcMonitorClient
    {
        Q_OBJECT;
    public:
        enum {_maxbufsize=1, _maxdatagramsize=0x800000};
        EventQueue(QObject *parent=0);
        ~EventQueue();

        void run();
        int  processDgram(Pds::Dgram*);

        Pds::Dgram* GetAndLockDatagram(uint32_t index);
        void UnlockDatagram(uint32_t index);

    signals:
        void nextEvent(quint32 index);
    public slots:
        void end();

    private:
        char       *_ringbuffer;
        char       *_ringbufferindizes[_maxbufsize];
        QMutex      _mutexes[_maxbufsize];
        uint32_t    _index;
        bool        _quit;
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
