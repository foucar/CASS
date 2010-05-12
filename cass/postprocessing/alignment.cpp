// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2010 Lutz Foucar

#include <algorithm>
#include <cassert>
#include <cmath>
#include <list>
#include <map>
#include <cmath>
#include <cstdlib>

#include "postprocessing/alignment.h"
#include "cass_event.h"
#include "histogram.h"



namespace cass
{
/*! Helper for postprocessors 131,

This class actually does all the work; see individual postprocessors for details.
*/
class helper_alignment_2
{
public:

    /*! create the instance if not it does not exist already */
    static helper_alignment_2 * instance();

    /*! destroy the instance */
    static void destroy();

    /*! center row of image */
    float center_row(const CASSEvent& event) { validate(event); return _values[event.id()][0]; };

    /*! center column of image */
    float center_column(const CASSEvent& event) { validate(event); return _values[event.id()][1]; };

    /*! \f$<cos^2\theta>\f$ */
    float cos2theta(const CASSEvent& event) { validate(event); return _values[event.id()][2]; };

    /*! Postprocessor dependencies of this helper */
    static std::list<PostProcessors::id_t> dependencies() {
        return std::list<PostProcessors::id_t>(1, PostProcessors::VmiRunningAverage);
    };


protected:

    /*! Validate and, if necessary, process event

    @param event CASSEvent to process by us
    */
    void validate(const CASSEvent& event) {
        if(_processed.end() == find(_processed.begin(), _processed.end(), event.id())) {
            process(event);
            while(5 < _processed.size()) {
                _values.erase(_processed.back());
                _processed.pop_back();
            }
        }
    };

    /*! Process event

    Do the actual calculations to derive all relevant paramters.
    This method must be called through validate in order to correctly clean up _values.
    @param event CASSEvent to process by us
    */
    void process(const CASSEvent& event);

    /*! list of processed (and available) event ids

    This is our houskeeping list for deleting the oldest ids in order.
    Newest events are added in front, whereas the oldest (the last)
    events are delted when appropriate.
    */
    std::list<uint64_t> _processed;

    /*! map of values for the available ids

    For each id we determine a vector of floats:
    0 -- center row
    1 -- center column
    2 -- \f$<cos^2\theta>\f$
    */
    std::map<uint64_t, std::vector<float> > _values;


private:

    /*! Private constructor of singleton */
    helper_alignment_2() {};

    /*! Prevent copy-construction of singleton */
    helper_alignment_2(const helper_alignment_2&);

    /*! Prevent assignment (potentially resulting in a copy) of singleton */
    helper_alignment_2& operator=(const helper_alignment_2);

    /*! Prevent destruction unless going through destroy */
    ~helper_alignment_2() {};

    /*! pointer to the singleton instance */
    static helper_alignment_2 *_instance;

    /*! Singleton operation locker */
    static QMutex _mutex;
};
helper_alignment_2 *helper_alignment_2::_instance(0);
QMutex helper_alignment_2::_mutex;
helper_alignment_2 * helper_alignment_2::instance() {
    QMutexLocker locker(&_mutex);
    if(0 == _instance)
        _instance = new helper_alignment_2();
    return _instance;
}
void helper_alignment_2::destroy() {
    QMutexLocker locker(&_mutex);
    delete _instance;
    _instance = 0;
}
void helper_alignment_2::process(const CASSEvent& event)
{
    // determine (and store) center position of image
    std::pair<float, float> center(0., 0.); // (_histograms[PostProcessors::VmiRunningAverage].center());
    _values[event.id()][0] = center.first;
    _values[event.id()][1] = center.second;
    // calculate <cos2theta>
    _values[event.id()][2] = 0.5;
    _processed.push_front(event.id());
};




/*! Helper for postprocessors 143-144

This class actually does all the work; see individual postprocessors for details.
*/
class helper_alignment_1
{
public:

    /*! create the instance if not it does not exist already */
    static helper_alignment_1 * instance();

    /*! destroy the instance */
    static void destroy();

