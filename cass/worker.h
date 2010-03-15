//Copyright (C) 2009,2010 lmf

#ifndef __WORKER_H__
#define __WORKER_H__

#include <QtCore/QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include <map>
#include <utility>

#include "cass.h"
#include "ringbuffer.h"
#include "cass_event.h"


namespace cass
{
  class Analyzer;
  class PostProcessor;
  class HistogramBackend;


  class CASSSHARED_EXPORT Worker : public QThread
  {
    Q_OBJECT;
  public:
    Worker(cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>&, QObject *parent=0);
    ~Worker();

    void run();
    void suspend();
    void resume();
    void waitUntilSuspended();

    const std::map<std::pair<size_t, size_t>, HistogramBackend*>& histograms()const;


  signals:
    void processedEvent();

  public slots:
    void end();
    void loadSettings(size_t what);
    void saveSettings();

  private:
    cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>  &_ringbuffer;
    Analyzer      *_analyzer;
    PostProcessor *_postprocessor;
    bool           _quit;
    QMutex         _pauseMutex;
    QWaitCondition _pauseCondition;
    bool           _pause;
    bool           _paused;
    QWaitCondition _waitUntilpausedCondition;

  };

  //a class that will handle the requested amount of workers
  class CASSSHARED_EXPORT Workers : public QObject
  {
    Q_OBJECT;
  public:
    Workers(cass::RingBuffer<cass::CASSEvent,cass::RingBufferSize>&, QObject *parent=0);
    ~Workers();

    void start();
    const std::map<std::pair<size_t, size_t>, HistogramBackend*>& histograms()const;

  public slots:
    void end();
    void loadSettings(size_t what);

  signals:
    void finished();
    void processedEvent();

  private:
    std::vector<cass::Worker*> _workers; //list of workers
  };

}

#endif
