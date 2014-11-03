// Copyright (C) 2010-2013 Lutz Foucar
// (C) 2010 Thomas White - Updated to new (outdated) PP framework

/** @file operations.cpp file contains definition of postprocessors that will
 *                       operate on histograms of other postprocessors
 * @author Lutz Foucar
 */

#include <QtCore/QString>
#include <iterator>
#include <algorithm>
#include <numeric>

#include "cass.h"
#include "operations.h"
#include "histogram.h"
#include "convenience_functions.h"
#include "cass_settings.h"
#include "log.h"
#include "input_base.h"


using namespace cass;
using namespace std;
using tr1::bind;
using tr1::placeholders::_1;
using tr1::placeholders::_2;
using tr1::placeholders::_3;

namespace cass
{
/** temporarliy provide own copy of min_element to be able to compile
 *
 * This is just a copy from the cpp reference guide in the internet
 */
template <class ForwardIterator>
ForwardIterator max_element ( ForwardIterator first, ForwardIterator last )
{
  if (first==last) return last;
  ForwardIterator largest = first;

  while (++first!=last)
    if (*largest<*first)
      largest=first;
  return largest;
}
/** temporarliy provide own copy of min_element to be able to compile
 *
 * This is just a copy from the cpp reference guide in the internet
 */
template <class ForwardIterator>
ForwardIterator min_element ( ForwardIterator first, ForwardIterator last )
{
  if (first==last) return last;
  ForwardIterator smallest = first;

  while (++first!=last)
    if (*first<*smallest)
      smallest=first;
  return smallest;
}
}//end namespace cass


// ************ Operation on two results *************

pp1::pp1(const name_t & name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp1::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _one = setupDependency("HistOne");
  _two = setupDependency("HistTwo");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;

  const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_one->result()));
  const HistogramFloatBase &two(dynamic_cast<const HistogramFloatBase&>(_two->result()));
  if (one.dimension() != two.dimension() ||
      one.memory().size() != two.memory().size())
    throw invalid_argument("pp1::loadSettings() '"+name()+"': HistOne '" + _one->name() +
                           "' with dimension '" + toString(one.dimension()) +
                           "' and memory size '" + toString(one.memory().size()) +
                           "' differs from HistTwo '" + _one->name() +
                           "' with has dimension '" + toString(two.dimension()) +
                           "' and memory size '" + toString(two.memory().size()));

  string operation(s.value("Operation","+").toString().toStdString());
  if (operation == "+")
    _op = plus<float>();
  else if (operation == "-")
    _op = minus<float>();
  else if (operation == "/")
    _op = divides<float>();
  else if (operation == "*")
    _op = multiplies<float>();
  else if (operation == "AND")
    _op = logical_and<bool>();
  else if (operation == "OR")
    _op = logical_or<bool>();
  else if (operation == ">")
    _op = greater<float>();
  else if (operation == ">=")
    _op = greater_equal<float>();
  else if (operation == "<")
    _op = less<float>();
  else if (operation == "<=")
    _op = less_equal<float>();
  else if (operation == "==")
    _op = equal_to<float>();
  else if (operation == "!=")
    _op = not_equal_to<float>();
  else
    throw invalid_argument("pp1::loadSettings() '" + name() +
                           "': operation '" + operation + "' is unkown.");

  createHistList(_one->result().copy_sptr());

  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' will do operation '"+  operation + "'with '" + _one->name() +
           "' as first and '" + _two->name() + "' as second argument" +
           "'. Condition is '" + _condition->name() + "'");
}

void pp1::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_one->result(evt.id())));
  const HistogramFloatBase &two
      (dynamic_cast<const HistogramFloatBase&>(_two->result(evt.id())));
  HistogramFloatBase &result(dynamic_cast<HistogramFloatBase&>(res));

  QReadLocker lock1(&one.lock);
  QReadLocker lock2(&two.lock);

  transform(one.memory().begin(), one.memory().end(),
            two.memory().begin(),
            result.memory().begin(),
            _op);
  result.nbrOfFills()=1;
}








// ************ Postprocessor 4: Apply boolean NOT to 0D histogram *************

pp2::pp2(const name_t &name)
 : PostProcessor(name)
{
 loadSettings(0);
}

void pp2::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _value = s.value("Value",1).toFloat();
  _retrieveValue = bind(&pp2::valueFromConst,this,_1);
  setupGeneral();
  _hist = setupDependency("HistName");
  const bool usePP(s.value("ValueName","DonnotUse").toString().toStdString() != "DonnotUse");
  bool ret (setupCondition());
  if (usePP)
  {
    _valuePP = setupDependency("ValueName");
    ret = _valuePP && ret;
    _retrieveValue = bind(&pp2::valueFromPP,this,_1);
  }
  if (!(_hist && ret))
    return;

  string operation(s.value("Operation","+").toString().toStdString());
  if (operation == "+")
    _op = plus<float>();
  else if (operation == "-")
    _op = minus<float>();
  else if (operation == "/")
    _op = divides<float>();
  else if (operation == "*")
    _op = multiplies<float>();
  else if (operation == "AND")
    _op = logical_and<bool>();
  else if (operation == "OR")
    _op = logical_or<bool>();
  else if (operation == ">")
    _op = greater<float>();
  else if (operation == ">=")
    _op = greater_equal<float>();
  else if (operation == "<")
    _op = less<float>();
  else if (operation == "<=")
    _op = less_equal<float>();
  else if (operation == "==")
    _op = equal_to<float>();
  else if (operation == "!=")
    _op = not_equal_to<float>();
  else
    throw invalid_argument("pp2::loadSettings() '" + name() +
                           "': operation '" + operation + "' is unkown.");

  string valuePos(s.value("ValuePos","first").toString().toStdString());
  if (valuePos == "first")
    _setParamPos = bind(&pp2::ValAtFirst,this,_1);
  else if (valuePos == "second")
    _setParamPos = bind(&pp2::ValAtSecond,this,_1);
  else
    throw invalid_argument("pp2::loadSettings() '" + name() +
                           "': value position '" + valuePos + "' is unkown.");

  createHistList(_hist->result().copy_sptr());
  Log::add(Log::INFO,"PostProcessor '" + name() + "' operation '" + operation +
           "' on '" + _hist->name() + "' with " +
           (usePP ? " value in '"+_valuePP->name()+"'" : "constant '"+toString(_value)+"'")
            + ". Condition is "+ _condition->name() + "'");
}

pp2::unaryoperation_t pp2::ValAtFirst(float val)
{
  return bind(_op,val,_1);
}

pp2::unaryoperation_t pp2::ValAtSecond(float val)
{
  return bind(_op,_1,val);
}

float pp2::valueFromConst(const CASSEvent::id_t&)
{
  return _value;
}

float pp2::valueFromPP(const CASSEvent::id_t& id)
{
  const Histogram0DFloat &value
      (dynamic_cast<const Histogram0DFloat&>(_valuePP->result(id)));
  QReadLocker lock(&value.lock);
  return value.getValue();
}

void pp2::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase &hist
      (dynamic_cast<const HistogramFloatBase&>(_hist->result(evt.id())));
  HistogramFloatBase &result(dynamic_cast<HistogramFloatBase&>(res));

  QReadLocker lock(&hist.lock);

  transform(hist.memory().begin(), hist.memory().end(),
            result.memory().begin(),
            _setParamPos(_retrieveValue(evt.id())));

  result.nbrOfFills()=1;
}










// ************ Postprocessor 4: Apply boolean NOT to 0D histogram *************

pp4::pp4(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp4::loadSettings(size_t)
{
  setupGeneral();
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret))
    return;
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));

  Log::add(Log::INFO,"PostProcessor '" + name() + "' will apply NOT to PostProcessor '" +
           _one->name() + "'. Condition is '" + _condition->name() + "'");
}

void pp4::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_one->result(evt.id())));
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  QReadLocker lock(&one.lock);

  result = !one.isTrue();
  result.nbrOfFills()=1;
}











// ********** Postprocessor 9: Check if histogram is in given range ************

pp9::pp9(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp9::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _range = make_pair(s.value("LowerLimit",0).toFloat(),
                     s.value("UpperLimit",0).toFloat());
  setupGeneral();
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret))
    return;
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));

  Log::add(Log::INFO,"PostProcessor '" + name()
           + "' will check whether hist in PostProcessor '" + _one->name() +
      "' is between '" + toString(_range.first) + "' and '" + toString(_range.second) +
      "'. Condition is '" + _condition->name() + "'");
}

