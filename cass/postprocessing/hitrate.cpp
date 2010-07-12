// Copyright (C) 2010 Lutz Foucar

#include <QtCore/QString>
#include <time.h>

#include "cass.h"
#include "hitrate.h"
#include "postprocessor.h"
#include "histogram.h"
#include "convenience_functions.h"
#include "cass_settings.h"

namespace cass
{

// *** postprocessor 589 finds Single particle hits ***
cass::pp300::pp300(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _integralimg(NULL), _rowsum(NULL)
{
  loadSettings(0);
}

cass::pp300::~pp300()
{
  delete _integralimg;
  _integralimg = NULL;
  delete _rowsum;
  _rowsum = NULL;
}

void cass::pp300::trainingFinished()
{
  _trainingSetsInserted = _nTrainingSetSize;
  _reTrain = false;
}

void cass::pp300::startNewTraining()
{
  _trainingSetsInserted = 0;
  _reTrain = 0;
}

void cass::pp300::readTrainingMatrices()
{
  std::ifstream infile(_trainingFile.c_str(), std::ios::binary|std::ios::in);
  infile.read(reinterpret_cast<char*>(_covI.data()), _covI.size()*sizeof(double));
  infile.read(reinterpret_cast<char*>(_mean.data()), _mean.size()*sizeof(double));
  infile.close();
}

void cass::pp300::saveTrainingMatrices()
{
  time_t rawtime;
  struct tm * timeinfo;
  time ( &rawtime );
  size_t dateSize=9+4+1;
  char date_and_time[dateSize];
  timeinfo = localtime ( &rawtime );
  strftime(date_and_time,dateSize,"%Y%m%d_%H%M",timeinfo);
  QString saveName( QString("sp_training_%1.dat").arg(date_and_time) );
  std::cout << "Saving Single Particle Data to " << saveName.toStdString() << std::endl;
  std::ofstream outfile(saveName.toStdString().c_str(), std::ios::binary|std::ios::out);
  if (outfile.is_open())
  {
    //write the training data to the file//
    outfile.write(reinterpret_cast<const char*>( _covI.data()), _covI.size()*sizeof(double) );
    outfile.write(reinterpret_cast<const char*>( _mean.data()), _mean.size()*sizeof(double) );
  }
  outfile.close();
}

void cass::pp300::printTrainingMatrices()
{
  typedef matrixType::traverser ttt;
  std::cout << std::endl << "_covI = [";
  for (ttt it0 = _covI.traverser_begin(); it0!=_covI.traverser_end(); ++it0) {
      std::cout << "[";
      for (ttt::next_type it1 = it0.begin(); it1!=it0.end(); ++it1) {
          std::cout << *it1 << ", ";
      }
      std::cout << "]; ";
  }
  std::cout << "] " << std::endl;
  std::cout << std::endl << "_mean= [";
  for (ttt it0 = _mean.traverser_begin(); it0!=_mean.traverser_end(); ++it0) {
      std::cout << "[";
      for (ttt::next_type it1 = it0.begin(); it1!=it0.end(); ++it1) {
          std::cout << *it1 << ", ";
      }
      std::cout << "]; ";
  }
  std::cout << "] " << std::endl;
}


void cass::pp300::saveSettings(size_t)
{
  saveTrainingMatrices();
}

void cass::pp300::loadSettings(size_t)
{
  using namespace std;
  CASSSettings settings;
  settings.beginGroup("PostProcessor");
  settings.beginGroup(_key.c_str());

  // Get the input
  _pHist = setupDependency("HistName");
  setupGeneral();
  bool ret (setupCondition());
  if (!(ret && _pHist))
    return;

  _xstart = settings.value("xstart", 0).toInt();
  _ystart = settings.value("ystart", 0).toInt();
  _xend = settings.value("xend", -1).toInt();
  _yend = settings.value("yend", -1).toInt();

  // Create result
  _result = new Histogram0DFloat();
  createHistList(2*cass::NbrOfWorkers);


  // outlier postprocessor:
  _nTrainingSetSize = settings.value("TrainingSetSize", 200).toInt();
  _nFeatures = 5;
  _variationFeatures = matrixType(_nTrainingSetSize, _nFeatures);
  _cov = matrixType( _nFeatures, _nFeatures );
  _covI = matrixType( _nFeatures, _nFeatures);
  _mean = matrixType( 1,_nFeatures );

  _saveTraining = settings.value("saveTraining",true).toBool();
  _readTraining = settings.contains("readTrainingFile");
  if (_readTraining)
  {
    _trainingFile = settings.value("readTrainingFile","sp_training.dat").toString().toStdString();
    readTrainingMatrices();
    std::cout << "read training matrices:" << std::endl;
    printTrainingMatrices();
    trainingFinished();
  }
  else
  {
    vigra::linalg::identityMatrix(_covI);
    startNewTraining();
  }


  const Histogram2DFloat &img
      (dynamic_cast<const Histogram2DFloat&>(_pHist->getHist(0)));

  _integralimg = new Histogram2DFloat(img);
  _rowsum = new Histogram2DFloat(img);

  //_pp.histograms_replace(_key,_integralimg);  // for debuging, output _integralimage instead of _result

  std::cout<<"Postprocessor "<<_key
      <<": detects Single particle hits in PostProcessor "<< _pHist->key()
      <<" ROI for detection: ["<<_xstart<<","<<_ystart<<","<<_xend<<","<<_yend<<"]"
      <<". Condition is"<<_condition->key()
      <<std::endl;
}

void cass::pp300::clearHistogramEvent()
{
  std::cout << std::endl << "single particle hit: cleared histogram -> restart training." << std::endl;
  startNewTraining();
}

void cass::pp300::processCommand(std::string command)
{
  if (command == "retrain") {
    std::cout << std::endl << "single particle hit: received command to restart training." << std::endl;
    startNewTraining();
  }
}

void cass::pp300::process(const CASSEvent& evt)
{
  using namespace std;
  _integralimg->lock.lockForWrite();
  _rowsum->lock.lockForWrite();
  const Histogram2DFloat &one
        (dynamic_cast<const Histogram2DFloat&>((*_pHist)(evt)));
  one.lock.lockForRead();

  const size_t nxbins (one.axis()[HistogramBackend::xAxis].nbrBins());
  const size_t nybins (one.axis()[HistogramBackend::yAxis].nbrBins());

  const HistogramFloatBase::storage_t& img_mem( one.memory() );
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

  // try to unify feature scales (avoid ill-conditioned matrices, also, matrix should not be singular):
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
        std::cout << "Hit_Helper2::process: " << E.what() << std::endl;
        vigra::linalg::identityMatrix(_covI);
        startNewTraining();  // try again: retrain... 
        one.lock.unlock();
        _integralimg->lock.unlock();
        _rowsum->lock.unlock();
        return;
    };

    //calculate collumn-average and store in _mean
    // transformMultArray reduces source-dimensions to scalar for each singleton-dest-dimension 
    /*transformMultiArray(srcMultiArrayRange(_variationFeatures),
                        destMultiArrayRange(_mean.insertSingletonDimension(1)),
                        FindAverage<double>());*/
    vigra::linalg::columnStatistics(_variationFeatures, _mean);
    printTrainingMatrices();

    // also possible:
    /*
    vigra::transformMultiArray(srcMultiArrayRange(_variationFeatures),
                        destMultiArrayRange(_mean),
                        vigra::FindAverage<double>());*/

    if (_saveTraining) saveTrainingMatrices();
    _reTrain = false;
  } //end reTrain
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



  one.lock.unlock();
  _integralimg->lock.unlock();
  _rowsum->lock.unlock();


  _result->lock.lockForWrite();
  dynamic_cast<Histogram0DFloat*>(_result)->fill( mahal_dist );
  _result->lock.unlock();
}
}
