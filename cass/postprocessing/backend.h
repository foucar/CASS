// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#ifndef _POSTPROCESSOR_BACKEND_H_
#define _POSTPROCESSOR_BACKEND_H_

#include <list>
#include <string>

#include "cass.h"
#include "postprocessor.h"

namespace cass
{
  //forward declaration
  class CASSEvent;

  /** @brief base class for postprocessors */
  class CASSSHARED_EXPORT PostprocessorBackend
  {
  public:
    /** constructor
     *
     * @param pp reference to the class that contains all postprocessors
     * @param key the key in the container of this postprocessor
     */
    PostprocessorBackend(PostProcessors& pp, const PostProcessors::key_t &key)
      : _key(key), _pp(pp)
    {}

    /** virtual destructor */
    virtual ~PostprocessorBackend() { }

    /** the operator called for each event */
    virtual void operator()(const CASSEvent&) = 0;

    /** Provide default implementation of loadSettings that does nothings */
    virtual void loadSettings(size_t)
    {
      VERBOSEOUT(std::cout << "calling backend' load settings"<<std::endl);
    }

    /** Define all postprocessors we depend on
     *
     * The dependencies must be run before the actual postprocessor is run by itself.
     */
    virtual PostProcessors::active_t dependencies() { return PostProcessors::active_t(); }

  protected:
    /** @return histogram of the actual postprocessor we call this for */
    virtual HistogramBackend *histogram_checkout() { return histogram_checkout(_key); }

    /** @overload
     *
     * @return histogram of the requested postprocessor
     */
    virtual HistogramBackend *histogram_checkout(std::string name)
    {
      try
      {
        PostProcessors::histograms_t hist(_pp.histograms_checkout());
        _pp.validate(name);
        _pp.histograms_release();
        return hist[name];
      }
      catch (InvalidHistogramError)
      {
        std::cout << "Invalid histogram: "<<name<<std::endl;
        _pp.histograms_release();
        return 0;
      }
    }

    /** release the histogram container readwritelock */
    void histogram_release() { _pp.histograms_release(); }

    /** the postprocessors key */
    PostProcessors::key_t _key;

    /** reference to the PostProcessors container */
    PostProcessors& _pp;
  };

} //end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
