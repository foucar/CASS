// Copyright (C) 2012 Koji Motomura
// Copyright (C) 2012 Marco Siano

/**
 * @file partial_covariance.cpp processors to calculate partical covariance
 *
 * @author Koji Motomura
 * @author Marco Siano
 */

#include <QtCore/QString>
#include <iterator>
#include <algorithm>
#include <iomanip>

#include "cass.h"
#include "operations.h"
#include "convenience_functions.h"
#include "cass_settings.h"
#include "partial_covariance.h"
#include "machine_device.hpp"
#include "log.h"

using namespace cass;
using namespace std;


namespace cass
{

/** short description
 *
 * details
 *
 * @author
 */
class PreAverage : std::binary_function<float,float,float>
{
public:
  /** constructor.
   *
   * initializes the \f$\alpha\f$ value
   *
   * @param alpha The \f$\alpha\f$ value
   */
  explicit PreAverage(float alpha)
    :_alpha(alpha)
  {}

  /** operator.
   *
   * the operator calculates the previous average using the function
   *
   * where when \f$\alpha\f$ is equal to N it is a cumulative moving average.
   *
   */
  float operator()(float currentValue, float Average_Nm1)
  {
    const double n = 1./_alpha;
    if (n<2) return 0;
    return (n*Average_Nm1 - currentValue)/(n-1.);
  }

protected:
  /** \f$\alpha\f$ for the average calculation */
  float _alpha;
};

}//end namespace cass



// ***  pp 400 ToF to Energ ***

pp400::pp400(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp400::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("Processor");
  settings.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const result_t &one(_pHist->result());
  if (1 != one.dim())
    throw invalid_argument("pp400::loadSettings(): Unsupported dimension of requested histogram");

  _userTofRange = make_pair(settings.value("TofLow",0).toFloat(),
                            settings.value("TofUp",1).toFloat());

  const result_t::axe_t &xaxis(one.axis(result_t::xAxis));
  binTofLow=xaxis.bin(_userTofRange.first);
  binTofUp=xaxis.bin(_userTofRange.second);

  t0 = settings.value("t0",0).toFloat();
  bint0 = xaxis.bin(t0);
  e0 = settings.value("e0",0).toFloat();
  alpha = settings.value("alpha",0).toFloat();
  NbrBins = settings.value("NbrBins",0).toUInt();

  tb1 = settings.value("tb1",0).toFloat();
  tb2 = settings.value("tb2",0).toFloat();
  bintb1=xaxis.bin(tb1);
  bintb2=xaxis.bin(tb2);

  createHistList
      (result_t::shared_pointer
        (new result_t
         (result_t::axe_t(NbrBins,
                          pow(alpha/(_userTofRange.second-t0),2)-e0,
                          pow(alpha/(_userTofRange.first-t0),2)-e0))));

  Log::add(Log::INFO,"Processor '"+ name() +
           "' converts ToF into Energy scale '" +  _pHist->name() +
           "' which has dimension '" + toString(one.dim()) + " test TofUp:" +
           toString(_userTofRange.second) + " binTofLow:" + toString(binTofLow) +
           " binTofUp:" + toString(binTofUp) + "'. Conversion parameters e0:" +
           toString(e0) + " t0(bin):" + toString(t0) + "(" + toString(bint0) + ") alpha:" +
           toString(alpha) + ", NbrBins:" + toString(NbrBins) + ". Condition on processor '" +
           _condition->name() + "'");
}

double pp400::calcEtoTof (double energy)
{
  if (energy<0) energy = 0;
  return (alpha / sqrt(energy + e0) + t0);
}

