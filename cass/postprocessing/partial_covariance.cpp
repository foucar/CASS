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

using namespace cass;


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

pp400::pp400(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp400::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->getHist(0)));
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

  _result = new Histogram1DFloat(NbrBins,pow(alpha/(_userTofRange.second-t0),2)-e0,pow(alpha/(_userTofRange.first-t0),2)-e0);//
  createHistList(2*NbrOfWorkers);

  cout<<endl<<"PostProcessor '"<<_key
      <<"' converts ToF into Energy scale '" << _pHist->key()
      <<"' which has dimension '"<<one.dimension()<<" test TofUp:"<<_userTofRange.second<<" binTofLow:"<<binTofLow<<" binTofUp:"<<binTofUp
      <<"'. Conversion parameters e0:"<<e0<<" t0(bin):"<<t0<<"("<<bint0<<")"<<" alpha:"<<alpha
      <<", NbrBins:"<<NbrBins
      <<". Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
}

void pp400::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;

  const AxisProperty &xaxis(in->axis()[HistogramBackend::xAxis]);
  binTofLow=xaxis.bin(_userTofRange.first);
  binTofUp=xaxis.bin(_userTofRange.second);
  bint0 = xaxis.bin(t0);
  bintb1=xaxis.bin(tb1);
  bintb2=xaxis.bin(tb2);

  _result = new Histogram1DFloat(NbrBins,pow(alpha/(_userTofRange.second-t0),2)-e0,pow(alpha/(_userTofRange.first-t0),2)-e0);
  createHistList(2*NbrOfWorkers);

  cout<<endl<<"PostProcessor '"<<_key
      <<"' converts ToF into Energy scale '" << _pHist->key()
      <<"' which has dimension '"<<in->dimension()<<"  TofUp:"<<_userTofRange.second<<" binTofLow:"<<binTofLow<<" binTofUp:"<<binTofUp
      <<"'. Conversion parameters e0:"<<e0<<" t0(bin):"<<t0<<"("<<bint0<<")"<<" alpha:"<<alpha
      <<", NbrBins:"<<NbrBins
      <<". Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
}


void pp400::process(const CASSEvent& evt)
{
  using namespace std;
  const HistogramFloatBase& input
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));

  HistogramFloatBase::storage_t& output
      (dynamic_cast<Histogram1DFloat*>(_result)->memory());

  input.lock.lockForRead();
  _result->lock.lockForWrite();

  offset = 0;
  for (size_t i = bintb1; i<=bintb2;i++){
    offset+=input.memory()[i];
  }
  offset = offset/(bintb2-bintb1+1);

  ToftoEnergy( input, output,offset );

  _result->nbrOfFills()=1;
  _result->lock.unlock();
  input.lock.unlock();
}

//----------------------------------

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


    //      std::cout<<" kup:"<<kup<<" klow:"<<klow<<std::endl;

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




// *** postprocessor 401 calculate variance ***

pp401::pp401(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp401::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  //unsigned average = settings.value("NbrOfVariance", 0).toUInt();
  //_alpha =  average ? 2./static_cast<float>(average+1.) : 0.;
  _pHist = setupDependency("HistName");
  _ave = setupDependency("AveHistName");
  bool ret (setupCondition());
  if (!(ret && _pHist && _ave))
    return;
  const HistogramBackend &one(_pHist->getHist(0));
  _result = one.clone();
  createHistList(2*NbrOfWorkers,true);
  cout<<endl<<"Postprocessor '"<<_key
     <<"'Calcurate variance '"<< _pHist->key()
   <<"'. Condition on postprocessor '"<<_condition->key()<<"'"
  <<endl;
}

void pp401::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  //the previous _result pointer is on the histlist and will be deleted
  //with the call to createHistList
  _result = in->clone();
  createHistList(2*NbrOfWorkers,true);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  VERBOSEOUT(cout<<"Postprocessor '"<<_key
             <<"': histograms changed => delete existing histo"
             <<" and create new one from input"<<endl);
}

void pp401::process(const CASSEvent& evt)
{
  using namespace std;
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));

  const HistogramFloatBase& ave
      (dynamic_cast<const HistogramFloatBase&>((*_ave)(evt)));

  HistogramFloatBase::storage_t averagePre(ave.memory().size());

  one.lock.lockForRead();
  ave.lock.lockForRead();
  _result->lock.lockForWrite();
  ++_result->nbrOfFills();
  float scale = 1./_result->nbrOfFills();

  transform(one.memory().begin(),one.memory().end(),
            ave.memory().begin(),
            averagePre.begin(),
            PreAverage(scale));

  calcVariance(one.memory(),
               averagePre,
               ave.memory(),
               dynamic_cast<HistogramFloatBase*>(_result)->memory(),1./scale);


  _result->lock.unlock();
  one.lock.unlock();
  ave.lock.unlock();
}

