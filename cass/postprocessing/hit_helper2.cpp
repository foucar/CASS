//Copyright (C) 2010 Lutz Foucar

#include <QtCore/QSettings>
#include <cmath>

#include "hit_helper2.h"
#include "ccd_device.h"
#include "pixel_detector.h"
#include "cass.h"
#include "histogram.h"

//initialize static members//
QMutex cass::Hit2::HitHelper2::_mutex;
cass::Hit2::HitHelper2 *cass::Hit2::HitHelper2::_instance(0);

cass::Hit2::HitHelper2* cass::Hit2::HitHelper2::instance()
{
  //lock this//
  QMutexLocker lock(&_mutex);
  //check if an instance of the helper class already exists//
  //return it, otherwise create one and return it//
  if (0 == _instance)
  {
    VERBOSEOUT(std::cout << "creating an instance of the Hit Helper"
               <<std::endl);
    _instance = new HitHelper2();
  }
  return _instance;
}

void cass::Hit2::HitHelper2::destroy()
{
  //lock this//
  QMutexLocker lock(&_mutex);
  delete _instance;
  _instance=0;
}


cass::Hit2::HitHelper2::HitHelper2()
{
  using namespace std;
  loadSettings();
  for (size_t i=0 ; i<NbrOfWorkers ; ++i) {
    results_t* emptyres = new results_t;
    emptyres->wasHit=0;
    emptyres->mahal_dist = 0;
    _conditionList.push_back( emptyres );
  }
}

void cass::Hit2::HitHelper2::loadSettings()
{

  nxbins=1024;
  nybins=1024;

  using namespace std;
  QSettings settings;
  settings.beginGroup("HitSpecial2");
  _threshold = settings.value("Threshold", 4e6).toFloat();

  _device = static_cast<CASSEvent::Device>(0);
  _detector = 1;

  using namespace std;
  _xstart = settings.value("xstart", 0).toInt();
  _ystart = settings.value("ystart", 0).toInt();
  _xend = settings.value("xend", -1).toInt();
  _yend = settings.value("yend", -1).toInt();

  // outlier postprocessor:
  _nTrainingSetSize = settings.value("TrainingSetSize", 100).toInt();
  _nFeatures = 5;
  _variationFeatures = matrixType(_nTrainingSetSize, _nFeatures);
  _cov = matrixType( _nFeatures, _nFeatures );
  _covI = matrixType( _nFeatures, _nFeatures);
  _mean = matrixType( 1,_nFeatures );
//  _mean = vigra::MultiArray<1,double>( vigra::MultiArray<1,float>::difference_type(_nTrainingSetSize) );
  _trainingSetsInserted = 0;
  _reTrain = false;

  _integralimg = new Histogram2DFloat(nxbins,0,1, nybins, 0,1);
  _rowsum = new Histogram2DFloat(nxbins,0,1, nybins, 0,1);

  std::cout<<"Hit Helper 2"
      <<": detects Single particle hits in pnccd detector 1"
      <<" threshold for detection:"<<_threshold
      <<" ROI for detection: ["<<_xstart<<","<<_ystart<<","<<_xend<<","<<_yend<<"]"
      <<std::endl;

}

