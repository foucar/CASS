#include <QtCore/QString>
#include <iterator>
#include <algorithm>
#include <iomanip>

#include "cass.h"
#include "operations.h"
#include "postprocessor.h"
#include "histogram.h"
#include "convenience_functions.h"
#include "cass_settings.h"
#include "partial_covariance.h"
#include "machine_device.h"
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
class SqAverage : std::binary_function<float,float,float>
{
public:
  /** constructor.
   *
   * initializes the \f$\alpha\f$ value
   *
   * @param alpha The \f$\alpha\f$ value
   */
  explicit SqAverage(float alpha)
    :_alpha(alpha)
  {}

  /** operator.
   *
   * the operator calculates the square average using the function
   * \f$Y_N = Y_{N-1} + \alpha(y*y-Y_{N-1})\f$
   * where when \f$\alpha\f$ is equal to N it is a cumulative moving average.
   */
  float operator()(float currentValue, float Average_Nm1)
  {
    return Average_Nm1 + _alpha*(currentValue*currentValue - Average_Nm1);
  }

protected:
  /** \f$\alpha\f$ for the average calculation */
  float _alpha;
};

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
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp400::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result()));
  if (1 != one.dimension())
    throw invalid_argument("pp400::loadSettings(): Unsupported dimension of requested histogram");

  _userTofRange = make_pair(settings.value("TofLow",0).toFloat(),
                            settings.value("TofUp",1).toFloat());

  const AxisProperty &xaxis(one.axis()[HistogramBackend::xAxis]);
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

  createHistList(
        tr1::shared_ptr<Histogram1DFloat>
        (new Histogram1DFloat(NbrBins,
                              pow(alpha/(_userTofRange.second-t0),2)-e0,
                              pow(alpha/(_userTofRange.first-t0),2)-e0)));

  Log::add(Log::INFO,"PostProcessor '"+ name() +
           "' converts ToF into Energy scale '" +  _pHist->name() +
           "' which has dimension '" + toString(one.dimension()) + " test TofUp:" +
           toString(_userTofRange.second) + " binTofLow:" + toString(binTofLow) +
           " binTofUp:" + toString(binTofUp) + "'. Conversion parameters e0:" +
           toString(e0) + " t0(bin):" + toString(t0) + "(" + toString(bint0) + ") alpha:" +
           toString(alpha) + ", NbrBins:" + toString(NbrBins) + ". Condition on postprocessor '" +
           _condition->name() + "'");
}

double pp400::calcEtoTof (double energy)
{
  if (energy<0) energy = 0;
  return (alpha / sqrt(energy + e0) + t0);
}

void pp400::ToftoEnergy(const HistogramFloatBase& TofHisto , HistogramFloatBase::storage_t& Energy, double offset)
{
  const AxisProperty &xaxis(TofHisto.axis()[HistogramBackend::xAxis]);
  const HistogramFloatBase::storage_t& Tof(TofHisto.memory());


  double bin_size=xaxis.position(2)-xaxis.position(1);

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

    else if ((kup - klow) == 1 ) Energy[i] = (xaxis.position(klow + 1) - tofBinLow)/bin_size * (Tof[klow+1]-offset) + (tofBinUp - xaxis.position(kup))/bin_size * (Tof[kup]-offset);

    else if ((kup - klow) > 1 )
    {
      Energy[i] = (xaxis.position(klow + 1) - tofBinLow)/bin_size * (Tof[klow+1]-offset) + (tofBinUp - xaxis.position(kup))/bin_size * (Tof[kup]-offset);
      for ( size_t j = klow + 1; j < kup; ++j)
        Energy[i] += (Tof[j]-offset);
    }
  }
}

void pp400::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase& input
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));
  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  HistogramFloatBase::storage_t& output(result.memory());

  QReadLocker lock(&input.lock);

  offset = 0;
  for (size_t i = bintb1; i<=bintb2;i++){
    offset+=input.memory()[i];
  }
  offset = offset/(bintb2-bintb1+1);

  ToftoEnergy( input, output,offset );

  result.nbrOfFills()=1;
}