void pp401::calcVariance(const HistogramFloatBase::storage_t& data ,
                               const HistogramFloatBase::storage_t& averageOld,
                               const HistogramFloatBase::storage_t& averageNew,
                               HistogramFloatBase::storage_t& variance,float n)
{
  for (unsigned int i=0; i<data.size(); i++)
  {
    variance[i] = ((variance[i]*(n-1)) + (data[i]-averageOld[i])* (data[i]-averageNew[i]))/n;
  }
}

// *** postprocessor 402 square averages histograms ***

pp402::pp402(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp402::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  unsigned average = settings.value("NbrOfAverages", 1).toUInt();
  _alpha =  average ? 2./static_cast<float>(average+1.) : 0.;
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramBackend &one(_pHist->getHist(0));
  _result = one.clone();
  createHistList(2*NbrOfWorkers,true);
  cout<<endl<<"Postprocessor '"<<_key
     <<"' sqaverages histograms from PostProcessor '"<< _pHist->key()
    <<"' alpha for the averaging '"<<_alpha
   <<"'. Condition on postprocessor '"<<_condition->key()<<"'"
  <<endl;
}

void pp402::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  //the previous _result pointer is on the histlist and will be deleted
  //with the call to createHistList
  _result = in->clone();
  createHistList(2*NbrOfWorkers,true);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  VERBOSEOUT(cout<<"Postprocessor '"<<_key
             <<"': histograms changed => delete existing histo"
             <<" and create new one from input"<<endl);
}

void pp402::process(const CASSEvent& evt)
{
  using namespace std;
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));
  one.lock.lockForRead();
  _result->lock.lockForWrite();
  ++_result->nbrOfFills();
  float scale = (1./_result->nbrOfFills() < _alpha) ?
        _alpha :
        1./_result->nbrOfFills();
  transform(one.memory().begin(),one.memory().end(),
            dynamic_cast<HistogramFloatBase*>(_result)->memory().begin(),
            dynamic_cast<HistogramFloatBase*>(_result)->memory().begin(),
            SqAverage(scale));
  _result->lock.unlock();
  one.lock.unlock();
}



// ***  pp 403 subsets a histogram and bins***

pp403::pp403(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp403::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->getHist(0)));
  if (0 == one.dimension())
    throw runtime_error("pp70::loadSettings(): Unknown dimension of incomming histogram");
  _userXRange = make_pair(settings.value("XLow",0).toFloat(),
                          settings.value("XUp",1).toFloat());

  _userBinSize = settings.value("BSize",1).toUInt();

  const AxisProperty &xaxis(one.axis()[HistogramBackend::xAxis]);
  _inputOffset=(xaxis.bin(_userXRange.first));
  const size_t binXUp (xaxis.bin(_userXRange.second));

  const size_t nXBins (static_cast<size_t>((binXUp-_inputOffset)/_userBinSize));

  const float xLow (xaxis.position(_inputOffset));
  const float xUp (xaxis.position(binXUp));
  if (1 == one.dimension())
  {
    _result = new Histogram1DFloat(nXBins,xLow,xUp);
  }
  else if (2 == one.dimension())
    throw invalid_argument("PostProcessor '" + _key + "' only operates on 1d histograms.");

  createHistList(2*NbrOfWorkers);

  cout<<endl<<"PostProcessor '"<<_key
     <<"' returns a subset of histogram in pp '" << _pHist->key()
    <<"' which has dimension '"<<one.dimension()
   <<"'. Subset is xLow:"<<xLow<<"("<<_inputOffset<<")"
  <<", xUp:"<<xUp<<"("<<binXUp<<")"
  <<", xNbrBins:"<<nXBins
  <<". Condition on postprocessor '"<<_condition->key()<<"'"
  <<endl;
}

