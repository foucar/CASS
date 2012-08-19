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
 * @cassttng Log/{MaxLoggingLevel} \n
 *           The maximum Level of output requested.
 * @cassttng Log/{Directory} \n
 *           The directory where the log file will be written to. Default is
 *           the directory that cass was started in.
 * @cassttng Log/{Filename} \n
 *           The name of the log file. Default is "casslog_%yyyyMMdd%".
 *
 * @author Lutz Foucar
 */
class Log
{
public:
  /** the logging levels available */
  enum Level{ERROR, WARNING, INFO, VERBOSEINFO,
             DEBUG0, DEBUG1, DEBUG2, DEBUG3, DEBUG4,nbrOfLogLevel};

  /** retrive a reference to the singleton instance
   *
   * @return reference to the log singleton instance
   */
  static Log& ref();

  /** add a string to the log
   *
   * @param level the level of importance that the log message has
   * @param line string containing the line that should be added to the log
   */
  static void add(Level level, const std::string& line);

  /** destructor
   *
   * closes the log file
   */
  ~Log();

  /** load the logging settings from the .ini file
   *
   * if the instance has not yet been created create the logging instance.
   */
  static void loadSettings();

private:
  /** constructor
   *
   * open the file that contains the logging output. One can set the directory
   * via the .ini file. The name is always composed out of "casslog_yyyyMMdd.log"
   * where yyyy encodes the year, MM the month and dd the day. The log file is
   * opened such that new entries will always be appended to the file.
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

  /** load the logging settings from the .ini file
   *
   * Sets the logging message level that will be put into the log file and the
   * position and name of the file.
   *
   * The log file will be created from the current date and the user defined
   * directory. It will only be opened if the file has not been opened yet. If
   * the name of the file (including the absolute path) has changed, close it
   * (if it was open) and open it with the new filename.
   */
  void load();

private:
  /** the instance */
  static std::tr1::shared_ptr<Log> _instance;

  /** the stream that we write the log messages to */
  std::ofstream _log;

  /** a buffer for concatinating the message */
  std::string _buffer;

  /** the filename of the log file */
  std::string _filename;

  /** mutex to lock the singleton */
  static QMutex _lock;

  /** the used logging level*/
  static Level _loggingLevel;

  /** map the level to a string */
  static const char* _level2string[];
};
}//end namespace cass

#endif
