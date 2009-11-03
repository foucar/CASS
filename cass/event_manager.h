// Copyright (C) 2009 lmf

#ifndef CASS_EVENTMANAGER_H
#define CASS_EVENTMANAGER_H

#include "cass.h"
#include "event_queue.h"

namespace cass
{
    class CASSEvent;

    class CASSSHARED_EXPORT EventManager
    {
//        Q_OBJECT;
    public:
        EventManager();
        ~EventManager();

    public:
        CASSEvent   *event(size_t index) {return _buffer[index];}

    private:
        CASSEvent   *_buffer[EventQueue::_maxbufsize];
    };

}//end namespace cass

#endif