void pp403::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  const AxisProperty &xaxis(in->axis()[HistogramBackend::xAxis]);
  _inputOffset=(xaxis.bin(_userXRange.first));
  const size_t binXUp (xaxis.bin(_userXRange.second));

  const size_t nXBins (static_cast<size_t>((binXUp-_inputOffset)/_userBinSize));

  const float xLow (xaxis.position(_inputOffset));
  const float xUp (xaxis.position(binXUp));
  if (1 == in->dimension())
    _result = new Histogram1DFloat(nXBins,xLow,xUp);
  else if (2 == in->dimension())
    throw invalid_argument("PostProcessor: '" + _key +"' only works on 1d histograms.");
  createHistList(2*NbrOfWorkers);
  cout<<endl<<"PostProcessor '"<<_key
     <<"' histogramsChanged: returns a subset of histogram in pp '" << _pHist->key()
    <<"' which has dimension '"<<in->dimension()
   <<"'. Subset is xLow:"<<xLow<<"("<<_inputOffset<<")"
  <<", xUp:"<<xUp<<"("<<binXUp<<")"
  <<", xNbrBins:"<<nXBins
  <<". Condition on postprocessor '"<<_condition->key()<<"'"
  <<endl;
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
}


void pp403::process(const CASSEvent& evt)
{
  using namespace std;
  const HistogramFloatBase& input
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));
  input.lock.lockForRead();
  _result->lock.lockForWrite();
  const HistogramFloatBase::storage_t &imem (input.memory());
  HistogramFloatBase::storage_t::const_iterator iit
      (imem.begin()+_inputOffset);
  HistogramFloatBase::storage_t &rmem
      (dynamic_cast<HistogramFloatBase*>(_result)->memory());
  HistogramFloatBase::storage_t::iterator rit(rmem.begin());

  const AxisProperty &xaxis(input.axis()[HistogramBackend::xAxis]);
  const size_t binXUp (xaxis.bin(_userXRange.second));
  for (size_t xBins=_inputOffset; xBins<binXUp; ++xBins)
  {
    rmem[static_cast<size_t>((xBins-_inputOffset)/_userBinSize)]=0.;
  }
  for (size_t xBins=_inputOffset; xBins<binXUp; ++xBins)
  {
    rmem[static_cast<size_t>((xBins-_inputOffset)/_userBinSize)]=rmem[static_cast<size_t>((xBins-_inputOffset)/_userBinSize)]+imem[xBins];
  }
  _result->nbrOfFills()=1;
  _result->lock.unlock();
  input.lock.unlock();
}



//*** Tof to Mass to Charge Ratio****
pp404::pp404(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp404::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->getHist(0)));
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

  cout<<endl<<"PostProcessor '"<<_key
     <<"' converts ToF into MassTo ChargeRatio scale'" << _pHist->key()
    <<"' which has dimension '"<<one.dimension()<<"  TofUp:"<<_userTofRange.second<<" binTofLow:"<<binTofLow<<" binTofUp:"<<binTofUp
   <<"'. Conversion parameters MtC0:"<<MtC0<<" t0(bin):"<<t0<<"'. Conversion parameters MtC1:"<<MtC1<<" t1(bin):"<<t1
  <<", NbrBins:"<<NbrBins;

  _result = new Histogram1DFloat(NbrBins,pow(_userTofRange.first/alpha-beta,2),pow(_userTofRange.second/alpha-beta,2));//

  cout <<". Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
  createHistList(2*NbrOfWorkers);
}

void pp404::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;

  const AxisProperty &xaxis(in->axis()[HistogramBackend::xAxis]);
  binTofLow=xaxis.bin(_userTofRange.first);
  binTofUp=xaxis.bin(_userTofRange.second);
  bintb1=xaxis.bin(tb1);
  bintb2=xaxis.bin(tb2);


  cout<<endl<<"PostProcessor '"<<_key
     <<"' converts ToF into MassToChargeRatio scale'" << _pHist->key()
    <<"' which has dimension '"<<in->dimension()<<"  TofUp:"<<_userTofRange.second<<" binTofLow:"<<binTofLow<<" binTofUp:"<<binTofUp
   <<"'. Conversion parameters MtC0:"<<MtC0<<" t0(bin):"<<t0<<"'. Conversion parameters MtC1:"<<MtC1<<" t1(bin):"<<t1
  <<", NbrBins:"<<NbrBins;


  _result = new Histogram1DFloat(NbrBins,pow(_userTofRange.first/alpha-beta,2),pow(_userTofRange.second/alpha-beta,2));

  cout <<". Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
  createHistList(2*NbrOfWorkers);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
}


void pp404::process(const CASSEvent& evt)
{
  using namespace std;
  const HistogramFloatBase& input
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));

  HistogramFloatBase::storage_t& output
      (dynamic_cast<Histogram1DFloat*>(_result)->memory());

  input.lock.lockForRead();
  _result->lock.lockForWrite();

  offset = 0;
  for (size_t i = bintb1; i<=bintb2;i++){
    offset+=input.memory()[i];
  }
  offset = offset/(bintb2-bintb1+1);

  ToftoMtC( input, output,offset );

  _result->nbrOfFills()=1;
  _result->lock.unlock();
  input.lock.unlock();
}