    /*! Gaussian width */
    float cos2theta(const CASSEvent& event) { validate(event); return _values[event.id()].first; };

    /*! Gaussian width */
    float width(const CASSEvent& event) { validate(event); return _values[event.id()].first; };

    /*! Gaussian height */
    float height(const CASSEvent& event) { validate(event); return _values[event.id()].second; };

    /*! Postprocessor dependencies of this helper */
    static std::list<PostProcessors::id_t> dependencies() {
        return std::list<PostProcessors::id_t>(1, PostProcessors::VmiRunningAverage);
    };


protected:

    /*! Validate and, if necessary, process event

    @param event CASSEvent to process by us
    */
    void validate(const CASSEvent& event) {
        if(_processed.end() == find(_processed.begin(), _processed.end(), event.id())) {
            process(event);
            while(5 < _processed.size()) {
                _values.erase(_processed.back());
                _processed.pop_back();
            }
        }
    };

    /*! Process event

    Calculate the following properties of the given histogram:
    - Gaussian width
    - Gaussian height

    @param event CASSEvent to process by us
    */
    void process(const CASSEvent& event) {
        // reduce 2d histogram to row- and column-integrated 1d histograms,
        // perform Gauss-fits to 1d histograms,and store FWHM
        _values[event.id()] = std::make_pair(float(0.), float(0.));
        _processed.push_front(event.id());
    };

    /*! list of processed (and available) event ids

    This is our houskeeping list for deleting the oldest ids in order.
    Newest events are added in front, whereas the oldest (the last)
    events are delted when appropriate.
    */
    std::list<uint64_t> _processed;

    /*! map of values for the available ids */
    std::map<uint64_t, std::pair<float, float> > _values;


private:

    /*! Private constructor of singleton */
    helper_alignment_1() {};

    /*! Prevent copy-construction of singleton */
    helper_alignment_1(const helper_alignment_1&);

    /*! Prevent assignment (potentially resulting in a copy) of singleton */
    helper_alignment_1& operator=(const helper_alignment_1);

    /*! Prevent destruction unless going through destroy */
    ~helper_alignment_1() {};

    /*! pointer to the singleton instance */
    static helper_alignment_1 *_instance;