void pp9::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_one->result(evt.id())));
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  QReadLocker lock(&one.lock);

  const float value (accumulate(one.memory().begin(),
                                one.memory().end(),
                                0.f));

  result = (_range.first < value &&  value < _range.second);
  result.nbrOfFills()=1;
}






// ********** Postprocessor 12: PP with constant value ************

pp12::pp12(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp12::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  string type(s.value("ValueType","0D").toString().toStdString());
  if (type == "0D")
  {
    _res = tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat());
  }
  else if (type == "1D")
  {
    _res = tr1::shared_ptr<Histogram1DFloat>
        (new Histogram1DFloat(s.value("XNbrBins",1).toInt(),
                              s.value("XLow",0).toFloat(),
                              s.value("XUp",1).toFloat(),
                              s.value("XTitle","x-axis").toString().toStdString()
                              ));
  }
  else if (type == "2D")
  {
    _res = tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(s.value("XNbrBins",1).toInt(),
                              s.value("XLow",0).toFloat(),
                              s.value("XUp",1).toFloat(),
                              s.value("YNbrBins",1).toInt(),
                              s.value("YLow",0).toFloat(),
                              s.value("YUp",1).toFloat(),
                              s.value("XTitle","x-axis").toString().toStdString(),
                              s.value("YTitle","y-axis").toString().toStdString()
                              ));
  }
  else
  {
    throw invalid_argument("pp12::loadSettings(): '" + name() +
                           "' unknown valuetype '" + type + "' provided");
  }

  float value(s.value("Value",0).toFloat());
  if (name() == "DefaultTrueHist")
    value = true;
  if (name() == "DefaultFalseHist")
    value = false;
  fill(_res->memory().begin(), _res->memory().end(), value);

  _hide = s.value("Hide",true).toBool();

  Log::add(Log::INFO,"PostProcessor '" +  name() + "' has constant value of '" +
           toString(value) + "' and is of type '" + type + "'");
}









// ************ Postprocessor 13: return the result (identity operation) *****

pp13::pp13(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp13::loadSettings(size_t)
{
  setupGeneral();
  _one = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_one && ret))
    return;
  createHistList(_one->result().copy_sptr());

  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' will return a copy of PostProcessor '" + _one->name() +
           "'. Condition is '" + _condition->name() + "'");
}

void pp13::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_one->result(evt.id())));
  HistogramFloatBase &result(dynamic_cast<HistogramFloatBase&>(res));

  QReadLocker lock(&one.lock);
  /** @note the below one has to be implemented otherwise one can't copy a table */
  //result.axis() = one.axis();
  result.nbrOfFills() = one.nbrOfFills();
  copy(one.memory().begin(),one.memory().end(),result.memory().begin());
}









// ********** Postprocessor 15: Check if value has changed ************

pp15::pp15(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp15::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _previousVal = 0;
  setupGeneral();
  _hist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(_hist && ret))
    return;
  if (_hist->result().dimension() != 0 )
    throw runtime_error("pp15::loadSettings: Hist '" + _hist->name() +
                        "' is not a 0D Hist");
  _difference = s.value("Difference",0.).toFloat();
  if (fabs(_difference) < std::numeric_limits<float>::epsilon() )
    _difference = std::numeric_limits<float>::epsilon();
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));

  Log::add(Log::INFO,"Postprocessor '" + name() +
           "' will check whether the difference between the current and the" +
           " previous value of '" + _hist->name() +
           "' is bigger than '"+ toString(_difference) +
           "'. It will use condition '" + _condition->name() +"'");

}

void pp15::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram0DFloat &val
      (dynamic_cast<const Histogram0DFloat&>(_hist->result(evt.id())));
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  QReadLocker lock1(&val.lock);

  const float value(val.getValue());

  /** @note the fuzzycompare doesn't work when using big numbers */
  result = fabs(value-_previousVal) > _difference;
  result.nbrOfFills()=1;
  QMutexLocker lock(&_mutex);
  _previousVal = value;
}










// ****************** Postprocessor 40: Threshold histogram ********************

pp40::pp40(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp40::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _threshold = s.value("Threshold", 0.0).toFloat();
  _one = setupDependency("HistName");
  setupGeneral();
  bool ret (setupCondition());
  if (!(_one && ret))
    return;
  createHistList(_one->result().copy_sptr());

  Log::add(Log::INFO,"PostProcessor '" + name() +
      "' will threshold Histogram in PostProcessor '" + _one->name() +
      "' above '" + toString(_threshold) + "'. Condition is '" + _condition->name() + "'");
}

void pp40::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_one->result(evt.id())));
  HistogramFloatBase &result(dynamic_cast<HistogramFloatBase&>(res));

  QReadLocker lock(&one.lock);

  transform(one.memory().begin(), one.memory().end(),
                 result.memory().begin(),
                 bind2nd(threshold(), _threshold));
  result.nbrOfFills()=1;
}





// ****************** Postprocessor 40: Threshold histogram ********************

pp41::pp41(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp41::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _one = setupDependency("HistName");
  _threshold = setupDependency("ThresholdName");
  bool ret (setupCondition());
  if (!(_one && _threshold && ret))
    return;

  _userVal = s.value("UserVal",0.f).toFloat();
  _lowerBound = s.value("LowerBound",0.5).toFloat();
  _upperBound = s.value("UpperBound",1.5).toFloat();

  if (_one->result().dimension() != _threshold->result().dimension())
    throw invalid_argument("pp41:loadSettings() '" + name() + "' Hist to threshold '" +
                           _one->name() + "' and theshold histo '" + _threshold->name() +
                           "' don't have the same dimension.");
  if (_one->result().dimension() == 1)
    if (_one->result().axis()[HistogramBackend::xAxis].size() !=
        _threshold->result().axis()[HistogramBackend::xAxis].size())
      throw invalid_argument("pp41:loadSettings() '" + name() + "' Hist to threshold '" +
                             _one->name() + "' (" +
                             toString(_one->result().axis()[HistogramBackend::xAxis].size()) +
                             ") and theshold histo '" + _threshold->name() + "' (" +
                              toString(_threshold->result().axis()[HistogramBackend::xAxis].size()) +
                             ") differ in the size.");
  if (_one->result().dimension() == 2)
    if (_one->result().axis()[HistogramBackend::xAxis].size() !=
        _threshold->result().axis()[HistogramBackend::xAxis].size() ||
        _one->result().axis()[HistogramBackend::yAxis].size() !=
        _threshold->result().axis()[HistogramBackend::yAxis].size())
      throw invalid_argument("pp41:loadSettings() '" + name() + "' Hist to threshold '" +
                             _one->name() + "' (" +
                             toString(_one->result().axis()[HistogramBackend::xAxis].size()) + "x" +
                             toString(_one->result().axis()[HistogramBackend::yAxis].size()) +
                             ") and theshold histo '" + _threshold->name() + "' (" +
                             toString(_threshold->result().axis()[HistogramBackend::xAxis].size()) + "x" +
                             toString(_threshold->result().axis()[HistogramBackend::yAxis].size()) +
                             ") differ in the shape.");
  createHistList(_one->result().copy_sptr());

  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' will threshold Histogram in PostProcessor '" + _one->name() +
           "' above pixels in image '" + _threshold->name() +
           "'. Condition is '" + _condition->name() + "'");
}

float pp41::checkrange(float val, float checkval)
{
  return (_lowerBound < checkval && checkval < _upperBound) ? _userVal : val;
}

void pp41::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase &image
      (dynamic_cast<const HistogramFloatBase&>(_one->result(evt.id())));
  const HistogramFloatBase &threshimage
      (dynamic_cast<const HistogramFloatBase&>(_threshold->result(evt.id())));
  HistogramFloatBase &result(dynamic_cast<HistogramFloatBase&>(res));

  QReadLocker lock1(&image.lock);
  QReadLocker lock2(&threshimage.lock);

  transform(image.memory().begin(), image.memory().end(),
            threshimage.memory().begin(),
            result.memory().begin(),
            bind(&pp41::checkrange,this,_1,_2));
  result.nbrOfFills()=1;
}












// *** postprocessors 50 projects 2d hist to 1d histo for a selected region of the axis ***

