// Copyright (C) 2013 Lutz Foucar

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include "pixel_detector_calibration.h"

#include "histogram.h"
#include "cass_settings.h"
#include "statistics_calculator.hpp"
#include "log.h"


using namespace cass;
using namespace std;
using tr1::bind;
using tr1::placeholders::_1;
using tr1::placeholders::_2;


//********** offset/noise calibrations ******************

pp330::pp330(const name_t &name)
  : AccumulatingPostProcessor(name)
{
  loadSettings(0);
}

void pp330::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _image = setupDependency("RawImage");
  setupGeneral();
  bool ret (setupCondition());
  if (!(_image && ret))
    return;

  /** load parameters from the ini file */
  _autoNoiseSNR = s.value("SNRNoiseAutoBoundaries",4).toFloat();
  _NoiseLowerBound = s.value("NoiseLowerBoundary",1).toFloat();
  _NoiseUpperBound = s.value("NoiseUpperBoundary",3).toFloat();
  _autoOffsetSNR = s.value("SNROffsetAutoBoundaries",-1).toFloat();
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
  _updatePeriod = s.value("UpdateBadPixAndSavePeriod",-1).toInt();

  /** reset the variables */
  _trainstorage.clear();
  _counter = 0;

  /** determine the offset of the output arrays from the size of the input image */
  pair<size_t,size_t> shape(dynamic_cast<const Histogram2DFloat&>(_image->result()).shape());
  const size_t imagesize(shape.first*shape.second);
  _meanBeginOffset = MEAN * imagesize;
  _meanEndOffset = (MEAN + 1) * imagesize;
  _stdvBeginOffset = STDV * imagesize;
  _stdvEndOffset = (STDV + 1) * imagesize;
  _bPixBeginOffset = BADPIX * imagesize;
  _bPixEndOffset = (BADPIX + 1) * imagesize;
  _nValBeginOffset = NVALS * imagesize;

  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(shape.first,nbrOfOutputs*shape.second)));
  loadCalibration();
  Log::add(Log::INFO,"Postprocessor " + name() +
           ": generates the calibration data from images contained in '" +
           _image->name() +
           "' autoNoiseSNR '" + toString(_autoNoiseSNR) +
           "' NoiselowerBound '" + toString(_NoiseLowerBound) +
           "' NoiseupperBound '" + toString(_NoiseUpperBound) +
           "' autoOffsetSNR '" + toString(_autoOffsetSNR) +
           "' OffsetLowerBound '" + toString(_OffsetLowerBound) +
           "' OffsetUpperBound '" + toString(_OffsetUpperBound) +
           "' minNbrPixels '" + toString(_minNbrPixels) +
           "' outputfilename '" + toString(_filename) +
           "' write '" + (_write?"true":"false") +
           "' train '" + (_train?"true":"false") +
           "' nbrTrainImages '" + toString(_minTrainImages) +
           "' SNR '" + toString(_snr) +
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
  const Histogram2DFloat &res(dynamic_cast<const Histogram2DFloat&>(*_result));
  const size_t sizeOfImage(res.shape().first*res.shape().second/nbrOfOutputs);
  if (size != sizeOfImage)
  {
    Log::add(Log::ERROR,"pp330::loadCalibration() '" + name() +
             "' The size of the loaded data '" + toString(size) + "' in '" +
             inname + "' does not match the size of the input image '" +
             toString(sizeOfImage) + "'. Skip loading the data.");
    return;
  }
  /** copy it to the result container */
  Histogram2DFloat::storage_t &result
      (dynamic_cast<Histogram2DFloat*>(_result.get())->memory());
  Histogram2DFloat::storage_t::iterator meanbegin(result.begin() + _meanBeginOffset);
  copy(offsets.begin(),offsets.end(),meanbegin);
  Histogram2DFloat::storage_t::iterator stdvbegin(result.begin() + _stdvBeginOffset);
  copy(noises.begin(),noises.end(),stdvbegin);
  /** set up the bad pixel map */
  setBadPixMap();
}

