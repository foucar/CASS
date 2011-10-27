// Copyright (C) 2010 Lutz Foucar

/**
 * @file convenience_functions.h file contains declaration of classes and
 *                               functions that help other postprocessors to do
 *                               their job.
 *
 * @author Lutz Foucar
 */

#ifndef __CONVENIENCE_FUNCTIONS_H__
#define __CONVENIENCE_FUNCTIONS_H__

#include "postprocessor.h"
#include "cass.h"

namespace cass
{
  class CASSSettings;

  namespace ACQIRIS
  {
    class DetectorBackend;

    /** predicate class for find_if.
     *
     * this helps finding the right key in the list of pairs
     * @see HelperAcqirisDetectors::_detectorList
     *
     * @author Lutz Foucar
     */
//    class IsKey
//    {
//    public:
//      /** initialize the key in the constructor*/
//      IsKey(const uint64_t key):_key(key){}

//      /** compares the first element of the pair to the key*/
//      bool operator()(const std::pair<uint64_t,DetectorBackend*>& p)const
//      { return (p.first == _key); }

//    private:
//      /** the key that we will compare to in the operator*/
//      const uint64_t _key;
//    };

    /** load detector from file
     *
     * after loading check whether it is a delayline detector, if not throw
     * invalid_argument exception.
     *
     * @return key containing detector name
     * @param s CASSSettings object to read the info from
     * @param ppNbr the Postprocessor number of the postprocessor calling this
     *              function
     * @param key the key of the postprocessor calling this function
     *
     * @author Lutz Foucar
     */
    std::string loadDelayDet(CASSSettings &s,
                             int ppNbr,
                             const PostProcessors::key_t& key);

    /** load particle for a specific detector
     *
     * after loading check whether it is a delayline detector, if not throw
     * invalid_argument exception.
     *
     * @return key containing detector name
     * @param s CASSSettings object to read the info from
     * @param detector the name of the detector that contains the layer
     * @param ppNbr the Postprocessor number of the postprocessor calling this
     *              function
     * @param key the key of the postprocessor calling this function
     *
     * @author Lutz Foucar
     */
    std::string loadParticle(CASSSettings &s,
                             const std::string &detector,
                             int ppNbr,
                             const PostProcessors::key_t& key);
  }

  /** predicate class for find_if.
   *
   * this helps finding the right key in the list of pairs eventid - Histogram
   *
   * @author Lutz Foucar
   */
//  class IsKey
//  {
//  public:
//    /** initialize the key in the constructor*/
//    IsKey(const uint64_t key):_key(key){}

//    /** compares the first element of the pair to the key*/
//    bool operator()(const std::pair<uint64_t,HistogramBackend*>& p)const
//    { return (_key == p.first); }

//  private:
//    /** the key that we will compare to in the operator*/
//    const uint64_t _key;
//  };

  /** Binary function for thresholding
   *
   * Returns the value if it is below threshold.  Otherwise, returns 0.
   *
   * @author Thomas White
   */
  class threshold : public std::binary_function<float, float, float>
  {
  public:
    /** operator */
    float operator() (float value, float thresh)const
    {
      return (value > thresh) ? value : 0.0;
    }
  };

  /** binary function for weighted subtraction.
   *
   * @author Lutz Foucar
   */
  class weighted_minus : std::binary_function<float, float, float>
  {
  public:
    /** constructor.
     *
     * @param first_weight the weight value of the first histogram
     * @param second_weight the weight value of the second histogram
     */
    weighted_minus(float first_weight, float second_weight)
      :_first_weight(first_weight),_second_weight(second_weight)
    {}
    /** operator */
    float operator() (const float first, const float second)
    { return first * _first_weight - second * _second_weight;}
  protected:
    float _first_weight, _second_weight;
  };



  /** Convert Waveform Point
   *
   * unary function for converting the waveform points that are in ADC Values to
   * Volts.
   *
   * @author Lutz Foucar
   */
//  class Adc2Volts : public std::unary_function<ACQIRIS::waveform_t::value_type, float>
//  {
//  public:
//    /** constructor.
//     *
//     * @param gain the gain of the channel that contains the waveform
//     * @param offset the offset in Volts of the channel
//     */
//    Adc2Volts(double gain, double offset):
//        _gain(gain),_offset(offset)
//    {}

//    /** operator. converts adc values to volts*/
//    float operator ()(ACQIRIS::waveform_t::value_type adc)
//    { return adc * _gain - _offset;}

//  protected:
//    double _gain;
//    double _offset;
//  };

  /** binary function for averaging.
   *
   * this operator is capable of performing a cumulative moving average and
   * a Exponential moving average.
   * @see http://en.wikipedia.org/wiki/Moving_average
   *
   * @author Lutz Foucar
   */
  class Average : std::binary_function<float,float,float>
  {
  public:
    /** constructor.
     *
     * initializes the \f$\alpha\f$ value
     *
     * @param alpha The \f$\alpha\f$ value
     */
    explicit Average(float alpha)
      :_alpha(alpha)
    {}

