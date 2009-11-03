#include "event_manager.h"
#include "cass_event.h"

cass::EventManager::EventManager()
{
    for (size_t i=0;i<EventQueue::_maxbufsize;++i)
        _buffer[i] = new CASSEvent();
}

cass::EventManager::~EventManager()
{
    for (size_t i=0;i<EventQueue::_maxbufsize;++i)
        delete _buffer[i];
}