void pp400::ToftoEnergy(const result_t &Tof , result_t &Energy, double offset)
{
  const result_t::axe_t &xaxis(Tof.axis(result_t::xAxis));

  double bin_size=xaxis.pos(2)-xaxis.pos(1);

  float TofLow = _userTofRange.first;
  float TofUp = _userTofRange.second;

  float Elow =pow(alpha/(TofUp-t0),2)-e0;
  float EUp =pow(alpha/(TofLow-t0),2)-e0;

  double SizeOfBins =  (EUp-Elow)/ NbrBins;
  for ( size_t i = 0; i < NbrBins; ++i)
  {
    Energy[i] = 0;

    double tofBinUp = calcEtoTof (SizeOfBins * i+Elow);
    double tofBinLow = calcEtoTof (SizeOfBins * ( i + 1 )+Elow);

    size_t klow(0), kup(0);

    klow = xaxis.bin(tofBinLow);
    kup = xaxis.bin(tofBinUp);

    if (klow == kup) Energy[i] = (tofBinUp - tofBinLow)/bin_size * (Tof[klow]-offset);

    else if ((kup - klow) == 1 ) Energy[i] = (xaxis.pos(klow + 1) - tofBinLow)/bin_size * (Tof[klow+1]-offset) + (tofBinUp - xaxis.pos(kup))/bin_size * (Tof[kup]-offset);

    else if ((kup - klow) > 1 )
    {
      Energy[i] = (xaxis.pos(klow + 1) - tofBinLow)/bin_size * (Tof[klow+1]-offset) + (tofBinUp - xaxis.pos(kup))/bin_size * (Tof[kup]-offset);
      for ( size_t j = klow + 1; j < kup; ++j)
        Energy[i] += (Tof[j]-offset);
    }
  }
}

void pp400::process(const CASSEvent& evt, result_t &result)
{
  const result_t& input(_pHist->result(evt.id()));
  QReadLocker lock(&input.lock);

  offset = 0;
  for (size_t i = bintb1; i<=bintb2;i++){
    offset+=input[i];
  }
  offset = offset/(bintb2-bintb1+1);

  ToftoEnergy(input, result, offset);
}










//*** Tof to Mass to Charge Ratio****
pp404::pp404(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp404::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("Processor");
  settings.beginGroup(name().c_str());
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const result_t &one(_pHist->result());
  if (1 != one.dim())
    throw runtime_error("pp400::loadSettings(): Unknown dimension of incomming histogram");

  _userTofRange = make_pair(settings.value("TofLow",0).toFloat(),
                            settings.value("TofUp",1).toFloat());
  const result_t::axe_t &xaxis(one.axis(result_t::xAxis));
  binTofLow=xaxis.bin(_userTofRange.first);
  binTofUp=xaxis.bin(_userTofRange.second);

  t0 = settings.value("t0",0).toFloat();
  t1 = settings.value("t1",0).toFloat();
  MtC0 = settings.value("MtC0",0).toFloat();
  MtC1 = settings.value("MtC1",0).toFloat();
  tb1 = settings.value("tb1",0).toFloat();
  tb2 = settings.value("tb2",0).toFloat();
  bintb1=xaxis.bin(tb1);
  bintb2=xaxis.bin(tb2);
  alpha = (t1-t0)/(sqrt(MtC1)-sqrt(MtC0));
  beta = t1-alpha*sqrt(MtC1);

  NbrBins = settings.value("NbrBins",0).toInt();

  createHistList
      (result_t::shared_pointer
        (new result_t
         (result_t::axe_t(NbrBins,
                          pow(_userTofRange.first/alpha-beta,2),
                          pow(_userTofRange.second/alpha-beta,2)))));

  Log::add(Log::INFO,"Processor '" + name() +
           "' converts ToF into MassTo ChargeRatio scale'" + _pHist->name() +
           "' which has dimension '" + toString(one.dim()) + "  TofUp:" +
           toString(_userTofRange.second) + " binTofLow:" + toString(binTofLow) +
           " binTofUp:" + toString(binTofUp) + "'. Conversion parameters MtC0:" +
           toString(MtC0) + " t0(bin):" + toString(t0) + "'. Conversion parameters MtC1:" +
           toString(MtC1) + " t1(bin):" + toString(t1) +", NbrBins:"+ toString(NbrBins) +
           ". Condition on processor '" + _condition->name() +"'");

}