// *** postprocessor 402 square averages histograms ***

pp402::pp402(const name_t &name)
  : AccumulatingPostProcessor(name)
{
  loadSettings(0);
}

void pp402::loadSettings(size_t)
{
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  unsigned average = settings.value("NbrOfAverages", 1).toUInt();
  _alpha =  average ? 2./static_cast<float>(average+1.) : 0.;
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  createHistList(_pHist->result().copy_sptr());
  Log::add(Log::INFO,"Postprocessor '" + name() +
           "' sqaverages histograms from PostProcessor '" +  _pHist->name() +
           "' alpha for the averaging '" + toString(_alpha) +
           "'. Condition on postprocessor '" + _condition->name() + "'");
}

void pp402::process(const CASSEvent& evt, HistogramBackend &res)
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
            SqAverage(scale));
}










//*** Tof to Mass to Charge Ratio****
pp404::pp404(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp404::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(name().c_str());
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result()));
  if (1 != one.dimension())
    throw runtime_error("pp400::loadSettings(): Unknown dimension of incomming histogram");

  _userTofRange = make_pair(settings.value("TofLow",0).toFloat(),
                            settings.value("TofUp",1).toFloat());
  const AxisProperty &xaxis(one.axis()[HistogramBackend::xAxis]);
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

  createHistList(
        tr1::shared_ptr<Histogram1DFloat>
        (new Histogram1DFloat(NbrBins,
                              pow(_userTofRange.first/alpha-beta,2),
                              pow(_userTofRange.second/alpha-beta,2))));

  Log::add(Log::INFO,"PostProcessor '" + name() +
           "' converts ToF into MassTo ChargeRatio scale'" + _pHist->name() +
           "' which has dimension '" + toString(one.dimension()) + "  TofUp:" +
           toString(_userTofRange.second) + " binTofLow:" + toString(binTofLow) +
           " binTofUp:" + toString(binTofUp) + "'. Conversion parameters MtC0:" +
           toString(MtC0) + " t0(bin):" + toString(t0) + "'. Conversion parameters MtC1:" +
           toString(MtC1) + " t1(bin):" + toString(t1) +", NbrBins:"+ toString(NbrBins) +
           ". Condition on postprocessor '" + _condition->name() +"'");

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

void pp404::ToftoMtC(const HistogramFloatBase& hist , HistogramFloatBase::storage_t& MtC, double offset)
{
  const AxisProperty &xaxis(hist.axis()[HistogramBackend::xAxis]);
  double bin_size=xaxis.position(2)-xaxis.position(1);

  const HistogramFloatBase::storage_t& Tof(hist.memory());
  double MtCDown = calcToftoMtC(_userTofRange.first);
  double MtCUp = calcToftoMtC(_userTofRange.second);
  double SizeOfBins = (MtCUp-MtCDown) / NbrBins;
  for ( size_t i = 0; i < NbrBins; ++i)
  {
    MtC [i] = 0;

    double tofBinLow = calcMtCtoTof (SizeOfBins * i + MtCDown);
    double tofBinUp = calcMtCtoTof (SizeOfBins * ( i + 1 )+MtCDown);

    long klow(0), kup(0);
    klow=xaxis.bin(tofBinLow);
    kup=xaxis.bin(tofBinUp);

    if (klow == kup)
    {
      MtC[i] =  (Tof[klow]-offset)*(tofBinUp-tofBinLow)/bin_size;
    }
    else if ((kup - klow) == 1 ) MtC[i] =(Tof[klow]-offset)*(xaxis.position(klow + 1) - tofBinLow)/bin_size + (Tof[kup]-offset)*(tofBinUp -xaxis.position(kup))/bin_size;
    else if ((kup - klow) > 1 )
    {
      MtC[i] = (Tof[klow]-offset)*(xaxis.position(klow + 1) - tofBinLow)/bin_size + (Tof[kup]-offset)*(tofBinUp -xaxis.position(kup))/bin_size;
      for ( long j = klow + 1; j < kup; ++j)
        MtC[i] += Tof[j]-offset;
    }
  }
}

