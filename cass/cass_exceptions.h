// Copyright (C) 2010 Lutz Foucar

/**
 * @file cass_exceptions.h file contains custom exceptions used in cass
 *
 * @author Lutz Foucar
 */

#ifndef __CASS_EXCEPTIONS_H__
#define __CASS_EXCEPTIONS_H__

#include <stdexcept>

namespace cass
{
/** Exception thrown when accessing invalid histogram
 *
 * @author Jochen Küpper
 */
class InvalidHistogramError : public std::out_of_range
{
public:
  explicit InvalidHistogramError(uint64_t id)
    : std::out_of_range("Event id  does not exist for histogram!"), _id(id)
  {}

  virtual const char* what() const throw()
  {
    std::ostringstream msg;
    msg << "Event id (" << _id << ") does not exist for histogram!";
    return msg.str().c_str();
  }

  virtual ~InvalidHistogramError() throw(){}

protected:
  uint64_t _id;
};


/** Exception thrown when accessing invalid postprocessor
 *
 * @author Lutz Foucar
 */
class InvalidPostProcessorError : public std::out_of_range
{
public:
  explicit InvalidPostProcessorError(const std::string &key)
    : std::out_of_range("Invalid postprocessor requested!"), _key(key)
  {}

  virtual const char* what() const throw()
  {
    std::ostringstream msg;
    msg << "Invalid histogram " << _key << " requested!";
    return msg.str().c_str();
  }

  virtual ~InvalidPostProcessorError() throw(){}

protected:
  std::string _key;
};

/** Exception thrown when there is a problem with deserializing QDataStreams
   *
   * @author Lutz Foucar
   */
class DeserializeError : public std::runtime_error
{
public:
  /** explicit constructor
     *
     * @param message the error message
     */
  explicit DeserializeError(const std::string & message)
    : std::runtime_error(message)
  {

  }

  virtual ~DeserializeError() throw() {}
};

/** Exception thrown when one needs to reastart the input loop
   *
   * @author Lutz Foucar
   */
class RestartInputLoop : public std::runtime_error
{
public:
  /** explicit constructor
     *
     * @param message the error message
     */
  explicit RestartInputLoop(const std::string & message ="")
    : std::runtime_error(message)
  {

  }

  virtual ~RestartInputLoop() throw() {}
};


/** Exception thrown when there is a problem during data generation
   *
   * @author Lutz Foucar
   */
class DataGenerationError : public std::runtime_error
{
public:
  /** explicit constructor
     *
     * @param message the error message
     */
  explicit DataGenerationError(const std::string & message)
    : std::runtime_error(message)
  {}

  virtual ~DataGenerationError() throw() {}
};


/** Exception thrown when there is a problem with the data
 *
 * @author Lutz Foucar
 */
class InvalidData : public std::logic_error
{
public:
  /** explicit constructor
   *
   * @param message the error message
   */
  explicit InvalidData(const std::string & message)
    : std::logic_error(message)
  {}

  virtual ~InvalidData() throw() {}
};

/** Exception thrown when a timeout occured
 *
 * When wating too long for a new processable element this should
 * be thrown
 */
class ProcessableTimedout : public std::runtime_error
{
public:
  /** explicit constructor
   *
   * @param message the error message
   */
  explicit ProcessableTimedout(const std::string & message)
    : std::runtime_error(message)
  {}

  virtual ~ProcessableTimedout() throw() {}

};

}//end namespace cass

#endif
