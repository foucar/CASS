// Copyright (C) 2010-2013 Lutz Foucar
// (C) 2010 Thomas White - Updated to (outdated) PP framework

/**
 * @file operations.cpp file contains definition of processors that will
 *                       operate on histograms of other processors
 * @author Lutz Foucar
 */

#include <QtCore/QString>
#include <iterator>
#include <algorithm>
#include <numeric>

#include "cass.h"
#include "operations.h"
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
/** provide own implementation of min_element to be able to compile
 *
 * It will search for the largest element in range but only checking real numbers
 *
 * This is just a slighly modfied copy from the cpp reference guide
 */
template <class ForwardIterator>
ForwardIterator max_element ( ForwardIterator first, ForwardIterator last )
{
  if (first==last) return last;
  ForwardIterator largest = first;
  while (!std::isfinite(*largest))
  {
    if (largest == last)
      return first;
    ++largest;
  }

  while (++first!=last)
    if (std::isfinite(*first))
      if (*largest<*first)
        largest=first;
  return largest;
}

/** provide own implementation of min_element to be able to compile
 *
 * It will search for the smallest element in range but only checking real numbers
 *
 * This is just a slightly modified copy from the cpp reference guide
 */
template <class ForwardIterator>
ForwardIterator min_element ( ForwardIterator first, ForwardIterator last )
{
  if (first==last) return last;
  ForwardIterator smallest = first;
  while (!std::isfinite(*smallest))
  {
    if (smallest == last)
      return first;
    ++smallest;
  }

  while (++first!=last)
    if (std::isfinite(*first))
      if (*first<*smallest)
        smallest=first;
  return smallest;
}
}//end namespace cass


// ************ Operation on two results *************

pp1::pp1(const name_t & name)
  : Processor(name)
{
  loadSettings(0);
}

void pp1::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _one = setupDependency("InputOne");
  _two = setupDependency("InputTwo");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;

  const result_t &one(_one->result());
  const result_t &two(_two->result());
  if (one.shape() != two.shape())
    throw invalid_argument("pp1::loadSettings() '"+name()+
                           "': HistOne '" + _one->name() +
                           "' with dimension '" + toString(one.dim()) +
                           "' and memory size '" + toString(one.size()) +
                           "' differs from HistTwo '" + _one->name() +
                           "' with has dimension '" + toString(two.dim()) +
                           "' and memory size '" + toString(two.size()));

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

  createHistList(_one->result().clone());

  Log::add(Log::INFO,"Processor '" + name() +
           "' will do operation '"+  operation + "'with '" + _one->name() +
           "' as first and '" + _two->name() + "' as second argument" +
           "'. Condition is '" + _condition->name() + "'");
}

void pp1::process(const CASSEvent& evt, result_t &result)
{
  const result_t &one(_one->result(evt.id()));
  QReadLocker lock1(&(one.lock));
  const result_t &two(_two->result(evt.id()));
  QReadLocker lock2(&(two.lock));

  transform(one.begin(),one.begin()+one.datasize(), two.begin(), result.begin(),
            _op);
}








// ************ processor 2: operation with const value

pp2::pp2(const name_t &name)
 : Processor(name)
{
 loadSettings(0);
}

void pp2::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _hist = setupDependency("InputName");
  bool ret (setupCondition());
  QString valuekey("Value");
  QString valueparam(s.value(valuekey,1).toString());
  bool IsFloatValue(false);
  result_t::value_t val(valueparam.toFloat(&IsFloatValue));
  if (!IsFloatValue)
  {
    _valuePP = setupDependency(valuekey.toStdString());
    ret = _valuePP && ret;
    _retrieveValue = bind(&pp2::valueFromPP,this,_1);
  }
  else
  {
    _retrieveValue = bind(&pp2::valueFromConst,this,_1);
    _value = val;
  }
  if (!(_hist && ret))
    return;

  if (!IsFloatValue &&  _valuePP->result().dim() != 0)
    throw invalid_argument("pp2::loadSettings() '"+ name()+ " value '" +
                           _valuePP->name() + "' is not a 0d result");

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

  createHistList(_hist->result().clone());
  Log::add(Log::INFO,"Processor '" + name() + "' operation '" + operation +
           "' on '" + _hist->name() + "' with " +
           (!IsFloatValue ? "value in '"+_valuePP->name() : "constant '"+
                            toString(_value)) +
           "'. Condition is "+ _condition->name() + "'");
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
  const result_t &value(_valuePP->result(id));
  QReadLocker lock(&value.lock);
  return value.getValue();
}

void pp2::process(const CASSEvent& evt, result_t &result)
{
  const result_t &input(_hist->result(evt.id()));
  QReadLocker lock(&input.lock);

  /** only do the operation on the datasize to avoid doing the operation on the
   *  statistics
   */
  transform(input.begin(), input.begin()+input.datasize(), result.begin(),
            _setParamPos(_retrieveValue(evt.id())));
}










// ************ processor 4: Apply boolean NOT to 0D histogram *************

pp4::pp4(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp4::loadSettings(size_t)
{
  setupGeneral();
  _one = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(_one && ret))
    return;
  if (_one->result().dim() != 0)
    throw invalid_argument("pp4::loadSettings() '"+ name()+ " value '" +
                           _one->name() + "' is not a 0d result, but needs to be");
  createHistList(result_t::shared_pointer(new result_t()));
  Log::add(Log::INFO,"Processor '" + name() + "' will apply NOT to Processor '" +
           _one->name() + "'. Condition is '" + _condition->name() + "'");
}

void pp4::process(const CASSEvent& evt, result_t &result)
{
  const result_t &one(_one->result(evt.id()));
  QReadLocker lock(&one.lock);

  result.setValue(!one.isTrue());
}











// ********** processor 9: Check if histogram is in given range ************

pp9::pp9(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp9::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _range = make_pair(s.value("LowerLimit",0).toFloat(),
                     s.value("UpperLimit",0).toFloat());
  setupGeneral();
  _one = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(_one && ret))
    return;
  createHistList(result_t::shared_pointer(new result_t()));

  Log::add(Log::INFO,"Processor '" + name()
           + "' will check whether hist in Processor '" + _one->name() +
           "' is between '" + toString(_range.first) + "' and '" +
           toString(_range.second) + "' Both values are exclusive." +
           " Condition is '" + _condition->name() + "'");
}

void pp9::process(const CASSEvent& evt, result_t &result)
{
  const result_t &one(_one->result(evt.id()));
  QReadLocker lock(&one.lock);

  /** only accumulate over the range that contains actual data */
  const float value (accumulate(one.begin(), one.begin()+one.datasize(), 0.f));

  result.setValue(_range.first < value &&  value < _range.second);
}






// ********** processor 12: PP with constant value ************

