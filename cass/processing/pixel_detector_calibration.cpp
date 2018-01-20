// Copyright (C) 2013 Lutz Foucar

/**
 * @file pixel_detector_calibration.cpp processors that do calibrations on pixel
 *                                      detector data
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include <numeric>
#include <algorithm>

#include "pixel_detector_calibration.h"

#include "cass_settings.h"
#include "statistics_calculator.hpp"
#include "log.h"
#include "convenience_functions.h"


using namespace cass;
using namespace std;
using tr1::bind;
using tr1::placeholders::_1;
using tr1::placeholders::_2;
using tr1::placeholders::_3;
using tr1::placeholders::_4;
using tr1::placeholders::_5;



//********** offset/noise calibrations ******************

pp330::pp330(const name_t &name)
  : AccumulatingProcessor(name),
    _lastwritten(0)
{
  loadSettings(0);
}

void pp330::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _image = setupDependency("RawImage");
  setupGeneral();
  bool ret (setupCondition());
  if (!(_image && ret))
    return;

  /** load parameters from the ini file */
  _autoNoiseSNR = s.value("SNRNoiseAutoBoundaries",4).toFloat();
  _autoNoiseSNRStat = s.value("SNRNoiseAutoBoundariesStat",4).toFloat();
  _NoiseLowerBound = s.value("NoiseLowerBoundary",1).toFloat();
  _NoiseUpperBound = s.value("NoiseUpperBoundary",3).toFloat();
  _autoOffsetSNR = s.value("SNROffsetAutoBoundaries",-1).toFloat();
  _autoOffsetSNRStat = s.value("SNROffsetAutoBoundariesStat",4).toFloat();
  _OffsetLowerBound = s.value("OffsetLowerBoundary",-1e20).toFloat();
  _OffsetUpperBound = s.value("OffsetUpperBoundary",1e20).toFloat();
  _minNbrPixels = s.value("MinNbrPixels",90).toFloat()/100.f;
  _filename = s.value("OutputFilename","NotSet").toString().toStdString();
  _infilename = s.value("InputFilename","NotSet").toString().toStdString();
  _write = s.value("WriteCal",true).toBool();
  _train = s.value("Train",true).toBool();
  _minTrainImages = s.value("NbrTrainingImages",200).toUInt();
  _snr = s.value("SNR",4).toFloat();
  _resetBadPixel = s.value("ResetBadPixels",false).toBool();
  _update = s.value("UpdateCalibration",true).toBool();
  string updateType = s.value("UpdateCalibrationType","cummulative").toString().toStdString();
  _updatePeriod = s.value("UpdateBadPixPeriod",-1).toInt();
  _updateWritePeriod = s.value("WritePeriod",0).toUInt();
  _alpha = (2./(s.value("NbrOfImages",200).toFloat()+1.));

  /** set up the type of update */
  if (updateType == "cummulative")
    _updateStatistics = bind(&pp330::cummulativeUpdate,this,_1,_2,_3,_4,_5);
  else if (updateType == "moving")
    _updateStatistics = bind(&pp330::movingUpdate,this,_1,_2,_3,_4,_5);
  else
    throw invalid_argument("pp330::loadSettings(): '" + name() +"' updateType '" +
                           updateType + "' is unknown.");


  /** reset the variables */
  _trainstorage.clear();
  _counter = 0;

  /** determine the offset of the output arrays from the size of the input image */
  result_t::shape_t shape(_image->result().shape());
  const size_t imagesize(shape.first*shape.second);
  _meanBeginOffset = MEAN * imagesize;
  _meanEndOffset = (MEAN + 1) * imagesize;
  _stdvBeginOffset = STDV * imagesize;
  _stdvEndOffset = (STDV + 1) * imagesize;
  _bPixBeginOffset = BADPIX * imagesize;
  _bPixEndOffset = (BADPIX + 1) * imagesize;
  _nValBeginOffset = NVALS * imagesize;
  _nValEndOffset = (NVALS + 1) * imagesize;

  createHistList
      (result_t::shared_pointer
        (new result_t(shape.first,nbrOfOutputs*shape.second)));
  loadCalibration();
  /** in case we want updating and no calibration was loaded or the last
   *  calibration is too outdated (now - _lastwritten > _updateWritePeriod),
   *  we need to start training now
   */
  const uint32_t now(QDateTime::currentDateTime().toTime_t());
  if (_update && (_updateWritePeriod <(now - _lastwritten))) _train = true;
  const string createdAt(_lastwritten ?
        QDateTime::fromTime_t(_lastwritten).toString("yyyyMMdd_HHmm").toStdString() :
        "never");
  Log::add(Log::INFO,"processor " + name() +
           ": generates the calibration data from images contained in '" +
           _image->name() +
           "' autoNoiseSNR '" + toString(_autoNoiseSNR) +
           "' autoNoiseSNRStat '" + toString(_autoNoiseSNRStat) +
           "' NoiselowerBound '" + toString(_NoiseLowerBound) +
           "' NoiseupperBound '" + toString(_NoiseUpperBound) +
           "' autoOffsetSNR '" + toString(_autoOffsetSNR) +
           "' autoOffsetSNRStat '" + toString(_autoOffsetSNRStat) +
           "' OffsetLowerBound '" + toString(_OffsetLowerBound) +
           "' OffsetUpperBound '" + toString(_OffsetUpperBound) +
           "' minNbrPixels '" + toString(_minNbrPixels) +
           "' outputfilename '" + _filename +
           "' inputfilename '" + _infilename +
           "'(created '" + createdAt +
           "') write '" + (_write?"true":"false") +
           "' train '" + (_train?"true":"false") +
           "' nbrTrainImages '" + toString(_minTrainImages) +
           "' SNR '" + toString(_snr) +
           "' alpha '" + toString(_alpha) +
           "' updateType '" + updateType +
           "' updatebadpixperiode '" + toString(_updatePeriod) +
           "' updatewriteperiode '" + toString(_updateWritePeriod) +
           "'. Condition is '" + _condition->name() + "'");
}