double pp404::calcMtCtoTof (double MasstoCharge)
{
  if (MasstoCharge<0) MasstoCharge = 0;
  return (alpha * sqrt(MasstoCharge) + beta);
}

double pp404::calcToftoMtC (double Timeoflight)
{
  return pow(Timeoflight/alpha - beta,2);
}

void pp404::ToftoMtC(const result_t& Tof , result_t& MtC, double offset)
{
  const result_t::axe_t &xaxis(Tof.axis(result_t::xAxis));
  double bin_size=xaxis.pos(2)-xaxis.pos(1);

  double MtCDown = calcToftoMtC(_userTofRange.first);
  double MtCUp = calcToftoMtC(_userTofRange.second);
  double SizeOfBins = (MtCUp-MtCDown) / NbrBins;
  for ( size_t i = 0; i < NbrBins; ++i)
  {
    MtC[i] = 0;

    double tofBinLow = calcMtCtoTof (SizeOfBins * i + MtCDown);
    double tofBinUp = calcMtCtoTof (SizeOfBins * ( i + 1 )+MtCDown);

    long klow(0), kup(0);
    klow=xaxis.bin(tofBinLow);
    kup=xaxis.bin(tofBinUp);

    if (klow == kup)
    {
      MtC[i] =  (Tof[klow]-offset)*(tofBinUp-tofBinLow)/bin_size;
    }
    else if ((kup - klow) == 1 ) MtC[i] =(Tof[klow]-offset)*(xaxis.pos(klow + 1) - tofBinLow)/bin_size + (Tof[kup]-offset)*(tofBinUp -xaxis.pos(kup))/bin_size;
    else if ((kup - klow) > 1 )
    {
      MtC[i] = (Tof[klow]-offset)*(xaxis.pos(klow + 1) - tofBinLow)/bin_size + (Tof[kup]-offset)*(tofBinUp -xaxis.pos(kup))/bin_size;
      for ( long j = klow + 1; j < kup; ++j)
        MtC[i] += Tof[j]-offset;
    }
  }
}

void pp404::process(const CASSEvent& evt, result_t &result)
{
  const result_t& input(_pHist->result(evt.id()));
  QReadLocker lock(&input.lock);

  double offset(0);
  for (size_t i = bintb1; i<=bintb2;i++){
    offset+=input[i];
  }
  offset = offset/(bintb2-bintb1+1);

  ToftoMtC(input, result, offset );
}










// *** processors 405 calcs pulse duration from bld ***