void pp330::writeCalibration()
{
  /** check if a proper name is given otherwise autogenerate a name from the
   *  name of the postprocessor and the current time
   */
  string outname;
  if (_filename == "NotSet")
    outname = name() + "_" +
              QDateTime::currentDateTime().toString("yyyyMMdd_HHmm").toStdString() +
              ".cal";
  else
    outname = _filename;

  /** write the calibration to the file */
  ofstream out(outname.c_str(), ios::binary);
  if (!out.is_open())
    throw invalid_argument("pp330::writeCalibration(): Error opening file '" +
                           outname + "'");

  const Histogram2DFloat::storage_t &result
      (dynamic_cast<const Histogram2DFloat*>(_result.get())->memory());
  const Histogram2DFloat &res(dynamic_cast<const Histogram2DFloat&>(*_result));
  const size_t sizeOfImage(res.shape().first*res.shape().second/nbrOfOutputs);

  vector<double> offsets(sizeOfImage);
  Histogram2DFloat::storage_t::const_iterator meanbegin(result.begin() + _meanBeginOffset);
  Histogram2DFloat::storage_t::const_iterator meanend(result.begin() + _meanEndOffset);
  copy(meanbegin,meanend,offsets.begin());
  out.write(reinterpret_cast<char*>(&offsets[0]), offsets.size()*sizeof(double));

  vector<double> noises(sizeOfImage);
  Histogram2DFloat::storage_t::const_iterator stdvbegin(result.begin() + _stdvBeginOffset);
  Histogram2DFloat::storage_t::const_iterator stdvend(result.begin() + _stdvEndOffset);
  copy(stdvbegin,stdvend,noises.begin());
  out.write(reinterpret_cast<char*>(&noises[0]), noises.size()*sizeof(double));

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
  Histogram2DFloat::storage_t &store
      (dynamic_cast<Histogram2DFloat*>(_result.get())->memory());
  Histogram2DFloat &res(dynamic_cast<Histogram2DFloat&>(*_result));
  const size_t sizeOfImage(res.shape().first*res.shape().second/nbrOfOutputs);

  Histogram2DFloat::storage_t::const_iterator meanBegin(store.begin() + _meanBeginOffset);
  Histogram2DFloat::storage_t::const_iterator meanEnd(store.begin() + _meanEndOffset);
  Histogram2DFloat::storage_t::const_iterator stdvBegin(store.begin() + _stdvBeginOffset);
  Histogram2DFloat::storage_t::const_iterator stdvEnd(store.begin() + _stdvEndOffset);
  Histogram2DFloat::storage_t::const_iterator nValsBegin(store.begin() + _nValBeginOffset);
  Histogram2DFloat::storage_t::iterator badPixBegin(store.begin() + _bPixBeginOffset);

  /** boundaries for bad pixels based upon the noise map */
  float stdvLowerBound(_NoiseLowerBound);
  float stdvUpperBound(_NoiseUpperBound);
  if (_autoNoiseSNR > 0.f)
  {
    /** calculate the value boundaries for bad pixels from the statistics of
     *  the stdv values */
    CummulativeStatisticsCalculator<float> stat;
    stat.addDistribution(stdvBegin,stdvEnd);
    stdvLowerBound = stat.mean() - _autoNoiseSNR * stat.stdv();
    stdvUpperBound = stat.mean() + _autoNoiseSNR * stat.stdv();
    Log::add(Log::INFO,"pp330::setBadPixMap '" + name() +
             "': The automatically determined boundaries for bad pixels based " +
             "upon Noisemap are: up '" + toString(stdvUpperBound) + "' lower '" +
             toString(stdvLowerBound) + "'");
  }
  /** boundaries for bad pixels based upon the offset map */
  float meanLowerBound(_OffsetLowerBound);
  float meanUpperBound(_OffsetUpperBound);
  if (_autoOffsetSNR > 0.f)
  {
    /** calculate the value boundaries for bad pixels from the statistics of
     *  the stdv values */
    CummulativeStatisticsCalculator<float> stat;
    stat.addDistribution(meanBegin,meanEnd);
    meanLowerBound = stat.mean() - _autoOffsetSNR * stat.stdv();
    meanUpperBound = stat.mean() + _autoOffsetSNR * stat.stdv();
    Log::add(Log::INFO,"pp330::setBadPixMap '" + name() +
             "': The automatically determined boundaries for bad pixels  based "+
             "upon Offsetmap are: up '" + toString(meanUpperBound) + "' lower '" +
             toString(meanLowerBound) + "'");
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

void pp330::processCommand(string command)\
{
  if(command == "startDarkcal")
    _train = true;
}

void pp330::process(const CASSEvent &evt, HistogramBackend &res)
{
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>(_image->result(evt.id())));
  const size_t sizeOfImage = image.shape().first * image.shape().second;

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));
  Histogram2DFloat::storage_t::iterator meanAr(result.memory().begin()  + _meanBeginOffset);
  Histogram2DFloat::storage_t::iterator stdvAr(result.memory().begin()  + _stdvBeginOffset);
  Histogram2DFloat::storage_t::iterator nValsAr(result.memory().begin() + _nValBeginOffset);

  QReadLocker lock(&image.lock);

  if (_train || _update)
    ++_counter;

  if (_train)
  {
    if (_trainstorage.size() < _minTrainImages)
      _trainstorage.push_back(image.memory());
    /** @note we shouldn't use else here, because in this case we need one more,
     *        but unused image to start the calibration
     */
    if (_trainstorage.size() == _minTrainImages)
    {
      //generate the initial calibration
      for (size_t iPix=0; iPix < sizeOfImage; ++iPix)
      {
        vector<float> pixdistribution;
        for (size_t iStore=0; iStore < _trainstorage.size(); ++iStore)
          pixdistribution.push_back(_trainstorage[iStore][iPix]);
        sort(pixdistribution.begin(),pixdistribution.end());

        vector<float>::iterator lowPos(pixdistribution.begin());
        vector<float>::iterator upPos(pixdistribution.end());

        CummulativeStatisticsCalculator<float> stat;
        bool outliersdetected(false);
        do
        {
          stat.reset();
          stat.addDistribution(lowPos,upPos);

          const float lowBound(stat.mean() - _snr * stat.stdv());
          const float upBound(stat.mean() + _snr * stat.stdv());
          vector<float>::iterator newLowPos(lower_bound(pixdistribution.begin(), pixdistribution.end(), lowBound));
          vector<float>::iterator newUpPos(upper_bound (pixdistribution.begin(), pixdistribution.end(), upBound));

          /** outliers have been detected when the low and up iterators have changed */
          outliersdetected = ( newLowPos != lowPos || newUpPos != upPos);

          lowPos = newLowPos;
          upPos = newUpPos;
        }
        while (outliersdetected);

        meanAr[iPix] = stat.mean();
        stdvAr[iPix] = stat.stdv();
        nValsAr[iPix] = distance(lowPos,upPos);
      }
      /** mask bad pixels based upon the calibration */
      setBadPixMap();

      /** write the calibration */
      if (_write)
        writeCalibration();
      /** reset the training variables */
      _train = false;
      _trainstorage.clear();
    }
  }
  else if (_update)
  {
    /** add current image to statistics of the calibration */
    for(size_t iPix=0; iPix < sizeOfImage; ++iPix)
    {
      const float mean(meanAr[iPix]);
      const float stdv(stdvAr[iPix]);
      const float pix(image.memory()[iPix]);
      /** if pixel is outlier skip pixel */
      if(_snr * stdv < pix - mean)
        continue;
      const float nVals(nValsAr[iPix] + 1);
      const float alpha (1./nVals);
      const float delta(pix - mean);
      const float newmean(mean + (delta * alpha));
      const float tmp(stdv*stdv / alpha);
      const float newtmp(tmp + delta * (pix - mean) );
      const float newstdv(sqrt(newtmp*alpha));

      meanAr[iPix] = newmean;
      stdvAr[iPix] = newstdv;
      nValsAr[iPix] = nVals;

    }
    /** update the bad pix map and the file if requested */
    if ((_counter % _updatePeriod) == 0)
    {
      setBadPixMap();
      if (_write)
        writeCalibration();
    }
  }
}