void pp404::process(const CASSEvent& evt, HistogramBackend &res)
{
  using namespace std;
  const HistogramFloatBase& input
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));
  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  HistogramFloatBase::storage_t& output(result.memory());

  QReadLocker lock(&input.lock);

  offset = 0;
  for (size_t i = bintb1; i<=bintb2;i++){
    offset+=input.memory()[i];
  }
  offset = offset/(bintb2-bintb1+1);

  ToftoMtC( input, output,offset );

  result.nbrOfFills()=1;
}










// *** postprocessors 405 calcs pulse duration from bld ***

pp405::pp405(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp405::loadSettings(size_t)
{
  setupGeneral();
  if (!setupCondition())
    return;
  createHistList(tr1::shared_ptr<Histogram0DFloat>(new Histogram0DFloat()));

  Log::add(Log::INFO,"PostProcessor '" + name() + "' calc pulse duration from" +
            " beamline data. Condition is '" + _condition->name() + "'");
}

void pp405::process(const CASSEvent& evt, HistogramBackend &res)
{
  using namespace MachineData;
  const MachineDataDevice *mdev
      (dynamic_cast<const MachineDataDevice *>
       (evt.devices().find(CASSEvent::MachineData)->second));
  const MachineDataDevice::bldMap_t bld(mdev->BeamlineData());
  Histogram0DFloat &result(dynamic_cast<Histogram0DFloat&>(res));

  const double ebCharge
      (bld.find("EbeamCharge") == bld.end() ? 0 : bld.find("EbeamCharge")->second);
  const double peakCurrent
      (bld.find("EbeamPkCurrBC2") == bld.end() ? 0 : bld.find("EbeamPkCurrBC2")->second);

  const double puleduration (ebCharge/peakCurrent*1.0e-9);
  result = puleduration;
}










// ***  pp 406 ToF to Energy correct from 0D Histogram value***

pp406::pp406(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp406::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  _constHist = setupDependency("HistZeroD");
  bool ret (setupCondition());
  if (!(ret && _pHist && _constHist ))
    return;
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result()));
  if (1 != one.dimension())
    throw runtime_error("pp406::loadSettings(): Unknown dimension of incomming histogram");
  const HistogramFloatBase &constHist(
        dynamic_cast<const HistogramFloatBase&>(_constHist->result()));
  if (constHist.dimension() != 0 )
    throw invalid_argument("pp406::loadSettings(): HistZeroD '" + _constHist->name() +
                           "' is not a 0D histogram");

  _userTofRange = make_pair(settings.value("TofLow",0).toFloat(),
                            settings.value("TofUp",1).toFloat());

  const AxisProperty &xaxis(one.axis()[HistogramBackend::xAxis]);
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

  createHistList(
        tr1::shared_ptr<Histogram1DFloat>
        (new Histogram1DFloat(NbrBins,
                              pow(alpha/(_userTofRange.second-t0),2)-e0,
                              pow(alpha/(_userTofRange.first-t0),2)-e0)));

  Log::add(Log::INFO,"PostProcessor '"+ name() + "' converts ToF into Energy scale '" +
           _pHist->name() + "' which has dimension '" + toString(one.dimension()) +
           "' with constant in 0D Histogram in PostProcessor '" + _constHist->name() +
           " test TofUp:" + toString(_userTofRange.second) + " binTofLow:" +
           toString(binTofLow) + " binTofUp:" + toString(binTofUp) +
           "'. Conversion parameters e0:"+ toString(e0) + " t0(bin):" +
           toString(t0) + "(" + toString(bint0) + ") alpha:" + toString(alpha) +
            ", NbrBins:" + toString(NbrBins) + ". Condition on postprocessor '" +
            _condition->name() +"'");
}

double pp406::calcEtoTof (double energy)
{
  if (energy<0) energy = 0;
  return (alpha / sqrt(energy + ediff) + t0);
}