pp405::pp405(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp405::loadSettings(size_t)
{
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(result_t::shared_pointer(new result_t()));

  Log::add(Log::INFO,"Processor '" + name() + "' calc pulse duration from" +
            " beamline data. Condition is '" + _condition->name() + "'");
}

void pp405::process(const CASSEvent& evt, result_t &result)
{
  using namespace MachineData;
  const Device &md
      (dynamic_cast<const Device&>
       (*(evt.devices().find(CASSEvent::MachineData)->second)));
  const Device::bldMap_t bld(md.BeamlineData());

  const double ebCharge
      (bld.find("EbeamCharge") == bld.end() ? 0 : bld.find("EbeamCharge")->second);
  const double peakCurrent
      (bld.find("EbeamPkCurrBC2") == bld.end() ? 0 : bld.find("EbeamPkCurrBC2")->second);

  const double puleduration (ebCharge/peakCurrent*1.0e-9);
  result.setValue(puleduration);
}










// ***  pp 406 ToF to Energy correct from 0D Histogram value***

pp406::pp406(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp406::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("Processor");
  settings.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  _constHist = setupDependency("HistZeroD");
  bool ret (setupCondition());
  if (!(ret && _pHist && _constHist ))
    return;
  const result_t &one(_pHist->result());
  if (1 != one.dim())
    throw runtime_error("pp406::loadSettings(): Unknown dimension of incomming histogram");
  const result_t &constHist(_constHist->result());
  if (constHist.dim() != 0 )
    throw invalid_argument("pp406::loadSettings(): HistZeroD '" + _constHist->name() +
                           "' is not a 0D histogram");

  _userTofRange = make_pair(settings.value("TofLow",0).toFloat(),
                            settings.value("TofUp",1).toFloat());

  const result_t::axe_t &xaxis(one.axis(result_t::xAxis));
  binTofLow=xaxis.bin(_userTofRange.first);
  binTofUp=xaxis.bin(_userTofRange.second);

  t0 = settings.value("t0",0).toFloat();
  bint0 = xaxis.bin(t0);
  e0 = settings.value("e0",0).toFloat();
  alpha = settings.value("alpha",0).toFloat();
  NbrBins = settings.value("NbrBins",0).toUInt();

  tb1 = settings.value("tb1",0).toFloat();
  tb2 = settings.value("tb2",0).toFloat();
  bintb1=xaxis.bin(tb1);
  bintb2=xaxis.bin(tb2);

  createHistList
      (result_t::shared_pointer
        (new result_t
         (result_t::axe_t(NbrBins,
                          pow(alpha/(_userTofRange.second-t0),2)-e0,
                          pow(alpha/(_userTofRange.first-t0),2)-e0))));

  Log::add(Log::INFO,"Processor '"+ name() + "' converts ToF into Energy scale '" +
           _pHist->name() + "' which has dimension '" + toString(one.dim()) +
           "' with constant in 0D Histogram in Processor '" + _constHist->name() +
           " test TofUp:" + toString(_userTofRange.second) + " binTofLow:" +
           toString(binTofLow) + " binTofUp:" + toString(binTofUp) +
           "'. Conversion parameters e0:"+ toString(e0) + " t0(bin):" +
           toString(t0) + "(" + toString(bint0) + ") alpha:" + toString(alpha) +
            ", NbrBins:" + toString(NbrBins) + ". Condition on processor '" +
            _condition->name() +"'");
}

double pp406::calcEtoTof (double energy)
{
  if (energy<0) energy = 0;
  return (alpha / sqrt(energy + ediff) + t0);
}

void pp406::ToftoEnergy(const result_t &Tof, result_t &Energy, double offset)
{
  const result_t::axe_t &xaxis(Tof.axis(result_t::xAxis));

  double bin_size=xaxis.pos(2)-xaxis.pos(1);

  float TofLow = _userTofRange.first;
  float TofUp = _userTofRange.second;

  float Elow =pow(alpha/(TofUp-t0),2)-e0;
  float EUp =pow(alpha/(TofLow-t0),2)-e0;

  double SizeOfBins =  (EUp-Elow)/ NbrBins;
  for ( size_t i = 0; i < NbrBins; ++i)
  {
    Energy[i] = 0;

    double tofBinUp = calcEtoTof (SizeOfBins * i+Elow);
    double tofBinLow = calcEtoTof (SizeOfBins * ( i + 1 )+Elow);

    size_t klow(0), kup(0);

    klow = xaxis.bin(tofBinLow);
    kup = xaxis.bin(tofBinUp);


    if (klow == kup) Energy[i] = (tofBinUp - tofBinLow)/bin_size * (Tof[klow]-offset);

    else if ((kup - klow) == 1 ) Energy[i] = (xaxis.pos(klow + 1) - tofBinLow)/bin_size * (Tof[klow+1]-offset) + (tofBinUp - xaxis.pos(kup))/bin_size * (Tof[kup]-offset);

    else if ((kup - klow) > 1 )
    {
      Energy[i] = (xaxis.pos(klow + 1) - tofBinLow)/bin_size * (Tof[klow+1]-offset) + (tofBinUp - xaxis.pos(kup))/bin_size * (Tof[kup]-offset);
      for ( size_t j = klow + 1; j < kup; ++j)
        Energy[i] += (Tof[j]-offset);
    }
  }
}

void pp406::process(const CASSEvent& evt, result_t &result)
{
  const result_t& input(_pHist->result(evt.id()));
  QReadLocker lock(&input.lock);
  const result_t &constHist(_constHist->result(evt.id()));
  QReadLocker lock2(&constHist.lock);

  ediff = e0 + (constHist.getValue());

  double offset(0);
  for (size_t i = bintb1; i<=bintb2;i++){
    offset+=input[i];
  }
  offset = offset/(bintb2-bintb1+1);

  ToftoEnergy( input, result, offset );
}







//***Tof to Energy linear interpolation***
pp407::pp407(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp407::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("Processor");
  settings.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const result_t &one(_pHist->result());
  if (1 != one.dim())
    throw runtime_error("pp407::loadSettings(): Unknown dimension of incomming histogram");

  _userTofRange = make_pair(settings.value("TofLow",0).toFloat(),
                            settings.value("TofUp",1).toFloat());

  const result_t::axe_t &xaxis(one.axis(result_t::xAxis));
  binTofLow=xaxis.bin(_userTofRange.first);
  binTofUp=xaxis.bin(_userTofRange.second);

  t0 = settings.value("t0",0).toFloat();
  bint0 = xaxis.bin(t0);
  e0 = settings.value("e0",0).toFloat();
  alpha = settings.value("alpha",0).toFloat();
  NbrBins = settings.value("NbrBins",0).toUInt();

  tb1 = settings.value("tb1",0).toFloat();
  tb2 = settings.value("tb2",0).toFloat();
  bintb1=xaxis.bin(tb1);
  bintb2=xaxis.bin(tb2);

  createHistList
      (result_t::shared_pointer
        (new result_t
         (result_t::axe_t(NbrBins,
                          pow(alpha/(_userTofRange.second-t0),2)-e0,
                          pow(alpha/(_userTofRange.first-t0),2)-e0))));

  Log::add(Log::INFO,"Processor '" + name() + "' converts ToF into Energy scale '" +
           _pHist->name() + "' which has dimension '" + toString(one.dim()) +
           " test TofUp:" + toString(_userTofRange.second) + " binTofLow:" +
           toString(binTofLow) + " binTofUp:" + toString(binTofUp) +
           "'. Conversion parameters e0:" + toString(e0) + " t0(bin):" + toString(t0) +
           "(" + toString(bint0) + ") alpha:" + toString(alpha) + ", NbrBins:" +
           toString(NbrBins) + ". Condition on processor '" + _condition->name() + "'");
}

double pp407::calcEtoTof (double energy)
{
  if (energy<0) energy = 0;
  return (alpha / sqrt(energy + e0) + t0);
}

double pp407::calcTofValue(const double tofPos, const size_t binlow ,const double bin_size, const result_t& Tof)
{
  const result_t::axe_t &xaxis(Tof.axis(result_t::xAxis));

  return (Tof[binlow] +(Tof[binlow+1]-Tof[binlow])/bin_size * (tofPos - xaxis.pos(binlow)) );
}

void pp407::ToftoEnergy(const result_t& Tof , result_t& Energy, double offset)
{
  const result_t::axe_t &xaxis(Tof.axis(result_t::xAxis));

  const double bin_size=xaxis.pos(2)-xaxis.pos(1);

  float TofLow = _userTofRange.first;
  float TofUp = _userTofRange.second;

  float Elow =pow(alpha/(TofUp-t0),2)-e0;
  float EUp =pow(alpha/(TofLow-t0),2)-e0;

  double SizeOfBins =  (EUp-Elow)/ NbrBins;
  for ( size_t i = 0; i < NbrBins; ++i)
  {
    Energy[i] = 0;

    double tofBinUp = calcEtoTof (SizeOfBins * i+Elow);
    double tofBinLow = calcEtoTof (SizeOfBins * ( i + 1 )+Elow);

    size_t klow(0), kup(0);

    klow = xaxis.bin(tofBinLow);
    kup = xaxis.bin(tofBinUp);

    double tofValueUp = calcTofValue(tofBinUp,klow,bin_size,Tof);
    double tofValueLow = calcTofValue(tofBinLow,klow,bin_size,Tof);

    //        if (klow == kup) Energy[i] = (tofBinUp - tofBinLow)/bin_size * (Tof[klow]-offset);
    if (klow == kup) Energy[i] = (tofBinUp - tofBinLow)*((tofValueUp+tofValueLow)/2-offset)/bin_size;

    else if ((kup - klow) == 1 )
      Energy[i] = (xaxis.pos(klow + 1) - tofBinLow)*((Tof[klow + 1]+tofValueLow)/2-offset)/bin_size
          + (tofBinUp - xaxis.pos(kup))*((Tof[kup]+tofValueUp)/2-offset)/bin_size;

    else if ((kup - klow) > 1 )
    {
      Energy[i] = (xaxis.pos(klow + 1) - tofBinLow)*((Tof[klow + 1]+tofValueLow)/2-offset)/bin_size
          + (tofBinUp - xaxis.pos(kup))*((Tof[kup]+tofValueUp)/2-offset)/bin_size;

      for ( size_t j = klow + 1; j < kup; ++j)
        Energy[i] += (Tof[j]-offset);
    }
  }
}

void pp407::process(const CASSEvent& evt, result_t &result)
{
  const result_t& input(_pHist->result(evt.id()));
  QReadLocker lock(&input.lock);

  double offset(0);
  for (size_t i = bintb1; i<=bintb2;i++){
    offset+=input[i];
  }
  offset = offset/(bintb2-bintb1+1);

  ToftoEnergy( input, result ,offset );

}








// ***  pp 408 ToF to Energy correct from 0D Histogram value & linear corection***

pp408::pp408(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp408::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("Processor");
  settings.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  _constHist = setupDependency("HistZeroD");
  bool ret (setupCondition());
  if (!(ret && _pHist && _constHist ))
    return;
  const result_t &one(_pHist->result());
  if (1 != one.dim())
    throw runtime_error("pp408::loadSettings(): Unknown dimension of incomming histogram");
  const result_t &constHist(_constHist->result());
  if (constHist.dim() != 0 )
    throw invalid_argument("pp408::loadSettings(): HistZeroD '" + _constHist->name() +
                           "' is not a 0D histogram");

  _userTofRange = make_pair(settings.value("TofLow",0).toFloat(),
                            settings.value("TofUp",1).toFloat());

  const result_t::axe_t &xaxis(one.axis(result_t::xAxis));
  binTofLow=xaxis.bin(_userTofRange.first);
  binTofUp=xaxis.bin(_userTofRange.second);

  t0 = settings.value("t0",0).toFloat();
  bint0 = xaxis.bin(t0);
  e0 = settings.value("e0",0).toFloat();
  alpha = settings.value("alpha",0).toFloat();
  NbrBins = settings.value("NbrBins",0).toUInt();

  tb1 = settings.value("tb1",0).toFloat();
  tb2 = settings.value("tb2",0).toFloat();
  bintb1=xaxis.bin(tb1);
  bintb2=xaxis.bin(tb2);

  createHistList
      (result_t::shared_pointer
        (new result_t
           (result_t::axe_t(NbrBins,
                            pow(alpha/(_userTofRange.second-t0),2)-e0,
                            pow(alpha/(_userTofRange.first-t0),2)-e0))));

  Log::add(Log::INFO,"Processor '" + name() + "' converts ToF into Energy scale '" +
           _pHist->name() + "' which has dimension '" + toString(one.dim()) +
           "' with constant in 0D Histogram in Processor '" + _constHist->name() +
           " test TofUp:" + toString(_userTofRange.second) + " binTofLow:" +
           toString(binTofLow) + " binTofUp:" + toString(binTofUp) +
           "'. Conversion parameters e0:" + toString(e0) + " t0(bin):" +
           toString(t0) + "(" + toString(bint0) + ") alpha:"+ toString(alpha) +
           ", NbrBins:"+ toString(NbrBins) + ". Condition on processor '" +
           _condition->name() + "'");
}

double pp408::calcEtoTof (double energy)
{
  if (energy<0) energy = 0;
  if ((energy + ediff) < 0)
    return -1;
  return (alpha / sqrt(energy + ediff) + t0);
}
double pp408::calcTofValue(const double tofPos, const size_t binlow , const double bin_size, const result_t &Tof)
{
  const result_t::axe_t &xaxis(Tof.axis(result_t::xAxis));

  return (Tof[binlow] +(Tof[binlow+1]-Tof[binlow])/bin_size * (tofPos - xaxis.pos(binlow)) );
}

void pp408::ToftoEnergy(const result_t &Tof, result_t& Energy, double offset)
{
  const result_t::axe_t &xaxis(Tof.axis(result_t::xAxis));

  double bin_size=xaxis.pos(2)-xaxis.pos(1);

  float TofLow = _userTofRange.first;
  float TofUp = _userTofRange.second;

  float Elow =pow(alpha/(TofUp-t0),2)-e0;
  float EUp =pow(alpha/(TofLow-t0),2)-e0;

  double SizeOfBins =  (EUp-Elow)/ NbrBins;
  for ( size_t i = 0; i < NbrBins; ++i)
  {
    Energy[i] = 0;

    double tofBinUp = calcEtoTof (SizeOfBins * i+Elow);
    double tofBinLow = calcEtoTof (SizeOfBins * ( i + 1 )+Elow);
    if((tofBinUp < 0)|| (tofBinUp > TofUp))
      continue;
    size_t klow(0), kup(0);

    klow = xaxis.bin(tofBinLow);
    kup = xaxis.bin(tofBinUp);

    double tofValueUp = calcTofValue(tofBinUp,klow,bin_size,Tof);
    double tofValueLow = calcTofValue(tofBinLow,klow,bin_size,Tof);

    if (klow == kup) Energy[i] = (tofBinUp - tofBinLow)*((tofValueUp+tofValueLow)/2-offset)/bin_size;

    else if ((kup - klow) == 1 )
      Energy[i] = (xaxis.pos(klow + 1) - tofBinLow)*((Tof[klow + 1]+tofValueLow)/2-offset)/bin_size
          + (tofBinUp - xaxis.pos(kup))*((Tof[kup]+tofValueUp)/2-offset)/bin_size;

    else if ((kup - klow) > 1 )
    {
      Energy[i] = (xaxis.pos(klow + 1) - tofBinLow)*((Tof[klow + 1]+tofValueLow)/2-offset)/bin_size
          + (tofBinUp - xaxis.pos(kup))*((Tof[kup]+tofValueUp)/2-offset)/bin_size;

      for ( size_t j = klow + 1; j < kup; ++j)
        Energy[i] += (Tof[j]-offset);
    }
  }
}

void pp408::process(const CASSEvent& evt, result_t &result)
{
  const result_t& input(_pHist->result(evt.id()));
  QReadLocker lock(&input.lock);
  const result_t &constHist(_constHist->result(evt.id()));
  QReadLocker lock2(&constHist.lock);

  ediff = e0 + (constHist.getValue());

  double offset(0);
  for (size_t i = bintb1; i<=bintb2;i++){
    offset+=input[i];
  }
  offset = offset/(bintb2-bintb1+1);

  ToftoEnergy( input, result ,offset );

}



// *** processor 410 calculate covariance ***

pp410::pp410(const name_t &name)
  : AccumulatingProcessor(name)
{
  loadSettings(0);
}

void pp410::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("Processor");
  settings.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  _ave = setupDependency("AveHistName");
  bool ret (setupCondition());
  if (!(ret && _pHist && _ave))
    return;
  const result_t &one(_pHist->result());
  const result_t::axe_t &xaxis(one.axis(result_t::xAxis));
  createHistList(result_t::shared_pointer(new result_t(xaxis,xaxis)));
  Log::add(Log::INFO,"processor '" + name() + "'Calculate variance '"+
           _pHist->name() + "'. Condition on processor '" + _condition->name() + "'");
}