pp50::pp50(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp50::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _userRange = make_pair(s.value("LowerBound",-1e6).toFloat(),
                         s.value("UpperBound", 1e6).toFloat());
  _axis = static_cast<HistogramBackend::Axis>(s.value("Axis",HistogramBackend::xAxis).toUInt());
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  if (_pHist->result().dimension() != 2)
    throw invalid_argument("pp50::setupParameters()'" + name() +
                           "': Error the histogram we depend on '" + _pHist->name() +
                           "' is not a 2D Histogram.");
  const Histogram2DFloat &hist
      (dynamic_cast<const Histogram2DFloat&>(_pHist->result()));

  switch (_axis)
  {
  case (HistogramBackend::xAxis):
    _otherAxis = HistogramBackend::yAxis;
    break;
  case (HistogramBackend::yAxis):
    _otherAxis = HistogramBackend::xAxis;
    break;
  default:
    throw invalid_argument("pp50::loadSettings() '" + name() +
                           "': requested _axis '" + toString(_axis) +
                           "' does not exist.");
    break;
  }
  const AxisProperty &projAxis(hist.axis()[_axis]);
  const AxisProperty &otherAxis(hist.axis()[_otherAxis]);
  _range = make_pair(max(_userRange.first, otherAxis.lowerLimit()),
                     min(_userRange.second, otherAxis.upperLimit()));
  createHistList(
        tr1::shared_ptr<Histogram1DFloat>
        (new Histogram1DFloat(projAxis.nbrBins(),
                              projAxis.lowerLimit(), projAxis.upperLimit())));
  Log::add(Log::INFO,"PostProcessor '" + name() +
      "' will project histogram of PostProcessor '" + _pHist->name() + "' from '" +
      toString(_range.first) + "' to '" + toString(_range.second) + "' on axis '" +
      toString(_axis) + "'. Condition is '" + _condition->name() + "'");
}

void pp50::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>(_pHist->result(evt.id())));
  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  QReadLocker lock(&one.lock);

  result = one.project(_range,_axis);
  result.nbrOfFills()=1;
}













// *** postprocessors 51 calcs integral over a region in 1d histo ***

pp51::pp51(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp51::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(name().c_str());
  _area = make_pair(s.value("LowerBound",-1e6).toFloat(),
                    s.value("UpperBound", 1e6).toFloat());
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  if (_pHist->result().dimension() != 1)
    throw invalid_argument("pp51::loadSettings: '" + name() +
                           "': hist '" + _pHist->name() + "' is not a 1d hist.");
  const Histogram1DFloat &one
      (dynamic_cast<const Histogram1DFloat&>(_pHist->result()));
  _area.first  = max(_area.first, one.axis()[HistogramBackend::xAxis].lowerLimit());
  _area.second = min(_area.second,one.axis()[HistogramBackend::xAxis].upperLimit());

  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));

  Log::add(Log::INFO, "PostProcessor '" + name() +
      "' will create integral of 1d histogram in PostProcessor '" + _pHist->name() +
      "' from '" + toString(_area.first) + "' to '" + toString(_area.second) +
      "'. Condition is '" + _condition->name() + "'");
}

void pp51::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram1DFloat &one
      (dynamic_cast<const Histogram1DFloat&>(_pHist->result(evt.id())));
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  QReadLocker lock(&one.lock);

  result = one.integral(_area);
  result.nbrOfFills()=1;
}









// *** postprocessor 56 stores previous version of another histogram ***

pp56::pp56(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp56::loadSettings(size_t)
{
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_pHist->result()));
  createHistList(one.copy_sptr());
  _storage.resize(one.memory().size());
  Log::add(Log::INFO,"Postprocessor '" + name() +
           "' stores the previous histogram from PostProcessor '" + _pHist->name() +
           "'. Condition on postprocessor '" + _condition->name() +"'");
}

void pp56::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));
  HistogramFloatBase &result(dynamic_cast<HistogramFloatBase&>(res));
  HistogramFloatBase::storage_t &histmem(result.memory());

  QReadLocker rlock(&one.lock);
  QMutexLocker lock(&_mutex);
  copy(_storage.begin(),_storage.end(),histmem.begin());
  copy(one.memory().begin(),one.memory().end(),_storage.begin());
  result.nbrOfFills() = 1;
}


















// *** postprocessors 57 weighted projects 2d hist to 1d histo for a selected region of the axis ***

pp57::pp57(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp57::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  if (_pHist->result().dimension() != 2)
    throw invalid_argument("pp57::setupParameters()'" + name() +
                           "': Error the histogram we depend on '" + _pHist->name() +
                           "' is not a 2D Histogram.");
  const Histogram2DFloat &hist(dynamic_cast<const Histogram2DFloat&>(_pHist->result()));

  pair<float,float> userRange(make_pair(s.value("LowerBound",-1e6).toFloat(),
                                        s.value("UpperBound", 1e6).toFloat()));
  int projection_axis(s.value("Axis",HistogramBackend::xAxis).toUInt());
  _excludeVal = s.value("ExclusionValue",0).toFloat();

  const AxisProperty &xAxis(hist.axis()[HistogramBackend::xAxis]);
  const AxisProperty &yAxis(hist.axis()[HistogramBackend::yAxis]);
  _nX = xAxis.nbrBins();

  switch(projection_axis)
  {
  case (HistogramBackend::xAxis):
    _Xrange = make_pair(0,xAxis.nbrBins());
    _Yrange = make_pair(yAxis.bin(userRange.first),
                        yAxis.bin(userRange.second));
    _project = bind(&pp57::projectToX,this,_1,_2,_3);
    createHistList(
          tr1::shared_ptr<Histogram1DFloat>
          (new Histogram1DFloat(xAxis.nbrBins(),
                                xAxis.lowerLimit(), xAxis.upperLimit())));
    break;

  case (HistogramBackend::yAxis):
    _Xrange = make_pair(xAxis.bin(userRange.first),
                        xAxis.bin(userRange.second));
    _Yrange = make_pair(0,xAxis.nbrBins());
    _project = bind(&pp57::projectToY,this,_1,_2,_3);
    createHistList(
          tr1::shared_ptr<Histogram1DFloat>
          (new Histogram1DFloat(yAxis.nbrBins(),
                                yAxis.lowerLimit(), yAxis.upperLimit())));
    break;

  default:
    throw invalid_argument("pp57::loadSettings() '" + name() +
                           "': requested _axis '" + toString(projection_axis) +
                           "' does not exist.");
    break;
  }
  Log::add(Log::INFO,"PostProcessor '" + name() +
      "' will project histogram of PostProcessor '" + _pHist->name() + "' from '" +
      toString(userRange.first) + "' to '" + toString(userRange.second) + "' on axis '" +
      toString(projection_axis) + "'. Condition is '" + _condition->name() + "'");
}

void pp57::projectToX(const HistogramFloatBase::storage_t &src,
                      HistogramFloatBase::storage_t& result,
                      HistogramFloatBase::storage_t& norm)
{
  for(size_t y(_Yrange.first); y<_Yrange.second; ++y)
    for(size_t x(_Xrange.first); x<_Xrange.second; ++x)
    {
      const float pixval(src[y*_nX + x]);
      if (!qFuzzyCompare(pixval,_excludeVal))
      {
        result[x] += pixval;
        norm[x] += 1;
      }
    }
}

void pp57::projectToY(const HistogramFloatBase::storage_t &src,
                      HistogramFloatBase::storage_t& result,
                      HistogramFloatBase::storage_t& norm)
{
  for(size_t y(_Yrange.first); y<_Yrange.second; ++y)
    for(size_t x(_Xrange.first); x<_Xrange.second; ++x)
    {
      const float pixval(src[y*_nX + x]);
      if (!qFuzzyCompare(pixval,_excludeVal))
      {
        result[y] += pixval;
        norm[y] += 1;
      }
    }
}

void pp57::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram2DFloat &one
      (dynamic_cast<const Histogram2DFloat&>(_pHist->result(evt.id())));
  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  QReadLocker lock(&one.lock);

  vector<float> norm(result.memory().size(),0);
  _project(one.memory(),result.memory(),norm);
  transform(result.memory().begin(),result.memory().end(),
            norm.begin(),result.memory().begin(),
            divides<float>());

  result.nbrOfFills()=1;
}










// *** postprocessor 60 histograms 0D values ***

pp60::pp60(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp60::loadSettings(size_t)
{
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  createHistList(set1DHist(name()));
  Log::add(Log::INFO,"Postprocessor '" + name() +
      "' histograms values from PostProcessor '" +  _pHist->name() +
      "'. Condition on PostProcessor '" + _condition->name() + "'");
}

void pp60::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase &hist
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));
  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  QReadLocker lock(&hist.lock);

  const HistogramFloatBase::storage_t &histmem(hist.memory());
  HistogramFloatBase::storage_t::const_iterator value(histmem.begin());
  HistogramFloatBase::storage_t::const_iterator histEnd(histmem.end());

  result.clear();
  for (; value != histEnd; ++value)
    result.fill(*value);

  result.nbrOfFills() = 1;
}











