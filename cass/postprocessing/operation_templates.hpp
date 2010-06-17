// Copyright (C) 2010 Lutz Foucar

/** @file operations_templates.h file contains templated postprocessors that
 *                               will operate on histograms of other
 *                               postprocessors
 * @author Lutz Foucar
 */

#ifndef _OPERATION_TEMPLATES_H_
#define _OPERATION_TEMPLATES_H_

#include "backend.h"
#include "cass_event.h"
#include "histogram.h"
#include <time.h>

namespace cass
{


  /** Compare histogram for less than constant.
   *
   * @cassttng PostProcessor/\%name\%/{HistOne} \n
   *           the postprocessor name that contain the first histogram. Default
   *           is 0.
   * @cassttng PostProcessor/\%name\%/{Value} \n
   *           Value to compare the histograms value to. Default is 0.
   * @cassttng PostProcessor/\%name\%/{ConditionName} \n
   *           0D Postprocessor name that we check before filling image.
   *           if this setting is not defined, this postprocessor is unconditional.
   *           Therefore its always true.
   *
   * @tparam ComparisonOperator comaprison-operator that will work on the data
   * @author Lutz Foucar
   */
  template < class ComparisonOperator>
  class pp1 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp1(PostProcessors& hist, const PostProcessors::key_t&)
      : PostprocessorBackend(pp, key)
    {
      loadSettings(0);
    }

    /** load the settings of this pp */
    virtual void loadSettings(size_t)
    {
      QSettings settings;
      settings.beginGroup("PostProcessor");
      settings.beginGroup(_key.c_str());
      _value = settings.value("Value",0).toFloat();
      PostProcessors::key_t keyOne;
      _one = retrieve_and_validate(_pp,_key,"HistOne", keyOne);
      _dependencies.push_back(keyOne);
      bool ret (setupCondition());
      if (!_one || !ret) return;
      _result = new Histogram0DFloat();
      createHistList(2*cass::NbrOfWorkers);
      std::cout << "PostProcessor " << _key
          << ": will compare whether hist in PostProcessor " << keyOne
          << " is smaller than " << _value
          << std::endl;
    }

    /** process event */
    virtual void process(const CASSEvent&)
    {
      using namespace std;
      if ((*_condition)(evt).isTrue())
      {
        const HistogramFloatBase &one
            (dynamic_cast<const HistogramFloatBase&>((*_one)(evt)));
        one.lock.lockForRead();
        float first (accumulate(one.memory().begin(),
                                one.memory().end(),
                                0.f));
        one.lock.unlock();
        _result->lock.lockForWrite();
        *dynamic_cast<Histogram0DFloat*>(_result) = ComparisonOperator(first, _value);
        _result->lock.unlock();
      }
    }

  protected:
    /** pp containing the histogram */
    PostprocessorBackend *_one;

    /** constant value to compare to */
    float _value;
  };
