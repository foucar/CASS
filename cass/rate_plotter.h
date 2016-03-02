//Copyright (C) 2010,2013,2016 Lutz Foucar

/**
 * @file rate_plotter.h file contains declaration of class to plot the rate
 *                      calculated by ratemeters
 *
 * @author Lutz Foucar
 */

#ifndef _RATE_PLOTTER_H_
#define _RATE_PLOTTER_H_

#include <vector>
#include <list>
#include <string>
#include <tr1/memory>

#include <QtCore/QThread>

#include "cass.h"

namespace cass
{
//forward declarations
class Ratemeter;

/** Plotting information about the ongoing processing
 *
 * class that will plot various information about the ongoing process
 *
 * @cassttng ProcessingStatistics/{ShowInfo} \n
 *           If true, it will display the requested information. If false,
 *           no output is generated. Default is true.
 * @cassttng ProcessingStatistics/{Output} \n
 *           Filename to which the info will be written. If no parameter is
 *           given, output will be directed to cout.
 * @cassttng ProcessingStatistics/{UpdateInterval} \n
 *           The updating interval in s. Default is 1.
 * @cassttng ProcessingStatistics/{ShowInputRate} \n
 *           If true, it will show the input rate. Default is true.
 * @cassttng ProcessingStatistics/{ShowInputLoad} \n
 *           If true, it will show the input load. Default is true.
 * @cassttng ProcessingStatistics/{ShowAnalysisRate} \n
 *           If true, it will show the analysis rate. Default is true.
 * @cassttng ProcessingStatistics/{ShowProgress} \n
 *           If true, it will show the progress of the process. Default is true.
 * @cassttng ProcessingStatistics/{ShowNbrProcessedEvents} \n
 *           If true, it will show the number of events processed. Default is
 *           false.
 * @cassttng ProcessingStatistics/{NewLine} \n
 *           If true, it will show the input rate. Default is true.
 * @cassttng ProcessingStatistics/ValueProcessors/{size} \n
 *           the number of value like processors to be plotted.
 * @cassttng ProcessingStatistics/ValueProcessors/\%id\%/{Name} \n
 *           Name of the value like processor. If Unknown, then it won't be
 *           added to the list. Default is Unknown.
 * @cassttng ProcessingStatistics/ValueProcessors/\%id\%/{FieldWidth} \n
 *           Width of the value field. Used in setw(). Default is 10.
 * @cassttng ProcessingStatistics/ValueProcessors/\%id\%/{Precision} \n
 *           Precsion after the decimal point for floating. Used in
 *           setprecision(). Default is 7.
 * @cassttng ProcessingStatistics/ValueProcessors/\%id\%/{Unit} \n
 *           The unit to be displayed after the value. Default is "".
 *
 * @author Lutz Foucar
 */
class RatePlotter : public QThread
{
public:
  /** a shared pointer of this type */
  typedef std::tr1::shared_ptr<RatePlotter> shared_pointer;

  /** constructor.
   *
   * @param inputrate the ratemeter of the input thread
   * @param inputload  ratemeter to measure the data load
   * @param analyzerate the ratemeter of the worker threads
   * @param parent the qt parent of this object
   */
  RatePlotter(Ratemeter &inputrate,
              Ratemeter &inputload,
              Ratemeter &analyzerate,
              QObject *parent=0);

  /** destructor
   *
   * checks whether thread is still running in which case it will be terminated.
   * Then waits until thread has finished.
   */
  virtual ~RatePlotter();

protected:
  /** the plotting loop
   *
   * sleep for interval time and then retrieve the rate from the ratemeters
   * and plot it.
   */
  void run();

private:
  /** reference to the input Ratemeter */
  Ratemeter &_inputrate;

  /** reference to the input Ratemeter */
  Ratemeter &_inputload;

  /** reference to the workers (analysis) Ratemeter */
  Ratemeter &_analyzerate;

  /** flag to tell whether to show the info at all */
  bool _showInfo;

  /** the interval in which the rate is plottet in s */
  int _interval;

  /** the filename to which the status will be written */
  std::string _filename;

  /** flag to tell whether the input rate should be reported */
  bool _showInputRate;

  /** flag to tell whether the input load should be reported */
  bool _showInputLoad;

  /** flag to tell whether the analysis rate should be reported */
  bool _showAnalysisRate;

  /** flag to tell whether the how much is processed ratio should be reported */
  bool _showProgress;

  /** flag to tell whether to report on how many events have been processed */
  bool _showNProcessedEvents;

  /** flag to tell whether the updated info should be put into a new line */
  bool _newLine;

  /** define the processor properties */
  struct ProcProperties
  {
    /** the name of the processor */
    std::string name;

    /** how much space should be used for the output of the variable */
    int fieldWidth;

    /** how much digits after the decimal point should be used */
    int precision;

    /** the unit that should be displayed after the value */
    std::string unit;
  };

  /** define the processor list */
  typedef std::list<ProcProperties> proclist_t;

  /** list of value like processors that should be reported on */
  proclist_t _procs;
};
}//end namespace cass

#endif // RATEMETER_H