// *** postprocessor 61 averages histograms ***

pp61::pp61(const name_t &name)
  : AccumulatingPostProcessor(name)
{
  loadSettings(0);
}

void pp61::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  unsigned average = s.value("NbrOfAverages", 1).toUInt();
  _alpha =  average ? 2./static_cast<float>(average+1.) : 0.;
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  createHistList(_pHist->result().copy_sptr());
  Log::add(Log::INFO,"Postprocessor '" + name() +
      "' averages histograms from PostProcessor '" +  _pHist->name() +
      "' alpha for the averaging '" + toString(_alpha) +
      "'. Condition on postprocessor '" + _condition->name() + "'");
}

void pp61::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));

  HistogramFloatBase &result(dynamic_cast<HistogramFloatBase&>(res));

  QReadLocker lock(&one.lock);

  ++result.nbrOfFills();
  float scale = (1./result.nbrOfFills() < _alpha) ?
                _alpha :
                1./result.nbrOfFills();

  transform(one.memory().begin(),one.memory().end(),
            result.memory().begin(),
            result.memory().begin(),
            Average(scale));
}













// *** postprocessor 62 sums up histograms ***

pp62::pp62(const name_t &name)
  : AccumulatingPostProcessor(name)
{
  loadSettings(0);
}

void pp62::loadSettings(size_t)
{
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  createHistList(_pHist->result().copy_sptr());
  Log::add(Log::INFO,"Postprocessor '" + name() +
      "' sums up histograms from PostProcessor '" +  _pHist->name() +
      "'. Condition on postprocessor '" + _condition->name() + "'");
}

void pp62::process(const CASSEvent& evt,HistogramBackend &res)
{
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));
  HistogramFloatBase &result(dynamic_cast<HistogramFloatBase&>(res));

  QReadLocker lock(&one.lock);

  transform(one.memory().begin(),one.memory().end(),
            result.memory().begin(),
            result.memory().begin(),
            plus<float>());
  ++result.nbrOfFills();
}








// *** postprocessors 63 calculate the time average of a 0d/1d/2d hist given the number
//     of samples that are going to be used in the calculation ***

pp63::pp63(const name_t &name)
  : AccumulatingPostProcessor(name),
    _num_seen_evt(0),
    _when_first_evt(0),
    _first_fiducials(0)
{
  loadSettings(0);
}

void pp63::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  const size_t min_time_user (s.value("MinTime",0).toUInt());
  const size_t max_time_user (s.value("MaxTime",300).toUInt());
  _timerange = make_pair(min_time_user,max_time_user);
  _nbrSamples=s.value("NumberOfSamples",5).toUInt();
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  createHistList(_pHist->result().copy_sptr());
  Log::add(Log::INFO,"PostProcessor '" + name() +
      "' will calculate the time average of histogram of PostProcessor '" + _pHist->name() +
      "' from now '" + toString(s.value("MinTime",0).toUInt()) + "' to '" +
      toString(s.value("MaxTime",300).toUInt()) + "' seconds '" + toString(_timerange.first) +
      "' ; '" + toString(_timerange.second) + "' each bin is equivalent to up to '" +
      toString(_nbrSamples) + "' measurements," +
      " Condition on PostProcessor '" + _condition->name() + "'");
}

void pp63::process(const CASSEvent& evt, HistogramBackend &res)
{
  //#define debug1
#ifdef debug1
  char timeandday[40];
  struct tm * timeinfo;
#endif
  uint32_t fiducials;
  time_t now_of_event;
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));
  HistogramFloatBase &result(dynamic_cast<HistogramFloatBase&>(res));

  // using docu at http://asg.mpimf-heidelberg.mpg.de/index.php/Howto_retrieve_the_Bunch-/Event-id
  //remember the time of the first event
  if(_when_first_evt==0)
  {
    _when_first_evt=static_cast<time_t>(evt.id()>>32);
#ifdef debug1
    timeinfo = localtime ( &_when_first_evt );
    strftime(timeandday,40,"%Y%m%d %H%M%S",timeinfo);
    cout<<"Starting now is "<< timeandday <<" "<<_num_seen_evt<< " "<<_nbrSamples <<endl;
#endif
    _num_seen_evt=1;
    _first_fiducials=static_cast<uint32_t>(evt.id() & 0x000000001FFFFF00);
  }
  now_of_event=static_cast<time_t>(evt.id()>>32);
#ifdef debug1
  timeinfo = localtime ( &now_of_event );
  strftime(timeandday,40,"%Y%m%d %H%M%S",timeinfo);
#endif
  fiducials=static_cast<uint32_t>(evt.id() & 0x000000001FFFFF00);
#ifdef debug1
  timeinfo = localtime ( &now_of_event );
  strftime(timeandday,40,"%Y%m%d %H%M%S",timeinfo);
  //      cout<<"Starting now is "<< timeandday <<" "<<_num_seen_evt<< " "<<_nbrSamples <<endl;
  cout<<"ora "<< timeandday<<" "<<_first_fiducials << " " <<fiducials << " " << _num_seen_evt <<endl;
#endif
  if(fiducials>_first_fiducials+(_nbrSamples-1)*4608 /*0x1200 ??why*/
     && now_of_event>=_when_first_evt)
  {
#ifdef debug1
    cout <<"time to forget"<<endl;
#endif
    //remember the time also whenever (_num_seen_evt-1)%_nbrSamples==0
    _first_fiducials=fiducials;
    _when_first_evt=now_of_event;
    _num_seen_evt=1;
  }

  //    if(now_of_event<_when_first_evt-100)
  if(_first_fiducials>fiducials)
  {
#ifdef debug1
    cout <<"extra time to forget"<<endl;
#endif
    //remember the time also whenever (_num_seen_evt-1)%_nbrSamples==0
    _first_fiducials=fiducials;
    _when_first_evt=now_of_event;
    _num_seen_evt=1;
  }
  QReadLocker lock(&one.lock);

  result.nbrOfFills();
  transform(one.memory().begin(),one.memory().end(),
            result.memory().begin(),
            result.memory().begin(),
            TimeAverage(float(_num_seen_evt)));
  ++_num_seen_evt;
  if(_num_seen_evt>_nbrSamples+1) cout<<"pp64::process(): How... it smells like fish! "
      <<_num_seen_evt
      <<" "<<_nbrSamples
      <<endl;
}













// ***  pp 64 takes a 0d histogram (value) as input and writes it in the last bin of a 1d histogram
//    *** while shifting all other previously saved values one bin to the left.

pp64::pp64(const name_t &name)
  : AccumulatingPostProcessor(name)
{
  loadSettings(0);
}

void pp64::loadSettings(size_t)
{
  CASSSettings s;

  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));

  _hist = setupDependency("HistName");

  bool ret (setupCondition());
  if ( !(_hist && ret) ) return;

  setupGeneral();
  _size = s.value("Size", 10000).toUInt();

  createHistList(tr1::shared_ptr<Histogram1DFloat>(new Histogram1DFloat(_size, 0, _size-1,"Shots")));

  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' will make a history of values of histogram in pp '" +
           _hist->name() + ", size of history '" + toString(_size) +
           "' Condition on postprocessor '" + _condition->name() + "'");
}

void pp64::process(const CASSEvent &evt, HistogramBackend &res)
{
  const HistogramFloatBase &hist
      (dynamic_cast<const HistogramFloatBase &>(_hist->result(evt.id())));
  HistogramFloatBase &result(dynamic_cast<HistogramFloatBase&>(res));

  QReadLocker lock(&hist.lock);

  const HistogramFloatBase::storage_t &values(hist.memory());
  HistogramFloatBase::storage_t::const_iterator value(values.begin());
  HistogramFloatBase::storage_t::const_iterator valueEnd(values.end());

  HistogramFloatBase::storage_t &mem(result.memory());

  for(; value != valueEnd ;++value)
  {
    rotate(mem.begin(), mem.begin()+1, mem.end());
    mem[_size-1] = *value;
  }
}







// *** postprocessor 65 histograms 2 0D values to 2D histogram ***

