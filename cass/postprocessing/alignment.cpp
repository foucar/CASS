// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2010 Lutz Foucar

#include <algorithm>
#include <cassert>
#include <cmath>
#include <list>
#include <map>
#include <cmath>
#include <cstdlib>

#include "alignment.h"
#include "cass_event.h"
#include "histogram.h"
#include "convenience_functions.h"
#include "cass_settings.h"



namespace cass
{
  ///*! Helper for postprocessors 131,
  //
  //This class actually does all the work; see individual postprocessors for details.
  //*/
  //class helper_alignment_2
  //{
  //public:
  //
  //    /*! create the instance if not it does not exist already */
  //    static helper_alignment_2 * instance();
  //
  //    /*! destroy the instance */
  //    static void destroy();
  //
  //    /*! center row of image */
  //    float center_row(const CASSEvent& event) { validate(event); return _values[event.id()][0]; };
  //
  //    /*! center column of image */
  //    float center_column(const CASSEvent& event) { validate(event); return _values[event.id()][1]; };
  //
  //    /*! \f$<cos^2\theta>\f$ */
  //    float cos2theta(const CASSEvent& event) { validate(event); return _values[event.id()][2]; };
  //
  //    /*! Postprocessor dependencies of this helper */
  //    static std::list<PostProcessors::id_t> dependencies() {
  //        return std::list<PostProcessors::id_t>(1, PostProcessors::VmiRunningAverage);
  //    };
  //
  //
  //protected:
  //
  //    /*! Validate and, if necessary, process event
  //
  //    @param event CASSEvent to process by us
  //    */
  //    void validate(const CASSEvent& event) {
  //        if(_processed.end() == find(_processed.begin(), _processed.end(), event.id())) {
  //            process(event);
  //            while(5 < _processed.size()) {
  //                _values.erase(_processed.back());
  //                _processed.pop_back();
  //            }
  //        }
  //    };
  //
  //    /*! Process event
  //
  //    Do the actual calculations to derive all relevant paramters.
  //    This method must be called through validate in order to correctly clean up _values.
  //    @param event CASSEvent to process by us
  //    */
  //    void process(const CASSEvent& event);
  //
  //    /*! list of processed (and available) event ids
  //
  //    This is our houskeeping list for deleting the oldest ids in order.
  //    Newest events are added in front, whereas the oldest (the last)
  //    events are delted when appropriate.
  //    */
  //    std::list<uint64_t> _processed;
  //
  //    /*! map of values for the available ids
  //
  //    For each id we determine a vector of floats:
  //    0 -- center row
  //    1 -- center column
  //    2 -- \f$<cos^2\theta>\f$
  //    */
  //    std::map<uint64_t, std::vector<float> > _values;
  //
  //
  //private:
  //
  //    /*! Private constructor of singleton */
  //    helper_alignment_2() {};
  //
  //    /*! Prevent copy-construction of singleton */
  //    helper_alignment_2(const helper_alignment_2&);
  //
  //    /*! Prevent assignment (potentially resulting in a copy) of singleton */
  //    helper_alignment_2& operator=(const helper_alignment_2);
  //
  //    /*! Prevent destruction unless going through destroy */
  //    ~helper_alignment_2() {};
  //
  //    /*! pointer to the singleton instance */
  //    static helper_alignment_2 *_instance;
  //
  //    /*! Singleton operation locker */
  //    static QMutex _mutex;
  //};
  //helper_alignment_2 *helper_alignment_2::_instance(0);
  //QMutex helper_alignment_2::_mutex;
  //helper_alignment_2 * helper_alignment_2::instance() {
  //    QMutexLocker locker(&_mutex);
  //    if(0 == _instance)
  //        _instance = new helper_alignment_2();
  //    return _instance;
  //}
  //void helper_alignment_2::destroy() {
  //    QMutexLocker locker(&_mutex);
  //    delete _instance;
  //    _instance = 0;
  //}
  //void helper_alignment_2::process(const CASSEvent& event)
  //{
  //    // determine (and store) center position of image
  //    std::pair<float, float> center(0., 0.); // (_histograms[PostProcessors::VmiRunningAverage].center());
  //    _values[event.id()][0] = center.first;
  //    _values[event.id()][1] = center.second;
  //    // calculate <cos2theta>
  //    _values[event.id()][2] = 0.5;
  //    _processed.push_front(event.id());
  //};
  //
  //
  //
  //
  ///*! Helper for postprocessors 143-144
  //
  //This class actually does all the work; see individual postprocessors for details.
  //*/
  //class helper_alignment_1
  //{
  //public:
  //
  //    /*! create the instance if not it does not exist already */
  //    static helper_alignment_1 * instance();
  //
  //    /*! destroy the instance */
  //    static void destroy();
  //
  //    /*! Gaussian width */
  //    float cos2theta(const CASSEvent& event) { validate(event); return _values[event.id()].first; };
  //
  //    /*! Gaussian width */
  //    float width(const CASSEvent& event) { validate(event); return _values[event.id()].first; };
  //
  //    /*! Gaussian height */
  //    float height(const CASSEvent& event) { validate(event); return _values[event.id()].second; };
  //
  //    /*! Postprocessor dependencies of this helper */
  //    static std::list<PostProcessors::id_t> dependencies() {
  //        return std::list<PostProcessors::id_t>(1, PostProcessors::VmiRunningAverage);
  //    };
  //
  //
  //protected:
  //
  //    /*! Validate and, if necessary, process event
  //
  //    @param event CASSEvent to process by us
  //    */
  //    void validate(const CASSEvent& event) {
  //        if(_processed.end() == find(_processed.begin(), _processed.end(), event.id())) {
  //            process(event);
  //            while(5 < _processed.size()) {
  //                _values.erase(_processed.back());
  //                _processed.pop_back();
  //            }
  //        }
  //    };
  //
  //    /*! Process event
  //
  //    Calculate the following properties of the given histogram:
  //    - Gaussian width
  //    - Gaussian height
  //
  //    @param event CASSEvent to process by us
  //    */
  //    void process(const CASSEvent& event) {
  //        // reduce 2d histogram to row- and column-integrated 1d histograms,
  //        // perform Gauss-fits to 1d histograms,and store FWHM
  //        _values[event.id()] = std::make_pair(float(0.), float(0.));
  //        _processed.push_front(event.id());
  //    };
  //
  //    /*! list of processed (and available) event ids
  //
  //    This is our houskeeping list for deleting the oldest ids in order.
  //    Newest events are added in front, whereas the oldest (the last)
  //    events are delted when appropriate.
  //    */
  //    std::list<uint64_t> _processed;
  //
  //    /*! map of values for the available ids */
  //    std::map<uint64_t, std::pair<float, float> > _values;
  //
  //
  //private:
  //
  //    /*! Private constructor of singleton */
  //    helper_alignment_1() {};
  //
  //    /*! Prevent copy-construction of singleton */
  //    helper_alignment_1(const helper_alignment_1&);
  //
  //    /*! Prevent assignment (potentially resulting in a copy) of singleton */
  //    helper_alignment_1& operator=(const helper_alignment_1);
  //
  //    /*! Prevent destruction unless going through destroy */
  //    ~helper_alignment_1() {};
  //
  //    /*! pointer to the singleton instance */
  //    static helper_alignment_1 *_instance;
  //
  //    /*! Singleton operation locker */
  //    static QMutex _mutex;
  //};
  //helper_alignment_1 *helper_alignment_1::_instance(0);
  //QMutex helper_alignment_1::_mutex;
  //helper_alignment_1 * helper_alignment_1::instance() {
  //    QMutexLocker locker(&_mutex);
  //    if(0 == _instance)
  //        _instance = new helper_alignment_1();
  //    return _instance;
  //}
  //void helper_alignment_1::destroy() {
  //    QMutexLocker locker(&_mutex);
  //    delete _instance;
  //    _instance = 0;
  //}
  //
  //
  //
  //
  //pp131::pp131(PostProcessors& pp, cass::PostProcessors::id_t id)
  //    : PostprocessorBackend(pp, id), _value(new Histogram0DFloat)
  //{
  //    _pp.histograms_replace(_id, _value);
  //}
  //
  //
  //
  //pp131::~pp131()
  //{
  //    _pp.histograms_delete(_id);
  //}
  //
  //
  //
  //std::list<PostProcessors::id_t> pp131::dependencies()
  //{
  //    return helper_alignment_1::dependencies();
  //};
  //
  //
  //
  //void pp131::operator()(const CASSEvent& event)
  //{
  //    *_value = helper_alignment_1::instance()->cos2theta(event);
  //}
  //
  //
  //
  //pp143::pp143(PostProcessors& pp, cass::PostProcessors::id_t id)
  //    : PostprocessorBackend(pp, id), _value(new Histogram0DFloat)
  //{
  //    _pp.histograms_replace(_id, _value);
  //}
  //
  //
  //
  //pp143::~pp143()
  //{
  //    _pp.histograms_delete(_id);
  //}
  //
  //
  //
  //std::list<PostProcessors::id_t> pp143::dependencies()
  //{
  //    return helper_alignment_1::dependencies();
  //};
  //
  //
  //
  //void pp143::operator()(const CASSEvent& event)
  //{
  //    _value->lock.lockForWrite();
  //    *_value = helper_alignment_1::instance()->width(event);
  //    _value->lock.unlock();
  //}
  //
  //
  //
  //pp144::pp144(PostProcessors& pp, cass::PostProcessors::id_t id)
  //    : PostprocessorBackend(pp, id), _value(new Histogram0DFloat)
  //{
  //    _pp.histograms_replace(_id, _value);
  //}
  //
  //
  //
  //pp144::~pp144()
  //{
  //    _pp.histograms_delete(_id);
  //}
  //
  //
  //
  //void pp144::operator()(const CASSEvent& event)
  //{
  //    *_value = helper_alignment_1::instance()->height(event);
  //}
  //
  //
  //
  //std::list<PostProcessors::id_t> pp144::dependencies()
  //{
  //    return helper_alignment_1::dependencies();
  //};