void pp410::calcCovariance(const result_t &data ,
                           const result_t::storage_t &averageOld,
                           const result_t &averageNew,
                           result_t &variance, float n)
{
  const result_t &one(_pHist->result());
  const size_t nbrBins(one.axis(result_t::xAxis).nBins);
  for (unsigned int i=0; i<nbrBins; i++)
    for (unsigned int j=0; j<nbrBins; j++)
    {
      variance[i*nbrBins+j] = ((variance[i*nbrBins+j]*(n-1)) + (data[i]-averageOld[i])* (data[j]-averageNew[j]))/n;
    }
}

void pp410::process(const CASSEvent& evt, result_t &result)
{
  const result_t& one(_pHist->result(evt.id()));
  QReadLocker lock(&one.lock);
  const result_t& ave(_ave->result(evt.id()));
  QReadLocker lock2(&ave.lock);

  result_t::storage_t averagePre(ave.size());

  ++_nbrEventsAccumulated;
  float scale = 1./_nbrEventsAccumulated;

  transform(one.begin(),one.end(), ave.begin(), averagePre.begin(),
            PreAverage(scale));

  calcCovariance(one, averagePre, ave, result, 1./scale);
}







//------------pp412 calculate covariance for intensity correction

pp412::pp412(const name_t &name)
  : AccumulatingProcessor(name)
{
  loadSettings(0);
}