//----------------------------------

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
  //    const size_t binTofLow=xaxis.bin(_userTofRange.first);
  //    const size_t binTofUp=xaxis.bin(_userTofRange.second);

  //HistogramFloatBase::storage_t& output
  //(dynamic_cast<Histogram1DFloat*>(_result)->memory());
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
    //std::cout << "klow: "<<klow << std::endl;
    //std::cout << "kup: "<<kup << std::endl;
    //klow = static_cast<long>(tofBinLow);
    //kup = static_cast<long>(tofBinUp);


    if (klow == kup) {
      MtC[i] =  (Tof[klow]-offset)*(tofBinUp-tofBinLow)/bin_size;
      //std::cout << Tof[klow] << std:: endl;
      //std::cout << tofBinUp-tofBinLow << std:: endl;
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


// *** postprocessors 405 calcs pulse duration from bld ***

pp405::pp405(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp405::loadSettings(size_t)
{
  if (!setupCondition())
    return;
  _result = new Histogram0DFloat();
  setupGeneral();
  createHistList(2*NbrOfWorkers);
  std::cout << "PostProcessor: "<<_key
            <<" calc pulse duration from beamline data"
           <<". Condition is"<<_condition->key()
          <<std::endl;
}

void pp405::process(const CASSEvent& evt)
{
  using namespace MachineData;
  const MachineDataDevice *mdev
      (dynamic_cast<const MachineDataDevice *>
       (evt.devices().find(CASSEvent::MachineData)->second));
  const MachineDataDevice::bldMap_t bld(mdev->BeamlineData());

  const double ebCharge
      (bld.find("EbeamCharge") == bld.end() ? 0 : bld.find("EbeamCharge")->second);
  const double peakCurrent
      (bld.find("EbeamPkCurrBC2") == bld.end() ? 0 : bld.find("EbeamPkCurrBC2")->second);

  const double puleduration (ebCharge/peakCurrent*1.0e-9);

  _result->lock.lockForWrite();
  *dynamic_cast<Histogram0DFloat*>(_result) = puleduration;
  //  std::cout << "puleduration:" <<puleduration<< " EbeamChage:"<<ebCharge<<" EbeamPkCurrBC2:" <<peakCurrent <<  std::endl;
  _result->lock.unlock();
}






// ***  pp 406 ToF to Energy correct from 0D Histogram value***

pp406::pp406(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp406::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  _pHist = setupDependency("HistName");
  _constHist = setupDependency("HistZeroD");
  bool ret (setupCondition());
  if (!(ret && _pHist && _constHist ))
    return;
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->getHist(0)));
  if (1 != one.dimension())
    throw runtime_error("pp406::loadSettings(): Unknown dimension of incomming histogram");
  const HistogramFloatBase &constHist(dynamic_cast<const HistogramFloatBase&>(_constHist->getHist(0)));
  if (constHist.dimension() != 0 )
  {
    stringstream ss;
    ss << "pp406::loadSettings(): HistZeroD '"<<constHist.key()
       <<"' is not a 0D histogram";
    throw invalid_argument(ss.str());
  }

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

  cout<<endl<<"PostProcessor '"<<_key
     <<"' converts ToF into Energy scale '" << _pHist->key()
    <<"' which has dimension '"<<one.dimension()
   <<"' with constant in 0D Histogram in PostProcessor '" << _constHist->key()
  <<" test TofUp:"<<_userTofRange.second<<" binTofLow:"<<binTofLow<<" binTofUp:"<<binTofUp
  <<"'. Conversion parameters e0:"<<e0<<" t0(bin):"<<t0<<"("<<bint0<<")"<<" alpha:"<<alpha
  <<", NbrBins:"<<NbrBins;

  _result = new Histogram1DFloat(NbrBins,pow(alpha/(_userTofRange.second-t0),2)-e0,pow(alpha/(_userTofRange.first-t0),2)-e0);//

  cout <<". Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
  createHistList(2*NbrOfWorkers);
}