//********** gain calibrations ******************

pp331::pp331(const name_t &name)
  : AccumulatingPostProcessor(name)
{
  loadSettings(0);
}

void pp331::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
  s.beginGroup(QString::fromStdString(name()));
  _image = setupDependency("Image");
  setupGeneral();
  bool ret (setupCondition());
  if (!(_image && ret))
    return;

  if (_image->result().dimension() != 2)
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

  const Histogram2DFloat &image(dynamic_cast<const Histogram2DFloat&>(_image->result()));
  pair<size_t,size_t> shape(image.shape());
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

  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(shape.first,3*shape.second)));

  loadCalibration();
  Log::add(Log::INFO,"Postprocessor " + name() +
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

  const Histogram2DFloat &image(dynamic_cast<const Histogram2DFloat&>(*_result));

  const Histogram2DFloat::storage_t &gains(image.memory());
  out.write(reinterpret_cast<const char*>(&gains.front()), _sizeOfImage*sizeof(float));
}

void pp331::aboutToQuit()
{
  calculateGainMap(dynamic_cast<Histogram2DFloat&>(*_result));
  if (_write)
    writeCalibration();
}

void pp331::calculateGainMap(Histogram2DFloat& gainmap)
{
  /** calculate the average of the average pixelvalues, disregarding pixels
   *  that have not seen enough photons in the right ADU range.
   */
  int counter(0);
  double average(0);

  Histogram2DFloat::storage_t::iterator gain(gainmap.memory().begin() + _gainOffset);
  Histogram2DFloat::storage_t::const_iterator count(gainmap.memory().begin() + _countOffset);
  Histogram2DFloat::storage_t::const_iterator ave(gainmap.memory().begin() + _aveOffset);
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
  count = gainmap.memory().begin() + _countOffset;
  ave = gainmap.memory().begin() + _aveOffset;
  for (size_t i(0); i < _sizeOfImage; ++i, ++gain, ++count, ++ave)
    *gain = (*count < _minPhotonCount) ? _constGain : average/(*ave);
}

