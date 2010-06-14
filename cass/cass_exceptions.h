// Copyright (C) 2010 Lutz Foucar

#ifndef __POSTPROCESSOR_H__
#define __POSTPROCESSOR_H__

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

}

#endif