void pp330::loadCalibration()
{
  /**  If offset filename is a link, try to deduce the real filename
   *   Otherwise check if file exists
   */
  string inname(_infilename =="NotSet"? name() + ".lnk" : _infilename);
  QFileInfo innameInfo(QString::fromStdString(inname));
  if (innameInfo.isSymLink())
  {
    if (innameInfo.exists())
      inname = innameInfo.symLinkTarget().toStdString();
    else
    {
      Log::add(Log::ERROR,"pp330::loadCalibration() '" + name() +
               "': The given input filename '" + inname +
               "' is a link to a non existing file. Skip loading the data!");
      return;
    }
  }
  else if(!innameInfo.exists())
  {
    Log::add(Log::ERROR,"pp330::loadCalibration() '"+ name() +
             "': The given input filename '" + inname +
             "' does not exist. Skip loading the data!");
    return;
  }
  /** in case the previous was just a link set the info the real name and get
   *  the creation data
   */
  innameInfo.setFile(QString::fromStdString(inname));
  _lastwritten = innameInfo.created().toTime_t();
  /** read the data from the file */
  Log::add(Log::VERBOSEINFO, "pp330::loadCalibration(): '" + name() +
           "' Load Darkcal data from file '" + inname +"'");
  ifstream in(inname.c_str(), ios::binary);
  if (!in.is_open())
  {
    Log::add(Log::ERROR,"pp330::loadCalibration() '" + name() +
             "' Could not open '" + inname + "'. Skip loading the data.");
    return;
  }
  in.seekg(0,std::ios::end);
  const size_t size(in.tellg() / 2 / sizeof(double));
  in.seekg(0,std::ios::beg);
  vector<double> offsets(size);
  in.read(reinterpret_cast<char*>(&offsets[0]), size*sizeof(double));
  vector<double> noises(size);
  in.read(reinterpret_cast<char*>(&noises[0]), size*sizeof(double));
  /** check if data is of the right size */
  result_t &result(*_result);
  const size_t sizeOfImage(result.shape().first*result.shape().second/nbrOfOutputs);
  if (size != sizeOfImage)
  {
    Log::add(Log::ERROR,"pp330::loadCalibration() '" + name() +
             "' The size of the loaded data '" + toString(size) + "' in '" +
             inname + "' does not match the size of the input image '" +
             toString(sizeOfImage) + "'. Skip loading the data.");
    return;
  }
  /** copy it to the result container */
  result_t::iterator meanbegin(result.begin() + _meanBeginOffset);
  copy(offsets.begin(),offsets.end(),meanbegin);
  result_t::iterator stdvbegin(result.begin() + _stdvBeginOffset);
  copy(noises.begin(),noises.end(),stdvbegin);
  /** set the number of fills to the number of training images */
  result_t::iterator nValsBegin(result.begin() + _nValBeginOffset);
  result_t::iterator nValsEnd(result.begin() + _nValEndOffset);
  fill(nValsBegin,nValsEnd,_minTrainImages);
  /** set up the bad pixel map */
  setBadPixMap();
}

