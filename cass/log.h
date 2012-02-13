// Copyright (C) 2012 Lutz Foucar

/**
 * @file log.h contains a logger for cass
 *
 * @author Lutz Foucar
 */

#ifndef __LOG_HPP__
#define __LOG_HPP__

#include <tr1/memory>
#include <string>
#include <fstream>
#include <map>

#include <QtCore/QMutex>

namespace cass
{
/** a logger for logging debug and info messages
 *
 * class is a singleton so one can call it from everywhere
 *
 * @author Lutz Foucar
 */
class Log
{
public:
  /** the logging levels available */
  enum Level{ERROR, WARNING, INFO, DEBUG, DEBUG1, DEBUG2, DEBUG3, DEBUG4};

  /** retrieve a reference to the logging instance
   *
   * @return reference to the log singleton instance
   * @param type the type of logging that one wants to add
   */
  static void add(Level level, const std::string& line);

  /** destructor
   *
   * closes the log file
   */
  ~Log();

  /** load the logging settings from the .ini file */
  void loadSettings();

private:
  /** constructor
   *
   * open the file that contains the logging output. One can set the directory
   * via the .ini file. The name is always composed out of "casslog_yyyyMMdd.log"
   * where yyyy encodes the year, MM the month and dd the day. The log file is
   * opened such that new entries will always be appended to the file.
   *
   * @cassttng Log/{MaxLoggingLevel} \n
   *           The maximum Level of output requested.
   * @cassttng Log/{Directory} \n
   *           The directory where the log file will be written to. Default is
   *           the directory that cass was started in.
   *
   * @note private so one can only create the instance from the static
   *       function
   */
  Log();

  /** add a string to the log
   *
   * will put the date, time, message level and then the string to the logging
   *
   * @param level the level of importance that the log message has
   * @param line string containing the line that should be added to the log
   */
  void addline(Level level, const std::string& line);

private:
  /** the instance */
  static std::tr1::shared_ptr<Log> _instance;

  /** the stream that we write the log messages to */
  std::ofstream _log;

  /** a buffer for concatinating the message */
  std::string _buffer;

  /** mutex to lock the singleton */
  static QMutex _lock;

  /** the used logging level*/
  static Level _loggingLevel;

  /** map the level to a string */
  static const char* _level2string[];
};
}//end namespace cass

#endif
