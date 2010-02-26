#ifndef __WORKER_H__
#define __WORKER_H__

#include <QtCore/QObject>
#include <QThread>
#include <QMutex>


#include "cass.h"
#include "ringbuffer.h"
#include "cass_event.h"


namespace cass
{
  class Analyzer;
  class PostProcessor;

  class CASSSHARED_EXPORT Worker : public QThread
  {
    Q_OBJECT;
  public:
    Worker(lmf::RingBuffer<cass::CASSEvent,cass::RingBufferSize>&, QObject *parent=0);
    ~Worker();

    void run();

  signals:
    void processedEvent();

  public slots:
    void end();
    void loadSettings();
    void saveSettings();

  private:
    lmf::RingBuffer<cass::CASSEvent,cass::RingBufferSize>  &_ringbuffer;
    Analyzer      *_analyzer;
    PostProcessor *_postprocessor;
    bool           _quit;
  };

  //a class that will handle the requested amount of workers
  class CASSSHARED_EXPORT Workers : public QObject
  {
    Q_OBJECT;
  public:
    Workers(lmf::RingBuffer<cass::CASSEvent,cass::RingBufferSize>&, QObject *parent=0);
    ~Workers();

    void start();

  public slots:
    void end();

  signals:
    void finished();

  private:
    std::vector<cass::Worker*> _workers; //list of workers
  };

}

#endif