pp65::pp65(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp65::loadSettings(size_t)
{
  setupGeneral();
  _one = setupDependency("HistOne");
  _two = setupDependency("HistTwo");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  if (_one->result().dimension() != 0 ||
      _two->result().dimension() != 0)
    throw std::runtime_error("PP type 65: Either HistOne or HistTwo is not a 0D Hist");
  createHistList(set2DHist(name()));
  Log::add(Log::INFO,"Postprocessor '" + name() +
      "': histograms values from PostProcessor '" +  _one->name() +"' and '" +
      _two->name() + ". condition on PostProcessor '" + _condition->name() + "'");
}

void pp65::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram0DFloat &one
      (dynamic_cast<const Histogram0DFloat&>(_one->result(evt.id())));
  const Histogram0DFloat &two
      (dynamic_cast<const Histogram0DFloat&>(_two->result(evt.id())));
  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  QReadLocker lock1(&one.lock);
  QReadLocker lock2(&two.lock);


  result.clear();
  result.fill(one.getValue(),two.getValue());
  result.nbrOfFills()=1;
}












// *** postprocessor 66 histograms 2 1D values to 2D histogram ***

pp66::pp66(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp66::loadSettings(size_t)
{
  setupGeneral();
  _one = setupDependency("HistOne");
  _two = setupDependency("HistTwo");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_one->result()));
  const HistogramFloatBase &two(dynamic_cast<const HistogramFloatBase&>(_two->result()));
  if (one.dimension() != 1 || two.dimension() != 1)
    throw runtime_error("pp66::loadSettings(): HistOne '" + _one->name() +
                        "' with dimension '" + toString(one.dimension()) +
                        "' or HistTwo '" + _two->name() + "' has dimension '" +
                        toString(two.dimension()) + "' does not have dimension 1");
  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(one.axis()[HistogramBackend::xAxis].nbrBins(),
                              one.axis()[HistogramBackend::xAxis].lowerLimit(),
                              one.axis()[HistogramBackend::xAxis].upperLimit(),
                              two.axis()[HistogramBackend::xAxis].nbrBins(),
                              two.axis()[HistogramBackend::xAxis].lowerLimit(),
                              two.axis()[HistogramBackend::xAxis].upperLimit(),
                              one.axis()[HistogramBackend::xAxis].title(),
                              two.axis()[HistogramBackend::xAxis].title())));

   Log::add(Log::INFO,"Postprocessor '" + name() +
           "' creates a two dim histogram from PostProcessor '" + _one->name() +
           "' and '" +  _two->name() + "'. condition on PostProcessor '" +
           _condition->name() + "'");
}

void pp66::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram1DFloat &one
      (dynamic_cast<const Histogram1DFloat&>(_one->result(evt.id())));
  const Histogram1DFloat &two
      (dynamic_cast<const Histogram1DFloat&>(_two->result(evt.id())));
  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  QReadLocker lock1(&one.lock);
  QReadLocker lock2(&two.lock);

  HistogramFloatBase::storage_t &memory(result.memory());
  const size_t oneNBins(one.axis()[HistogramBackend::xAxis].nbrBins());
  const size_t twoNBins(two.axis()[HistogramBackend::xAxis].nbrBins());
  for (size_t j(0); j < twoNBins; ++j)
    for (size_t i(0); i < oneNBins; ++i)
      memory[j*oneNBins+i] = one.memory()[i]*two.memory()[j];
  result.nbrOfFills()=1;
}















// *** postprocessor 67 histograms 2 0D values to 1D histogram add weight ***

pp67::pp67(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp67::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _one = setupDependency("HistOne");
  _two = setupDependency("HistTwo");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  if (_one->result().dimension() != _two->result().dimension() )
    throw invalid_argument("pp67::loadSettings() '" + name() + "': '" +
                           _one->name() + "' and '" + _two->name() +
                           "' are not of the same type");
  if (dynamic_cast<const HistogramFloatBase&>(_one->result()).memory().size() !=
      dynamic_cast<const HistogramFloatBase&>(_two->result()).memory().size())
    throw invalid_argument("pp67::loadSettings() '" + name() + "': '" +
                           _one->name() + "' and '" + _two->name() +
                           "' are not of the same size");

  switch (_one->result().dimension())
  {
  case 0: _statsize = 0; break;
  case 1: _statsize = HistogramBackend::Underflow+1; break;
  case 2: _statsize = HistogramBackend::LowerRight+1; break;
  default:  break;
  }

  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(s.value("XNbrBins",1).toUInt(),
                              s.value("XLow",0).toFloat(),
                              s.value("XUp",0).toFloat(),
                              2,0,2, /** @note this allows to address the y bin with 0.1 and 1.1 */
                              s.value("XTitle","x-axis").toString().toStdString(),
                              "bins")));

  Log::add(Log::INFO,"Postprocessor '" + name() +
      "' makes a 1D Histogram where '" + _one->name() +
      "' defines the x bin to fill and '" +  _two->name() +
      "' defines the weight of how much to fill the x bin" +
      ". Condition on PostProcessor '" + _condition->name() + "'");
}

void pp67::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_one->result(evt.id())));
  const HistogramFloatBase &two
      (dynamic_cast<const HistogramFloatBase&>(_two->result(evt.id())));
  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  QReadLocker lock1(&one.lock);
  QReadLocker lock2(&two.lock);

  const HistogramFloatBase::storage_t &xmem(one.memory());
  HistogramFloatBase::storage_t::const_iterator xx(xmem.begin());
  HistogramFloatBase::storage_t::const_iterator xEnd(xmem.end()-_statsize);

  const HistogramFloatBase::storage_t &ymem(two.memory());
  HistogramFloatBase::storage_t::const_iterator yy(ymem.begin());

  result.clear();
  for (;xx != xEnd; ++xx, ++yy)
  {
    result.fill(*xx,0.1,*yy);
    result.fill(*xx,1.1);
  }
}















// *** postprocessor 68 histograms 0D and 1d Histogram to 2D histogram ***

pp68::pp68(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp68::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _one = setupDependency("HistOne");
  _two = setupDependency("HistTwo");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  if (_one->result().dimension() != 1 || _two->result().dimension() != 0)
    throw runtime_error("pp68::loadSettings() '" + name() + "': Either '" +
                        _one->name() + "' is not 1D or '" + _two->name() +
                        "' is not a 0D Hist");
  const Histogram1DFloat &one(dynamic_cast<const Histogram1DFloat&>(_one->result()));
  const AxisProperty &xaxis(one.axis()[HistogramBackend::xAxis]);
  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(xaxis.nbrBins(),
                              xaxis.lowerLimit(),
                              xaxis.upperLimit(),
                              s.value("YNbrBins",1).toUInt(),
                              s.value("YLow",0).toFloat(),
                              s.value("YUp",0).toFloat(),
                              xaxis.title(),
                              s.value("YTitle","y-axis").toString().toStdString())));
  Log::add(Log::INFO,"Postprocessor '" + name() +
           "' makes a 2D Histogram where '" + _one->name() +
           "' defines the x axis to fill and '" + _two->name() +
           "' defines the y axis bin. Condition on PostProcessor '" +
           _condition->name() + "'");
}

void pp68::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram1DFloat &one
      (dynamic_cast<const Histogram1DFloat&>(_one->result(evt.id())));
  const Histogram0DFloat &two
      (dynamic_cast<const Histogram0DFloat&>(_two->result(evt.id())));
  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  QReadLocker lock1(&one.lock);
  QReadLocker lock2(&two.lock);

  result.clear();
  try
  {
    size_t bin(result.axis()[HistogramBackend::yAxis].bin(two.getValue()));
    HistogramFloatBase::storage_t::iterator memorypointer
        (result.memory().begin() + bin*result.axis()[HistogramBackend::xAxis].nbrBins());
    copy(one.memory().begin(),one.memory().end()-2,memorypointer);
  }
  catch (const out_of_range& error)
  {

  }
}









// *** postprocessor 69 histograms 2 0D values to 1D scatter plot ***

pp69::pp69(const name_t &name)
  : AccumulatingPostProcessor(name)
{
  loadSettings(0);
}

void pp69::loadSettings(size_t)
{
  setupGeneral();
  _one = setupDependency("HistOne");
  _two = setupDependency("HistTwo");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  if (_one->result().dimension() != 0 || _two->result().dimension() != 0)
    throw runtime_error("pp69::loadSettings() '" + name() + "': Either '" +
                        _one->name() + "' or '" + _two->name() + "' is not a 0D Hist");
  createHistList(set1DHist(name()));
  Log::add(Log::INFO,"Postprocessor '" + name() +
           "' makes a 1D Histogram where '"+  _one->name() +
           "' defines the x bin to set and '" + _two->name() +
           "' defines the y value of the x bin"
           ". Condition on PostProcessor '" + _condition->name() + "'");
}