void pp406::ToftoEnergy(const HistogramFloatBase& TofHisto , HistogramFloatBase::storage_t& Energy, double offset)
{
  const AxisProperty &xaxis(TofHisto.axis()[HistogramBackend::xAxis]);
  const HistogramFloatBase::storage_t& Tof(TofHisto.memory());


  double bin_size=xaxis.position(2)-xaxis.position(1);

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

    else if ((kup - klow) == 1 ) Energy[i] = (xaxis.position(klow + 1) - tofBinLow)/bin_size * (Tof[klow+1]-offset) + (tofBinUp - xaxis.position(kup))/bin_size * (Tof[kup]-offset);

    else if ((kup - klow) > 1 )
    {
      Energy[i] = (xaxis.position(klow + 1) - tofBinLow)/bin_size * (Tof[klow+1]-offset) + (tofBinUp - xaxis.position(kup))/bin_size * (Tof[kup]-offset);
      for ( size_t j = klow + 1; j < kup; ++j)
        Energy[i] += (Tof[j]-offset);
    }
  }
}

void pp406::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase& input
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));
  const Histogram0DFloat &constHist
      (dynamic_cast<const Histogram0DFloat&>(_constHist->result(evt.id())));
  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  HistogramFloatBase::storage_t& output(result.memory());

  QReadLocker lock(&input.lock);
  QReadLocker lock2(&constHist.lock);

  ediff = e0 + (constHist.getValue());

  offset = 0;
  for (size_t i = bintb1; i<=bintb2;i++){
    offset+=input.memory()[i];
  }
  offset = offset/(bintb2-bintb1+1);

  ToftoEnergy( input, output,offset );

  result.nbrOfFills()=1;
}



//***Tof to Energy linear interpolation***
pp407::pp407(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp407::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result()));
  if (1 != one.dimension())
    throw runtime_error("pp407::loadSettings(): Unknown dimension of incomming histogram");

  _userTofRange = make_pair(settings.value("TofLow",0).toFloat(),
                            settings.value("TofUp",1).toFloat());

  const AxisProperty &xaxis(one.axis()[HistogramBackend::xAxis]);
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

  createHistList(
        tr1::shared_ptr<Histogram1DFloat>
        (new Histogram1DFloat(NbrBins,
                              pow(alpha/(_userTofRange.second-t0),2)-e0,
                              pow(alpha/(_userTofRange.first-t0),2)-e0)));

  Log::add(Log::INFO,"PostProcessor '" + name() + "' converts ToF into Energy scale '" +
           _pHist->name() + "' which has dimension '" + toString(one.dimension()) +
           " test TofUp:" + toString(_userTofRange.second) + " binTofLow:" +
           toString(binTofLow) + " binTofUp:" + toString(binTofUp) +
           "'. Conversion parameters e0:" + toString(e0) + " t0(bin):" + toString(t0) +
           "(" + toString(bint0) + ") alpha:" + toString(alpha) + ", NbrBins:" +
           toString(NbrBins) + ". Condition on postprocessor '" + _condition->name() + "'");
}

double pp407::calcEtoTof (double energy)
{
  if (energy<0) energy = 0;
  return (alpha / sqrt(energy + e0) + t0);
}

double pp407::calcTofValue(const double tofPos, const size_t binlow ,const double bin_size, const HistogramFloatBase& TofHisto)
{
  const AxisProperty &xaxis(TofHisto.axis()[HistogramBackend::xAxis]);
  const HistogramFloatBase::storage_t& Tof(TofHisto.memory());

  return (Tof[binlow] +(Tof[binlow+1]-Tof[binlow])/bin_size * (tofPos - xaxis.position(binlow)) );
}

