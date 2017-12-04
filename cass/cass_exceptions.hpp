// Copyright (C) 2010 Lutz Foucar

/**
 * @file cass_exceptions.hpp file contains custom exceptions used in cass
 *
 * @author Lutz Foucar
 */

#ifndef __CASS_EXCEPTIONS_HPP__
#define __CASS_EXCEPTIONS_HPP__

#include <stdexcept>

namespace cass
{
/** Exception thrown when accessing invalid histogram
 *
 * @author Jochen Küpper
 */
class InvalidResultError : public std::out_of_range
{
public:
  explicit InvalidResultError(const std::string &name, uint64_t id)
    : std::out_of_range("no result with requested event id exists"),
      _id(id),
      _name(name)
  {}

  virtual const char* what() const throw()
  {
    std::ostringstream msg;
    msg << "event with id (" << _id << ") is not in cache for "<<_name;
    return msg.str().c_str();
  }

  virtual ~InvalidResultError() throw(){}

protected:
  uint64_t _id;
  std::string _name;
};


/** Exception thrown when accessing invalid processor
 *
 * @author Lutz Foucar
 */
class InvalidProcessorError : public std::out_of_range
{
public:
  explicit InvalidProcessorError(const std::string &key)
    : std::out_of_range("Invalid processor requested!"), _key(key)
  {}

  virtual const char* what() const throw()
  {
    std::ostringstream msg;
    msg << "Invalid processor " << _key << " requested!";
    return msg.str().c_str();
  }

  virtual ~InvalidProcessorError() throw(){}

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

/** Exception thrown when SACLA tag is outdated
 *
 * In the online version of SACLA input this should be thrown when
 * the requested tag is outdated and not available anymore
 */
class TagOutdated : public std::runtime_error
{
public:
  /** explicit constructor
   *
   * @param message the error message
   */
  explicit TagOutdated(const std::string & message, bool thrown=true)
    : std::runtime_error(message),
      _wasThrown(thrown)
  {}

  virtual ~TagOutdated() throw() {}

  operator bool()const {return _wasThrown;}

private:
  bool _wasThrown;
};


/** Exception thrown when there is an error with a SACLA Pixel Detector
 *
 * In case there is some error with a SACLA pixel detector this should be thrown
 */
class SaclaPixDetError : public std::runtime_error
{
public:
  /** explicit constructor
   *
   * @param message the error message
   */
  explicit SaclaPixDetError(const std::string & message, bool thrown=true)
    : std::runtime_error(message),
      _wasThrown(thrown)
  {}

  virtual ~SaclaPixDetError() throw() {}

  operator bool()const {return _wasThrown;}

private:
  bool _wasThrown;
};

}//end namespace cass

#endif