void pp330::writeCalibration()
{
  /** check if a proper name is given otherwise autogenerate a name from the
   *  name of the processor and the current time
   */
  const QDateTime now(QDateTime::currentDateTime());
  string outname;
  if (_filename == "NotSet")
    outname = name() + "_" + now.toString("yyyyMMdd_HHmm").toStdString() + ".cal";
  else
    outname = _filename;

  /** write the calibration to the file */
  ofstream out(outname.c_str(), ios::binary);
  if (!out.is_open())
    throw invalid_argument("pp330::writeCalibration(): Error opening file '" +
                           outname + "'");

  const result_t &result(*_result);
  const size_t sizeOfImage(result.shape().first*result.shape().second/nbrOfOutputs);

  vector<double> offsets(sizeOfImage);
  result_t::const_iterator meanbegin(result.begin() + _meanBeginOffset);
  result_t::const_iterator meanend(result.begin() + _meanEndOffset);
  copy(meanbegin,meanend,offsets.begin());
  out.write(reinterpret_cast<char*>(&offsets[0]), offsets.size()*sizeof(double));

  vector<double> noises(sizeOfImage);
  result_t::const_iterator stdvbegin(result.begin() + _stdvBeginOffset);
  result_t::const_iterator stdvend(result.begin() + _stdvEndOffset);
  copy(stdvbegin,stdvend,noises.begin());
  out.write(reinterpret_cast<char*>(&noises[0]), noises.size()*sizeof(double));

  /** remember when this was written */
  _lastwritten = now.toTime_t();

  /** if no proper filename was given, create a link to the current file
   *  with a more general filename
   */
  if (_filename == "NotSet")
  {
    string linkname(name() +".lnk");
    if (QFile::exists(QString::fromStdString(linkname)))
      if(!QFile::remove(QString::fromStdString(linkname)))
        throw runtime_error("pp330::writeCalibration: '" + name() +
                            "' could not remove already existing link '" +
                            linkname + "'");
    if (!QFile::link(QString::fromStdString(outname),QString::fromStdString(linkname)))
      throw runtime_error("pp330::writeCalibration: '" + name() +
                          "' could not create a link named '"+ linkname +
                          "' that points to the outputfile '" + outname + "'");
  }
}

void pp330::setBadPixMap()
{
  /** get iterators to the mean, mask, nVals and stdv values */
  result_t &result(*_result);
  const size_t sizeOfImage(result.shape().first*result.shape().second/nbrOfOutputs);

  result_t::const_iterator meanBegin(result.begin() + _meanBeginOffset);
  result_t::const_iterator meanEnd(result.begin() + _meanEndOffset);
  result_t::const_iterator stdvBegin(result.begin() + _stdvBeginOffset);
  result_t::const_iterator stdvEnd(result.begin() + _stdvEndOffset);
  result_t::const_iterator nValsBegin(result.begin() + _nValBeginOffset);
  result_t::iterator badPixBegin(result.begin() + _bPixBeginOffset);

  /** boundaries for bad pixels based upon the noise map */
  float stdvLowerBound(_NoiseLowerBound);
  float stdvUpperBound(_NoiseUpperBound);
  if (_autoNoiseSNR > 0.f)
  {
    /** calculate the value boundaries for bad pixels from the statistics of
     *  the stdv values remove outliers when calculating the mean and stdv */
    CummulativeStatisticsNoOutlier<float> stat(_autoNoiseSNRStat);
    stat.addDistribution(stdvBegin,stdvEnd);
    stdvLowerBound = stat.mean() - (_autoNoiseSNR * stat.stdv());
    stdvUpperBound = stat.mean() + (_autoNoiseSNR * stat.stdv());
    Log::add(Log::INFO,"pp330::setBadPixMap '" + name() +
             "': The automatically determined boundaries for bad pixels based " +
             "upon Noisemap are: low '" + toString(stdvLowerBound) + "' up '" +
             toString(stdvUpperBound) + "'. (Center '" + toString(stat.mean()) +
             "', Width '" + toString(stat.stdv()) + "')");
  }

  /** boundaries for bad pixels based upon the offset map */
  float meanLowerBound(_OffsetLowerBound);
  float meanUpperBound(_OffsetUpperBound);
  if (_autoOffsetSNR > 0.f)
  {
    /** calculate the value boundaries for bad pixels from the statistics of
     *  the stdv values */
    CummulativeStatisticsNoOutlier<float> stat(_autoOffsetSNRStat);
    stat.addDistribution(meanBegin,meanEnd);
    meanLowerBound = stat.mean() - (_autoOffsetSNR * stat.stdv());
    meanUpperBound = stat.mean() + (_autoOffsetSNR * stat.stdv());
    Log::add(Log::INFO,"pp330::setBadPixMap '" + name() +
             "': The automatically determined boundaries for bad pixels  based "+
             "upon Offsetmap are: low '" + toString(meanLowerBound) + "' up '" +
             toString(meanUpperBound) + "'. (Center '" + toString(stat.mean()) +
             "', Width '" + toString(stat.stdv()) + "')");
  }
  /** set all pixels as bad, whos noise or offset value is an outlier of the
   *  statistics of the noise values, or offset values.
   */
  float minpixels(_minNbrPixels*_counter);
  for (size_t iPix=0; iPix < sizeOfImage; ++iPix)
  {
    bool badpix(false);
    if (stdvBegin[iPix] < stdvLowerBound ||
        stdvBegin[iPix] > stdvUpperBound)
      badpix = true;
    if (meanBegin[iPix] < meanLowerBound ||
        meanBegin[iPix] > meanUpperBound)
      badpix = true;
    if (nValsBegin[iPix] < minpixels)
      badpix = true;
    /** set bad pixel or reset if requested */
    if (badpix)
      badPixBegin[iPix] = 1;
    else if (_resetBadPixel)
      badPixBegin[iPix] = 0;
  }
}

void pp330::aboutToQuit()
{
  if (_write)
    writeCalibration();
}