void pp406::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  //return when the incomming histogam is the 0D histogram that contains
  //the constant for the operation
  if (in->key() == _constHist->key())
    return;

  const AxisProperty &xaxis(in->axis()[HistogramBackend::xAxis]);
  binTofLow=xaxis.bin(_userTofRange.first);
  binTofUp=xaxis.bin(_userTofRange.second);
  bint0 = xaxis.bin(t0);
  bintb1=xaxis.bin(tb1);
  bintb2=xaxis.bin(tb2);


  cout<<endl<<"PostProcessor '"<<_key
     <<"' converts ToF into Energy scale '" << _pHist->key()
    <<"' which has dimension '"<<in->dimension()
   <<"' with constant in 0D Histogram in PostProcessor '" << _constHist->key()
  <<"  TofUp:"<<_userTofRange.second<<" binTofLow:"<<binTofLow<<" binTofUp:"<<binTofUp
  <<"'. Conversion parameters e0:"<<e0<<" t0(bin):"<<t0<<"("<<bint0<<")"<<" alpha:"<<alpha
  <<", NbrBins:"<<NbrBins;


  _result = new Histogram1DFloat(NbrBins,pow(alpha/(_userTofRange.second-t0),2)-e0,pow(alpha/(_userTofRange.first-t0),2)-e0);

  cout <<". Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
  createHistList(2*NbrOfWorkers);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
}


void pp406::process(const CASSEvent& evt)
{
  using namespace std;
  const HistogramFloatBase& input
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));
  const Histogram0DFloat &constHist
      (dynamic_cast<const Histogram0DFloat&>((*_constHist)(evt)));

  HistogramFloatBase::storage_t& output
      (dynamic_cast<Histogram1DFloat*>(_result)->memory());

  input.lock.lockForRead();
  constHist.lock.lockForRead();
  _result->lock.lockForWrite();
  ediff = e0 + (constHist.getValue());

  offset = 0;
  for (size_t i = bintb1; i<=bintb2;i++){
    offset+=input.memory()[i];
  }
  offset = offset/(bintb2-bintb1+1);

  ToftoEnergy( input, output,offset );

  _result->nbrOfFills()=1;
  _result->lock.unlock();
  constHist.lock.unlock();
  input.lock.unlock();
}

//----------------------------------

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


    //      std::cout<<" kup:"<<kup<<" klow:"<<klow<<std::endl;

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


//***Tof to Energy linear interpolation***
pp407::pp407(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp407::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  _pHist = setupDependency("HistName");
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->getHist(0)));
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

  cout<<endl<<"PostProcessor '"<<_key
     <<"' converts ToF into Energy scale '" << _pHist->key()
    <<"' which has dimension '"<<one.dimension()<<" test TofUp:"<<_userTofRange.second<<" binTofLow:"<<binTofLow<<" binTofUp:"<<binTofUp
   <<"'. Conversion parameters e0:"<<e0<<" t0(bin):"<<t0<<"("<<bint0<<")"<<" alpha:"<<alpha
  <<", NbrBins:"<<NbrBins;

  _result = new Histogram1DFloat(NbrBins,pow(alpha/(_userTofRange.second-t0),2)-e0,pow(alpha/(_userTofRange.first-t0),2)-e0);//

  cout <<". Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
  createHistList(2*NbrOfWorkers);
}

void pp407::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;

  const AxisProperty &xaxis(in->axis()[HistogramBackend::xAxis]);
  binTofLow=xaxis.bin(_userTofRange.first);
  binTofUp=xaxis.bin(_userTofRange.second);
  bint0 = xaxis.bin(t0);
  bintb1=xaxis.bin(tb1);
  bintb2=xaxis.bin(tb2);


  cout<<endl<<"PostProcessor '"<<_key
     <<"' converts ToF into Energy scale '" << _pHist->key()
    <<"' which has dimension '"<<in->dimension()<<"  TofUp:"<<_userTofRange.second<<" binTofLow:"<<binTofLow<<" binTofUp:"<<binTofUp
   <<"'. Conversion parameters e0:"<<e0<<" t0(bin):"<<t0<<"("<<bint0<<")"<<" alpha:"<<alpha
  <<", NbrBins:"<<NbrBins;


  _result = new Histogram1DFloat(NbrBins,pow(alpha/(_userTofRange.second-t0),2)-e0,pow(alpha/(_userTofRange.first-t0),2)-e0);

  cout <<". Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
  createHistList(2*NbrOfWorkers);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
}


void pp407::process(const CASSEvent& evt)
{
  using namespace std;
  const HistogramFloatBase& input
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));

  HistogramFloatBase::storage_t& output
      (dynamic_cast<Histogram1DFloat*>(_result)->memory());

  input.lock.lockForRead();
  _result->lock.lockForWrite();

  offset = 0;
  for (size_t i = bintb1; i<=bintb2;i++){
    offset+=input.memory()[i];
  }
  offset = offset/(bintb2-bintb1+1);

  ToftoEnergy( input, output,offset );

  _result->nbrOfFills()=1;
  _result->lock.unlock();
  input.lock.unlock();
}