pp12::pp12(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp12::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  string type(s.value("ValueType","0D").toString().toStdString());
  if (type == "0D")
  {
    _res = result_t::shared_pointer(new result_t());
  }
  else if (type == "1D")
  {
    _res = result_t::shared_pointer
        (new result_t
         (result_t::axe_t(s.value("XNbrBins",1).toInt(),
                          s.value("XLow",0).toFloat(),
                          s.value("XUp",1).toFloat(),
                          s.value("XTitle","x-axis").toString().toStdString()
                          )));
  }
  else if (type == "2D")
  {
    _res = result_t::shared_pointer
        (new result_t
         (result_t::axe_t(s.value("XNbrBins",1).toInt(),
                          s.value("XLow",0).toFloat(),
                          s.value("XUp",1).toFloat(),
                          s.value("XTitle","x-axis").toString().toStdString()),
          result_t::axe_t(s.value("YNbrBins",1).toInt(),
                          s.value("YLow",0).toFloat(),
                          s.value("YUp",1).toFloat(),
                          s.value("YTitle","y-axis").toString().toStdString())));
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
  fill(_res->begin(), _res->end(), value);

  _hide = s.value("Hide",true).toBool();

  Log::add(Log::INFO,"Processor '" +  name() + "' has constant value of '" +
           toString(value) + "' and is of type '" + type + "'");
}









// ************ processor 13: return the result (identity operation) *****

pp13::pp13(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp13::loadSettings(size_t)
{
  setupGeneral();
  _one = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(_one && ret))
    return;
  createHistList(_one->result().clone());

  Log::add(Log::INFO,"Processor '" + name() +
           "' will return a copy of Processor '" + _one->name() +
           "'. Condition is '" + _condition->name() + "'");
}

void pp13::process(const CASSEvent& evt, result_t &result)
{
  const result_t &one(_one->result(evt.id()));
  QReadLocker lock(&one.lock);

  result.assign(one);
}









// ********** processor 15: Check if value has changed ************

pp15::pp15(const name_t &name)
  : AccumulatingProcessor(name)
{
  loadSettings(0);
}

void pp15::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _previousVal = 0;
  setupGeneral();
  _hist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(_hist && ret))
    return;
  if (_hist->result().dim() != 0 )
    throw runtime_error("pp15::loadSettings() " + name() +": Result '" +
                        _hist->name() + "' is not 0D, but needs to be");
  _difference = s.value("Difference",0.).toFloat();
  if (fabs(_difference) < std::numeric_limits<result_t::value_t>::epsilon() )
    _difference = std::numeric_limits<result_t::value_t>::epsilon();
  createHistList(result_t::shared_pointer(new result_t()));

  Log::add(Log::INFO,"processor '" + name() +
           "' will check whether the difference between the current and the" +
           " previous value of '" + _hist->name() +
           "' is bigger than '"+ toString(_difference) +
           "'. It will use condition '" + _condition->name() +"'");
}

void pp15::process(const CASSEvent& evt, result_t &result)
{
  const result_t &val(_hist->result(evt.id()));
  QReadLocker lock1(&val.lock);

  const float value(val.getValue());

  result.setValue(fabs(value-_previousVal) > _difference);
  _previousVal = value;
}










// ****************** processor 40: Threshold result ********************

pp40::pp40(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp40::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _threshold = s.value("Threshold", 0.0).toFloat();
  _one = setupDependency("InputName");
  setupGeneral();
  bool ret (setupCondition());
  if (!(_one && ret))
    return;
  createHistList(_one->result().clone());
  Log::add(Log::INFO,"Processor '" + name() +
           "' will threshold Histogram in Processor '" + _one->name() +
           "' above '" + toString(_threshold) +
           "'. Condition is '" + _condition->name() + "'");
}

void pp40::process(const CASSEvent& evt, result_t &result)
{
  const result_t &one(_one->result(evt.id()));
  QReadLocker lock(&one.lock);

  /** only threshold the area containing data */
  transform(one.begin(), one.begin()+one.datasize(), result.begin(),
            bind2nd(threshold(), _threshold));
}





// ****************** processor 41: more advanced threshold histogram ********************

pp41::pp41(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp41::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _one = setupDependency("InputName");
  _threshold = setupDependency("ThresholdName");
  bool ret (setupCondition());
  if (!(_one && _threshold && ret))
    return;
  _userVal = s.value("UserVal",0.f).toFloat();
  _lowerBound = s.value("LowerBound",0.5).toFloat();
  _upperBound = s.value("UpperBound",1.5).toFloat();
  if (_one->result().shape() != _threshold->result().shape())
    throw invalid_argument("pp41:loadSettings() '" + name() +
                           "' Shape of hist to threshold '" + _one->name() + "'(" +
                           toString(_one->result().shape().first) + "x" +
                           toString(_one->result().shape().second) +
                           ")  and the threshold '" + _threshold->name() + "' (" +
                           toString(_threshold->result().shape().first) + "x" +
                           toString(_threshold->result().shape().second) + ") differ.");
  createHistList(_one->result().clone());
  Log::add(Log::INFO,"Processor '" + name() +
           "' will set bins in Processor '" + _one->name() +
           "' to '" + toString(_userVal) + "' when the corresponding bin in '"
           + _threshold->name() + "' is between '" + toString(_lowerBound) +
           "' and '" + toString(_upperBound) +
           "' where the boarders are exclusive.  Condition is '"
           + _condition->name() + "'");
}

float pp41::checkrange(float val, float checkval)
{
  return (_lowerBound < checkval && checkval < _upperBound) ? _userVal : val;
}

void pp41::process(const CASSEvent& evt, result_t &result)
{
  const result_t &input(_one->result(evt.id()));
  QReadLocker lock1(&input.lock);
  const result_t &thresh(_threshold->result(evt.id()));
  QReadLocker lock2(&thresh.lock);

  /** only check the data area of the input */
  transform(input.begin(), input.begin()+input.datasize(), thresh.begin(),
            result.begin(), bind(&pp41::checkrange,this,_1,_2));
}












// *** processors 50 projects 2d hist to 1d histo for a selected region of the axis ***

pp50::pp50(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp50::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  if (_pHist->result().dim() != 2)
    throw invalid_argument("pp50::loadSettings()'" + name() +
                           "': Result '" + _pHist->name() +
                           "' is not a 2D Result.");
  const result_t &hist(_pHist->result());

  const result_t::axe_t &xAxis(hist.axis(result_t::xAxis));
  const result_t::axe_t &yAxis(hist.axis(result_t::yAxis));
  _nX = xAxis.nBins;

  pair<float,float> userRange(make_pair(s.value("LowerBound",-1e6).toFloat(),
                                        s.value("UpperBound", 1e6).toFloat()));
  int axis(s.value("Axis",result_t::xAxis).toInt());

  switch(axis)
  {
  case (result_t::xAxis):
    _xRange = make_pair(0,xAxis.nBins);
    if (userRange.first < yAxis.low)
      throw invalid_argument("pp50::loadSettings: '" + name() +
                             "': LowerBound '" + toString(userRange.first) +
                             "' is smaller than the lowest possible value '" +
                             toString(yAxis.low) + "'");
    if (userRange.first > yAxis.up)
      throw invalid_argument("pp50::loadSettings: '" + name() +
                             "': LowerBound '" + toString(userRange.first) +
                             "' is higher than the highest possible value '" +
                             toString(yAxis.up) + "'");
    if (userRange.second < yAxis.low)
      throw invalid_argument("pp50::loadSettings: '" + name() +
                             "': UpperBound '" + toString(userRange.second) +
                             "' is smaller than the lowest possible value '" +
                             toString(yAxis.low) + "'");
    if (userRange.second > yAxis.up)
      throw invalid_argument("pp50::loadSettings: '" + name() +
                             "': UpperBound '" + toString(userRange.second) +
                             "' is higher than the highest possible value '" +
                             toString(yAxis.up) + "'");
    _yRange.first  = yAxis.bin(userRange.first);
    _yRange.second = yAxis.bin(userRange.second);
    _project = bind(&pp50::projectToX,this,_1,_2);
    createHistList(result_t::shared_pointer(new result_t(xAxis)));
    break;

  case (result_t::yAxis):
    if (userRange.first < xAxis.low)
      throw invalid_argument("pp50::loadSettings: '" + name() +
                             "': LowerBound '" + toString(userRange.first) +
                             "' is smaller than the lowest possible value '" +
                             toString(xAxis.low) + "'");
    if (userRange.first > xAxis.up)
      throw invalid_argument("pp50::loadSettings: '" + name() +
                             "': LowerBound '" + toString(userRange.first) +
                             "' is higher than the highest possible value '" +
                             toString(xAxis.up) + "'");
    if (userRange.second < xAxis.low)
      throw invalid_argument("pp50::loadSettings: '" + name() +
                             "': UpperBound '" + toString(userRange.second) +
                             "' is smaller than the lowest possible value '" +
                             toString(xAxis.low) + "'");
    if (userRange.second > xAxis.up)
      throw invalid_argument("pp50::loadSettings: '" + name() +
                             "': UpperBound '" + toString(userRange.second) +
                             "' is higher than the highest possible value '" +
                             toString(xAxis.up) + "'");
    _xRange.first  = xAxis.bin(userRange.first);
    _xRange.second = xAxis.bin(userRange.second);
    _yRange = make_pair(0,yAxis.nBins);
    _project = bind(&pp50::projectToY,this,_1,_2);
    createHistList(result_t::shared_pointer(new result_t(yAxis)));
    break;

  default:
    throw invalid_argument("pp50::loadSettings() '" + name() +
                           "': requested axis '" + toString(axis) +
                           "' does not exist.");
    break;
  }

  Log::add(Log::INFO,"Processor '" + name() +
           "' will project histogram of Processor '" + _pHist->name() +
           "' from '" + toString(userRange.first) + "' to '" +
           toString(userRange.second) + "' to axis '" + toString(axis) +
           "' The area in histogram coordinates lower left '" +
           toString(_xRange.first) + "x" + toString(_yRange.first) +
           "'; upper right '" + toString(_xRange.second) + "x" +
           toString(_yRange.second) +"'. Condition is '" + _condition->name() +
           "'");
}