void pp330::processCommand(string command)
{
  /** if the command orders us to start the darkcal,
   *  clear all variables and start the calibration
   */
  if(command == "startDarkcal")
  {
    Log::add(Log::INFO,"pp330::processCommand: '" + name() +
             "'starts collecting data for dark calibration");
    _trainstorage.clear();
    _counter = 0;
    _train = true;
  }
}

void pp330::cummulativeUpdate(result_t::const_iterator image,
                              result_t::iterator meanAr,
                              result_t::iterator stdvAr,
                              result_t::iterator nValsAr,
                              const size_t sizeOfImage)
{
  for(size_t iPix(0); iPix < sizeOfImage; ++iPix)
  {
    const float mean(meanAr[iPix]);
    const float stdv(stdvAr[iPix]);
    const float pix(image[iPix]);

    /** if pixel is outlier skip pixel */
    if(_snr * stdv < pix - mean)
      return;

    const float nVals(nValsAr[iPix] + 1);
    const float delta(pix - mean);
    const float newmean(mean + (delta / nVals));
    const float M2(stdv*stdv * (nVals-2));
    const float newM2(M2 + delta * (pix - mean) );
    const float newstdv((nVals < 2) ? 0 : sqrt(newM2/(nVals-1)));

    meanAr[iPix] = newmean;
    stdvAr[iPix] = newstdv;
    nValsAr[iPix] = nVals;
  }
}

void pp330::movingUpdate(result_t::const_iterator image,
                         result_t::iterator meanAr,
                         result_t::iterator stdvAr,
                         result_t::iterator nValsAr,
                         const size_t sizeOfImage)
{
  for(size_t iPix(0); iPix < sizeOfImage; ++iPix)
  {
    const float mean(meanAr[iPix]);
    const float stdv(stdvAr[iPix]);
    const float pix(image[iPix]);
    const float nVals(nValsAr[iPix] + 1);

    /** if pixel is outlier skip pixel */
    if(_snr * stdv < pix - mean)
      continue;

    /** update the estimate of the mean and stdv */
    const float newmean((1-_alpha)*mean + _alpha*pix);
    const float newstdv(sqrt(_alpha*(pix - mean)*(pix - mean) + (1.f - _alpha)*stdv*stdv));

    meanAr[iPix] = newmean;
    stdvAr[iPix] = newstdv;
    nValsAr[iPix] = nVals;
  }
}

void pp330::process(const CASSEvent &evt, result_t &result)
{
  const result_t &image(_image->result(evt.id()));
  const size_t sizeOfImage(image.shape().first * image.shape().second);

  result_t::iterator meanAr(result.begin()  + _meanBeginOffset);
  result_t::iterator stdvAr(result.begin()  + _stdvBeginOffset);
  result_t::iterator nValsAr(result.begin() + _nValBeginOffset);

  QReadLocker lock(&image.lock);

  if (_train || _update)
    ++_counter;

  if (_train)
  {
    if (_trainstorage.size() < _minTrainImages)
      _trainstorage.push_back(image.clone());
    /** @note we shouldn't use else here, because in this case we need one more,
     *        but unused image to start the calibration
     */
    if (_trainstorage.size() == _minTrainImages)
    {
      Log::add(Log::INFO,"pp330::process: '" + name() +
               "' done collecting images for darkcalibration. Calculating maps");
      //generate the initial calibration
      for (size_t iPix=0; iPix < sizeOfImage; ++iPix)
      {
        CummulativeStatisticsNoOutlier<float> stat(_snr);
        for (size_t iStore=0; iStore < _trainstorage.size(); ++iStore)
          stat.addDatum((*(_trainstorage[iStore]))[iPix]);

        meanAr[iPix] = stat.mean();
        stdvAr[iPix] = stat.stdv();
        nValsAr[iPix] = stat.nbrPointsUsed();
      }
      /** mask bad pixels based upon the calibration */
      setBadPixMap();

      /** write the calibration */
      if (_write)
        writeCalibration();
      /** reset the training variables */
      _train = false;
      _trainstorage.clear();
      Log::add(Log::INFO,"pp330::process: '" + name() +
               "' done calculating maps");
    }
  }
  else if (_update)
  {
    /** add current image to statistics of the calibration */
    _updateStatistics(image.begin(),meanAr,stdvAr,nValsAr,sizeOfImage);

    /** update the bad pix map and the file if requested */
    if ((_counter % _updatePeriod) == 0)
      setBadPixMap();
    const uint32_t now(QDateTime::currentDateTime().toTime_t());
    if (_write && (_updateWritePeriod < (now - _lastwritten)))
      writeCalibration();
  }
}





//********** gain calibrations ******************

pp331::pp331(const name_t &name)
  : AccumulatingProcessor(name)
{
  loadSettings(0);
}

