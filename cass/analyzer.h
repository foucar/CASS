// Copyright (C) 2009 Jochen Küpper
// Copyright (C) 2009, 2010 Lutz Foucar

/**
 * @file analyzer.h file contains declaration of class handling the pre
 *                  analyzers
 *
 * @author Lutz Foucar
 */

#ifndef CASS_ANALYZER_H
#define CASS_ANALYZER_H

#include <map>
#include <set>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include "cass.h"

namespace cass
{
  class CASSEvent;
  class AnalysisBackend;


  /** Container and call handler for all Pre Analyzers.
   *
   * @cassttng PreAnalyzer/{useCommercialCCDAnalyzer}\n
   *           Should pre analysis run: default true
   * @cassttng PreAnalyzer/{useAcqirisAnalyzer}\n
   *           Should pre analysis run: default false
   * @cassttng PreAnalyzer/{useMachineAnalyzer}\n
   *           Should pre analysis run: default false
   * @cassttng PreAnalyzer/{usepnCCDAnalyzer}\n
   *           Should pre analysis run: default true
   *
   * @author Jochen Kuepper
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT Analyzer : public QObject
  {
    Q_OBJECT;

  public:
    /** list of known individual analyzers*/
    enum Analyzers {Acqiris, ccd, MachineData, pnCCD};

    /** creates an instance if it does not exist already*/
    static Analyzer *instance();

    /** this destroys the the instance*/
    static void destroy();

    /** function to use the analyzers for the different instruments*/
    void processEvent(cass::CASSEvent*);

    /** function that one can implement when one wants to do something just before quitting*/
    void aboutToQuit() {};

  public slots:
    /** save the settings of the analyzers*/
    void saveSettings();

    /** load the settings of the analyzers*/
    void loadSettings(size_t what);

  protected:
    /** protected constructor, should only be called through instance*/
    Analyzer();

    /** protected destructor, should only be called through destroy*/
    ~Analyzer();

  public:
    typedef std::map<Analyzers, AnalysisBackend*> analyzers_t;
    typedef std::set<Analyzers> active_analyzers_t;

  protected:
    /** map of available analyzers*/
    analyzers_t _analyzer;

    /** a set of the active analyzers*/
    active_analyzers_t _activeAnalyzers;

    /** pointer to the instance*/
    static Analyzer *_instance;

    /** Singleton operation locker in a multi-threaded environment.*/
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
