//Copyright (C) 2010 lmf

#ifndef __POSTPROCESSOR_H__
#define __POSTPROCESSOR_H__

#include <QtCore/QMutex>

#include <map>
#include <pair>

#include "cass.h"

namespace cass
{
  class CASSEvent;
  class HistogramBackend;

  class PostProcessor
  {
  public:
    //creates an instace if not it does not exist already//
    static PostProcessor *instance(const char* OutputFileName);
    //this destroys the the instance//
    static void destroy();

  public:
    void postProcess(CASSEvent&);
    void loadSettings(size_t) {}
    void saveSettings() {}

  public:
    typedef std::map<std::pair<size_t, size_t>, HistogramBackend*> histograms_t;

  public://setters/getters
    const histograms_t  &histograms()const  {return _histograms;}
    histograms_t        &histograms()       {return _histograms;}

  protected:
    PostProcessor(const char* OutputFileName) {}
    ~PostProcessor()                          {}

    //pointer to the instance//
    static PostProcessor *_instance;
    //Singleton operation locker in a multi-threaded environment.//
    static QMutex _mutex;

    //container for all histograms//
    histograms_t _histograms;
  };
}

#endif