void pp331::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _image = setupDependency("Image");
  setupGeneral();
  bool ret (setupCondition());
  if (!(_image && ret))
    return;

  if (_image->result().dim() != 2)
    throw invalid_argument("pp331::loadSettings: '" + name() + "' input '" +
                           _image->name() + "' is not a 2d histogram");

  _isPnCCDNoCTE = s.value("IsPnCCDNoCTE",false).toBool();

  _counter = 0;
  _nFrames = s.value("NbrOfFrames",-1).toInt();
  _filename = s.value("Filename","out.cal").toString().toStdString();
  _write = s.value("WriteCal",true).toBool();
  _aduRange = make_pair(s.value("ADURangeLow",0).toFloat(),
                        s.value("ADURangeHigh",0).toFloat());
  _minPhotonCount = s.value("MinimumNbrPhotons",200).toUInt();
  _constGain = s.value("DefaultGainValue",1).toFloat();

  const result_t &image(_image->result());
  result_t::shape_t shape(image.shape());
  if (_isPnCCDNoCTE && (shape.first != 1024 || shape.second != 1024))
    throw invalid_argument("pp331::loadSettings(): '" + name() +
                           "' should be a pnCCD, but cols '" +
                           toString(shape.first) + "' and rows '"
                           + toString(shape.second) +
                           "' don't indicate a pnCCD");

  _sizeOfImage = shape.first * shape.second;
  _gainOffset  = 0*_sizeOfImage;
  _countOffset = 1*_sizeOfImage;
  _aveOffset   = 2*_sizeOfImage;

  createHistList(result_t::shared_pointer(new result_t(shape.first,3*shape.second)));

  loadCalibration();
  Log::add(Log::INFO,"processor " + name() +
           ": generates the gain calibration from images contained in '" +
           _image->name() + "'. Condition is '" + _condition->name() + "'");
}

void pp331::processCommand(std::string command)
{
  if (command == "startGain")
  {

  }
}

void pp331::loadCalibration()
{

}

void pp331::writeCalibration()
{
  ofstream out(_filename.c_str(), ios::binary);
  if (!out.is_open())
    throw invalid_argument("pp331::writeCalibration(): Error opening file '" +
                           _filename + "'");
  const result_t &image(*_result);
  out.write(reinterpret_cast<const char*>(&image.front()), _sizeOfImage*sizeof(float));
}

void pp331::aboutToQuit()
{
  calculateGainMap(*_result);
  if (_write)
    writeCalibration();
}

void pp331::calculateGainMap(result_t &gainmap)
{
  /** calculate the average of the average pixelvalues, disregarding pixels
   *  that have not seen enough photons in the right ADU range.
   */
  int counter(0);
  double average(0);

  result_t::iterator gain(gainmap.begin() + _gainOffset);
  result_t::const_iterator count(gainmap.begin() + _countOffset);
  result_t::const_iterator ave(gainmap.begin() + _aveOffset);
  for (size_t i(0); i < _sizeOfImage; ++i, ++count, ++ave)
  {
    if (*count < _minPhotonCount)
      continue;
    ++counter;
    average += (*ave - average)/counter;
  }

  /** assining the gain value for each pixel that has seen enough statistics.
   *  gain is calculated by formula
   *  \f$ gain = frac{average_average_pixelvalue}{average_pixelvalue} \f$
   *  If not enough photons are in the pixel, set the predefined user value
   */
  count = gainmap.begin() + _countOffset;
  ave = gainmap.begin() + _aveOffset;
  for (size_t i(0); i < _sizeOfImage; ++i, ++gain, ++count, ++ave)
    *gain = (*count < _minPhotonCount) ? _constGain : average/(*ave);
}

void pp331::process(const CASSEvent &evt, result_t &result)
{
  const result_t &image(_image->result(evt.id()));
  QReadLocker lock(&image.lock);

  const size_t cols(image.shape().first);

  result_t::const_iterator pixel(image.begin());
  result_t::const_iterator ImageEnd(image.end());

  result_t::iterator gain(result.begin() + _gainOffset);
  result_t::iterator count(result.begin()+ _countOffset);
  result_t::iterator ave(result.begin()  + _aveOffset);

  /** go though all pixels of image*/
  for (size_t i(0); pixel != ImageEnd; ++i, ++pixel, ++gain, ++count, ++ave)
  {
    /** check if pixel is within the 1 photon adu range */
    if (_aduRange.first < *pixel && *pixel < _aduRange.second)
    {
      /** calculate the mean pixel value */
      *count += 1;
      *ave += ((*pixel - *ave) / *count);

      /** set the same value as for the pixel for all pixels in the columns
       *  in the quadrant
       */
      if (_isPnCCDNoCTE)
      {
        /** find out which which column and row we're currently working on */
        const size_t col (i % cols);
        const size_t row (i / cols);
        /** get pointers to the corresponding starts of the maps */
        result_t::iterator gaincol(result.begin() + _gainOffset + col);
        result_t::iterator countcol(result.begin()+ _countOffset + col);
        result_t::iterator avecol(result.begin()  + _aveOffset + col);
        /** when the pixel is in the upper half of the detector advance the
         *  pointer to the upper half
         */
        if (row >= 512)
        {
          gaincol  += 512*1024;
          countcol += 512*1024;
          avecol   += 512*1024;
        }

        const float currentgain(*gain);
        const float currentcount(*count);
        const float currentave(*ave);
        for (int ii=0; ii<512; ++ii)
        {
          *gaincol = currentgain;
          gaincol += 1024;

          *countcol = currentcount;
          countcol += 1024;

          *avecol = currentave;
          avecol += 1024;
        }
      }
    }
  }

  /** if we have reached the requested nbr of frames calculate the gain map */
  ++_counter;
  if (_counter % _nFrames == 0)
    calculateGainMap(result);
}