//----------------------------------

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


    //      std::cout<<" kup:"<<kup<<" klow:"<<klow<<std::endl;

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

// ***  pp 408 ToF to Energy correct from 0D Histogram value & linear corection***

pp408::pp408(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp408::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  _pHist = setupDependency("HistName");
  _constHist = setupDependency("HistZeroD");
  bool ret (setupCondition());
  if (!(ret && _pHist && _constHist ))
    return;
  const HistogramFloatBase &one
      (dynamic_cast<const HistogramFloatBase&>(_pHist->getHist(0)));
  if (1 != one.dimension())
    throw runtime_error("pp408::loadSettings(): Unknown dimension of incomming histogram");
  const HistogramFloatBase &constHist(dynamic_cast<const HistogramFloatBase&>(_constHist->getHist(0)));
  if (constHist.dimension() != 0 )
  {
    stringstream ss;
    ss << "pp408::loadSettings(): HistZeroD '"<<constHist.key()
       <<"' is not a 0D histogram";
    throw invalid_argument(ss.str());
  }

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

  cout<<endl<<"PostProcessor '"<<_key
     <<"' converts ToF into Energy scale '" << _pHist->key()
    <<"' which has dimension '"<<one.dimension()
   <<"' with constant in 0D Histogram in PostProcessor '" << _constHist->key()
  <<" test TofUp:"<<_userTofRange.second<<" binTofLow:"<<binTofLow<<" binTofUp:"<<binTofUp
  <<"'. Conversion parameters e0:"<<e0<<" t0(bin):"<<t0<<"("<<bint0<<")"<<" alpha:"<<alpha
  <<", NbrBins:"<<NbrBins;

  _result = new Histogram1DFloat(NbrBins,pow(alpha/(_userTofRange.second-t0),2)-e0,pow(alpha/(_userTofRange.first-t0),2)-e0);//

  cout <<". Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
  createHistList(2*NbrOfWorkers);
}

void pp408::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  //return when the incomming histogam is the 0D histogram that contains
  //the constant for the operation
  if (in->key() == _constHist->key())
    return;

  const AxisProperty &xaxis(in->axis()[HistogramBackend::xAxis]);
  binTofLow=xaxis.bin(_userTofRange.first);
  binTofUp=xaxis.bin(_userTofRange.second);
  bint0 = xaxis.bin(t0);
  bintb1=xaxis.bin(tb1);
  bintb2=xaxis.bin(tb2);


  cout<<endl<<"PostProcessor '"<<_key
     <<"' converts ToF into Energy scale '" << _pHist->key()
    <<"' which has dimension '"<<in->dimension()
   <<"' with constant in 0D Histogram in PostProcessor '" << _constHist->key()
  <<"  TofUp:"<<_userTofRange.second<<" binTofLow:"<<binTofLow<<" binTofUp:"<<binTofUp
  <<"'. Conversion parameters e0:"<<e0<<" t0(bin):"<<t0<<"("<<bint0<<")"<<" alpha:"<<alpha
  <<", NbrBins:"<<NbrBins;


  _result = new Histogram1DFloat(NbrBins,pow(alpha/(_userTofRange.second-t0),2)-e0,pow(alpha/(_userTofRange.first-t0),2)-e0);

  cout <<". Condition on postprocessor '"<<_condition->key()<<"'"
      <<endl;
  createHistList(2*NbrOfWorkers);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
}


void pp408::process(const CASSEvent& evt)
{
  using namespace std;
  const HistogramFloatBase& input
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));
  const Histogram0DFloat &constHist
      (dynamic_cast<const Histogram0DFloat&>((*_constHist)(evt)));

  HistogramFloatBase::storage_t& output
      (dynamic_cast<Histogram1DFloat*>(_result)->memory());

  input.lock.lockForRead();
  constHist.lock.lockForRead();
  _result->lock.lockForWrite();
  ediff = e0 + (constHist.getValue());

  offset = 0;
  for (size_t i = bintb1; i<=bintb2;i++){
    offset+=input.memory()[i];
  }
  offset = offset/(bintb2-bintb1+1);

  ToftoEnergy( input, output,offset );

  _result->nbrOfFills()=1;
  _result->lock.unlock();
  constHist.lock.unlock();
  input.lock.unlock();
}

//----------------------------------