void pp331::process(const CASSEvent &evt, HistogramBackend &res)
{
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>(_image->result(evt.id())));

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  QReadLocker lock(&image.lock);
  const size_t cols(image.shape().first);

  Histogram2DFloat::storage_t::const_iterator pixel(image.memory().begin());
  Histogram2DFloat::storage_t::const_iterator ImageEnd(image.memory().end());

  Histogram2DFloat::storage_t::iterator gain(result.memory().begin() + _gainOffset);
  Histogram2DFloat::storage_t::iterator count(result.memory().begin()+ _countOffset);
  Histogram2DFloat::storage_t::iterator ave(result.memory().begin()  + _aveOffset);

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
        Histogram2DFloat::storage_t::iterator gaincol(result.memory().begin() + _gainOffset + col);
        Histogram2DFloat::storage_t::iterator countcol(result.memory().begin()+ _countOffset + col);
        Histogram2DFloat::storage_t::iterator avecol(result.memory().begin()  + _aveOffset + col);
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
  : AccumulatingPostProcessor(name)
{
  loadSettings(0);
}

void pp332::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
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

  const Histogram2DFloat &image(dynamic_cast<const Histogram2DFloat&>(_image->result()));
  pair<size_t,size_t> shape(image.shape());
  createHistList(
        tr1::shared_ptr<Histogram2DFloat>
        (new Histogram2DFloat(shape.first,2*shape.second)));
  loadHotPixelMap();
  Log::add(Log::INFO,"Postprocessor " + name() +
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

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(*_result));

  const size_t sizeOfImage(result.shape().first*result.shape().second/2);
  if (sizeOfImage != size)
  {
    Log::add(Log::WARNING,"pp332::loadHotPixelMap: Size of mask to load '" +
             toString(size) + "' does not fit with size of image '" +
             toString(sizeOfImage) + "'. Skipping reading the hot pixels mask in '" +
             _filename +"'.");
    return;
  }
  copy(hotpixmask.begin(),hotpixmask.end(),result.memory().begin());
}