//********** hot pixel detection ******************

pp332::pp332(const name_t &name)
  : AccumulatingProcessor(name)
{
  loadSettings(0);
}

void pp332::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _image = setupDependency("Image");
  setupGeneral();
  bool ret (setupCondition());
  if (!(_image && ret))
    return;

  _counter = 0;
  _nFrames = s.value("NbrOfFrames",-1).toInt();
  _filename = s.value("Filename","out.cal").toString().toStdString();
  _write = s.value("WriteCal",true).toBool();
  _aduRange = make_pair(s.value("ADURangeLow",0).toFloat(),
                        s.value("ADURangeUp",0).toFloat());
  _maxConsecutiveCount = s.value("MaximumConsecutiveFrames",5).toUInt();
  _maxADUVal = s.value("MaxADUValue",1e6).toFloat();

  const result_t &image(_image->result());
  result_t::shape_t shape(image.shape());
  createHistList(result_t::shared_pointer(new result_t(shape.first,2*shape.second)));
  loadHotPixelMap();
  Log::add(Log::INFO,"processor " + name() +
           ": generates the hot pixel map from images contained in '" +
           _image->name() + "'. Condition is '" + _condition->name() + "'");
}

void pp332::loadHotPixelMap()
{
  ifstream in(_filename.c_str(), ios::binary);
  if (!in.is_open())
  {
    Log::add(Log::WARNING,"pp332::loadHotPixelMap: Could not open '" + _filename +
             "'. Skipping reading the hot pixels mask.");
    return;
  }
  in.seekg(0,std::ios::end);
  const int valuesize(sizeof(mask_t));
  const size_t size = in.tellg() / valuesize;
  in.seekg(0,std::ios::beg);
  vector<mask_t> hotpixmask(size);
  in.read(reinterpret_cast<char*>(&hotpixmask.front()), size*valuesize);

  result_t &result(*_result);

  const size_t sizeOfImage(result.shape().first*result.shape().second/2);
  if (sizeOfImage != size)
  {
    Log::add(Log::WARNING,"pp332::loadHotPixelMap: Size of mask to load '" +
             toString(size) + "' does not fit with size of image '" +
             toString(sizeOfImage) + "'. Skipping reading the hot pixels mask in '" +
             _filename +"'.");
    return;
  }
  copy(hotpixmask.begin(),hotpixmask.end(),result.begin());
}

void pp332::writeHotPixelMap()
{
  ofstream out(_filename.c_str(), ios::binary);
  if (!out.is_open())
    throw invalid_argument("pp332::writeCalibration(): Error opening file '" +
                           _filename + "'");

  const result_t &result(*_result);
  const size_t sizeOfImage(result.shape().first*result.shape().second/2);

  vector<mask_t> mask(sizeOfImage);
  copy(result.begin(),result.begin()+sizeOfImage,mask.begin());
  out.write(reinterpret_cast<char*>(&mask.front()), sizeOfImage*sizeof(mask_t));
}

void pp332::aboutToQuit()
{
  if (_write)
    writeHotPixelMap();
}

void pp332::process(const CASSEvent &evt, result_t &result)
{
  const result_t &image(_image->result(evt.id()));
  QReadLocker lock(&image.lock);

  result_t::const_iterator pixel(image.begin());
  result_t::const_iterator ImageEnd(image.end());
  const size_t sizeOfImage(image.shape().first * image.shape().second);

  result_t::iterator hotpix(result.begin());
  result_t::iterator count(result.begin()+1*sizeOfImage);

  /** go though all pixels of image*/
  for (; pixel != ImageEnd; ++pixel, ++count, ++hotpix)
  {
    /** check if pix is not masked as hot */
    if (fuzzycompare(*hotpix,-1.f))
      continue;

    /** check if pixel is within the hot pixel adu range */
    if (_aduRange.first < *pixel && *pixel < _aduRange.second)
    {
      *count += 1;
      if (_maxConsecutiveCount <= *count)
        *hotpix = -1;
    }
    else
      *count = 0;

    /** check if pixel exceeds maximum allowed adu value */
    if (_maxADUVal < *pixel)
    {
      *hotpix = -1;
    }
  }

//  /** if we have reached the requested nbr of frames calculate the gain map */
//  ++_counter;
//  if (_counter % _nFrames == 0)
//    calculateGainMap(result);

}








//********** common mode background calculation ******************