double pp408::calcEtoTof (double energy)
{
  if (energy<0) energy = 0;
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



// *** postprocessor 410 calculate covariance ***

pp410::pp410(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp410::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  //unsigned average = settings.value("NbrOfVariance", 0).toUInt();
  //_alpha =  average ? 2./static_cast<float>(average+1.) : 0.;
  _pHist = setupDependency("HistName");
  _ave = setupDependency("AveHistName");
  bool ret (setupCondition());
  if (!(ret && _pHist && _ave))
    return;
  const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_pHist->getHist(0)));
  _result = new Histogram2DFloat(one.axis()[HistogramBackend::xAxis].nbrBins(),
                                 one.axis()[HistogramBackend::xAxis].lowerLimit(),
                                 one.axis()[HistogramBackend::xAxis].upperLimit(),
                                 one.axis()[HistogramBackend::xAxis].nbrBins(),
                                 one.axis()[HistogramBackend::xAxis].lowerLimit(),
                                 one.axis()[HistogramBackend::xAxis].upperLimit(),
                                 one.axis()[HistogramBackend::xAxis].title(),
                                 one.axis()[HistogramBackend::xAxis].title());
  createHistList(2*NbrOfWorkers,true);
  cout<<endl<<"Postprocessor '"<<_key
     <<"'Calcurate variance '"<< _pHist->key()
   <<"'. Condition on postprocessor '"<<_condition->key()<<"'"
  <<endl;
}

void pp410::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  //the previous _result pointer is on the histlist and will be deleted
  //with the call to createHistList
  const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_pHist->getHist(0)));
  _result = new Histogram2DFloat(one.axis()[HistogramBackend::xAxis].nbrBins(),
                                 one.axis()[HistogramBackend::xAxis].lowerLimit(),
                                 one.axis()[HistogramBackend::xAxis].upperLimit(),
                                 one.axis()[HistogramBackend::xAxis].nbrBins(),
                                 one.axis()[HistogramBackend::xAxis].lowerLimit(),
                                 one.axis()[HistogramBackend::xAxis].upperLimit(),
                                 one.axis()[HistogramBackend::xAxis].title(),
                                 one.axis()[HistogramBackend::xAxis].title());
  createHistList(2*NbrOfWorkers,true);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  VERBOSEOUT(cout<<"Postprocessor '"<<_key
             <<"': histograms changed => delete existing histo"
             <<" and create new one from input"<<endl);
}

void pp410::process(const CASSEvent& evt)
{
  using namespace std;
  const HistogramFloatBase& one
      (dynamic_cast<const HistogramFloatBase&>((*_pHist)(evt)));

  const HistogramFloatBase& ave
      (dynamic_cast<const HistogramFloatBase&>((*_ave)(evt)));

  HistogramFloatBase::storage_t averagePre(ave.memory().size());

  one.lock.lockForRead();
  ave.lock.lockForRead();
  _result->lock.lockForWrite();
  ++_result->nbrOfFills();
  float scale = 1./_result->nbrOfFills();

  transform(one.memory().begin(),one.memory().end(),
            ave.memory().begin(),
            averagePre.begin(),
            PreAverage(scale));

  calcCovariance(one.memory(),
                 averagePre,
                 ave.memory(),
                 dynamic_cast<HistogramFloatBase*>(_result)->memory(),1./scale);

  _result->lock.unlock();
  one.lock.unlock();
  ave.lock.unlock();
}

//** function for covariance.
void pp410::calcCovariance(const HistogramFloatBase::storage_t& data ,
                                 const HistogramFloatBase::storage_t& averageOld,
                                 const HistogramFloatBase::storage_t& averageNew,
                                 HistogramFloatBase::storage_t& variance,float n)
{
  const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_pHist->getHist(0)));
  size_t nbrBins(one.axis()[HistogramBackend::xAxis].nbrBins());
  for (unsigned int i=0; i<nbrBins; i++)
    for (unsigned int j=0; j<nbrBins; j++)
    {
      variance[i*nbrBins+j] = ((variance[i*nbrBins+j]*(n-1)) + (data[i]-averageOld[i])* (data[j]-averageNew[j]))/n;
    }
}

//------------pp412 calculate covariance for intensity correction

pp412::pp412(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp412::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  //    unsigned average = settings.value("NbrOfVariance", 0).toUInt();
  //    _alpha =  average ? 2./static_cast<float>(average+1.) : 0.;
  _hist1D = setupDependency("HistName1D");
  _ave1D = setupDependency("AveHistName1D");
  _hist0D = setupDependency("HistName0D");
  _ave0D = setupDependency("AveHistName0D");
  bool ret (setupCondition());
  if (!(ret && _hist1D && _ave1D && _hist0D && _ave0D))
    return;
  const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_hist1D->getHist(0)));
  _result = one.clone();
  createHistList(2*NbrOfWorkers,true);
  cout<<endl<<"Postprocessor '"<<_key
     <<"'Calcurate variance '"<< _hist1D->key()
       //      <<"' alpha for the averaging '"<<_alpha
    <<"'. Condition on postprocessor '"<<_condition->key()<<"'"
   <<endl;
}