  //---postprocessor calculating cos2theta of requested averaged image----------
  cass::pp200::pp200(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
  {
    loadSettings(0);
  }

  void cass::pp200::loadSettings(size_t)
  {
    using namespace std;
    CASSSettings settings;
    settings.beginGroup("PostProcessor");
    settings.beginGroup(_key.c_str());
    _drawCircle = settings.value("DrawInnerOuterRadius",false).toBool();
    setupGeneral();
    _image = setupDependency("HistName");
    bool ret = setupCondition();
    if (!(_image && ret))
      return;
    _result = new Histogram0DFloat();
    createHistList(2*cass::NbrOfWorkers);
    const HistogramBackend &hist (_image->getHist(0));
    // Width of image - we assum the images are square
    _imageWidth = hist.axis()[HistogramBackend::xAxis].size();
    // center of the image -- this is the center of the angluar distribution of the signal
    _center = std::make_pair(settings.value("ImageXCenter", 500).toFloat(),
                             settings.value("ImageYCenter", 500).toFloat());
    // symmetry angle is the angle of in-plane rotation of the image with respect to its vertical axis
    _symAngle = settings.value("SymmetryAngle", 0).toFloat();
    // Set the minimum radius within range - must be within image
    _maxRadiusUser = settings.value("MaxIncludedRadius",10).toFloat();
    _minRadiusUser = settings.value("MinIncludedRadius",0).toFloat();
    _maxRadius = _maxRadiusUser;
    _maxRadius = min(min(_maxRadius, _center.first  + 0.5f), _imageWidth - _center.first - 0.5f);
    _maxRadius = min(min(_maxRadius, _center.second + 0.5f), _imageWidth - _center.second - 0.5f);
    _minRadius = max(0.1f, min(_maxRadius - 1.0f , _minRadiusUser));
    // Set number of points on grid
    _nbrRadialPoints = size_t(floor(_maxRadius-_minRadius));
    _nbrAngularPoints = 360;
    cout<<endl<< "PostProcessor '"<<_key
        <<"' calculates cos2theta of image from PostProcessor '"<<_image->key()
        <<"' Center is x'"<<_center.first
        <<"' y'"<<_center.second
        <<"' Symmetry angle is '"<<_symAngle
        <<"' Min radius the user requested is '"<<_minRadiusUser
        <<"' Max radius the user requested is '"<<_maxRadiusUser
        <<"' Therefore Min radius is '"<<_minRadius
        <<"' Max radius is '"<<_maxRadius
        <<"' where the image width is '"<<_imageWidth
        <<"' This results in Number of radial Points '"<<_nbrRadialPoints
        <<"'. Condition is '"<<_condition->key()<<"'"
        <<endl;
  }

  void pp200::histogramsChanged(const HistogramBackend *in)
  {
    using namespace std;
    _imageWidth = in->axis()[HistogramBackend::xAxis].size();
    _maxRadius = _maxRadiusUser;
    _maxRadius = min(min(_maxRadius, _center.first  + 0.5f), _imageWidth - _center.first - 0.5f);
    _maxRadius = min(min(_maxRadius, _center.second + 0.5f), _imageWidth - _center.second - 0.5f);
    _minRadius = max(0.1f, min(_maxRadius - 1.0f , _minRadiusUser));
    // Set number of points on grid
    _nbrRadialPoints = size_t(floor(_maxRadius-_minRadius));
    cout<<"pp200::histogramsChanged(): hist has changed. The new settings for '"<<_key
        <<"' are: "
        <<"' Min radius is '"<<_minRadius
        <<"' Max radius is '"<<_maxRadius
        <<"' where the image width is '"<<_imageWidth
        <<"' This results in Number of radial Points '"<<_nbrRadialPoints<<"'"
        <<endl;
  }

  void pp200::process(const CASSEvent& evt)
  {
    using namespace std;
    const Histogram2DFloat &image
        (dynamic_cast<const Histogram2DFloat&>((*_image)(evt)));
    image.lock.lockForRead();
    const HistogramFloatBase::storage_t &imageMemory(image.memory());
    float nom(0), denom(0), maxval(0);
    float symangle(_symAngle/180*M_PI);
    for(size_t jr = 0; jr<_nbrRadialPoints; jr++)
    {
      for(size_t jth = 1; jth<_nbrAngularPoints; jth++)
      {
        const float radius(_minRadius + jr);
        const float angle(2.*M_PI * float(jth) / float(_nbrAngularPoints));
        size_t col(size_t(_center.first  + radius*sin(angle + symangle)));
        size_t row(size_t(_center.second + radius*cos(angle + symangle)));
        float val = imageMemory[col + row * _imageWidth];
        denom += val * square(radius);
        nom   += val * square(cos(angle)) * square(radius);
        maxval = max(val,maxval);
      }
    }
    image.lock.unlock();
//      jocassview should be able to draw a circle or a line, it should not be done here
//      if (_drawCircle)
//      {
//        maxval/= 4.;
//        image.lock.lockForWrite();
//        //max circle
//        for(size_t jth = 0; jth<_nbrAngularPoints; jth++)
//        {
//          const float radius(_minRadius + _nbrRadialPoints);
//          const float angle(2.*M_PI * static_cast<float>(jth) / static_cast<float>(_nbrAngularPoints));
//          size_t col (static_cast<size_t>(round(_center.first  + radius*sin(angle + symangle))));
//          size_t row (static_cast<size_t>(round(_center.second + radius*cos(angle + symangle))));
//          imageMemory[col + row * _imageWidth] = maxval;
//        }
//        //min circle
//        for(size_t jth = 0; jth<_nbrAngularPoints; jth++)
//        {
//          const float radius(_minRadius);
//          const float angle(2.*M_PI * static_cast<float>(jth) / static_cast<float>(_nbrAngularPoints));
//          size_t col (static_cast<size_t>(round(_center.first  + radius*sin(angle + symangle))));
//          size_t row (static_cast<size_t>(round(_center.second + radius*cos(angle + symangle))));
//          imageMemory[col + row * _imageWidth] = maxval;
//        }
//
//        //the following seems to be not precise enough...
//        int32_t  xlocal1,ylocal1,xlocal2,ylocal2;
//        // the max number of points I have/want to consider
//        //this is in principle not the largest possible value if the centre of the circles
//        //is not around the middle of the frame and the circles are small
//        int32_t index_max=static_cast<int32_t>(_imageWidth*7/5/2);
//        /**
//       * @todo improve the value of index_max
//       * //to calculate the max value of index_max...
//       * // I have to consider the max distance to the sides
//       * // but this may still be too much... if the circles are on one side and the line is not
//       * // "too much" tilted than some of the distances in the following are never to be reached...
//       * index_max=std::max(static_cast<size_t>(_center.first),_imageWidth-static_cast<size_t>(_center.first),
//       *                    static_cast<size_t>(_center.second),_imageWidth-static_cast<size_t>(_center.second));
//       * //and then take into account the slope...
//       */
//
//        int32_t this_index;
//        const int32_t _imageSize=static_cast<int32_t>(_imageWidth*_imageWidth);
//        const int32_t s_imageWidth=static_cast<int32_t>(_imageWidth);
//        const double sin_dslope=std::sin(symangle+M_PI/2.);
//        const double cos_dslope=std::cos(symangle+M_PI/2.);
//        double d_minRadius= static_cast<double>(_minRadius);
//        double d_maxRadius= static_cast<double>(_maxRadius);
//        /**
//       * @todo improve the loop over iFrame to minimize the number of if statement that need to be evaluated
//       * //the following are the xpoints of the line with the 2 circles.
//       * // But to trace the segments I would have to recalculate the
//       * // stepsize in order to reach the intersections and worry of the fact that cos/sin may be zero...
//       * int32_t x_cross_i=static_cast<int32_t>(d_minRadius*cos_dslope);
//       * int32_t y_cross_i=static_cast<int32_t>(d_minRadius*sin_dslope);
//       * int32_t x_cross_e=static_cast<int32_t>(d_maxRadius*cos_dslope);
//       * int32_t y_cross_e=static_cast<int32_t>(d_maxRadius*sin_dslope);
//       */
//        for(int32_t iFrame=0;iFrame<index_max; ++iFrame)
//        {
//          double d_iFrame=static_cast<double>(iFrame);
//          xlocal1=static_cast<int32_t>(_center.first) + static_cast<int32_t>(d_iFrame*cos_dslope);
//          xlocal2=static_cast<int32_t>(_center.first) - static_cast<int32_t>(d_iFrame*cos_dslope);
//          ylocal1=static_cast<int32_t>(_center.second) + static_cast<int32_t>(d_iFrame *sin_dslope);
//          ylocal2=static_cast<int32_t>(_center.second) - static_cast<int32_t>(d_iFrame *sin_dslope);
//          const double dthis_distance=pow(d_iFrame*cos_dslope,2) + pow(d_iFrame*sin_dslope,2)  ;
//          //Inside the first radius
//          if( dthis_distance < d_minRadius * d_minRadius )
//          {
//            if(xlocal1>0 && ylocal1>0 && xlocal1<s_imageWidth && ylocal1<s_imageWidth)
//            {
//              this_index=xlocal1 + s_imageWidth * (ylocal1);
//              if (this_index>=0 && (this_index < _imageSize ) )
//                imageMemory[static_cast<size_t>(this_index)] = maxval;
//            }
//            if(xlocal2>0 && ylocal2>0 && xlocal2<s_imageWidth && ylocal2<s_imageWidth)
//            {
//              this_index=xlocal2 + s_imageWidth * (ylocal2);
//              if (this_index>=0 && (this_index < _imageSize ))
//                imageMemory[static_cast<size_t>(this_index)] = maxval;
//            }
//          }
//
//          else
//          {
//            //Outside the second radius
//            if( dthis_distance > (d_maxRadius * d_maxRadius) )
//            {
//              if(xlocal1>0 && ylocal1>0 && xlocal1<s_imageWidth && ylocal1<s_imageWidth)
//              {
//                this_index=xlocal1 + s_imageWidth * (ylocal1);
//                if (this_index>=0 && (this_index < _imageSize) )
//                  imageMemory[static_cast<size_t>(this_index)] = maxval;
//              }
//              if(xlocal2>0 && ylocal2>0 && xlocal2<s_imageWidth && ylocal2<s_imageWidth)
//              {
//                this_index=xlocal2 + s_imageWidth * (ylocal2);
//                if (this_index>=0 && (this_index < _imageSize) )
//                  imageMemory[static_cast<size_t>(this_index)] = maxval;
//              }
//            }
//          }
//        }
//        image->lock.unlock();
//      }
    _result->lock.lockForWrite();
    *dynamic_cast<Histogram0DFloat*>(_result) = (abs(denom) < 1e-15)?0.5:nom/denom;
    _result->lock.unlock();
  }


  // *** postprocessors 201 projects 2d hist to the radius for a selected center ***

  cass::pp201::pp201(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
  {
    loadSettings(0);
  }

  void cass::pp201::loadSettings(size_t)
  {
    using namespace std;
    CASSSettings settings;
    settings.beginGroup("PostProcessor");
    settings.beginGroup(_key.c_str());
    _userCenter = make_pair(settings.value("ImageXCenter", 500).toFloat(),
                            settings.value("ImageYCenter", 500).toFloat());
    _radiusRangeUser = make_pair(settings.value("MinIncludedRadius",10).toFloat(),
                                 settings.value("MaxIncludedRadius",0).toFloat());
    _nbrAngularPoints = settings.value("NbrAngularPoints",360.).toUInt();
    setupGeneral();
    _image = setupDependency("HistName");
    bool ret (setupCondition());
    if (!(ret && _image))
      return;
    const Histogram2DFloat &one
        (dynamic_cast<const Histogram2DFloat&>(_image->getHist(0)));
    setupParameters(one);
    _result = new Histogram1DFloat(_nbrAngularPoints, 0., 360.);
    createHistList(2*cass::NbrOfWorkers);
    cout<<endl << "PostProcessor '"<<_key
        <<"' will calculate the angular distribution of '"<<_image->key()
        <<"' from radia '"<<_radiusRange.first
        <<"' to '"<<_radiusRange.second
        <<"' with center x:"<<_center.first
        <<" y:"<<_center.second
        <<". Number of Points on the axis '"<<_nbrAngularPoints
        <<"'. Condition is '"<<_condition->key()<<"'"
        <<endl;
  }

  void pp201::histogramsChanged(const HistogramBackend *in)
  {
    using namespace std;
    //return when there is no incomming histogram
    if(!in)
      return;
    //return when the incomming histogram is not a direct dependant
    if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
      return;
    setupParameters(*in);
    VERBOSEOUT(cout <<"pp201::histogramsChanged(): hist has changed. The new settings for '"<<_key
                    <<"' are: "
                    <<"' Min radius is '"<<_radiusRange.first
                    <<"' Max radius is '"<<_radiusRange.second
                    <<"' This results in Number of radial Points '"<<_nbrRadialPoints<<"'"
                    <<endl);
  }

  void pp201::setupParameters(const HistogramBackend &hist)
  {
    using namespace std;
    const AxisProperty &xaxis(hist.axis()[HistogramBackend::xAxis]);
    const AxisProperty &yaxis(hist.axis()[HistogramBackend::yAxis]);
    size_t imagewidth (xaxis.nbrBins());
    _center = make_pair(xaxis.bin(_userCenter.first),
                        yaxis.bin(_userCenter.second));
    _radiusRange = _radiusRangeUser;
    _radiusRange.second = min(min(_radiusRange.second, _center.first  + 0.5f), imagewidth - _center.first - 0.5f);
    _radiusRange.second = min(min(_radiusRange.second, _center.second + 0.5f), imagewidth - _center.second - 0.5f);
    _radiusRange.first = max(0.1f, min(_radiusRange.second - 1.0f , _radiusRangeUser.first));
    // Set number of points on grid
    _nbrRadialPoints = size_t(floor(_radiusRange.second - _radiusRange.first));
  }

  void pp201::process(const CASSEvent& evt)
  {
    using namespace std;
    const Histogram2DFloat &image
        (dynamic_cast<const Histogram2DFloat&>((*_image)(evt)));
    image.lock.lockForRead();
    _result->lock.lockForWrite();
    const HistogramFloatBase::storage_t &imagememory(image.memory());
    size_t width(image.axis()[HistogramBackend::xAxis].nbrBins());
    HistogramFloatBase::storage_t &histmemory(dynamic_cast<Histogram1DFloat*>(_result)->memory());
    histmemory.clear();
    for(size_t jr = 0; jr<_nbrRadialPoints ; jr++)
    {
      for(size_t jth = 0; jth<_nbrAngularPoints; jth++)
      {
        const float radius(jr+_radiusRange.first);
        const float angle_deg(jth*360/_nbrAngularPoints);
        const float angle(angle_deg * M_PI/ 180.f);
        const float x(_center.first  + radius*sin(angle));
        const float y(_center.second + radius*cos(angle));
        const size_t x1(static_cast<size_t>(x));
        const size_t x2(x1 + 1);
        const size_t y1(static_cast<size_t>(y));
        const size_t y2(y1 + 1);
        const float f11 (imagememory[y1 * width + x1]);
        const float f21 (imagememory[y1 * width + x2]);
        const float f12 (imagememory[y2 * width + x1]);
        const float f22 (imagememory[y2 * width + x2]);
        const float interpolateValue = f11*(x2 - x )*(y2 - y )+
                                       f21*(x   -x1)*(y2 - y )+
                                       f12*(x2 - x )*(y  - y1)+
                                       f22*(x  - x1)*(y  - y1);
        histmemory[jth] += interpolateValue;
      }
    }
    _result->nbrOfFills()=1;
    _result->lock.unlock();
    image.lock.unlock();
  }





  // *** postprocessor 202 transform 2d kartisian hist to polar coordinates ***

  cass::pp202::pp202(PostProcessors& pp, const cass::PostProcessors::key_t &key)
    : PostprocessorBackend(pp, key)
  {
    loadSettings(0);
  }

  void cass::pp202::loadSettings(size_t)
  {
    using namespace std;
    CASSSettings settings;
    settings.beginGroup("PostProcessor");
    settings.beginGroup(_key.c_str());
    _userCenter = make_pair(settings.value("ImageXCenter", 500).toFloat(),
                            settings.value("ImageYCenter", 500).toFloat());
    _nbrAngularPoints = settings.value("NbrAngularPoints",360.).toUInt();
    _nbrRadialPoints  = settings.value("NbrRadialPoints",500.).toUInt();
    setupGeneral();
    _image = setupDependency("HistName");
    bool ret (setupCondition());
    if (!(ret && _image))
      return;
    const Histogram2DFloat &one
        (dynamic_cast<const Histogram2DFloat&>(_image->getHist(0)));
    setupParameters(one);
    createHistList(2*cass::NbrOfWorkers);
    cout<<endl << "PostProcessor '"<<_key
        <<"' will transform  '"<<_image->key()
        <<"' to polar coordinates."
        <<". Center x:"<<_center.first
        <<" y:"<<_center.second
        <<". Maximum radius is '"<<_maxRadius
        <<"'. Number of Points on the phi '"<<_nbrAngularPoints
        <<"'. Number of Points on the radius '"<<_nbrRadialPoints
        <<"'. Condition is '"<<_condition->key()<<"'"
        <<endl;
  }

  void pp202::histogramsChanged(const HistogramBackend *in)
  {
    using namespace std;
    //return when there is no incomming histogram
    if(!in)
      return;
    //return when the incomming histogram is not a direct dependant
    if (find(_dependencies.begin(),_dependencies.end(),in->key()) == _dependencies.end())
      return;
    setupParameters(*in);
    //notify all pp that depend on us that our histograms have changed
    PostProcessors::keyList_t dependands (_pp.find_dependant(_key));
    PostProcessors::keyList_t::iterator it (dependands.begin());
    for (; it != dependands.end(); ++it)
      _pp.getPostProcessor(*it).histogramsChanged(_result);
    VERBOSEOUT(cout <<"pp202::histogramsChanged(): hist has changed. '"<<_key
                    <<"' maxradius is '"<<_maxRadius<<"'"
                    <<endl);
  }

  void pp202::setupParameters(const HistogramBackend &hist)
  {
    using namespace std;
    if (hist.dimension() != 2)
    {
      stringstream ss;
      ss <<"pp202::setupParameters()'"<<_key<<"': Error the histogram we depend on '"<<hist.key()
          <<"' is not a 2D Histogram.";
      throw invalid_argument(ss.str());
    }
    const AxisProperty &xaxis(hist.axis()[HistogramBackend::xAxis]);
    const AxisProperty &yaxis(hist.axis()[HistogramBackend::yAxis]);
    _center = make_pair(xaxis.bin(_userCenter.first),
                        yaxis.bin(_userCenter.second));
    const size_t imagewidth (xaxis.nbrBins());
    const size_t imageheight (yaxis.nbrBins());
    const size_t dist_center_x_right(imagewidth - _center.first);
    const size_t dist_center_y_top(imageheight - _center.second);
    const size_t min_dist_x (min(dist_center_x_right, _center.first));
    const size_t min_dist_y (min(dist_center_y_top, _center.second));
    _maxRadius = min(min_dist_x, min_dist_y);
    _result = new Histogram2DFloat(_nbrAngularPoints, 0., 360.,
                                   _nbrRadialPoints,0., _maxRadius,
                                   "#phi","r");
  }

  void pp202::process(const CASSEvent& evt)
  {
    using namespace std;
    const Histogram2DFloat &image
        (dynamic_cast<const Histogram2DFloat&>((*_image)(evt)));
    image.lock.lockForRead();
    _result->lock.lockForWrite();
    const HistogramFloatBase::storage_t &imagememory(image.memory());
    const size_t width(image.axis()[HistogramBackend::xAxis].nbrBins());
    HistogramFloatBase::storage_t &resulthistmemory
        (dynamic_cast<Histogram2DFloat*>(_result)->memory());
    fill(resulthistmemory.begin(),resulthistmemory.end(),0.f);
    for(size_t jr = 0; jr<_nbrRadialPoints ; jr++)
    {
      for(size_t jth = 0; jth<_nbrAngularPoints; jth++)
      {
        const float radius(jr*_maxRadius/_nbrRadialPoints);
        const float angle_deg(jth*360/_nbrAngularPoints);
        const float angle(angle_deg * M_PI/ 180.f);
        const float x(_center.first  + radius*sin(angle));
        const float y(_center.second + radius*cos(angle));
        const size_t x1(static_cast<size_t>(x));
        const size_t x2(x1 + 1);
        const size_t y1(static_cast<size_t>(y));
        const size_t y2(y1 + 1);
        const float f11 (imagememory[y1 * width + x1]);
        const float f21 (imagememory[y1 * width + x2]);
        const float f12 (imagememory[y2 * width + x1]);
        const float f22 (imagememory[y2 * width + x2]);
        const float interpolateValue = f11*(x2 - x )*(y2 - y )+
                                       f21*(x   -x1)*(y2 - y )+
                                       f12*(x2 - x )*(y  - y1)+
                                       f22*(x  - x1)*(y  - y1);
        resulthistmemory[jr*_nbrAngularPoints + jth] += interpolateValue;
      }
    }
    _result->nbrOfFills()=1;
    _result->lock.unlock();
    image.lock.unlock();
  }


} // end namespace cass



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "gnu"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