void pp407::ToftoEnergy(const HistogramFloatBase& TofHisto , HistogramFloatBase::storage_t& Energy, double offset)
{
  const AxisProperty &xaxis(TofHisto.axis()[HistogramBackend::xAxis]);
  const HistogramFloatBase::storage_t& Tof(TofHisto.memory());


  const double bin_size=xaxis.position(2)-xaxis.position(1);

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

    double tofValueUp = calcTofValue(tofBinUp,klow,bin_size,TofHisto);
    double tofValueLow = calcTofValue(tofBinLow,klow,bin_size,TofHisto);

    //        if (klow == kup) Energy[i] = (tofBinUp - tofBinLow)/bin_size * (Tof[klow]-offset);
    if (klow == kup) Energy[i] = (tofBinUp - tofBinLow)*((tofValueUp+tofValueLow)/2-offset)/bin_size;

    else if ((kup - klow) == 1 )
      Energy[i] = (xaxis.position(klow + 1) - tofBinLow)*((Tof[klow + 1]+tofValueLow)/2-offset)/bin_size
          + (tofBinUp - xaxis.position(kup))*((Tof[kup]+tofValueUp)/2-offset)/bin_size;

    else if ((kup - klow) > 1 )
    {
      Energy[i] = (xaxis.position(klow + 1) - tofBinLow)*((Tof[klow + 1]+tofValueLow)/2-offset)/bin_size
          + (tofBinUp - xaxis.position(kup))*((Tof[kup]+tofValueUp)/2-offset)/bin_size;

      for ( size_t j = klow + 1; j < kup; ++j)
        Energy[i] += (Tof[j]-offset);
    }
  }
}

void pp407::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase& input
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));
  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  HistogramFloatBase::storage_t& output(result.memory());

  QReadLocker lock(&input.lock);

  offset = 0;
  for (size_t i = bintb1; i<=bintb2;i++){
    offset+=input.memory()[i];
  }
  offset = offset/(bintb2-bintb1+1);

  ToftoEnergy( input, output,offset );

  result.nbrOfFills()=1;
}








// ***  pp 408 ToF to Energy correct from 0D Histogram value & linear corection***

pp408::pp408(const name_t &name)
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp408::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  _constHist = setupDependency("HistZeroD");
  bool ret (setupCondition());
  if (!(ret && _pHist && _constHist ))
    return;
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result()));
  if (1 != one.dimension())
    throw runtime_error("pp408::loadSettings(): Unknown dimension of incomming histogram");
  const HistogramFloatBase &constHist(dynamic_cast<const HistogramFloatBase&>(_constHist->result()));
  if (constHist.dimension() != 0 )
    throw invalid_argument("pp408::loadSettings(): HistZeroD '" + _constHist->name() +
                           "' is not a 0D histogram");

  _userTofRange = make_pair(settings.value("TofLow",0).toFloat(),
                            settings.value("TofUp",1).toFloat());

  const AxisProperty &xaxis(one.axis()[HistogramBackend::xAxis]);
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

  createHistList(
        tr1::shared_ptr<Histogram1DFloat>
        (new Histogram1DFloat(NbrBins,
                              pow(alpha/(_userTofRange.second-t0),2)-e0,
                              pow(alpha/(_userTofRange.first-t0),2)-e0)));

  Log::add(Log::INFO,"PostProcessor '" + name() + "' converts ToF into Energy scale '" +
           _pHist->name() + "' which has dimension '" + toString(one.dimension()) +
           "' with constant in 0D Histogram in PostProcessor '" + _constHist->name() +
           " test TofUp:" + toString(_userTofRange.second) + " binTofLow:" +
           toString(binTofLow) + " binTofUp:" + toString(binTofUp) +
           "'. Conversion parameters e0:" + toString(e0) + " t0(bin):" +
           toString(t0) + "(" + toString(bint0) + ") alpha:"+ toString(alpha) +
           ", NbrBins:"+ toString(NbrBins) + ". Condition on postprocessor '" +
           _condition->name() + "'");
}

