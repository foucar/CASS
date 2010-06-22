//Copyright (C) 2010 Lutz Foucar

#ifndef _HIT_HELPER2_H_
#define _HIT_HELPER2_H_

#include <stdint.h>
#include <utility>
#include <algorithm>
#include <list>
#include <map>
#include <iostream>

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include <vigra/linear_algebra.hxx>
#include <vigra/inspectimage.hxx>
#include <vigra/multi_pointoperators.hxx> 


#include "cass_event.h"
#include "histogram.h"
#include "backend.h"


namespace cass
{
  namespace Hit2
  {

    struct results_t
    {
       uint64_t eventId;
       bool wasHit;
       double mahal_dist;
    };

    /** predicate class for find_if.
     * this helps finding the right key in the list of pairs
     * @author Lutz Foucar
     */
    class IsKey
    {
    public:
      /** initialize the key in the constructor*/
      IsKey(const uint64_t key):_key(key){}
      /** compares the first element of the pair to the key*/
      bool operator()(results_t*& pres)const
      { return (pres->eventId == _key); }
    private:
      /** the key that we will compare to in the operator*/
      const uint64_t _key;
    };

    /** Helper for Hit.
     * this is just a helper that we need since the whole concept is not working
     * yet.
     * @author Lutz Foucar, Stephan Kassemeyer
     */
    class HitHelper2
    {
    public:
      /** static function creating instance of this. */
      static HitHelper2 * instance();

      /** destroy the whole helper*/
      static void destroy();

      /** retrieve detector for event. */
      bool wasHit(const CASSEvent& evt)  {return getResults(evt)->wasHit;}
      double mahalDist(const CASSEvent& evt) {return getResults(evt)->mahal_dist;}

      /** tell the detector owned by this instance to reload its settings*/
      void loadSettings();

      //Histogram2DFloat _hist2D;

    protected:
      /** typdef defining the list of conditions for more readable code*/
      typedef std::list<results_t*> conditionList_t;
      
      double _threshold;

      size_t nxbins;
      size_t nybins;


      /** CCD detector that contains the requested image */
      size_t _detector;

      /** device the ccd image comes from */
      cass::CASSEvent::Device _device;

      /** storage for integralimage (2d integral of entire image) */
      Histogram2DFloat* _integralimg;
   
      /** storage for row sum (1d integral of image) */
      Histogram2DFloat* _rowsum;

      /** xstart - ROI for calculations*/
      int _xstart;
      /** ystart - ROI for calculations*/
      int _ystart;
      /** xend - ROI for calculations*/
      int _xend;
      /** yend - ROI for calculations*/
      int _yend;

    // outlier detection postprocessor:
    typedef vigra::Matrix<double> matrixType;
    
      int _nTrainingSetSize;
      int _nFeatures;
      matrixType _variationFeatures;
      matrixType _mean; // mean (one scalar per column or feature)
  //    vigra::MultiArray<1,double> _mean; // mean (one scalar per column or feature)
      matrixType _cov;
      matrixType _covI;
      int _trainingSetsInserted; // counts how many training data items are already included in training set.
      int _reTrain; // manages retraining of mean and covariance matrix used to determine outliers.

      /** validation of event */
      results_t* getResults(const CASSEvent &evt)
      {
        //lock this so that only one helper will retrieve the detector at a time//
        QMutexLocker lock(&_mutex);
        //find the pair containing the detector//
        conditionList_t::iterator it =
          std::find_if(_conditionList.begin(), _conditionList.end(), IsKey(evt.id()));

//        std::cout<< "HitHelper2::validate():"
//            <<" "<<it->first
//            <<" "<<it->second
//            <<std::endl;
        //check wether id is not already on the list//
        if(_conditionList.end() == it)
        {
//          std::cout<< "HitHelper2::validate(): condition "<<cond<<std::endl;
          //create a new key from the id with the reloaded detector
          results_t* newresults = new results_t;
          process(evt, newresults);
          //put it to the beginning of the list//
          _conditionList.push_front(newresults);
          //erase the outdated element at the back//
          _conditionList.pop_back();
          //make the iterator pointing to the just added element of the list//
          it = _conditionList.begin();
        }
        return *it;
      }

      /** process an event and find the condition */
      bool process(const CASSEvent& evt, results_t* results);

    protected:
      /** list of pairs of id-condition */
      conditionList_t _conditionList;

      /** the condition type */
      enum ConditionType{Tof, VMI, PnCCD, PnCCDPhotonhit} _conditiontype;

      /** tof boundaries for calc integral */
      std::pair<float,float> _tofBound;

      /** range for condition of integral */
      std::pair<float,float> _tofCond;

      /** ccd integral boundaries */
      std::pair< std::pair<float,float> , std::pair<float,float> > _ccdBound;

      /** second ccd integral boundaries */
      std::pair< std::pair<float,float> , std::pair<float,float> > _ccdBound2;

      /** range for condition of integral */
      std::pair<float,float> _ccdCond;

    private:
      /** prevent people from constructin other than using instance().*/
      HitHelper2();

      /** prevent copy-construction*/
      HitHelper2(const HitHelper2&);

      /** private desctuctor.
       * prevent destruction other than trhough destroy(),
       * delete the detector and the detectorlist for this instance
       */
      ~HitHelper2(){}

      /** prevent assingment */
      HitHelper2& operator=(const HitHelper2&);

      /** Singleton Mutex to lock write operations*/
      static QMutex _mutex;

      /** a static instance of this */
      static HitHelper2* _instance;
    };
  }


}

#endif