void pp412::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("Processor");
  settings.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _hist1D = setupDependency("HistName1D");
  _ave1D = setupDependency("AveHistName1D");
  _hist0D = setupDependency("HistName0D");
  _ave0D = setupDependency("AveHistName0D");
  bool ret (setupCondition());
  if (!(ret && _hist1D && _ave1D && _hist0D && _ave0D))
    return;
  createHistList(_hist1D->result().clone());
  Log::add(Log::INFO,"processor '" + name() + "'Calcurate variance '" +
           _hist1D->name() + "'. Condition on processor '" + _condition->name() + "'");
}

void pp412::calcCovariance(const result_t &waveTrace ,
                           const result_t &waveTraceAve ,
                           const float intensity,
                           const float intensityAveOld,
                           result_t &correction, float n)
{
  const result_t &one(_hist1D->result());
  size_t nbrBins(one.axis(result_t::xAxis).nBins);

  for (unsigned int i=0; i<nbrBins; i++)
  {
    correction[i] = (correction[i]*(n-1) + (waveTrace[i]-waveTraceAve[i]) * (intensity-intensityAveOld))/n;
  }
}

void pp412::process(const CASSEvent& evt, result_t &result)
{
  const result_t& waveTrace(_hist1D->result(evt.id()));
  QReadLocker lock(&waveTrace.lock);
  const result_t& waveTraceAve(_ave1D->result(evt.id()));
  QReadLocker lock1(&waveTraceAve.lock);
  const result_t& intensity(_hist0D->result(evt.id()));
  QReadLocker lock2(&intensity.lock);
  const result_t& intensityAve(_ave0D->result(evt.id()));
  QReadLocker lock3(&intensityAve.lock);

  ++_nbrEventsAccumulated;
  const float scale = 1./_nbrEventsAccumulated;

  const float intensityAvePre = intensityAve.getValue() - scale*(intensity.getValue() - intensityAve.getValue());

  calcCovariance(waveTrace,waveTraceAve,intensity.getValue(),intensityAvePre,
                 result,_nbrEventsAccumulated);
}
