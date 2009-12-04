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
  class FormatConverter;
  class PostProcessor;
  namespace database
  {
    class Database;
  }

  class CASSSHARED_EXPORT Worker : public QThread
  {
    Q_OBJECT;
    public:
      Worker(lmf::RingBuffer<cass::CASSEvent,4>&, QObject *parent=0);
      ~Worker();

      void run();

    signals:
      void processedEvent();

    public slots:
      void end();
      void loadSettings();
      void saveSettings();

    private:
      lmf::RingBuffer<cass::CASSEvent,4>  &_ringbuffer;
      Analyzer                            *_analyzer;
      FormatConverter                     *_converter;
      database::Database                  *_database;
      bool                                 _quit;
  };
}

#endif