double pp408::calcEtoTof (double energy)
{
  if (energy<0) energy = 0;
  if ((energy + ediff) < 0)
    return -1;
  return (alpha / sqrt(energy + ediff) + t0);
}
double pp408::calcTofValue(const double tofPos, const size_t binlow ,const double bin_size, const HistogramFloatBase& TofHisto)
{
  const AxisProperty &xaxis(TofHisto.axis()[HistogramBackend::xAxis]);
  const HistogramFloatBase::storage_t& Tof(TofHisto.memory());

  return (Tof[binlow] +(Tof[binlow+1]-Tof[binlow])/bin_size * (tofPos - xaxis.position(binlow)) );
}

void pp408::ToftoEnergy(const HistogramFloatBase& TofHisto , HistogramFloatBase::storage_t& Energy, double offset)
{
  const AxisProperty &xaxis(TofHisto.axis()[HistogramBackend::xAxis]);
  const HistogramFloatBase::storage_t& Tof(TofHisto.memory());


  double bin_size=xaxis.position(2)-xaxis.position(1);

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

    double tofValueUp = calcTofValue(tofBinUp,klow,bin_size,TofHisto);
    double tofValueLow = calcTofValue(tofBinLow,klow,bin_size,TofHisto);

    if (klow == kup) Energy[i] = (tofBinUp - tofBinLow)*((tofValueUp+tofValueLow)/2-offset)/bin_size;

    else if ((kup - klow) == 1 )
      Energy[i] = (xaxis.position(klow + 1) - tofBinLow)*((Tof[klow + 1]+tofValueLow)/2-offset)/bin_size
          + (tofBinUp - xaxis.position(kup))*((Tof[kup]+tofValueUp)/2-offset)/bin_size;

    else if ((kup - klow) > 1 )
    {
      Energy[i] = (xaxis.position(klow + 1) - tofBinLow)*((Tof[klow + 1]+tofValueLow)/2-offset)/bin_size
          + (tofBinUp - xaxis.position(kup))*((Tof[kup]+tofValueUp)/2-offset)/bin_size;

      for ( size_t j = klow + 1; j < kup; ++j)
        Energy[i] += (Tof[j]-offset);
    }
  }
}

void pp408::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase& input
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));
  const Histogram0DFloat &constHist
      (dynamic_cast<const Histogram0DFloat&>(_constHist->result(evt.id())));
  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  HistogramFloatBase::storage_t& output(result.memory());

  QReadLocker lock(&input.lock);
  QReadLocker lock2(&constHist.lock);

  ediff = e0 + (constHist.getValue());

  offset = 0;
  for (size_t i = bintb1; i<=bintb2;i++){
    offset+=input.memory()[i];
  }
  offset = offset/(bintb2-bintb1+1);

  ToftoEnergy( input, output,offset );

  result.nbrOfFills()=1;
}



// *** postprocessor 410 calculate covariance ***

pp410::pp410(const name_t &name)
  : AccumulatingPostProcessor(name)
{
  loadSettings(0);
}

void pp410::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _pHist = setupDependency("HistName");
  _ave = setupDependency("AveHistName");
  bool ret (setupCondition());
  if (!(ret && _pHist && _ave))
    return;
  const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_pHist->result()));
  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(one.axis()[HistogramBackend::xAxis].nbrBins(),
                                 one.axis()[HistogramBackend::xAxis].lowerLimit(),
                                 one.axis()[HistogramBackend::xAxis].upperLimit(),
                                 one.axis()[HistogramBackend::xAxis].nbrBins(),
                                 one.axis()[HistogramBackend::xAxis].lowerLimit(),
                                 one.axis()[HistogramBackend::xAxis].upperLimit(),
                                 one.axis()[HistogramBackend::xAxis].title(),
                                 one.axis()[HistogramBackend::xAxis].title())));
  Log::add(Log::INFO,"Postprocessor '" + name() + "'Calculate variance '"+
           _pHist->name() + "'. Condition on postprocessor '" + _condition->name() + "'");
}