void pp69::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram0DFloat &one
      (dynamic_cast<const Histogram0DFloat&>(_one->result(evt.id())));
  const Histogram0DFloat &two
      (dynamic_cast<const Histogram0DFloat&>(_two->result(evt.id())));
  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  QReadLocker lock1(&one.lock);
  QReadLocker lock2(&two.lock);

  Histogram1DFloat::storage_t &mem(result.memory());

  const float x(one.getValue());
  const float weight(two.getValue());
  const int nxBins(static_cast<const int>(result.axis()[HistogramBackend::xAxis].nbrBins()));
  const float xlow(result.axis()[HistogramBackend::xAxis].lowerLimit());
  const float xup(result.axis()[HistogramBackend::xAxis].upperLimit());
  const int xBin(static_cast<int>( nxBins * (x - xlow) / (xup-xlow)));

  //check whether the fill is in the right range//
  const bool xInRange = 0<=xBin && xBin<nxBins;
  // if in range fill the memory otherwise figure out whether over of underflow occured//
  if (xInRange)
    mem[xBin] = weight;
  else if (xBin >= nxBins)
    mem[nxBins+HistogramBackend::Overflow] += 1;
  else if (xBin < 0)
    mem[nxBins+HistogramBackend::Underflow] += 1;
  //increase the number of fills//
  ++(result.nbrOfFills());
}














// ***  pp 70 subsets a histogram ***

pp70::pp70(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp70::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;

  const HistogramFloatBase& hist(dynamic_cast<const HistogramFloatBase&>(_pHist->result()));
  if (hist.dimension() == 0)
    throw invalid_argument("pp70::loadSettings(): Dimension '" +
                           toString(hist.dimension()) + "' of histogram '" +
                           _pHist->name() + "'not supported");

  pair<float,float> userXRange(make_pair(s.value("XLow",0).toFloat(),
                                         s.value("XUp",1).toFloat()));
  const AxisProperty &xaxis(hist.axis()[HistogramBackend::xAxis]);
  _inputOffset=(xaxis.bin(userXRange.first));
  const size_t binXUp (xaxis.bin(userXRange.second));
  const size_t nXBins (binXUp-_inputOffset);
  const float xLow (xaxis.position(_inputOffset));
  const float xUp (xaxis.position(binXUp));
  string output("PostProcessor '" + name() +
                "' setup: returns a subset of histogram in pp '"  +  _pHist->name() +
                "' which has dimension '" + toString(hist.dimension()) +
                "'. Subset is xLow:" + toString(xLow) + "(" + toString(_inputOffset) +
                "), xUp:" + toString(xUp) + "(" + toString(binXUp) +
                "), xNbrBins:" + toString(nXBins));
  if (1 == hist.dimension())
  {
    createHistList(tr1::shared_ptr<Histogram1DFloat>
                   (new Histogram1DFloat(nXBins,xLow,xUp)));
  }
  else if (2 == hist.dimension())
  {
    pair<float,float> userYRange(make_pair(s.value("YLow",0).toFloat(),
                                           s.value("YUp",1).toFloat()));
    const AxisProperty &yaxis (hist.axis()[HistogramBackend::yAxis]);
    const size_t binYLow(yaxis.bin(userYRange.first));
    const size_t binYUp (yaxis.bin(userYRange.second));
    const size_t nYBins = (binYUp - binYLow);
    const float yLow (yaxis.position(binYLow));
    const float yUp (yaxis.position(binYUp));
    _inputOffset = static_cast<size_t>(binYLow*xaxis.nbrBins()+xLow +0.1);
    createHistList(
          tr1::shared_ptr<Histogram2DFloat>
          (new Histogram2DFloat(nXBins,xLow,xUp,
                                nYBins,yLow,yUp)));
    output += ", yLow:"+ toString(yLow) + "(" + toString(binYLow) +
        "), yUp:" + toString(yUp) + "(" + toString(binYLow) +
        "), yNbrBins:"+ toString(nYBins) + ", linearized offset is now:" +
        toString(_inputOffset);
  }

  output += ". Condition on postprocessor '" + _condition->name() + "'";
  Log::add(Log::INFO,output);
}

void pp70::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase& input
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));
  HistogramFloatBase &result(dynamic_cast<HistogramFloatBase&>(res));

  QReadLocker lock(&input.lock);

  const HistogramFloatBase::storage_t &imem (input.memory());
  HistogramFloatBase::storage_t::const_iterator iit(imem.begin()+_inputOffset);
  HistogramFloatBase::storage_t &rmem(result.memory());
  HistogramFloatBase::storage_t::iterator rit(rmem.begin());
  const size_t resultNbrXBins(result.axis()[HistogramBackend::xAxis].nbrBins());
  const size_t resultNbrYBins = (result.dimension() == 2) ?
                                 result.axis()[HistogramBackend::yAxis].nbrBins() :
                                 1;
  const size_t inputNbrXBins(input.axis()[HistogramBackend::xAxis].nbrBins());
  for (size_t yBins=0;yBins < resultNbrYBins; ++yBins)
  {
    copy(iit,iit+resultNbrXBins,rit);
    advance(iit,inputNbrXBins);
    advance(rit,resultNbrXBins);
  }
  result.nbrOfFills()=1;
}







// ***  pp 71 returns a user specific value of a Histogram ***

pp71::pp71(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp71::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  string functype(s.value("RetrieveType","max").toString().toStdString());
  if (functype == "max")
    _func = &max_element<HistogramFloatBase::storage_t::const_iterator>;
  else if (functype == "min")
    _func = &min_element<HistogramFloatBase::storage_t::const_iterator>;
  else
    throw invalid_argument("pp71::loadSettings(): RetrieveType '" + functype +
                           "' unknown.");

  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' returns the value in '" + _pHist->name() +
           "' that is retrieved by using function type '" + functype +
           "' .Condition on postprocessor '" + _condition->name() + "'");
}

void pp71::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  QReadLocker lock(&one.lock);

  result.fill(*(_func(one.memory().begin(), one.memory().end())));
  result.nbrOfFills()=1;
}








// ***  pp 75 clears a histogram ***

pp75::pp75(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp75::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _hist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _hist))
    return;
  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' clears the histogram in pp '" + _hist->name() +
           "'. Condition is "+ _condition->name());
}

const HistogramBackend& pp75::result(const CASSEvent::id_t)
{
  throw logic_error("pp75::result: '"+name()+"' should never be called");
}

void pp75::processEvent(const CASSEvent& evt)
{
  if (_condition->result(evt.id()).isTrue())
    const_cast<HistogramBackend&>(_hist->result(evt.id())).clear();
}






// ***  pp 76 quit program

pp76::pp76(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp76::loadSettings(size_t)
{
  setupGeneral();
  if (!setupCondition())
    return;
  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' will quit CASS.. Condition is "+ _condition->name());
}

const HistogramBackend& pp76::result(const CASSEvent::id_t)
{
  throw logic_error("pp75::result: '"+name()+"' should never be called");
}

void pp76::processEvent(const CASSEvent& evt)
{
  if (_condition->result(evt.id()).isTrue())
    InputBase::reference().end();
}











// ***  pp 77 checks ids

pp77::pp77(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp77::loadSettings(size_t)
{
  setupGeneral();
  if (!setupCondition())
    return;
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _list.clear();
  string filename(s.value("List","").toString().toStdString());
  ifstream file(filename.c_str(),ios::in);
  if (!file.is_open())
    throw invalid_argument("pp77::loadSettings(): list file '" + filename +
                           "' could not be opened.");
  uint64_t id;
  while (file.good())
  {
    file >> id;
    _list.push_back(id);
  }
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));

  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' will check whether eventid in file '" + filename +
           "'. Condition is "+ _condition->name());
}

void pp77::process(const CASSEvent& evt, HistogramBackend &res)
{
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  result = (find(_list.begin(),_list.end(),evt.id()) != _list.end());
  result.nbrOfFills()=1;
}









// ***  pp 78 counter ***

pp78::pp78(const name_t &name)
  : AccumulatingPostProcessor(name)
{
  loadSettings(0);
}

void pp78::loadSettings(size_t)
{
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' counts how many times its process is called. '"  +
           "'. Condition is '"+ _condition->name() + "'");
}

void pp78::process(const CASSEvent&, HistogramBackend &res)
{
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));
  ++(result.memory()[0]);
  result.nbrOfFills()=1;
}













// ***  pp 81 returns the highest bin of a 1D Histogram ***