void pp412::histogramsChanged(const HistogramBackend* in)
{
  using namespace std;
  QWriteLocker lock(&_histLock);
  //return when there is no incomming histogram
  if(!in)
    return;
  //return when the incomming histogram is not a direct dependant
  if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
    return;
  //the previous _result pointer is on the histlist and will be deleted
  //with the call to createHistList
  const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_hist1D->getHist(0)));
  _result = one.clone();
  createHistList(2*NbrOfWorkers,true);
  //notify all pp that depend on us that our histograms have changed
  PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
  PostProcessors::keyList_t::iterator it (dependands.begin());
  for (; it != dependands.end(); ++it)
    _pp.getPostProcessor(*it).histogramsChanged(_result);
  VERBOSEOUT(cout<<"Postprocessor '"<<_key
             <<"': histograms changed => delete existing histo"
             <<" and create new one from input"<<endl);
}

void pp412::process(const CASSEvent& evt)
{
  using namespace std;
  const HistogramFloatBase& waveTrace
      (dynamic_cast<const HistogramFloatBase&>((*_hist1D)(evt)));
  const HistogramFloatBase& waveTraceAve
      (dynamic_cast<const HistogramFloatBase&>((*_ave1D)(evt)));
  const Histogram0DFloat& intensity
      (dynamic_cast<const Histogram0DFloat&>((*_hist0D)(evt)));
  const Histogram0DFloat& intensityAve
      (dynamic_cast<const Histogram0DFloat&>((*_ave0D)(evt)));


  waveTrace.lock.lockForRead();
  waveTraceAve.lock.lockForRead();
  intensity.lock.lockForRead();
  intensityAve.lock.lockForRead();
  _result->lock.lockForWrite();
  ++_result->nbrOfFills();
  float scale = 1./_result->nbrOfFills();

  float intensityAvePre = intensityAve.getValue() - scale*(intensity.getValue() - intensityAve.getValue());

  calcCovariance(waveTrace.memory(),waveTraceAve.memory(),
                 intensity.getValue(),intensityAvePre,
                 dynamic_cast<HistogramFloatBase*>(_result)->memory(),_result->nbrOfFills());

  _result->lock.unlock();
  intensityAve.lock.unlock();
  intensity.lock.unlock();
  waveTraceAve.lock.unlock();
  waveTrace.lock.unlock();
}

//** function for calc correction map
void pp412::calcCovariance(const HistogramFloatBase::storage_t& waveTrace ,
                                 const HistogramFloatBase::storage_t& waveTraceAve ,
                                 const float intensity,
                                 const float intensityAveOld,
                                 HistogramFloatBase::storage_t& correction,float n)
{
  const HistogramFloatBase &one(dynamic_cast<const HistogramFloatBase&>(_hist1D->getHist(0)));
  size_t nbrBins(one.axis()[HistogramBackend::xAxis].nbrBins());

  for (unsigned int i=0; i<nbrBins; i++)
  {
    correction[i] = (correction[i]*(n-1) + (waveTrace[i]-waveTraceAve[i]) * (intensity-intensityAveOld))/n;
  }
}

// *** postprocessors 420 indicate number of event  ***

pp420::pp420(PostProcessors& pp, const PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key)
{
  loadSettings(0);
}

void pp420::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());
  setupGeneral();
  //    _pHist = setupDependency("HistName");
  if (!setupCondition() /*&& _pHist*/)
    return;
  freq = settings.value("Frequency",1).toUInt();
  _result = new Histogram0DFloat();
  createHistList(2*NbrOfWorkers,true);
  cout << "PostProcessor: "<<_key
       <<" indicate the number of event"
       <<". Condition is"<<_condition->key()
       <<" Frequency:"<<freq
       <<endl;
}

void pp420::process(const CASSEvent& /*evt*/)
{

  _result->lock.lockForWrite();
  ++_result->nbrOfFills();
  *dynamic_cast<Histogram0DFloat*>(_result) = _result->nbrOfFills();
  _result->lock.unlock();

  if (_result->nbrOfFills() % freq == 0)
    std::cout <<" Event Counter :" << _result->nbrOfFills()<<std::endl;

}