void pp410::calcCovariance(const HistogramFloatBase::storage_t& data ,
                                 const HistogramFloatBase::storage_t& averageOld,
                                 const HistogramFloatBase::storage_t& averageNew,
                                 HistogramFloatBase::storage_t& variance,float n)
{
  const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_pHist->result()));
  size_t nbrBins(one.axis()[HistogramBackend::xAxis].nbrBins());
  for (unsigned int i=0; i<nbrBins; i++)
    for (unsigned int j=0; j<nbrBins; j++)
    {
      variance[i*nbrBins+j] = ((variance[i*nbrBins+j]*(n-1)) + (data[i]-averageOld[i])* (data[j]-averageNew[j]))/n;
    }
}

void pp410::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->result(evt.id())));
  const HistogramFloatBase& ave
      (dynamic_cast<const HistogramFloatBase&>(_ave->result(evt.id())));
  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));

  QReadLocker lock(&one.lock);
  QReadLocker lock2(&ave.lock);

  HistogramFloatBase::storage_t averagePre(ave.memory().size());

  ++result.nbrOfFills();
  float scale = 1./result.nbrOfFills();

  transform(one.memory().begin(),one.memory().end(),
            ave.memory().begin(),
            averagePre.begin(),
            PreAverage(scale));

  calcCovariance(one.memory(),
                 averagePre,
                 ave.memory(),
                 result.memory(),1./scale);
}







//------------pp412 calculate covariance for intensity correction

pp412::pp412(const name_t &name)
  : AccumulatingPostProcessor(name)
{
  loadSettings(0);
}

void pp412::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(QString::fromStdString(name()));
  setupGeneral();
  _hist1D = setupDependency("HistName1D");
  _ave1D = setupDependency("AveHistName1D");
  _hist0D = setupDependency("HistName0D");
  _ave0D = setupDependency("AveHistName0D");
  bool ret (setupCondition());
  if (!(ret && _hist1D && _ave1D && _hist0D && _ave0D))
    return;
  createHistList(_hist1D->result().copy_sptr());
  Log::add(Log::INFO,"Postprocessor '" + name() + "'Calcurate variance '" +
           _hist1D->name() + "'. Condition on postprocessor '" + _condition->name() + "'");
}

void pp412::calcCovariance(const HistogramFloatBase::storage_t& waveTrace ,
                                 const HistogramFloatBase::storage_t& waveTraceAve ,
                                 const float intensity,
                                 const float intensityAveOld,
                                 HistogramFloatBase::storage_t& correction,float n)
{
  const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_hist1D->result()));
  size_t nbrBins(one.axis()[HistogramBackend::xAxis].nbrBins());

  for (unsigned int i=0; i<nbrBins; i++)
  {
    correction[i] = (correction[i]*(n-1) + (waveTrace[i]-waveTraceAve[i]) * (intensity-intensityAveOld))/n;
  }
}

void pp412::process(const CASSEvent& evt, HistogramBackend &res)
{
  const HistogramFloatBase& waveTrace
      (dynamic_cast<const HistogramFloatBase&>(_hist1D->result(evt.id())));
  const HistogramFloatBase& waveTraceAve
      (dynamic_cast<const HistogramFloatBase&>(_ave1D->result(evt.id())));
  const Histogram0DFloat& intensity
      (dynamic_cast<const Histogram0DFloat&>(_hist0D->result(evt.id())));
  const Histogram0DFloat& intensityAve
      (dynamic_cast<const Histogram0DFloat&>(_ave0D->result(evt.id())));
  Histogram1DFloat &result(dynamic_cast<Histogram1DFloat&>(res));


  QReadLocker lock(&waveTrace.lock);
  QReadLocker lock1(&waveTraceAve.lock);
  QReadLocker lock2(&intensity.lock);
  QReadLocker lock3(&intensityAve.lock);

  ++result.nbrOfFills();
  float scale = 1./result.nbrOfFills();

  float intensityAvePre = intensityAve.getValue() - scale*(intensity.getValue() - intensityAve.getValue());

  calcCovariance(waveTrace.memory(),waveTraceAve.memory(),
                 intensity.getValue(),intensityAvePre,
                 result.memory(),result.nbrOfFills());
}
