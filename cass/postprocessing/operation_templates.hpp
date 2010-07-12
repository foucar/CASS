// Copyright (C) 2010 Lutz Foucar

/**
 * @file operation_templates.hpp file contains templated postprocessors that
 *                               will operate on histograms of other
 *                               postprocessors
 * @author Lutz Foucar
 */

#ifndef _OPERATION_TEMPLATES_H_
#define _OPERATION_TEMPLATES_H_

#include <typeinfo>

#include "cass_settings.h"
#include "backend.h"
#include "histogram.h"
#include "convenience_functions.h"

namespace cass
{


  /** Compare to a constant constant.
   *
   * this templated class will compare the sum of all bins to a constant value
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           the postprocessor name that contain the first histogram. Needs to
   *           be implemented, because default is "", which is invalid.
   * @cassttng PostProcessor/\%name\%/{Value} \n
   *           Value to compare the histograms value to. Default is 0.
   *
   * @tparam ComparisonOperator comaprison-operator that will work on the data
   * @author Lutz Foucar
   */
  template <class ComparisonOperator>
  class pp1 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp1(PostProcessors& pp, const PostProcessors::key_t& key, const ComparisonOperator& x)
      : PostprocessorBackend(pp, key), op (x)
    {
      loadSettings(0);
    }

    /** load the settings of this pp */
    virtual void loadSettings(size_t)
    {
      CASSSettings settings;
      settings.beginGroup("PostProcessor");
      settings.beginGroup(_key.c_str());
      _value = settings.value("Value",0).toFloat();
      setupGeneral();
      _one = setupDependency("HistName");
      bool ret (setupCondition());
      if (!_one || !ret) return;
      _result = new Histogram0DFloat();
      createHistList(2*cass::NbrOfWorkers);
      std::cout<< "PostProcessor "<<_key
          <<": will compare hist in PostProcessor "<<_one->key()
          <<" to constant "<<_value
          <<" using "<< typeid(op).name()
          <<". Condition is "<<_condition->key()
          << std::endl;
    }

    /** process event */
    virtual void process(const CASSEvent& evt)
    {
      using namespace std;
      const HistogramFloatBase &one
          (dynamic_cast<const HistogramFloatBase&>((*_one)(evt)));
      one.lock.lockForRead();
      float first (accumulate(one.memory().begin(),
                              one.memory().end(),
                              0.f));
      one.lock.unlock();
      _result->lock.lockForWrite();
      *dynamic_cast<Histogram0DFloat*>(_result) = op(first, _value);
      _result->lock.unlock();
    }

  protected:
    /** pp containing the histogram */
    PostprocessorBackend *_one;

    /** constant value to compare to */
    float _value;