void pp50::projectToX(result_t::const_iterator src, result_t::iterator dest)
{
  for(size_t y(_yRange.first); y<_yRange.second; ++y)
    for(size_t x(_xRange.first); x<_xRange.second; ++x)
      dest[x] += src[y*_nX + x];
}

void pp50::projectToY(result_t::const_iterator src, result_t::iterator dest)
{
  for(size_t y(_yRange.first); y<_yRange.second; ++y)
    for(size_t x(_xRange.first); x<_xRange.second; ++x)
      dest[y] += src[y*_nX + x];
}

void pp50::process(const CASSEvent& evt, result_t &result)
{
  const result_t &one(_pHist->result(evt.id()));
  QReadLocker lock(&one.lock);

  _project(one.begin(),result.begin());
}













// *** processors 51 calcs integral over a region in 1d histo ***

pp51::pp51(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp51::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  pair<float,float> userarea(make_pair(s.value("LowerBound",-1e6).toFloat(),
                                       s.value("UpperBound", 1e6).toFloat()));
  setupGeneral();
  _pHist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  if (_pHist->result().dim() != 1)
    throw invalid_argument("pp51::loadSettings: '" + name() +
                           "': hist '" + _pHist->name() + "' is not a 1d hist.");
  const result_t &one(_pHist->result());
  const result_t::axe_t &xaxis(one.axis(result_t::xAxis));
  if (userarea.first < xaxis.low)
    throw invalid_argument("pp51::loadSettings: '" + name() +
                           "': LowerBound '" + toString(userarea.first) +
                           "' is smaller than the lowest possible value '" +
                           toString(xaxis.low) + "'");
  if (userarea.first > xaxis.up)
    throw invalid_argument("pp51::loadSettings: '" + name() +
                           "': LowerBound '" + toString(userarea.first) +
                           "' is higher than the highest possible value '" +
                           toString(xaxis.up) + "'");
  if (userarea.second < xaxis.low)
    throw invalid_argument("pp51::loadSettings: '" + name() +
                           "': UpperBound '" + toString(userarea.second) +
                           "' is smaller than the lowest possible value '" +
                           toString(xaxis.low) + "'");
  if (userarea.second > xaxis.up)
    throw invalid_argument("pp51::loadSettings: '" + name() +
                           "': UpperBound '" + toString(userarea.second) +
                           "' is higher than the highest possible value '" +
                           toString(xaxis.up) + "'");

  _area.first  = xaxis.bin(userarea.first);
  _area.second = xaxis.bin(userarea.second);

  createHistList(result_t::shared_pointer(new result_t()));

  Log::add(Log::INFO, "Processor '" + name() +
      "' will create integral of 1d histogram in Processor '" + _pHist->name() +
      "' from '" + toString(userarea.first) +  "(" + toString(_area.first) +
      ")' to '" + toString(userarea.second) + "(" + toString(_area.second) +
      ")'. Condition is '" + _condition->name() + "'");
}

void pp51::process(const CASSEvent& evt, result_t &result)
{
  const result_t &one(_pHist->result(evt.id()));
  QReadLocker lock(&one.lock);

  result_t::const_iterator begin(one.begin()+_area.first);
  result_t::const_iterator end(one.begin()+_area.second);
  result.setValue(accumulate(begin, end, 0.f));
}









// *** processor 56 stores previous version of another histogram ***