pp81::pp81(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp81::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  if (_pHist->result().dimension() != 1)
    throw invalid_argument("pp81::loadSettings() '" + name() + "': '" +
                           _pHist->name() + "' is not a 1D hist");
  string functype(s.value("RetrieveType","max").toString().toStdString());
  if (functype == "max")
    _func = &max_element<HistogramFloatBase::storage_t::const_iterator>;
  else if (functype == "min")
    _func = &min_element<HistogramFloatBase::storage_t::const_iterator>;
  else
    throw invalid_argument("pp81::loadSettings(): RetrieveType '" + functype +
                           "' unknown.");

  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' returns the maximum bin in '" + _pHist->name() +
           "' .Condition on postprocessor '" + _condition->name() + "'");
}

void pp81::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  QReadLocker lock(&one.lock);

  HistogramFloatBase::storage_t::const_iterator it
      (_func(one.memory().begin(), one.memory().end()));
  size_t bin(distance(one.memory().begin(),it));
  result.fill(one.axis()[HistogramBackend::xAxis].position(bin));
  result.nbrOfFills()=1;
}
















// ***  pp 82 returns statistics value of all bins in a Histogram ***

pp82::pp82(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp82::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;

  string functype(s.value("Statistics","sum").toString().toStdString());
  if (functype == "sum")
    _val = &stat_t::sum;
  else if (functype == "mean")
    _val = &stat_t::mean;
  else if (functype == "stdv")
    _val = &stat_t::stdv;
  else if (functype == "variance")
    _val = &stat_t::variance;
  else
    throw invalid_argument("pp71::loadSettings(): RetrieveType '" + functype +
                           "' unknown.");

  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));

  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' returns the mean of all bins in '" + _pHist->name() +
           "' .Condition on postprocessor '" + _condition->name() + "'");
}

void pp82::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  QReadLocker lock(&one.lock);

  stat_t stat;
  stat.addDistribution(one.memory().begin(),one.memory().end());
  result.fill(_val(stat));
  result.nbrOfFills() = 1;
}


















// ***  pp 85 return full width at half maximum in given range of 1D histgoram ***

pp85::pp85(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp85::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  pair<float,float> userXRange(make_pair(s.value("XLow",0).toFloat(),
                                         s.value("XUp",1).toFloat()));
  _fraction = s.value("Fraction",0.5).toFloat();

  const HistogramBackend &hist(_pHist->result());
  if (hist.dimension() != 1)
    throw invalid_argument("pp85::loadSettings()'" + name() +
                           "': Error the histogram we depend on '" + _pHist->name() +
                           "' is not a 1D Histogram.");
  const AxisProperty &xaxis(hist.axis()[HistogramBackend::xAxis]);
  _xRange = make_pair(xaxis.bin(userXRange.first),
                      xaxis.bin(userXRange.second));

  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' returns the full width at half maximum in '" + _pHist->name() +
           "' of the range from xlow '" + toString(userXRange.first) +
           "' to xup '" + toString(userXRange.second) +
           "' .Condition on postprocessor '" + _condition->name() + "'");
}

void pp85::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram1DFloat& one
      (dynamic_cast<const Histogram1DFloat&>(_pHist->result(evt.id())));
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  QReadLocker lock(&one.lock);

  HistogramFloatBase::storage_t::const_iterator xRangeBegin
      (one.memory().begin()+_xRange.first);
  HistogramFloatBase::storage_t::const_iterator xRangeEnd
      (one.memory().begin()+_xRange.second);
  HistogramFloatBase::storage_t::const_iterator maxElementIt
      (std::max_element(xRangeBegin, xRangeEnd));
  HistogramFloatBase::storage_t::const_iterator minElementIt
      (std::min_element(xRangeBegin, xRangeEnd));
  const float fracMax((*maxElementIt-*minElementIt) * _fraction + *minElementIt);

  HistogramFloatBase::storage_t::const_iterator leftSide;
  HistogramFloatBase::storage_t::const_iterator rightSide;
  for(HistogramFloatBase::storage_t::const_iterator iVal(maxElementIt);
      (iVal != xRangeBegin) && (*iVal > fracMax);
      --iVal)
  {
    leftSide = iVal;
  }
  for(HistogramFloatBase::storage_t::const_iterator iVal(maxElementIt);
      (iVal != xRangeEnd) && (*iVal > fracMax);
      ++iVal)
  {
    rightSide = iVal;
  }
  const float lowerdist (one.axis()[HistogramBackend::xAxis].hist2user(distance(leftSide,rightSide)));
  const float upperdist (one.axis()[HistogramBackend::xAxis].hist2user(distance(leftSide-1,rightSide+1)));
  const float fwfm((upperdist+lowerdist)*0.5);

  result.fill(fwfm);
  result.nbrOfFills()=1;
}






















// ***  pp 86 return x position of a step in 1D histgoram ***

pp86::pp86(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp86::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  pair<float,float> userXRangeStep(make_pair(s.value("XLow",0).toFloat(),
                                             s.value("XUp",1).toFloat()));
  pair<float,float> userXRangeBaseline(make_pair(s.value("BaselineLow",0).toFloat(),
                                                 s.value("BaselineUp",1).toFloat()));
  _userFraction = s.value("Fraction",0.5).toFloat();
  const HistogramBackend &hist(_pHist->result());
  if (hist.dimension() != 1)
    throw invalid_argument("pp86::setupParameters()'" + name() +
                           "': Error the histogram we depend on '" + _pHist->name() +
                           "' is not a 1D Histogram.");
  const AxisProperty &xaxis(hist.axis()[HistogramBackend::xAxis]);
  _xRangeStep = make_pair(xaxis.bin(userXRangeStep.first),
                          xaxis.bin(userXRangeStep.second));
  _xRangeBaseline = make_pair(xaxis.bin(userXRangeBaseline.first),
                              xaxis.bin(userXRangeBaseline.second));
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  Log::add(Log::INFO, "PostProcessor '" + name() +
           "' returns the postion of the step in '" + _pHist->name() +
           "' in the range from xlow '" + toString(userXRangeStep.first) +
           "' to xup '" + toString(userXRangeStep.second) +
           "'. Where the baseline is defined in range '" + toString(userXRangeBaseline.first) +
           "' to '" + toString(userXRangeBaseline.second) + "'. The Fraction is '" +
           toString(_userFraction) + "' .Condition on postprocessor '" +
           _condition->name() + "'");
}

void pp86::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram1DFloat& one
      (dynamic_cast<const Histogram1DFloat&>(_pHist->result(evt.id())));
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  QReadLocker lock(&one.lock);

  HistogramFloatBase::storage_t::const_iterator baselineBegin
      (one.memory().begin()+_xRangeBaseline.first);
  HistogramFloatBase::storage_t::const_iterator baselineEnd
      (one.memory().begin()+_xRangeBaseline.second);
  const float baseline(accumulate(baselineBegin,baselineEnd,0.f) /
                       static_cast<float>(distance(baselineBegin,baselineEnd)));

  HistogramFloatBase::storage_t::const_iterator stepRangeBegin
      (one.memory().begin()+_xRangeStep.first);
  HistogramFloatBase::storage_t::const_iterator stepRangeEnd
      (one.memory().begin()+_xRangeStep.second);

  HistogramFloatBase::storage_t::const_iterator maxElementIt
      (std::max_element(stepRangeBegin, stepRangeEnd));
  const float halfMax((*maxElementIt+baseline) * _userFraction);

  HistogramFloatBase::storage_t::const_iterator stepIt(stepRangeBegin+1);
  for ( ; stepIt != maxElementIt ; stepIt++ )
    if ( *(stepIt-1) <= halfMax && halfMax < *stepIt )
      break;
  const size_t steppos(distance(one.memory().begin(),stepIt));

  result.fill(steppos);
  result.nbrOfFills()=1;
}
















// ***  pp 87 return center of mass in range of 1D histgoram ***

pp87::pp87(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp87::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  pair<float,float> userXRange(make_pair(s.value("XLow",0).toFloat(),
                                         s.value("XUp",1).toFloat()));
  const HistogramBackend &hist(_pHist->result());
  if (hist.dimension() != 1)
    throw invalid_argument("pp87::setupParameters()'" + name() +
                           "': Error the histogram we depend on '" + _pHist->name() +
                           "' is not a 1D Histogram.");
  const AxisProperty &xaxis(hist.axis()[HistogramBackend::xAxis]);
  _xRange = make_pair(xaxis.bin(userXRange.first),
                      xaxis.bin(userXRange.second));

  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  Log::add(Log::INFO, "PostProcessor '" + name() +
           "' returns the center of mass in '" + _pHist->name() +
           "' in the range from xlow '" + toString(userXRange.first) +
           "' to xup '" + toString(userXRange.second) +
           "' .Condition on postprocessor '"+_condition->name()+"'");
}

