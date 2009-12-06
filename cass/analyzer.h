// Copyright (C) 2009 Jochen Küpper

#ifndef CASS_ANALYZER_H
#define CASS_ANALYZER_H

#include <map>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include "cass.h"

namespace cass
{
  class CASSEvent;
  class AnalysisBackend;


  class CASSSHARED_EXPORT Analyzer : public QObject
  {
    Q_OBJECT;

  public:
    //list of known individual analyzers//
    enum Analyzers {REMI, VMI, MachineData, pnCCD};
    //creates an instace if not it does not exist already//
    static Analyzer *instance();
    //this destroys the the instance//
    static void destroy();
    //function to use the analyzers for the different instruments//
    void processEvent(cass::CASSEvent*);

    //slots called by the gui//
  public slots:
    void saveSettings();
    void loadSettings();

  protected:
    Analyzer();
    ~Analyzer();

    //map of available analyzers//
    std::map<Analyzers, AnalysisBackend*> _analyzer;
    //pointer to the instance//
    static Analyzer *_instance;
    //Singleton operation locker in a multi-threaded environment.//
    static QMutex _mutex;

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