pp56::pp56(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp56::loadSettings(size_t)
{
  setupGeneral();
  _pHist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const result_t &one(_pHist->result());
  createHistList(one.clone());
  Log::add(Log::INFO,"processor '" + name() +
           "' stores the previous histogram from Processor '" + _pHist->name() +
           "'. Condition on processor '" + _condition->name() +"'");
}

void pp56::process(const CASSEvent& evt, result_t &result)
{
  /** get reference to the input */
  const result_t& one(_pHist->result(evt.id()));

  /** lock this and the input */
  QReadLocker rlock(&one.lock);
  QMutexLocker lock(&_mutex);

  /** copy the old to the result */
  result.assign(_previous);

  /** copy the current to the store */
  _previous.assign(one);
}


















// *** processors 57 weighted projects 2d hist to 1d histo for a selected region of the axis ***

pp57::pp57(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp57::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  if (_pHist->result().dim() != 2)
    throw invalid_argument("pp57::setupParameters()'" + name() +
                           "': Error the histogram we depend on '" + _pHist->name() +
                           "' is not a 2D Histogram.");
  const result_t &hist(_pHist->result());

  pair<float,float> userRange(make_pair(s.value("LowerBound",-1e6).toFloat(),
                                        s.value("UpperBound", 1e6).toFloat()));
  int projection_axis(s.value("Axis",result_t::xAxis).toUInt());
  _excludeVal = s.value("ExclusionValue",0).toFloat();

  const result_t::axe_t &xAxis(hist.axis(result_t::xAxis));
  const result_t::axe_t &yAxis(hist.axis(result_t::yAxis));
  _nX = xAxis.nBins;

  switch(projection_axis)
  {
  case (result_t::xAxis):
    _Xrange = make_pair(0,xAxis.nBins);
    if (userRange.first < yAxis.low)
      throw invalid_argument("pp57::loadSettings: '" + name() +
                             "': LowerBound '" + toString(userRange.first) +
                             "' is smaller than the lowest possible value '" +
                             toString(yAxis.low) + "'");
    if (userRange.first > yAxis.up)
      throw invalid_argument("pp57::loadSettings: '" + name() +
                             "': LowerBound '" + toString(userRange.first) +
                             "' is higher than the highest possible value '" +
                             toString(yAxis.up) + "'");
    if (userRange.second < yAxis.low)
      throw invalid_argument("pp57::loadSettings: '" + name() +
                             "': UpperBound '" + toString(userRange.second) +
                             "' is smaller than the lowest possible value '" +
                             toString(yAxis.low) + "'");
    if (userRange.second > yAxis.up)
      throw invalid_argument("pp57::loadSettings: '" + name() +
                             "': UpperBound '" + toString(userRange.second) +
                             "' is higher than the highest possible value '" +
                             toString(yAxis.up) + "'");
    _Yrange.first  = yAxis.bin(userRange.first);
    _Yrange.second = yAxis.bin(userRange.second);
    _project = bind(&pp57::projectToX,this,_1,_2,_3);
    createHistList(result_t::shared_pointer(new result_t(xAxis)));
    break;

  case (result_t::yAxis):
    if (userRange.first < xAxis.low)
      throw invalid_argument("pp57::loadSettings: '" + name() +
                             "': LowerBound '" + toString(userRange.first) +
                             "' is smaller than the lowest possible value '" +
                             toString(xAxis.low) + "'");
    if (userRange.first > xAxis.up)
      throw invalid_argument("pp57::loadSettings: '" + name() +
                             "': LowerBound '" + toString(userRange.first) +
                             "' is higher than the highest possible value '" +
                             toString(xAxis.up) + "'");
    if (userRange.second < xAxis.low)
      throw invalid_argument("pp57::loadSettings: '" + name() +
                             "': UpperBound '" + toString(userRange.second) +
                             "' is smaller than the lowest possible value '" +
                             toString(xAxis.low) + "'");
    if (userRange.second > xAxis.up)
      throw invalid_argument("pp57::loadSettings: '" + name() +
                             "': UpperBound '" + toString(userRange.second) +
                             "' is higher than the highest possible value '" +
                             toString(xAxis.up) + "'");
    _Xrange.first  = xAxis.bin(userRange.first);
    _Xrange.second = xAxis.bin(userRange.second);
    _Yrange = make_pair(0,yAxis.nBins);
    _project = bind(&pp57::projectToY,this,_1,_2,_3);
    createHistList(result_t::shared_pointer(new result_t(yAxis)));
    break;

  default:
    throw invalid_argument("pp57::loadSettings() '" + name() +
                           "': requested _axis '" + toString(projection_axis) +
                           "' does not exist.");
    break;
  }
  Log::add(Log::INFO,"Processor '" + name() +
      "' will project histogram of Processor '" + _pHist->name() + "' from '" +
      toString(userRange.first) + "' to '" + toString(userRange.second) + "' on axis '" +
      toString(projection_axis) + "'. Condition is '" + _condition->name() + "'");
}

void pp57::projectToX(result_t::const_iterator src, result_t::iterator result,
                      result_t::iterator norm)
{
  for(size_t y(_Yrange.first); y<_Yrange.second; ++y)
    for(size_t x(_Xrange.first); x<_Xrange.second; ++x)
    {
      const float pixval(src[y*_nX + x]);
      if (!fuzzycompare(pixval,_excludeVal))
      {
        result[x] += pixval;
        norm[x] += 1;
      }
    }
}

void pp57::projectToY(result_t::const_iterator src, result_t::iterator result,
                      result_t::iterator norm)
{
  for(size_t y(_Yrange.first); y<_Yrange.second; ++y)
    for(size_t x(_Xrange.first); x<_Xrange.second; ++x)
    {
      const float pixval(src[y*_nX + x]);
      if (!fuzzycompare(pixval,_excludeVal))
      {
        result[y] += pixval;
        norm[y] += 1;
      }
    }
}

void pp57::process(const CASSEvent& evt, result_t &result)
{
  const result_t &one(_pHist->result(evt.id()));
  QReadLocker lock(&one.lock);

  /** project to axis and remember how many pixels have been added at which
   *  bin. Then normalize to the number of added pixels
   */
  result_t::storage_t norm(result.size(),0);
  _project(one.begin(),result.begin(),norm.begin());
  transform(result.begin(),result.begin()+result.datasize(),
            norm.begin(),result.begin(), divides<result_t::value_t>());
}










// *** processor 60 histograms results ***

pp60::pp60(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp60::loadSettings(size_t)
{
  setupGeneral();
  _pHist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  createHistList(set1DHist(name()));
  Log::add(Log::INFO,"processor '" + name() +
      "' histograms values from Processor '" +  _pHist->name() +
      "'. Condition on Processor '" + _condition->name() + "'");
}

void pp60::process(const CASSEvent& evt, result_t &result)
{
  const result_t &input(_pHist->result(evt.id()));
  QReadLocker lock(&input.lock);

  result_t::const_iterator value(input.begin());
  result_t::const_iterator histEnd(input.begin()+input.datasize());

  for (; value != histEnd; ++value)
    result.histogram(*value);
}











// *** processor 61 averages histograms ***

pp61::pp61(const name_t &name)
  : AccumulatingProcessor(name)
{
  loadSettings(0);
}

void pp61::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  unsigned average = s.value("NbrOfAverages", 1).toUInt();
  _alpha =  average ? 2./static_cast<float>(average+1.) : 0.;
  _pHist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  QString averageType(s.value("AveragingType","Normal").toString());
  if (averageType == "Normal")
    _func = bind(&pp61::average,this,_1,_2,_3);
  else if (averageType == "Square")
    _func = bind(&pp61::squareAverage,this,_1,_2,_3);
  else
    throw invalid_argument("pp61::loadSettings() " + name() +
                           ": requested Averaging type '" +
                           averageType.toStdString() + "' unknown.");
  createHistList(_pHist->result().clone());
  Log::add(Log::INFO,"processor '" + name() +
      "' averages histograms from Processor '" +  _pHist->name() +
      "' alpha for the averaging '" + toString(_alpha) +
      "'. Condition on processor '" + _condition->name() + "'");
}

Processor::result_t::value_t pp61::average(result_t::value_t val,
                                           result_t::value_t aveOld,
                                           result_t::value_t scale)
{
  return aveOld + scale*(val - aveOld);
}

Processor::result_t::value_t pp61::squareAverage(result_t::value_t val,
                                                 result_t::value_t aveOld,
                                                 result_t::value_t scale)
{
  return aveOld + scale*(val*val - aveOld);
}

void pp61::process(const CASSEvent& evt, result_t &result)
{
  const result_t& one(_pHist->result(evt.id()));
  QReadLocker lock(&one.lock);

  ++_nbrEventsAccumulated;
  const result_t::value_t scale((1./_nbrEventsAccumulated < _alpha) ?
                                  _alpha :
                                  1./_nbrEventsAccumulated);
  transform(one.begin(), one.begin()+one.datasize(),
            result.begin(), result.begin(), bind(_func,_1,_2,scale));
}












// *** processor 62 sums up histograms ***

pp62::pp62(const name_t &name)
  : AccumulatingProcessor(name)
{
  loadSettings(0);
}

void pp62::loadSettings(size_t)
{
  setupGeneral();
  _pHist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  createHistList(_pHist->result().clone());
  Log::add(Log::INFO,"processor '" + name() +
      "' sums up results from Processor '" +  _pHist->name() +
      "'. Condition on processor '" + _condition->name() + "'");
}

void pp62::process(const CASSEvent& evt, result_t &result)
{
  const result_t& one(_pHist->result(evt.id()));
  QReadLocker lock(&one.lock);

  transform(one.begin(),one.end(),result.begin(),result.begin(),plus<float>());
  ++_nbrEventsAccumulated;
}








// *** processors 63 calculate the time average of a 0d/1d/2d hist given the number
//     of samples that are going to be used in the calculation ***

pp63::pp63(const name_t &name)
  : AccumulatingProcessor(name),
    _num_seen_evt(0),
    _when_first_evt(0),
    _first_fiducials(0)
{
  loadSettings(0);
}

void pp63::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
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
  createHistList(_pHist->result().clone());
  Log::add(Log::INFO,"Processor '" + name() +
      "' will calculate the time average of histogram of Processor '" + _pHist->name() +
      "' from now '" + toString(s.value("MinTime",0).toUInt()) + "' to '" +
      toString(s.value("MaxTime",300).toUInt()) + "' seconds '" + toString(_timerange.first) +
      "' ; '" + toString(_timerange.second) + "' each bin is equivalent to up to '" +
      toString(_nbrSamples) + "' measurements," +
      " Condition on Processor '" + _condition->name() + "'");
}