    /*! Singleton operation locker */
    static QMutex _mutex;
};
helper_alignment_1 *helper_alignment_1::_instance(0);
QMutex helper_alignment_1::_mutex;
helper_alignment_1 * helper_alignment_1::instance() {
    QMutexLocker locker(&_mutex);
    if(0 == _instance)
        _instance = new helper_alignment_1();
    return _instance;
}
void helper_alignment_1::destroy() {
    QMutexLocker locker(&_mutex);
    delete _instance;
    _instance = 0;
}




pp131::pp131(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id), _value(new Histogram0DFloat)
{
    _pp.histograms_replace(_id, _value);
}



pp131::~pp131()
{
    _pp.histograms_delete(_id);
}



std::list<PostProcessors::id_t> pp131::dependencies()
{
    return helper_alignment_1::dependencies();
};



void pp131::operator()(const CASSEvent& event)
{
    *_value = helper_alignment_1::instance()->cos2theta(event);
}



pp143::pp143(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id), _value(new Histogram0DFloat)
{
    _pp.histograms_replace(_id, _value);
}



pp143::~pp143()
{
    _pp.histograms_delete(_id);
}



std::list<PostProcessors::id_t> pp143::dependencies()
{
    return helper_alignment_1::dependencies();
};



void pp143::operator()(const CASSEvent& event)
{
    _value->lock.lockForWrite();
    *_value = helper_alignment_1::instance()->width(event);
    _value->lock.unlock();
}



pp144::pp144(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id), _value(new Histogram0DFloat)
{
    _pp.histograms_replace(_id, _value);
}



pp144::~pp144()
{
    _pp.histograms_delete(_id);
}



void pp144::operator()(const CASSEvent& event)
{
    *_value = helper_alignment_1::instance()->height(event);
}



std::list<PostProcessors::id_t> pp144::dependencies()
{
    return helper_alignment_1::dependencies();
};






//---postprocessor calculating cos2theta of requested avaeraged image----------
//----------------pp150--------------------------------------------------------
pp150::pp150(PostProcessors& pp, cass::PostProcessors::id_t id)
    : PostprocessorBackend(pp, id), _value(new Histogram0DFloat)
{
  _pp.histograms_replace(_id, _value);
  loadSettings(0);
}

pp150::~pp150()
{
    _pp.histograms_delete(_id);
    _value=0;
}

std::list<PostProcessors::id_t> pp150::dependencies()
{
  return std::list<PostProcessors::id_t>(1, _imageId);
}

void cass::pp150::loadSettings(size_t)
{
  using namespace std;

  QSettings param;
  param.beginGroup("PostProcessor");
  param.beginGroup(QString("p")+QString::number(_id));

  _imageId = static_cast<PostProcessors::id_t>(param.value("ImageId",104).toInt());

  std::cout <<std::endl<< "load the parameters of postprocessor "<<_id
      <<" it calcluates cos2theta"
      <<" of image "<<_imageId
      <<std::endl;

  try
  {
    _pp.validate(_imageId);
  }
  catch (InvalidHistogramError)
  {
    _reinitialize = true;
    return;
  }
  HistogramBackend * hist (_pp.histograms_checkout().find(_imageId)->second);
  _pp.histograms_release();


  // Set width and symmetry angle
  _imageWith = hist->axis()[HistogramBackend::xAxis].size();
  _center = std::make_pair<float,float>(param.value("ImageXCenter",0).toFloat(),
                                        param.value("ImageYCenter",0).toFloat());
  _symAngle = param.value("SymmetryAngle",0).toFloat();

  // Set the minimum radius within range
  float tmpr = param.value("MaxIncludedRadius",0).toFloat();

  tmpr = min(_center.first + 0.5f , tmpr);
  tmpr = min(hist->axis()[HistogramBackend::yAxis].size() - _center.first - 0.5f , tmpr);

  tmpr = min(_center.second + 0.5f , tmpr);
  tmpr = min(_imageWith- _center.second - 0.5f , tmpr);

  _minRadius = min(tmpr - 1.0f , param.value("MinIncludedRadius",0).toFloat());
  _minRadius = max(0.5f , _minRadius);

  // Set number of points on grid
  _nbrRadialPoints = static_cast<int32_t>(floor(tmpr-_minRadius));
  _nbrAngularPoints = 360;
}


void pp150::operator()(const CASSEvent& /*event*/)
{
  using namespace std;
  Histogram2DFloat *image(dynamic_cast<Histogram2DFloat*>(_pp.histograms_checkout().find(_imageId)->second));
  _pp.histograms_release();
  image->lock.lockForRead();
  HistogramFloatBase::storage_t &imageMemory(image->memory());
  float nom(0), denom(0);
  float symangle(_symAngle/180*M_PI);
  for(int jr = 0; jr<_nbrRadialPoints; jr++)
  {
    for(int jth = 0; jth<_nbrAngularPoints; jth++)
    {
      const float radius(_minRadius + jr);
      const float angle(2*M_PI * jth / _nbrAngularPoints);
      size_t col (static_cast<size_t>(round(_center.first  + radius*sin(angle + symangle))));
      size_t row (static_cast<size_t>(round(_center.second + radius*cos(angle + symangle))));
      float val = imageMemory[col + row * _imageWith] * square(radius); // * abs(sin(angle));
      denom += val;
      nom   += val * square(cos(angle));
    }
  }
  image->lock.unlock();
//  std::cout <<"pp150: "<<nom<<" "<<denom<<" "<<_nbrRadialPoints<<" "<<_nbrAngularPoints<<" "<<_minRadius<<" "<<_center.first<<" "<<_center.second<<std::endl;

  _value->lock.lockForWrite();
  *_value = (abs(denom) < 1e-15)?0.5:nom/denom;
  _value->lock.unlock();
}



} // end namespace cass



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