pp333::pp333(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp333::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _image = setupDependency("Image");
  setupGeneral();
  bool ret (setupCondition());
  if (!(_image && ret))
    return;

  _width = s.value("Width",-1).toInt();
  _snr = s.value("SNR",4).toFloat();
  string calctype(s.value("CalculationType","mean").toString().toStdString());
  if(calctype == "mean")
    _calcCommonmode = bind(&pp333::meanCalc,this,_1,_2);
  else if(calctype == "median")
    _calcCommonmode = bind(&pp333::medianCalc,this,_1,_2);
  else
    throw invalid_argument("pp333::loadSettings() '" + name() +
                           "': Calculation type '" + calctype + "' is unkown.");


//  const Histogram2DFloat &image(dynamic_cast<const Histogram2DFloat&>(_image->result()));
  createHistList(_image->result().clone());
  Log::add(Log::INFO,"processor " + name() +
           ": generates the common mode background level of  '" +
           _image->name() + "' using calculation type '" + calctype +
           "'. Condition is '" + _condition->name() + "'");
}

float pp333::meanCalc(result_t::const_iterator begin,
                      result_t::const_iterator end)
{
  CummulativeStatisticsNoOutlier<float> stat(_snr);
  stat.addDistribution(begin,end);
  return stat.mean();
}

float pp333::medianCalc(result_t::const_iterator begin,
                        result_t::const_iterator end)
{
  MedianCalculator<float> stat;
  stat.addDistribution(begin,end);
  return stat.median();
}

void pp333::process(const CASSEvent &evt, result_t &result)
{
  const result_t &image(_image->result(evt.id()));
  QReadLocker lock(&image.lock);

  /** retrieve iterators to the storages and the size of the image */
  result_t::const_iterator imageIt(image.begin());
  result_t::iterator resultIt(result.begin());
  const size_t sizeOfImage(image.shape().first * image.shape().second);
  const size_t parts(sizeOfImage / _width);

  /** go though all common mode parts of image*/
  for (size_t part(0); part < parts; ++part)
  {
    /** calculate the common mode for the part */
    result_t::const_iterator startPart_Image(imageIt + part*_width);
    result_t::const_iterator endPart_Image(startPart_Image + _width);
    const float commonmodeLevel(_calcCommonmode(startPart_Image,endPart_Image));

    /** fill the result part with the calculated common mode */
    result_t::iterator startPart_Res(resultIt + part*_width);
    result_t::iterator endPart_Res(startPart_Res + _width);
    fill(startPart_Res, endPart_Res, commonmodeLevel);
  }
}








//********** common mode background calculation using histogram **************

pp334::pp334(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp334::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _image = setupDependency("Image");
  _width = s.value("Width",30).toFloat();
  _maxDist = s.value("MaxDistance",5).toFloat();
  _checks = s.value("EnableChecks",false).toBool();
  setupGeneral();
  bool ret (setupCondition());
  if (!(_image && ret))
    return;

  result_t::shared_pointer result(_image->result().clone());
  if (_checks)
  {
    result_t::storage_t rows(64*result->shape().first);
    result->appendRows(rows);
  }
  createHistList(result);
  Log::add(Log::INFO,"processor " + name() +
           ": generates the common mode background level of  '" +
           _image->name() + "' using either histogram or unbonded pixels"+
           "'. Condition is '" + _condition->name() + "'");
}