bool cass::Hit2::HitHelper2::process(const CASSEvent& event, results_t* results)
{
  bool cond(false);
  using namespace std;

  if (event.devices().find(_device)->second->detectors()->size() <= _detector)
    throw std::runtime_error(QString("Hit Helper: Detector %2 does not exist in Device %3")
                             .arg(_detector)
                             .arg(_device).toStdString());

  const PixelDetector::frame_t& img_mem
      ((*(event.devices().find(_device)->second)->detectors())[_detector].frame());
  

{
  using namespace std;
  _integralimg->lock.lockForWrite();
  _rowsum->lock.lockForWrite();

  // todo: retrain on cleared histogram ( maybe retrain() function)

  HistogramFloatBase::storage_t& rowsum_mem( _rowsum->memory() );
  HistogramFloatBase::storage_t& integralimg_mem( _integralimg->memory() );

  // sanity checks and auto-set nxbins. todo: restore -1 so that nxbins/nybins gets updated on size change?
  if (_xstart < 0) _xstart = 0;
  if (_ystart < 0) _ystart = 0;
  if (_xend >= static_cast<int>(nxbins)) _xend = nxbins-1;
  if (_yend >= static_cast<int>(nybins)) _yend = nybins-1;
  if (_xend < 0) _xend = nxbins-1;
  if (_yend < 0) _yend = nybins-1;


  // calculate integral-image:
  // row sum initialize first row:
  for (int xx = _xstart; xx<=_xend; ++xx)
    rowsum_mem[xx-_xstart] = img_mem[xx + _ystart*nxbins];
  // row sum rest:
  for (int xx = _xstart; xx<=_xend; ++xx)
    for (int yy=_ystart+1; yy<=_yend; ++yy)
      rowsum_mem[xx-_xstart + (yy-_ystart)*nxbins] = rowsum_mem[xx-_xstart + (yy-_ystart-1)*nxbins] + img_mem[xx + yy*nxbins];
  // intImg first col:
  for (int yy = _ystart; yy<=_yend; ++yy)
    integralimg_mem[(yy-_ystart)*nxbins] = rowsum_mem[(yy-_ystart)*nxbins];
  // intImg rest:
  for (int xx = _xstart+1; xx<=_xend; ++xx)
    for (int yy = _ystart; yy<=_yend; ++yy)
      integralimg_mem[xx-_xstart + (yy-_ystart)*nxbins] = integralimg_mem[xx-_xstart-1 + (yy-_ystart)*nxbins] + rowsum_mem[xx-_xstart + (yy-_ystart)*nxbins];

  // calculate variation features:
  int xsize_intimg = _xend-_xstart+1;
  int ysize_intimg = _yend-_ystart+1;

  //
  // 1st variation feature: sum(cell-avg)^2
  double var0 = 0;
  { // start new scope
  int xsteps = 3;
  int ysteps = 3;
  double avg = integralimg_mem[xsize_intimg-1 + (ysize_intimg-1)*nxbins];
  for (int ii=0; ii<xsteps; ++ii)
  {
    int xx_prev = lround( static_cast<double>(ii)*xsize_intimg/xsteps );
    int xx = lround(  static_cast<double>(ii+1)*xsize_intimg/xsteps-1 );
    for (int jj=0; jj<ysteps; ++jj)
    {
      int yy_prev = lround( static_cast<double>(jj)*ysize_intimg/ysteps );
      int yy = lround(  static_cast<double>(jj+1)*ysize_intimg/ysteps-1 );
      double val = integralimg_mem[xx + yy*nxbins] + integralimg_mem[xx_prev + yy_prev*nxbins] - integralimg_mem[xx + yy_prev*nxbins] - integralimg_mem[xx_prev + yy*nxbins];
      var0 += (val-avg)*(val-avg);
    }
  }
  var0 /= (avg*avg);  // norm result
  } //end scope

  //
  // 2nd variation feature:  (bottom to top, sum(cell-topneighbour)^2
  double var1 = 0;
  { // start new scope
  int xsteps = 10;
  int ysteps = 10;
  // ToDo: check orientation. maybe go left-right instead top-bottom!
  for (int ii=0; ii<xsteps; ++ii)
  {
    int xx_prev = lround( static_cast<double>(ii)*xsize_intimg/xsteps );
    int xx = lround(  static_cast<double>(ii+1)*xsize_intimg/xsteps-1 );
    for (int jj=0; jj<ysteps-2; ++jj)
    {
      int yy_prev = lround( static_cast<double>(jj)*ysize_intimg/ysteps );
      int yy = lround(  static_cast<double>(jj+1)*ysize_intimg/ysteps-1 );
      int yy_next = lround( static_cast<double>(jj+2)*ysize_intimg/ysteps);
      double diff = 2*integralimg_mem[xx+yy*nxbins] + integralimg_mem[xx_prev+yy_prev*nxbins] - integralimg_mem[xx+yy_prev*nxbins] - 2*integralimg_mem[xx_prev+yy*nxbins] - integralimg_mem[xx+yy_next*nxbins] + integralimg_mem[xx_prev+yy_next*nxbins];
      var1 += diff*diff;
    }
  }
  } // end scope

  //
  // 3rd variation feature:  (bottom to top, sum(cell-topneighbour)^2  like 2nd one, but with different scale
  double var2 = 0;
  { // start new scope
  int xsteps = 15;
  int ysteps = 15;
  // ToDo: check orientation. maybe go left-right instead top-bottom!
  for (int ii=0; ii<xsteps; ++ii)
  {
    int xx_prev = lround( static_cast<double>(ii)*xsize_intimg/xsteps );
    int xx = lround(  static_cast<double>(ii+1)*xsize_intimg/xsteps-1 );
    for (int jj=0; jj<ysteps-2; ++jj)
    {
      int yy_prev = lround( static_cast<double>(jj)*ysize_intimg/ysteps );
      int yy = lround(  static_cast<double>(jj+1)*ysize_intimg/ysteps-1 );
      int yy_next = lround( static_cast<double>(jj+2)*ysize_intimg/ysteps);
      double diff = 2*integralimg_mem[xx+yy*nxbins] + integralimg_mem[xx_prev+yy_prev*nxbins] - integralimg_mem[xx+yy_prev*nxbins] - 2*integralimg_mem[xx_prev+yy*nxbins] - integralimg_mem[xx+yy_next*nxbins] + integralimg_mem[xx_prev+yy_next*nxbins];
      var2 += diff*diff;
    }
  }
  } // end scope

  //
  // 4th variation feature: chequerboard
  double var3 = 0;
  { // start new scope
  int xsteps = 15;
  int ysteps = 15;
  int xfactor = -1;
  int yfactor = -1;
  int factor;
  for (int ii=0; ii<xsteps; ++ii)
  {
    xfactor = -xfactor;
    int xx_prev = lround( static_cast<double>(ii)*xsize_intimg/xsteps );
    int xx = lround(  static_cast<double>(ii+1)*xsize_intimg/xsteps-1 );
    for (int jj=0; jj<ysteps; ++jj)
    {
      yfactor = -yfactor;
      factor = xfactor*yfactor;
      int yy_prev = lround( static_cast<double>(jj)*ysize_intimg/ysteps );
      int yy = lround(  static_cast<double>(jj+1)*ysize_intimg/ysteps-1 );
      var3 += factor*(integralimg_mem[xx + yy*nxbins] + integralimg_mem[xx_prev + yy_prev*nxbins] - integralimg_mem[xx + yy_prev*nxbins] - integralimg_mem[xx_prev + yy*nxbins]);
    }
  }
  } //end scope  //

  // 5th variation feature: integral intensity
  double var4 = integralimg_mem[xsize_intimg-1 + (ysize_intimg-1)*nxbins];

  // try to unify feature scales:
  var0 /= 1;
  var1 /= 1e12;
  var2 /= 1e12;
  var3 /= 1e8;
  var4 /= 1e7;

  // output current features:
  VERBOSEOUT(std::cout<< "current features: " << var0 << ",  " << var1 << ",  " << var2 << ",  " << var3 << ",  " << var4 << std::endl);

  // populate Trainingset
  if ( _trainingSetsInserted < _nTrainingSetSize )
  {
    _variationFeatures[matrixType::difference_type(_trainingSetsInserted, 0)] = var0;
    _variationFeatures[matrixType::difference_type(_trainingSetsInserted, 1)] = var1;
    _variationFeatures[matrixType::difference_type(_trainingSetsInserted, 2)] = var2;
    _variationFeatures[matrixType::difference_type(_trainingSetsInserted, 3)] = var3;
    _variationFeatures[matrixType::difference_type(_trainingSetsInserted, 4)] = var4;
    ++_trainingSetsInserted;
    if ( _trainingSetsInserted == _nTrainingSetSize ) _reTrain = true;
    VERBOSEOUT(std::cout << "inserted: " << _trainingSetsInserted << std::endl);
  }
  if ( _reTrain )
  {

  VERBOSEOUT(std::cout << "rows: " << vigra::rowCount(_cov) << std::endl);
  VERBOSEOUT(std::cout << "cols: " << vigra::columnCount(_cov) << std::endl);

    _cov = vigra::linalg::covarianceMatrixOfColumns( _variationFeatures.subarray(vigra::Matrix<double>::difference_type(0,0), vigra::Matrix<double>::difference_type(_trainingSetsInserted,_nFeatures)) );

    typedef matrixType::traverser ttt;
#ifdef VERBOSE
    std::cout << std::endl << "_cov= [";
    for (ttt it0 = _cov.traverser_begin(); it0!=_cov.traverser_end(); ++it0) {
        std::cout << "[";
        for (ttt::next_type it1 = it0.begin(); it1!=it0.end(); ++it1) {
            std::cout << *it1 << ", ";
        }
        std::cout << "]; ";
    }
    std::cout << "] " << std::endl;
    std::cout << std::endl << "_variationFeatures= [";
    for (ttt it0 = _variationFeatures.traverser_begin(); it0!=_variationFeatures.traverser_end(); ++it0) {
        std::cout << "[";
        for (ttt::next_type it1 = it0.begin(); it1!=it0.end(); ++it1) {
            std::cout << *it1 << ", ";
        }
        std::cout << "]; ";
    }
    std::cout << "] " << std::endl;
#endif

    try{
        _covI = vigra::linalg::inverse(_cov);
     }
    catch( vigra::PreconditionViolation E ) {
        // matrix is singular. use normalized euclidean distance instead of mahalanobis:
        std::cout << E.what() << std::endl;
        vigra::linalg::identityMatrix(_covI); 
    };

#ifdef VERBOSE
    std::cout << std::endl << "_covI = [";
    for (ttt it0 = _covI.traverser_begin(); it0!=_covI.traverser_end(); ++it0) {
        std::cout << "[";
        for (ttt::next_type it1 = it0.begin(); it1!=it0.end(); ++it1) {
            std::cout << *it1 << ", ";
        }
        std::cout << "]; ";
    }
    std::cout << "] " << std::endl;
#endif

    _reTrain = false;
    //calculate collumn-average and store in _mean
    // transformMultArray reduces source-dimensions to scalar for each singleton-dest-dimension 
    /*transformMultiArray(srcMultiArrayRange(_variationFeatures),
                        destMultiArrayRange(_mean.insertSingletonDimension(1)),
                        FindAverage<double>());*/
    vigra::linalg::columnStatistics(_variationFeatures, _mean);
#ifdef VERBOSE
    std::cout << std::endl << "_mean= [";
    for (ttt it0 = _mean.traverser_begin(); it0!=_mean.traverser_end(); ++it0) {
        std::cout << "[";
        for (ttt::next_type it1 = it0.begin(); it1!=it0.end(); ++it1) {
            std::cout << *it1 << ", ";
        }
        std::cout << "]; ";
    }
    std::cout << "] " << std::endl;
#endif

    // also possible:
    /*
    vigra::transformMultiArray(srcMultiArrayRange(_variationFeatures),
                        destMultiArrayRange(_mean),
                        vigra::FindAverage<double>());*/

  }
  // use mean and covariance to predict outliers:
  // mahalanobis^2 = (y-mean)T Cov^-1 y
  matrixType y(1, _nFeatures);
  y[0] = var0; y[1] = var1; y[2] = var2; y[3] = var3; y[4] = var4; 
  //y = _variationFeatures.subarray( matrixType::difference_type(0,0), matrixType::difference_type(1,_nFeatures));

  /*
  std::cout << "rows: " << vigra::rowCount(y) << std::endl;
  std::cout << "cols: " << vigra::columnCount(y) << std::endl;
  std::cout << "rows: " << vigra::rowCount(_covI) << std::endl;
  std::cout << "cols: " << vigra::columnCount(_covI) << std::endl;*/
  double mahal_dist = vigra::linalg::mmul(  (y-_mean), vigra::linalg::mmul( _covI , (y-_mean).transpose() ))[0];



  _integralimg->lock.unlock();
  _rowsum->lock.unlock();
  results->wasHit = mahal_dist>=_threshold;
  results->mahal_dist = mahal_dist;
}


  return cond;
}