    /** the comparison operation done with the data */
    ComparisonOperator op;
  };









  /** Boolean Comparison of two 0d pp.
   *
   * this templated class will boolean compare two 0d histograms
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
   *           the postprocessor names that contain the first histogram and second
   *           histogram for the boolean comparison. Default is "" for both. This
   *           will result in an exception. Since pp "" is not implemented.
   *
   * @tparam BooleanOperator boolean operator to operate on 0D Hists
   * @author Lutz Foucar
   */
  template <class BooleanOperator>
  class pp5 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp5(PostProcessors& pp, const PostProcessors::key_t& key, const BooleanOperator& x)
      : PostprocessorBackend(pp, key), op(x)
    {
      loadSettings(0);
    }

    /** load the settings of this pp */
    virtual void loadSettings(size_t)
    {
      setupGeneral();
      _one = setupDependency("HistOne");
      _two = setupDependency("HistTwo");
      bool ret (setupCondition());
      if ( !(_one && _two && ret) ) return;
      if (_one->getHist(0).dimension() != 0 ||
          _two->getHist(0).dimension() != 0)
        throw std::runtime_error("PP type 5: Either HistOne or HistTwo is not a 0D Hist");
      _result = new Histogram0DFloat();
      createHistList(2*cass::NbrOfWorkers);
      std::cout << "PostProcessor " << _key
          << ": will boolean compare PostProcessor " << _one->key()
          << " to PostProcessor " << _two->key()
          <<" using "<< typeid(op).name()
          <<". Condition is "<<_condition->key()
          << std::endl;
    }

    /** process event */
    virtual void process(const CASSEvent& evt)
    {
      const HistogramFloatBase &one
          (dynamic_cast<const HistogramFloatBase&>((*_one)(evt)));
      const HistogramFloatBase &two
          (dynamic_cast<const HistogramFloatBase&>((*_two)(evt)));
      one.lock.lockForRead();
      two.lock.lockForRead();
      _result->lock.lockForWrite();
      *dynamic_cast<Histogram0DFloat*>(_result) = op(one.isTrue(), two.isTrue());
      _result->lock.unlock();
      two.lock.unlock();
      one.lock.unlock();
    }

  protected:
    /** pp containing first histogram */
    PostprocessorBackend *_one;

    /** pp containing second histogram */
    PostprocessorBackend *_two;

    /** the boolean operation done with the 0D data */
    BooleanOperator op;
  };









  /** Compare two histograms.
   *
   * This template compares the sum of all bins of two histograms.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
   *           the postprocessor names that contain the first and second
   *           histogram. Needs to be implemented, because default is "",
   *           which is invalid.
   *
   * @tparam ComparisonOperator comaprison-operator that will work on the data
   * @author Lutz Foucar
   */
  template <class ComparisonOperator>
  class pp7 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp7(PostProcessors& pp, const PostProcessors::key_t& key, const ComparisonOperator& x)
      : PostprocessorBackend(pp, key), op (x)
    {
      loadSettings(0);
    }

    /** load the settings of this pp */
    virtual void loadSettings(size_t)
    {
      setupGeneral();
      _one = setupDependency("HistOne");
      _two = setupDependency("HistTwo");
      bool ret (setupCondition());
      if ( !(_one && _two && ret) ) return;
      if (_one->getHist(0).dimension() != _two->getHist(0).dimension())
        throw std::runtime_error("PP type 7: HistOne is not the same type "
                                 " as HistTwo, or they have not the same size.");
      _result = new Histogram0DFloat();
      createHistList(2*cass::NbrOfWorkers);
      std::cout << "PostProcessor " << _key
          << ": compares Histogram in PostProcessor " << _one->key()
          << " to Histogram in PostProcessor " << _two->key()
          <<" using "<< typeid(op).name()
          <<". Condition is "<<_condition->key()
          << std::endl;
    }

    /** process event */
    virtual void process(const CASSEvent& evt)
    {
      const HistogramFloatBase &one
          (dynamic_cast<const HistogramFloatBase&>((*_one)(evt)));
      const HistogramFloatBase &two
          (dynamic_cast<const HistogramFloatBase&>((*_two)(evt)));
      one.lock.lockForRead();
      two.lock.lockForRead();
      float first (accumulate(one.memory().begin(),
                              one.memory().end(),
                              0.f));
      float second (accumulate(two.memory().begin(),
                               two.memory().end(),
                               0.f));
      two.lock.unlock();
      one.lock.unlock();
      _result->lock.lockForWrite();
      *dynamic_cast<Histogram0DFloat*>(_result) = op(first,second);
      _result->lock.unlock();
    }

  protected:
    /** pp containing first histogram */
    PostprocessorBackend *_one;

    /** pp containing second histogram */
    PostprocessorBackend *_two;

    /** the comparison operation done with the data */
    ComparisonOperator op;
  };





  /** Operation on two histograms.
   *
   * templated operation on two histograms. The operation will be performed
   * bin by bin.
   *
   * The two histograms need to be of the same dimension and size.
   *
   * The resulting histogram will be created using the size and dimension of the
   * first histogram.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistOne|HistTwo} \n
   *           the postprocessor names that contain the first and second
   *           histogram. Needs to be implemented, because default is "",
   *           which is invalid.
   *
   * @tparam Operator operator that will work on the data
   * @author Lutz Foucar
   */
  template <class Operator>
  class pp20 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp20(PostProcessors& pp, const PostProcessors::key_t& key, const Operator& x)
      : PostprocessorBackend(pp, key), op (x)
    {
      loadSettings(0);
    }

    /** load the settings of this pp */
    virtual void loadSettings(size_t)
    {
      setupGeneral();
      _one = setupDependency("HistOne");
      _two = setupDependency("HistTwo");
      bool ret (setupCondition());
      if ( !(_one && _two && ret) ) return;
      const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_one->getHist(0)));
      const HistogramFloatBase &two(dynamic_cast<const HistogramFloatBase&>(_two->getHist(0)));
      if (one.dimension() != two.dimension() ||
          one.memory().size() !=
          two.memory().size())
        throw std::runtime_error("PP type 20: HistOne is not the same type "
                                 " as HistTwo, or they have not the same size.");
      _result = one.clone();
      createHistList(2*cass::NbrOfWorkers);
      std::cout << "PostProcessor " << _key
          << ": operation "<< typeid(op).name()
          << " on Histogram in PostProcessor " << _one->key()
          << " with Histogram in PostProcessor " << _two->key()
          <<". Condition is "<<_condition->key()
          << std::endl;
    }

    /** process event */
    virtual void process(const CASSEvent& evt)
    {
      const HistogramFloatBase &one
          (dynamic_cast<const HistogramFloatBase&>((*_one)(evt)));
      const HistogramFloatBase &two
          (dynamic_cast<const HistogramFloatBase&>((*_two)(evt)));
      one.lock.lockForRead();
      two.lock.lockForRead();
      _result->lock.lockForWrite();
      transform(one.memory().begin(), one.memory().end(),
                two.memory().begin(),
                (dynamic_cast<HistogramFloatBase *>(_result))->memory().begin(),
                op);
      _result->lock.unlock();
      one.lock.unlock();
      two.lock.unlock();
    }

  protected:
    /** pp containing first histogram */
    PostprocessorBackend *_one;

    /** pp containing second histogram */
    PostprocessorBackend *_two;

    /** the operation done with the data */
    Operator op;
  };






  /** Operate histogram with constant.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName} \n
   *           the postprocessor name that contain the first histogram. Needs to
   *           be implemented, because default is "", which is invalid.
   * @cassttng PostProcessor/\%name\%/{Value} \n
   *           Value for the operation. Default is 1.
   *
   * @tparam Operator operator that will work on the data
   * @author Lutz Foucar
   */
  template <class Operator>
  class pp23 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp23(PostProcessors& pp, const PostProcessors::key_t& key, const Operator& x)
      : PostprocessorBackend(pp, key), op (x)
    {
      loadSettings(0);
    }

    /** load the settings of this pp */
    virtual void loadSettings(size_t)
    {
      CASSSettings settings;
      settings.beginGroup("PostProcessor");
      settings.beginGroup(_key.c_str());
      _value = settings.value("Factor",1).toFloat();
      setupGeneral();
      _one = setupDependency("HistName");
      bool ret (setupCondition());
      if (!(_one && ret)) return;
      const HistogramBackend &one(_one->getHist(0));
      _result = one.clone();
      createHistList(2*cass::NbrOfWorkers);
      std::cout << "PostProcessor " << _key
          << ": operation "<< typeid(op).name()
          << " on Histogram in PostProcessor " << _one->key()
          << " with " << _value
          <<". Condition is "<<_condition->key()
          << std::endl;
    }

    /** process event */
    virtual void process(const CASSEvent& evt)
    {
    using namespace std;
    const HistogramFloatBase &one
        (dynamic_cast<const HistogramFloatBase&>((*_one)(evt)));
    one.lock.lockForRead();
    _result->lock.lockForWrite();
    transform(one.memory().begin(), one.memory().end(),
              dynamic_cast<HistogramFloatBase *>(_result)->memory().begin(),
              bind2nd(op,_value));
    _result->lock.unlock();
    one.lock.unlock();
  }

  protected:
    /** pp containing input histogram */
    PostprocessorBackend *_one;

    /** the factor we mulitply the histogram with */
    float _value;

    /** the operation done with the data */
    Operator op;
  };



}//end namespace
#endif