void pp87::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram1DFloat& hist
      (dynamic_cast<const Histogram1DFloat&>(_pHist->result(evt.id())));
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  QReadLocker lock(&hist.lock);

  const AxisProperty &xAxis(hist.axis()[HistogramBackend::xAxis]);
  const HistogramFloatBase::storage_t &data(hist.memory());

  float integral(0);
  float weight(0);
  for (size_t i(_xRange.first); i < _xRange.second; ++i)
  {
    integral += (data[i]);
    const float pos(xAxis.position(i));
    weight += (data[i]*pos);
  }
  const float com(weight/integral);

  result.fill(com);
  result.nbrOfFills()=1;
}







// ***  pp 88 returns an axis parameter ***

pp88::pp88(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp88::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));

  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;

  QString axisparam(s.value("AxisParameter","XNbrBins").toString());
  if (axisparam == "XNbrBins")
  {
    _axisId = HistogramBackend::xAxis;
    _func = &AxisProperty::size;
  }
  else if (axisparam == "XLow")
  {
    _axisId = HistogramBackend::xAxis;
    _func = &AxisProperty::lowerLimit;
  }
  else if (axisparam == "XUp")
  {
    _axisId = HistogramBackend::xAxis;
    _func = &AxisProperty::upperLimit;
  }
  else if (axisparam == "YNbrBins")
  {
    _axisId = HistogramBackend::yAxis;
    _func = &AxisProperty::size;
  }
  else if (axisparam == "YLow")
  {
    _axisId = HistogramBackend::yAxis;
    _func = &AxisProperty::lowerLimit;
  }
  else if (axisparam == "YUp")
  {
    _axisId = HistogramBackend::yAxis;
    _func = &AxisProperty::upperLimit;
  }
  else
    throw invalid_argument("pp88 '" + name() + "' AxisParameter '" +
                           axisparam.toStdString() + "' is unknown.");

  if (_pHist->result().dimension() == 0)
    throw invalid_argument("pp88 '" + name() + "' histogram '" + _pHist->name() +
                           "' has dimension 0, which has no axis properties.");

  if ((axisparam == "YNbrBins" || axisparam == "YLow" || axisparam == "YUp")
      && _pHist->result().dimension() == 1)
    throw invalid_argument("pp88 '" + name() + "' histogram '" + _pHist->name() +
                           "' has dimension 1, which is incompatible with property '" +
                           axisparam.toStdString() + "'");

  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));
  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' returns axis parameter'"+ axisparam.toStdString() +
           "' of histogram in pp '" + _pHist->name() +
           "'. Condition on PostProcessor '" + _condition->name() + "'");
}

void pp88::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase& hist
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  QReadLocker lock(&hist.lock);

  result = _func(hist.axis()[_axisId]);
  result.nbrOfFills()=1;
}








// ***  pp 89 high/low pass filter ***

pp89::pp89(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp89::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));

  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;

  const float RC(1.f/(s.value("Cutoff",100.f).toFloat() * 2 * 3.1415));
  const float dt(1.f/s.value("SampleRate",100.f).toFloat());

  QString filtertype(s.value("FilterType","LowPass").toString());
  if (filtertype == "LowPass")
  {
    //  float RC = 1.0/(CUTOFF*2*3.14);
    //  float dt = 1.0/SAMPLE_RATE;
    //  float alpha = dt/(RC+dt);
    _alpha = dt/(RC+dt);
    _func = bind(&pp89::lowPass,this,_1,_2);
  }
  else if (filtertype == "HighPass")
  {
    //  float RC = 1.0/(CUTOFF*2*3.14);
    //  float dt = 1.0/SAMPLE_RATE;
    //  float alpha = RC/(RC + dt);
    _alpha = RC/(RC+dt);
    _func = bind(&pp89::highPass,this,_1,_2);
  }
  else
    throw invalid_argument("pp89 '" + name() + "' FilterType '" +
                           filtertype.toStdString() + "' is unknown.");

  if (_pHist->result().dimension() != 1)
    throw invalid_argument("pp89 '" + name() + "' histogram '" + _pHist->name() +
                           "' is not a 1D histograms");

  createHistList(_pHist->result().copy_sptr());
  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' does "+ filtertype.toStdString() +
           "' operation on histogram in pp '" + _pHist->name() +
           "'. Condition on PostProcessor '" + _condition->name() + "'");
}

void pp89::highPass(HistogramFloatBase::storage_t::const_iterator &orig,
                    HistogramFloatBase::storage_t::iterator &filtered)
{
//  float RC = 1.0/(CUTOFF*2*3.14);
//  float dt = 1.0/SAMPLE_RATE;
//  float alpha = RC/(RC + dt);
//  float filteredArray[numSamples];
//  filteredArray[0] = data.recordedSamples[0];
//  for (i = 1; i<numSamples; i++){
//    filteredArray[i] = alpha * (filteredArray[i-1] + data.recordedSamples[i] - data.recordedSamples[i-1]);
//  }
//  data.recordedSamples = filteredArray;

  *filtered = _alpha * (*(filtered-1) + *orig - *(orig-1));

}

void pp89::lowPass(HistogramFloatBase::storage_t::const_iterator &orig,
                   HistogramFloatBase::storage_t::iterator &filtered)
{
//  float RC = 1.0/(CUTOFF*2*3.14);
//  float dt = 1.0/SAMPLE_RATE;
//  float alpha = dt/(RC+dt);
//  float filteredArray[numSamples];
//  filteredArray[0] = data.recordedSamples[0];
//  for(i=1; i<numSamples; i++){
//    filteredArray[i] = filteredArray[i-1] + (alpha*(data.recordedSamples[i] - filteredArray[i-1]));
//  }
//  data.recordedSamples = filteredArray;

  *filtered = *(filtered-1) + (_alpha * (*orig - *(filtered-1)));
}

void pp89::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram1DFloat& hist
      (dynamic_cast<const Histogram1DFloat&>(_pHist->result(evt.id())));
  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  QReadLocker lock(&hist.lock);

  const HistogramFloatBase::storage_t &in(hist.memory());

  HistogramFloatBase::storage_t &out(result.memory());

  HistogramFloatBase::storage_t::const_iterator inIt(in.begin());
  HistogramFloatBase::storage_t::const_iterator inEnd(in.end());
  HistogramFloatBase::storage_t::iterator outIt(out.begin());

  *outIt++ = *inIt++;
  while (inIt != inEnd)
  {
    _func(inIt,outIt);
    ++inIt;
    ++outIt;
  }
  result.nbrOfFills()=1;
}







// ***  pp 91 return a list of minima in a histogram ***

pp91::pp91(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp91::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramBackend &hist(_pHist->result());
  if (hist.dimension() != 1)
    throw invalid_argument("pp91::loadSettings()'" + name() +
                           "': Error the histogram we depend on '" + _pHist->name() +
                           "' is not a 1D Histogram.");
  _range =  s.value("Range",10).toUInt();

  createHistList(tr1::shared_ptr<Histogram2DFloat>(new Histogram2DFloat(nbrOf)));

  Log::add(Log::INFO, "PostProcessor '" + name() +
           "' returns a list of local minima in '" + _pHist->name() +
           "'. The local minimum is the minimum within a range of +- '" +
           toString(_range) + "' .Condition on postprocessor '"+_condition->name()
           +"'");
}

void pp91::process(const CASSEvent& evt, HistogramBackend &res)
{
  const Histogram1DFloat& hist
      (dynamic_cast<const Histogram1DFloat&>(_pHist->result(evt.id())));
  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  QReadLocker lock(&hist.lock);

  const AxisProperty &xAxis(hist.axis()[HistogramBackend::xAxis]);
  const HistogramFloatBase::storage_t &data(hist.memory());

  result.clearTable();
  table_t candidate(nbrOf,0);

  for (size_t i=_range;i < data.size()-_range; ++i)
  {
    if (isnan(data[i]))
      continue;

    float curval(data[i]);
    bool isSmaller(true);
    for (size_t j=i-_range; j < i+_range; ++j)
    {
      if(isnan(data[j]) || curval > data[j])
        isSmaller = false;
    }
    if (isSmaller)
    {
      candidate[Index] = i;
      candidate[Position] = xAxis.hist2user(i);
      candidate[Value] = data[i];
      result.appendRows(candidate);
    }
  }

//  float dist(0);
//  if (candidates.size() > 1)
//    dist = xAxis.hist2user(candidates[1]) - xAxis.hist2user(candidates[0]);
//
//  result.fill(dist);
  result.nbrOfFills()=1;
}







