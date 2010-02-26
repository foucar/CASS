//Copyright (C) 2010 lmf

#ifndef __POSTPROCESSOR_H__
#define __POSTPROCESSOR_H__

#include <QtCore/QMutex>

#include "cass.h"

namespace cass
{
  class CASSEvent;

  class PostProcessor
  {
  public:
    //creates an instace if not it does not exist already//
    static PostProcessor *instance(const char* OutputFileName);
    //this destroys the the instance//
    static void destroy();

  public:
    void postProcess(CASSEvent&);
    void loadSettings() {}
    void saveSettings() {}

  protected:
    PostProcessor(const char* OutputFileName) {}
    ~PostProcessor()                          {}

    //pointer to the instance//
    static PostProcessor *_instance;
    //Singleton operation locker in a multi-threaded environment.//
    static QMutex _mutex;
  };
}


#endif