void pp63::process(const CASSEvent& evt, result_t &result)
{
  //#define debug1
#ifdef debug1
  char timeandday[40];
  struct tm * timeinfo;
#endif
  uint32_t fiducials;
  time_t now_of_event;
  const result_t& one(_pHist->result(evt.id()));

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

  transform(one.begin(),one.end(), result.begin(), result.begin(),
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
  : AccumulatingProcessor(name)
{
  loadSettings(0);
}

void pp64::loadSettings(size_t)
{
  CASSSettings s;

  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));

  _hist = setupDependency("InputName");

  bool ret (setupCondition());
  if ( !(_hist && ret) ) return;

  setupGeneral();
  _size = s.value("Size", 10000).toUInt();

  createHistList(result_t::shared_pointer(new result_t(_size)));

  Log::add(Log::INFO,"Processor '" + name() +
           "' will make a history of values of histogram in pp '" +
           _hist->name() + ", size of history '" + toString(_size) +
           "' Condition on processor '" + _condition->name() + "'");
}

void pp64::process(const CASSEvent &evt, result_t &result)
{
  const result_t &values(_hist->result(evt.id()));
  QReadLocker lock(&(values.lock));

  /** @note since the rotate algorithm is quite fast, it might be faster to do
   *        it like this. Otherwise we have to branch, which might be slower
   */
  result_t::const_iterator value(values.begin());
  result_t::const_iterator valueEnd(values.end());
  for(; value != valueEnd ;++value)
  {
    rotate(result.begin(), result.begin()+1, result.end());
    result[_size-1] = *value;
  }
}







// *** processor 65 histograms 2 0D values to 2D histogram ***