    /** operator.
     *
     * the operator calculates the average using the function
     * \f$Y_N = Y_{N-1} + \alpha(y-Y_{N-1})\f$
     * where when \f$\alpha\f$ is equal to N it is a cumulative moving average,
     * otherwise it will be a exponential moving average.
     */
    float operator()(float currentValue, float Average_Nm1)
    {
      return Average_Nm1 + _alpha*(currentValue - Average_Nm1);
    }

  protected:
    /** \f$\alpha\f$ for the average calculation */
    float _alpha;
  };


  /** binary function for averaging.
   *
   * this operator performs a moving sum
   *
   * @author Nicola Coppola
   */
  class TimeAverage : std::binary_function<float,float,float>
  {
  public:
    /** constructor.
     *
     * initializes the nEvents value
     *
     * @param nEvents The number of Events used up to now
     */
    explicit TimeAverage(float nEvents)
      :_nEvents(nEvents)
    {}

    /** the operator calculates the average over the last _nEvents */
    float operator()(float currentValue, float Average_Nm1)
    {
      if(_nEvents!=0)
        return ( Average_Nm1 * (_nEvents-1) + currentValue ) /_nEvents;
      else
        return currentValue;
    }

  protected:
    /** nEvents for the average calculation */
    float _nEvents;
  };


  /** function to set the 1d histogram properties from the ini file.
   *
   * @param[out] hist pointer to the 1D Histogram whos properties should be updated
   *            (will be deleted and created with new settings)
   * @param[in] key the key of the postprocessor too look up in cass.ini
   *
   * @author Lutz Foucar
   */
  void set1DHist(cass::HistogramBackend*& hist, PostProcessors::key_t key);


  /** function to set the 2d histogram properties from the ini file.
   *
   * @param[out] hist pointer to the 2D Histogram whos properties should be updated
   *            (will be deleted and created with new settings)
   * @param[in] key the key of the postprocessor too look up in cass.ini
   *
   * @author Lutz Foucar
   */
  void set2DHist(cass::HistogramBackend*& hist, PostProcessors::key_t key);


  /** Qt names of known/supported Qt image formats
   *
   * @param fmt the Image Format
   * @return Qt name of format
   */
  inline const std::string imageformatName(ImageFormat fmt)
  {
      std::string fmtname;
      switch(fmt) {
      case PNG:  fmtname = std::string("PNG"); break;
      case TIFF: fmtname = std::string("TIFF"); break;
      case JPEG: fmtname = std::string("JPEG"); break;
      case GIF:  fmtname = std::string("GIF"); break;
      case BMP:  fmtname = std::string("BMP"); break;
      }
      return fmtname;
  };


  /** File extensions of known/supported Qt image formats
   *
   * @param fmt the Image Format
   * @return File extension of format
   */
  inline const std::string imageExtension(ImageFormat fmt)
  {
      std::string fmtname;
      switch(fmt) {
      case PNG:  fmtname = std::string("png"); break;
      case TIFF: fmtname = std::string("tiff"); break;
      case JPEG: fmtname = std::string("jpg"); break;
      case GIF:  fmtname = std::string("gif"); break;
      case BMP:  fmtname = std::string("bmp"); break;
      }
      return fmtname;
  };


  /** MIMI names of known/supported Qt image formats.
   *
   * @param fmt the Image Format
   * @return MIME/type of format
   */
  inline const std::string imageformatMIMEtype(ImageFormat fmt)
  {
      std::string fmtname;
      switch(fmt) {
      case PNG:  fmtname = std::string("image/png"); break;
      case TIFF: fmtname = std::string("image/tiff"); break;
      case JPEG: fmtname = std::string("image/jpeg"); break;
      case GIF:  fmtname = std::string("image/gif"); break;
      case BMP:  fmtname = std::string("image/bmp"); break;
      }
      return fmtname;
  };


  /** Names of known/supported Qt image formats
   *
   * @param name format name
   * @return ImageFormat
   */
  inline ImageFormat imageformat(const std::string& name)
  {
      ImageFormat fmt(PNG);
      if(std::string("PNG") == name) fmt = PNG;
      else if(std::string("TIFF") == name) fmt = TIFF;
      else if(std::string("JPEG") == name) fmt = TIFF;
      else if(std::string("GIF") == name) fmt = TIFF;
      else if(std::string("BMP") == name) fmt = TIFF;
      return fmt;
  };

  /** Helper function to delete duplicates from a std::list
   *
   * This keeps the earliest entry in the list and removes all later ones
   *
   * @param l List to remove duplicates from.
   */
  template<typename T>
  inline void unique(std::list<T>& l)
  {
    // shorten list by removing consecutive duplicates
    l.unique();
    // now remove remaining (harder) duplicates
    for(typename std::list<T>::iterator i1 = l.begin();
    i1 != l.end();
    ++i1) {
      typename std::list<T>::iterator i2(i1);
      ++i2;
      while(l.end() != (i2 = find(i2, l.end(), *i1)))
        l.erase(i2);
    }
  }
}

#endif
