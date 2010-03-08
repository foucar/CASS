// Copyright (C) 2009 lmf

#ifndef __SHAREDMEMORYINPUT_H__
#define __SHAREDMEMORYINPUT_H__

#include <QtCore/QObject>
#include <QThread>
#include <QMutex>

#include "cass.h"
#include "pdsdata/app/XtcMonitorClient.hh"
#include "ringbuffer.h"
#include "cass_event.h"

namespace cass
{
  class FormatConverter;

  class CASSSHARED_EXPORT SharedMemoryInput : public QThread, Pds::XtcMonitorClient
  {
    Q_OBJECT;
  public:
    SharedMemoryInput(char * PartitionTag, cass::RingBuffer<cass::CASSEvent,4>&, QObject *parent=0);
    ~SharedMemoryInput();

    void run();
    int  processDgram(Pds::Dgram*);

  signals:
    void newEventAdded();

  public slots:
    void end();

  private:
    cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>  &_ringbuffer;
    char                                *_partitionTag;
    bool                                 _quit;
    FormatConverter                     *_converter;

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