pp65::pp65(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp65::loadSettings(size_t)
{
  setupGeneral();
  _one = setupDependency("XValue");
  _two = setupDependency("YValue");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  if (_one->result().dim() != 0 || _two->result().dim() != 0)
    throw runtime_error("pp65::loadSettings() " + name() +
                        " Either HistOne or HistTwo is not a 0D Hist");
  createHistList(set2DHist(name()));
  Log::add(Log::INFO,"processor '" + name() +
      "': histograms values from Processor '" +  _one->name() +"' and '" +
      _two->name() + ". condition on Processor '" + _condition->name() + "'");
}

void pp65::process(const CASSEvent& evt, result_t &result)
{
  const result_t &one(_one->result(evt.id()));
  QReadLocker lock1(&one.lock);
  const result_t &two(_two->result(evt.id()));
  QReadLocker lock2(&two.lock);
  result.histogram(make_pair(one.getValue(),two.getValue()));
}












// *** processor 66 histograms 2 1D values to 2D histogram ***

pp66::pp66(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp66::loadSettings(size_t)
{
  setupGeneral();
  _one = setupDependency("XName");
  _two = setupDependency("YName");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  const result_t &one(_one->result());
  const result_t &two(_two->result());
  if (one.dim() != 1 || two.dim() != 1)
    throw invalid_argument("pp66::loadSettings() "+name()+": First '" +
                           _one->name() + "' with dimension '" +
                           toString(one.dim()) + "' or Second '" +
                           _two->name() + "' has dimension '" +
                           toString(two.dim()) + "' does not have dimension 1");
  createHistList
      (result_t::shared_pointer
        (new result_t(one.axis(result_t::xAxis), two.axis(result_t::xAxis))));

   Log::add(Log::INFO,"processor '" + name() +
           "' creates a two dim histogram from Processor '" + _one->name() +
           "' and '" +  _two->name() + "'. condition on Processor '" +
           _condition->name() + "'");
}

void pp66::process(const CASSEvent& evt, result_t &result)
{
  const result_t &one(_one->result(evt.id()));
  QReadLocker lock1(&one.lock);
  const result_t &two(_two->result(evt.id()));
  QReadLocker lock2(&two.lock);

  const size_t oneNBins(one.shape().first);
  const size_t twoNBins(two.shape().first);
  for (size_t j(0); j < twoNBins; ++j)
    for (size_t i(0); i < oneNBins; ++i)
      result[j*oneNBins+i] = one[i]*two[j];
}















// *** processor 67 histograms 2 0D,1D or 2D values to 1D histogram add weight ***

pp67::pp67(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp67::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _one = setupDependency("ValuesName");
  _two = setupDependency("WeightsName");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  if (_one->result().dim() != _two->result().dim() )
    throw invalid_argument("pp67::loadSettings() '" + name() + "': '" +
                           _one->name() + "' and '" + _two->name() +
                           "' are not of the same type");
  if (_one->result().shape() != _two->result().shape())
    throw invalid_argument("pp67::loadSettings() '" + name() + "': data of'" +
                           _one->name() + "' and '" + _two->name() +
                           "' are not of the same size");
  if (_one->result().size() != _two->result().size())
    throw invalid_argument("pp67::loadSettings() '" + name() + "': storage of '" +
                           _one->name() + "' and '" + _two->name() +
                           "' are not of the same size");
  createHistList
      (result_t::shared_pointer
        (new result_t
         (result_t::axe_t(s.value("XNbrBins",1).toUInt(),
                          s.value("XLow",0).toFloat(),
                          s.value("XUp",0).toFloat(),
                          s.value("XTitle","x-axis").toString().toStdString()),
          /** @note this allows to address the y bin with 0.1 and 1.1 */
          result_t::axe_t(2,0,2,"bins"))));

  Log::add(Log::INFO,"processor '" + name() +
      "' makes a 1D Histogram where '" + _one->name() +
      "' defines the x bin to fill and '" +  _two->name() +
      "' defines the weight of how much to fill the x bin" +
      ". Condition on Processor '" + _condition->name() + "'");
}

void pp67::process(const CASSEvent& evt, result_t &result)
{
  const result_t &one(_one->result(evt.id()));
  QReadLocker lock1(&one.lock);
  const result_t &two(_two->result(evt.id()));
  QReadLocker lock2(&two.lock);

  result_t::const_iterator xx(one.begin());
  result_t::const_iterator xEnd(one.begin()+one.datasize());

  result_t::const_iterator yy(two.begin());

  for (;xx != xEnd; ++xx, ++yy)
  {
    result.histogram(make_pair(*xx,0.1),*yy);
    result.histogram(make_pair(*xx,1.1));
  }
}















// *** processor 68 histograms 0D and 1d Histogram to 2D histogram ***

pp68::pp68(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp68::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _one = setupDependency("XName");
  _two = setupDependency("YName");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  if (_one->result().dim() != 1 || _two->result().dim() != 0)
    throw runtime_error("pp68::loadSettings() '" + name() + "': Either '" +
                        _one->name() + "' is not 1D or '" + _two->name() +
                        "' is not a 0D Hist");
  const result_t &one(_one->result());
  const result_t::axe_t &xaxis(one.axis(result_t::xAxis));
  createHistList
      (result_t::shared_pointer
        (new result_t
         (xaxis, result_t::axe_t( s.value("YNbrBins",1).toUInt(),
                                  s.value("YLow",0).toFloat(),
                                  s.value("YUp",0).toFloat(),
                                  s.value("YTitle","y-axis").toString().toStdString()))));
  Log::add(Log::INFO,"processor '" + name() +
           "' makes a 2D Histogram where '" + _one->name() +
           "' defines the x axis to fill and '" + _two->name() +
           "' defines the y axis bin. Condition on Processor '" +
           _condition->name() + "'");
}

void pp68::process(const CASSEvent& evt, result_t &result)
{
  const result_t &one(_one->result(evt.id()));
  QReadLocker lock1(&one.lock);
  const result_t &two(_two->result(evt.id()));
  QReadLocker lock2(&two.lock);

  const result_t::shape_t::first_type nxbins(result.shape().first);
  const result_t::axe_t yaxis(result.axis(result_t::yAxis));
  const size_t ybin(yaxis.bin(two.getValue()));
  if (!yaxis.isOverflow(ybin) && !yaxis.isUnderflow(ybin))
    copy(one.begin(), one.begin()+one.datasize(), result.begin()+(ybin*nxbins));
}









// *** processor 69 histograms 2 0D values to 1D scatter plot ***

pp69::pp69(const name_t &name)
  : AccumulatingProcessor(name)
{
  loadSettings(0);
}

void pp69::loadSettings(size_t)
{
  setupGeneral();
  _one = setupDependency("XName");
  _two = setupDependency("YName");
  bool ret (setupCondition());
  if ( !(_one && _two && ret) )
    return;
  if (_one->result().dim() != 0 || _two->result().dim() != 0)
    throw runtime_error("pp69::loadSettings() '" + name() + "': Either '" +
                        _one->name() + "' or '" + _two->name() + "' is not a 0D Hist");
  createHistList(set1DHist(name()));
  Log::add(Log::INFO,"processor '" + name() +
           "' makes a 1D Histogram where '"+  _one->name() +
           "' defines the x bin to set and '" + _two->name() +
           "' defines the y value of the x bin"
           ". Condition on Processor '" + _condition->name() + "'");
}

void pp69::process(const CASSEvent& evt, result_t &result)
{
  const result_t &one(_one->result(evt.id()));
  QReadLocker lock1(&one.lock);
  const result_t &two(_two->result(evt.id()));
  QReadLocker lock2(&two.lock);

  /** histogram the first value, which will return an iterator pointing to the
   *  correct bin. We can then write the second value to the correct bin
   */
  *(result.histogram(one.getValue())) = two.getValue();
  /** increase the number of fills */
  ++_nbrEventsAccumulated;
}














// ***  pp 70 subsets a histogram ***

pp70::pp70(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp70::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;

  const result_t& hist(_pHist->result());
  if (hist.dim() == 0)
    throw invalid_argument("pp70::loadSettings(): Dimension '" +
                           toString(hist.dim()) + "' of histogram '" +
                           _pHist->name() + "'not supported");

  pair<float,float> userXRange(make_pair(s.value("XLow",0).toFloat(),
                                         s.value("XUp",1).toFloat()));
  const result_t::axe_t &xaxis(hist.axis(result_t::xAxis));
  _inputOffset = xaxis.bin(userXRange.first);
  const size_t binXUp(xaxis.bin(userXRange.second));
  /** @note the total number of bin between low and up is up-low+1 */
  const size_t nXBins(binXUp-_inputOffset+1);
  const float xLow(xaxis.pos(_inputOffset));
  const float xUp(xaxis.pos(binXUp));
  string output("Processor '" + name() +
                "' setup: returns a subset of histogram in pp '"  +  _pHist->name() +
                "' which has dimension '" + toString(hist.dim()) +
                "'. Subset is xLow:" + toString(xLow) + "(" + toString(_inputOffset) +
                "), xUp:" + toString(xUp) + "(" + toString(binXUp) +
                "), xNbrBins:" + toString(nXBins));
  if (1 == hist.dim())
  {
    createHistList(result_t::shared_pointer
                   (new result_t(result_t::axe_t(nXBins,xLow,xUp,xaxis.title))));
  }
  else if (2 == hist.dim())
  {
    pair<float,float> userYRange(make_pair(s.value("YLow",0).toFloat(),
                                           s.value("YUp",1).toFloat()));
    const result_t::axe_t &yaxis(hist.axis(result_t::yAxis));
    const size_t binYLow(yaxis.bin(userYRange.first));
    const size_t binYUp (yaxis.bin(userYRange.second));
    /** @note the total number of bin between low and up is up-low +1 */
    const size_t nYBins = (binYUp - binYLow + 1);
    const float yLow (yaxis.pos(binYLow));
    const float yUp (yaxis.pos(binYUp));
    _inputOffset = static_cast<size_t>(binYLow*xaxis.nBins+xLow + 0.1);
    createHistList
        (result_t::shared_pointer
          (new result_t
           (result_t::axe_t(nXBins,xLow,xUp,xaxis.title),
            result_t::axe_t(nYBins,yLow,yUp,yaxis.title))));
    output += ", yLow:"+ toString(yLow) + "(" + toString(binYLow) +
        "), yUp:" + toString(yUp) + "(" + toString(binYLow) +
        "), yNbrBins:"+ toString(nYBins) + ", linearized offset is now:" +
        toString(_inputOffset);
  }

  output += ". Condition on processor '" + _condition->name() + "'";
  Log::add(Log::INFO,output);
}

void pp70::process(const CASSEvent& evt, result_t &result)
{
  const result_t& input(_pHist->result(evt.id()));
  QReadLocker lock(&input.lock);

  result_t::const_iterator iit(input.begin()+_inputOffset);
  result_t::iterator rit(result.begin());
  const size_t resultNbrXBins(result.shape().first);
  const size_t resultNbrYBins(result.shape().second);
  const size_t inputNbrXBins(input.axis(result_t::xAxis).nBins);
  for (size_t yBins=0;yBins < resultNbrYBins; ++yBins)
  {
    copy(iit,iit+resultNbrXBins,rit);
    advance(iit,inputNbrXBins);
    advance(rit,resultNbrXBins);
  }
}







// ***  pp 71 returns a user specific value of a Histogram ***

pp71::pp71(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp71::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  string functype(s.value("RetrieveType","max").toString().toStdString());
  if (functype == "max")
    _func = &cass::max_element<result_t::const_iterator>;
  else if (functype == "min")
    _func = &cass::min_element<result_t::const_iterator>;
  else
    throw invalid_argument("pp71::loadSettings(): RetrieveType '" + functype +
                           "' unknown.");

  createHistList(result_t::shared_pointer(new result_t()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' returns the value in '" + _pHist->name() +
           "' that is retrieved by using function type '" + functype +
           "' .Condition on processor '" + _condition->name() + "'");
}

void pp71::process(const CASSEvent& evt, result_t &result)
{
  const result_t& one(_pHist->result(evt.id()));
  QReadLocker lock(&one.lock);

  result.setValue(*(_func(one.begin(), one.begin()+one.datasize())));
}








// ***  pp 75 clears a histogram ***

pp75::pp75(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp75::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _hist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _hist))
    return;
  Log::add(Log::INFO,"Processor '" + name() +
           "' clears the result in processor '" + _hist->name() +
           "'. Condition is "+ _condition->name());
}

const Processor::result_t &pp75::result(const CASSEvent::id_t)
{
  throw logic_error("pp75::result: '"+name()+"' should never be called");
}

void pp75::processEvent(const CASSEvent& evt)
{
  if (_condition->result(evt.id()).isTrue())
    const_cast<result_t&>(_hist->result(evt.id())).clear();
}






// ***  pp 76 quit program

pp76::pp76(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp76::loadSettings(size_t)
{
  setupGeneral();
  if (!setupCondition())
    return;
  Log::add(Log::INFO,"Processor '" + name() +
           "' will quit CASS.. Condition is "+ _condition->name());
}

const Processor::result_t& pp76::result(const CASSEvent::id_t)
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
  : Processor(name)
{
  loadSettings(0);
}

void pp77::loadSettings(size_t)
{
  setupGeneral();
  if (!setupCondition())
    return;
  CASSSettings s;
  s.beginGroup("Processor");
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
  createHistList(result_t::shared_pointer(new result_t()));

  Log::add(Log::INFO,"Processor '" + name() +
           "' will check whether eventid in file '" + filename +
           "'. Condition is "+ _condition->name());
}

void pp77::process(const CASSEvent& evt, result_t &result)
{
  result.setValue(find(_list.begin(),_list.end(),evt.id()) != _list.end());
}









// ***  pp 78 counter ***

pp78::pp78(const name_t &name)
  : AccumulatingProcessor(name)
{
  loadSettings(0);
}

void pp78::loadSettings(size_t)
{
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(result_t::shared_pointer(new result_t()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' counts how many times its process is called. '"  +
           "'. Condition is '"+ _condition->name() + "'");
}

void pp78::process(const CASSEvent&, result_t &result)
{
  ++(result[0]);
}













// ***  pp 81 returns the highest bin of a 1D Histogram ***

pp81::pp81(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp81::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  if (_pHist->result().dim() != 1)
    throw invalid_argument("pp81::loadSettings() '" + name() + "': '" +
                           _pHist->name() + "' is not a 1D hist");
  string functype(s.value("RetrieveType","max").toString().toStdString());
  if (functype == "max")
    _func = &cass::max_element<result_t::const_iterator>;
  else if (functype == "min")
    _func = &cass::min_element<result_t::const_iterator>;
  else
    throw invalid_argument("pp81::loadSettings(): RetrieveType '" + functype +
                           "' unknown.");

  createHistList(result_t::shared_pointer(new result_t()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' returns the maximum bin in '" + _pHist->name() +
           "' .Condition on processor '" + _condition->name() + "'");
}

void pp81::process(const CASSEvent& evt, result_t &result)
{
  const result_t& one(_pHist->result(evt.id()));
  QReadLocker lock(&one.lock);

  result_t::const_iterator it(_func(one.begin(), one.begin()+one.datasize()));
  const size_t bin(distance(one.begin(),it));
  result.setValue(one.axis(result_t::xAxis).pos(bin));
}
















// ***  pp 82 returns statistics value of all bins in a Histogram ***

pp82::pp82(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp82::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("InputName");
  bool ret(setupCondition());
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

  createHistList(result_t::shared_pointer(new result_t()));

  Log::add(Log::INFO,"Processor '" + name() +
           "' returns the mean of all bins in '" + _pHist->name() +
           "' .Condition on processor '" + _condition->name() + "'");
}

void pp82::process(const CASSEvent& evt, result_t &result)
{
  const result_t &one(_pHist->result(evt.id()));
  QReadLocker lock(&one.lock);

  stat_t stat;
  stat.addDistribution(one.begin(),one.begin()+one.datasize());
  result.setValue(_val(stat));
}


















// ***  pp 85 return full width at half maximum in given range of 1D histgoram ***

pp85::pp85(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp85::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  pair<float,float> userXRange(make_pair(s.value("XLow",0).toFloat(),
                                         s.value("XUp",1).toFloat()));
  _fraction = s.value("Fraction",0.5).toFloat();

  const result_t &hist(_pHist->result());
  if (hist.dim() != 1)
    throw invalid_argument("pp85::loadSettings()'" + name() +
                           "': Error the histogram we depend on '" + _pHist->name() +
                           "' is not a 1D Histogram.");
  const result_t::axe_t &xaxis(hist.axis(result_t::xAxis));
  _xRange = make_pair(xaxis.bin(userXRange.first),
                      xaxis.bin(userXRange.second));

  createHistList(result_t::shared_pointer(new result_t()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' returns the full width at half maximum in '" + _pHist->name() +
           "' of the range from xlow '" + toString(userXRange.first) +
           "' to xup '" + toString(userXRange.second) +
           "' .Condition on processor '" + _condition->name() + "'");
}

void pp85::process(const CASSEvent& evt, result_t &result)
{
  const result_t& one(_pHist->result(evt.id()));
  QReadLocker lock(&one.lock);

  result_t::const_iterator xRangeBegin(one.begin()+_xRange.first);
  result_t::const_iterator xRangeEnd(one.begin()+_xRange.second);
  result_t::const_iterator maxElementIt(cass::max_element(xRangeBegin, xRangeEnd));
  result_t::const_iterator minElementIt(cass::min_element(xRangeBegin, xRangeEnd));
  const float fracMax((*maxElementIt-*minElementIt) * _fraction + *minElementIt);

  result_t::const_iterator leftSide;
  result_t::const_iterator rightSide;
  for(result_t::const_iterator iVal(maxElementIt);
      (iVal != xRangeBegin) && (*iVal > fracMax);
      --iVal)
  {
    leftSide = iVal;
  }
  for(result_t::const_iterator iVal(maxElementIt);
      (iVal != xRangeEnd) && (*iVal > fracMax);
      ++iVal)
  {
    rightSide = iVal;
  }
  const result_t::axe_t xaxis(one.axis((result_t::xAxis)));
  const float lowerLeftPos (xaxis.pos(distance(one.begin(),leftSide)));
  const float lowerRightPos(xaxis.pos(distance(one.begin(),rightSide)));
  const float upperLeftPos (xaxis.pos(distance(one.begin(),leftSide-1)));
  const float upperRightPos(xaxis.pos(distance(one.begin(),rightSide+1)));
  const float lowerdist(lowerRightPos - lowerLeftPos);
  const float upperdist(upperRightPos - upperLeftPos);
  const float fwfm((upperdist+lowerdist)*0.5);

  result.setValue(fwfm);
}






















// ***  pp 86 return x position of a step in 1D histgoram ***

pp86::pp86(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp86::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  pair<float,float> userXRangeStep(make_pair(s.value("XLow",0).toFloat(),
                                             s.value("XUp",1).toFloat()));
  pair<float,float> userXRangeBaseline(make_pair(s.value("BaselineLow",0).toFloat(),
                                                 s.value("BaselineUp",1).toFloat()));
  _userFraction = s.value("Fraction",0.5).toFloat();
  const result_t &hist(_pHist->result());
  if (hist.dim() != 1)
    throw invalid_argument("pp86::loadSettings()'" + name() +
                           "': Error the processor we depend on '" + _pHist->name() +
                           "' does not contain a 1D result.");
  const result_t::axe_t &xaxis(hist.axis(result_t::xAxis));
  _xRangeStep = make_pair(xaxis.bin(userXRangeStep.first),
                          xaxis.bin(userXRangeStep.second));
  _xRangeBaseline = make_pair(xaxis.bin(userXRangeBaseline.first),
                              xaxis.bin(userXRangeBaseline.second));
  createHistList(result_t::shared_pointer(new result_t()));
  Log::add(Log::INFO, "Processor '" + name() +
           "' returns the position of the step in '" + _pHist->name() +
           "' in the range from xlow '" + toString(userXRangeStep.first) +
           "' to xup '" + toString(userXRangeStep.second) +
           "'. Where the baseline is defined in range '" + toString(userXRangeBaseline.first) +
           "' to '" + toString(userXRangeBaseline.second) + "'. The Fraction is '" +
           toString(_userFraction) + "' .Condition on processor '" +
           _condition->name() + "'");
}

void pp86::process(const CASSEvent& evt, result_t &result)
{
  const result_t& one(_pHist->result(evt.id()));
  QReadLocker lock(&one.lock);

  result_t::const_iterator baselineBegin(one.begin()+_xRangeBaseline.first);
  result_t::const_iterator baselineEnd(one.begin()+_xRangeBaseline.second);
  const float baseline(accumulate(baselineBegin,baselineEnd,0.f) /
                       static_cast<float>(distance(baselineBegin,baselineEnd)));

  result_t::const_iterator stepRangeBegin(one.begin()+_xRangeStep.first);
  result_t::const_iterator stepRangeEnd(one.begin()+_xRangeStep.second);

  result_t::const_iterator maxElementIt
      (std::max_element(stepRangeBegin, stepRangeEnd));
  const float halfMax((*maxElementIt+baseline) * _userFraction);

  result_t::const_iterator stepIt(stepRangeBegin+1);
  for ( ; stepIt != maxElementIt ; stepIt++ )
    if ( *(stepIt-1) <= halfMax && halfMax < *stepIt )
      break;
  const size_t steppos(distance(one.begin(),stepIt));

  result.setValue(steppos);
}
















// ***  pp 87 return center of mass in range of 1D histgoram ***

pp87::pp87(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp87::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  pair<float,float> userXRange(make_pair(s.value("XLow",0).toFloat(),
                                         s.value("XUp",1).toFloat()));
  const result_t &hist(_pHist->result());
  if (hist.dim() != 1)
    throw invalid_argument("pp87::setupParameters()'" + name() +
                           "': Error the processor we depend on '" + _pHist->name() +
                           "' does not contain a 1D result.");
  const result_t::axe_t &xaxis(hist.axis(result_t::xAxis));
  _xRange = make_pair(xaxis.bin(userXRange.first),
                      xaxis.bin(userXRange.second));

  createHistList(result_t::shared_pointer(new result_t()));
  Log::add(Log::INFO, "Processor '" + name() +
           "' returns the center of mass in '" + _pHist->name() +
           "' in the range from xlow '" + toString(userXRange.first) +
           "' to xup '" + toString(userXRange.second) +
           "' .Condition on processor '"+_condition->name()+"'");
}

void pp87::process(const CASSEvent& evt, result_t &result)
{
  const result_t& data(_pHist->result(evt.id()));
  QReadLocker lock(&data.lock);

  const result_t::axe_t &xAxis(data.axis(result_t::xAxis));

  float integral(0);
  float weight(0);
  for (size_t i(_xRange.first); i < _xRange.second; ++i)
  {
    integral += (data[i]);
    const float pos(xAxis.pos(i));
    weight += (data[i]*pos);
  }
  const float com(weight/integral);

  result.setValue(com);
}







// ***  pp 88 returns an axis parameter ***

pp88::pp88(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp88::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));

  setupGeneral();
  _pHist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;

  QString axisparam(s.value("AxisParameter","XNbrBins").toString());
  if (axisparam == "XNbrBins")
  {
    _axisId = result_t::xAxis;
    _func = &result_t::axe_t::nBins;
  }
  else if (axisparam == "XLow")
  {
    _axisId = result_t::xAxis;
    _func = &result_t::axe_t::low;
  }
  else if (axisparam == "XUp")
  {
    _axisId = result_t::xAxis;
    _func = &result_t::axe_t::up;
  }
  else if (axisparam == "YNbrBins")
  {
    _axisId = result_t::yAxis;
    _func = &result_t::axe_t::nBins;
  }
  else if (axisparam == "YLow")
  {
    _axisId = result_t::yAxis;
    _func = &result_t::axe_t::low;
  }
  else if (axisparam == "YUp")
  {
    _axisId = result_t::yAxis;
    _func = &result_t::axe_t::up;
  }
  else
    throw invalid_argument("pp88 '" + name() + "' AxisParameter '" +
                           axisparam.toStdString() + "' is unknown.");

  if (_pHist->result().dim() == 0)
    throw invalid_argument("pp88 '" + name() + "' result '" + _pHist->name() +
                           "' has dimension 0, which has no axis properties.");

  if ((axisparam == "YNbrBins" || axisparam == "YLow" || axisparam == "YUp")
      && _pHist->result().dim() == 1)
    throw invalid_argument("pp88 '" + name() + "' result '" + _pHist->name() +
                           "' has dimension 1 thus doesn't contain '" +
                           axisparam.toStdString() + "'");

  createHistList(result_t::shared_pointer(new result_t()));
  Log::add(Log::INFO,"Processor '" + name() +
           "' returns axis parameter'"+ axisparam.toStdString() +
           "' of result in processor '" + _pHist->name() +
           "'. Condition on Processor '" + _condition->name() + "'");
}

void pp88::process(const CASSEvent& evt, result_t &result)
{
  const result_t& hist(_pHist->result(evt.id()));
  QReadLocker lock(&hist.lock);

  result.setValue(_func(hist.axis(_axisId)));
}








// ***  pp 89 high/low pass filter ***

pp89::pp89(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp89::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));

  setupGeneral();
  _pHist = setupDependency("InputName");
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

  if (_pHist->result().dim() != 1)
    throw invalid_argument("pp89 '" + name() + "' histogram '" + _pHist->name() +
                           "' is not a 1D histograms");

  createHistList(_pHist->result().clone());
  Log::add(Log::INFO,"Processor '" + name() +
           "' does "+ filtertype.toStdString() +
           "' operation on histogram in pp '" + _pHist->name() +
           "'. Condition on Processor '" + _condition->name() + "'");
}

void pp89::highPass(result_t::const_iterator orig,
                    result_t::iterator filtered)
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

void pp89::lowPass(result_t::const_iterator orig,
                   result_t::iterator filtered)
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

void pp89::process(const CASSEvent& evt, result_t &result)
{
  const result_t& in(_pHist->result(evt.id()));
  QReadLocker lock(&in.lock);

  result_t::const_iterator inIt(in.begin());
  result_t::const_iterator inEnd(in.begin()+in.datasize());
  result_t::iterator outIt(result.begin());

  *outIt++ = *inIt++;
  while (inIt != inEnd)
  {
    _func(inIt,outIt);
    ++inIt;
    ++outIt;
  }
}







// ***  pp 91 return a list of minima in a histogram ***

pp91::pp91(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp91::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("InputName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const result_t &hist(_pHist->result());
  if (hist.dim() != 1)
    throw invalid_argument("pp91::loadSettings()'" + name() +
                           "': Error the processor we depend on '" + _pHist->name() +
                           "' does not contain a 1D result.");
  _range =  s.value("Range",10).toUInt();

  createHistList(result_t::shared_pointer(new result_t(nbrOf,0)));

  Log::add(Log::INFO, "Processor '" + name() +
           "' returns a list of local minima in '" + _pHist->name() +
           "'. The local minimum is the minimum within a range of +- '" +
           toString(_range) + "' .Condition on processor '"+_condition->name()
           +"'");
}

void pp91::process(const CASSEvent& evt, result_t &result)
{
  const result_t& data(_pHist->result(evt.id()));
  QReadLocker lock(&data.lock);

  const result_t::axe_t &xAxis(data.axis(result_t::xAxis));

  result.resetTable();
  table_t candidate(nbrOf,0);

  for (size_t i=_range;i < data.datasize()-_range; ++i)
  {
    if (!std::isfinite(data[i]))
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
      candidate[Position] = xAxis.pos(i);
      candidate[Value] = data[i];
      result.appendRows(candidate);
    }
  }
}