void pp332::writeHotPixelMap()
{
  ofstream out(_filename.c_str(), ios::binary);
  if (!out.is_open())
    throw invalid_argument("pp332::writeCalibration(): Error opening file '" +
                           _filename + "'");

  const Histogram2DFloat &image(dynamic_cast<const Histogram2DFloat&>(*_result));
  const size_t sizeOfImage(image.shape().first*image.shape().second/2);

  const Histogram2DFloat::storage_t &hpmask(image.memory());
  vector<mask_t> mask(sizeOfImage);
  copy(hpmask.begin(),hpmask.begin()+sizeOfImage,mask.begin());
  out.write(reinterpret_cast<char*>(&mask.front()), sizeOfImage*sizeof(mask_t));
}

void pp332::aboutToQuit()
{
  if (_write)
    writeHotPixelMap();
}

void pp332::process(const CASSEvent &evt, HistogramBackend &res)
{
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>(_image->result(evt.id())));

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  QReadLocker lock(&image.lock);
  Histogram2DFloat::storage_t::const_iterator pixel(image.memory().begin());
  Histogram2DFloat::storage_t::const_iterator ImageEnd(image.memory().end());
  const size_t sizeOfImage(image.shape().first * image.shape().second);

  Histogram2DFloat::storage_t::iterator hotpix(result.memory().begin());
  Histogram2DFloat::storage_t::iterator count(result.memory().begin()+1*sizeOfImage);

  /** go though all pixels of image*/
  for (; pixel != ImageEnd; ++pixel, ++count, ++hotpix)
  {
    /** check if pix is not masked as hot */
    if (qFuzzyCompare(*hotpix,-1.f))
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
  : PostProcessor(name)
{
  loadSettings(0);
}

void pp333::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("PostProcessor");
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
  createHistList(_image->result().copy_sptr());
  Log::add(Log::INFO,"Postprocessor " + name() +
           ": generates the common mode background level of  '" +
           _image->name() + "' using calculation type '" + calctype +
           "'. Condition is '" + _condition->name() + "'");
}

float pp333::meanCalc(HistogramFloatBase::storage_t::const_iterator begin,
                      HistogramFloatBase::storage_t::const_iterator end)
{
  CummulativeStatisticsNoOutlier<float> stat(_snr);
  stat.addDistribution(begin,end);
  return stat.mean();
}

float pp333::medianCalc(HistogramFloatBase::storage_t::const_iterator begin,
                        HistogramFloatBase::storage_t::const_iterator end)
{
  MedianCalculator<float> stat;
  stat.addDistribution(begin,end);
  return stat.median();
}

void pp333::process(const CASSEvent &evt, HistogramBackend &res)
{
  const Histogram2DFloat &image
      (dynamic_cast<const Histogram2DFloat&>(_image->result(evt.id())));

  Histogram2DFloat &result(dynamic_cast<Histogram2DFloat&>(res));

  QReadLocker lock(&image.lock);
  /** retrieve iterators to the storages and the size of the image */
  Histogram2DFloat::storage_t::const_iterator imageIt(image.memory().begin());
  Histogram2DFloat::storage_t::iterator resultIt(result.memory().begin());
  const size_t sizeOfImage(image.shape().first * image.shape().second);
  const size_t parts(sizeOfImage / _width);

  /** go though all common mode parts of image*/
  for (size_t part(0); part < parts; ++part)
  {
    /** calculate the common mode for the part */
    Histogram2DFloat::storage_t::const_iterator startPart_Image(imageIt + part*_width);
    Histogram2DFloat::storage_t::const_iterator endPart_Image(startPart_Image + _width);
    const float commonmodeLevel(_calcCommonmode(startPart_Image,endPart_Image));

    /** fill the result part with the calculated common mode */
    Histogram2DFloat::storage_t::iterator startPart_Res(resultIt + part*_width);
    Histogram2DFloat::storage_t::iterator endPart_Res(startPart_Res + _width);
    fill(startPart_Res, endPart_Res, commonmodeLevel);
  }
}