void pp334::process(const CASSEvent &evt, result_t &result)
{
  const result_t &image(_image->result(evt.id()));
  QReadLocker lock(&image.lock);

  /** define a few things for the hisogram and asics */
  const uint32_t nAsics(64);
  const uint32_t nAsicsPerRow(2);
  const uint32_t nColsAsic(194);
  const uint32_t nRowsAsic(185);
  const uint32_t nColsInput(nAsicsPerRow*nColsAsic);
  const float low(-60);
  const float up(100);
  const int32_t nBins(up-low);

  /** create list of histograms */
  typedef vector<double> hists_t;
  hists_t hists(nAsics*nBins,0);

  /** iterate through image */
  for (int i(0); i < static_cast<int>(image.datasize()); ++i)
  {
    /** get the pixel value */
    const result_t::value_t pixelval(image[i]);
    /** skip pixel if it is masked */
    if (fuzzyIsNull(pixelval))
      continue;
    /** find out in which row of the image we're */
    const uint16_t row(i/nColsInput);
    /** find out where we're on the row */
    const uint16_t colInRow(i%nColsInput);
    /** find out on which asic of the chip we're */
    const uint8_t asicOnChip(colInRow/nColsAsic);
    /** find out which chip we're on */
    const uint8_t chip(row/nRowsAsic);
    /** calc which asic we're on */
    const uint8_t asic(nAsicsPerRow*chip + asicOnChip);
    /** determine the bin in which the pixels values will fall */
    const int32_t bin(static_cast<int32_t>(nBins*(pixelval-low)/(up-low)));
    /** check if the bin is out of bounds, if so skip it */
    if ((bin < 0) || (nBins <= bin))
      continue;
    /** add the pixel to the histogram */
    hists[(asic*nBins) + bin] += 1;
  }

  /** determine the center of mass of the first peak in each histogram */
  hists_t histsCMVals(nAsics,0);
  for (uint32_t asic(0); asic<nAsics; ++asic)
  {
    hists_t::const_iterator histStart(hists.begin()+(asic*nBins));
    hists_t::const_iterator histEnd(histStart+nBins);
    hists_t::const_iterator pToMax(max_element(histStart,histEnd));
    int bin(distance(histStart,pToMax));
    if (((bin-_width) < 0) || ((bin+_width+1) > nBins))
    {
      histsCMVals[asic] = 1e6;
      continue;
    }

    hists_t::const_iterator peakBegin(pToMax-_width);
    hists_t::const_iterator peakEnd(pToMax+_width+1);
    const double integral(accumulate(peakBegin,peakEnd,0.));
    hists_t bins;
    bins.reserve(_width*2+1);
    for (int i(bin-_width); i<bin+_width+1;++i)
      bins.push_back(low + (i*(up-low)/nBins));
    hists_t weights;
    weights.reserve(_width*2+1);
    transform(peakBegin,peakEnd,bins.begin(),back_inserter(weights),
              multiplies<double>());
    const double weight(accumulate(weights.begin(),weights.end(),0.));
    const double com(weight/integral);

    histsCMVals[asic] = com;
  }

  /** go through unbonded pixels of each asic of image and calculate the mean */
  hists_t unbondedPixCMVals(nAsics,0);
  const int sizeBetweenAsics(nColsAsic);
  const int sizeBetweenUnbondedPixels(3696);
  const int sizeBetweenChips(1566);
  const int nUBP(19);
  const int nChips(16);

  result_t::const_iterator pointer(image.begin());
  for (int chip=0; chip<nChips; ++chip)
  {
    int asic(2*chip);
    for (int up=0; up<nUBP-1; ++up)
    {
      //cout <<"UBP'"<<up<< "', Asic'" << asic << "'(" << pointer<< ")"<<endl;
      unbondedPixCMVals[asic] += *pointer;
      pointer += sizeBetweenAsics;
      //cout <<"UBP'"<<up<< "', Asic'" << asic+1 << "'(" << pointer<< ")"<<endl;
      pointer += sizeBetweenUnbondedPixels;
      unbondedPixCMVals[asic+1] += *pointer;
    }
    //cout <<"UBP'"<<18<< "', Asic'" << asic << "'(" << pointer<< ")"<<endl;
    unbondedPixCMVals[asic] += *pointer;
    pointer += sizeBetweenAsics;
    //cout <<"UBP'"<<18<< "', Asic'" << asic+1 << "'(" << pointer<< ")"<<endl;
    unbondedPixCMVals[asic+1] += *pointer;
    pointer += sizeBetweenChips;
  }

  for (uint32_t asic(0); asic < nAsics; ++asic)
  {
    unbondedPixCMVals[asic] /= static_cast<float>(nUBP);
  }


  /** check if the maximum of the histogram is close to the mean of the unbonded
   *  pixels, if so than the common mode is the value determined by the
   *  histogram, otherwise its the mean value of the unbonded pixels
   */
  hists_t CMVals(nAsics,0);
  for (uint32_t asic(0); asic < nAsics; ++asic)
  {
     CMVals[asic] = (fabs(unbondedPixCMVals[asic]-histsCMVals[asic]) < _maxDist) ?
           histsCMVals[asic] : unbondedPixCMVals[asic];
  }

  /** set the pixels of the result to the determined common mode value */
  /** iterate through image */
  for (int i(0); i < static_cast<int>(image.datasize()); ++i)
  {
    /** get the pixel value of the original image*/
    const result_t::value_t pixelval(image[i]);
    /** skip pixel if it is masked */
    if (fuzzyIsNull(pixelval))
      continue;
    /** find out in which row of the image we're */
    const uint16_t row(i/nColsInput);
    /** find out where we're on the row */
    const uint16_t colInRow(i%nColsInput);
    /** find out on which asic of the chip we're */
    const uint8_t asicOnChip(colInRow/nColsAsic);
    /** find out which chip we're on */
    const uint8_t chip(row/nRowsAsic);
    /** calc which asic we're on */
    const uint8_t asic(nAsicsPerRow*chip + asicOnChip);
    /** set the common mode value of the pixel */
    result[i] = CMVals[asic];
  }

  /** if the checks are enabled add them to the end of the result */
  if (_checks)
  {
    for (uint32_t asic(0); asic < nAsics; ++asic)
    {
      /** set the pos of the output line */
      result_t::storage_t::iterator res(result.begin() +
                                        image.datasize() +
                                        asic*image.shape().first);
      /** add the histogram */
      hists_t::const_iterator histStart(hists.begin()+(asic*nBins));
      hists_t::const_iterator histEnd(histStart+nBins);
      res = copy(histStart,histEnd,res);
      /** add the hist cm value */
      *res = histsCMVals[asic];
      ++res;
      /** add the unbonded pixel cm value */
      *res = unbondedPixCMVals[asic];
      ++res;
      /** add the used cm value */
      *res = CMVals[asic];
    }
  }
}
