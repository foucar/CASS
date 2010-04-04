//Copyright (C) 2010 Lutz Foucar

#ifndef _DELAYLINE_POSTPROCESSOR_H_
#define _DELAYLINE_POSTPROCESSOR_H_

#include "postprocessing/postprocessor.h"
#include "postprocessing/backend.h"

namespace cass
{
  //forward declarations//
  class Histogram1DFloat;
  class Histogram2DFloat;

  /*! @brief Number of Signals in MCP Waveform

  This postprocessor will output how many Signals have been found
  in the acqiris channel for the layers (pp 550 & pp600).

  @author Lutz Foucar
  */
  class pp550 : public PostprocessorBackend
  {
  public:
    /*! Constructor for Number of Signals*/
    pp550(PostProcessors::histograms_t&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp550();
    /*! Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /*! load the histogram settings from file*/
    virtual void loadParameters(size_t);

  protected:
    /*! The detector we are there for*/
    size_t _detector;

    /*! The Histogram storing the info*/
    Histogram1DFloat  *_nbrSignals;
  };



  /*! @brief Number of Signals in Anode Layers Waveform

  This postprocessor will output how many Signals have been found
  in the acqiris channel for the layers (pp551-556 & pp601-604)

  @author Lutz Foucar
  */
  class pp551 : public PostprocessorBackend
  {
  public:
    /*! Constructor for Number of Signals*/
    pp551(PostProcessors::histograms_t&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp551();
    /*! Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /*! load the histogram settings from file*/
    virtual void loadParameters(size_t);

  protected:
    /*! The detector we are there for*/
    size_t _detector;
    /*! The layer of the detector detector we are there for*/
    char _layer;
    /*! The Signal of the layer detector we are there for*/
    char _signal;

    /*! The Histogram storing the info*/
    Histogram1DFloat  *_nbrSignals;
  };



  /*! @brief Ratio of Peaks for two layers

  This postprocessor will output the Ratio of the Number
  of Signals with Respect to the other layer (pp577, 560 563, 605, 608)

  @author Lutz Foucar
  */
  class pp557 : public PostprocessorBackend
  {
  public:
    /*! Constructor for Ratio of the Signals of two Layers*/
    pp557(PostProcessors::histograms_t&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp557();
    /*! Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /*! load the histogram settings from file*/
    virtual void loadParameters(size_t);

  protected:
    /*! The detector we are there for*/
    size_t _detector;
    /*! The layer of the detector detector we are there for*/
    char _layer;

    /*! The Histogram storing the info*/
    Histogram1DFloat  *_ratio;
  };



  /*! @brief Ratio of Signals for Signal vs MCP

  This postprocessor will output the Ratio of the Number
  of Signals with Respect to the the number of Singals in
  the MCP Channel (pp558-559, pp561-562, pp564-565, pp606-507, pp609-610)

  @author Lutz Foucar
  */
  class pp558 : public PostprocessorBackend
  {
  public:
    /*! Constructor for Ratio of the Signals of Layers vs MCP*/
    pp558(PostProcessors::histograms_t&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp558();
    /*! Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /*! load the histogram settings from file*/
    virtual void loadParameters(size_t);

  protected:
    /*! The detector we are there for*/
    size_t _detector;
    /*! The layer of the detector detector we are there for*/
    char _layer;
    /*! The Signal of the layer detector we are there for*/
    char _wireend;
    /*! The Histogram storing the info*/
    Histogram1DFloat  *_ratio;
  };




  /*! @brief Ratio of Signals for Rekonstukted Hits vs MCP Hits

  This postprocessor will output the Ratio of the Number
  rekonstructed detector hits with respect to the the number
  of Singals in the MCP Channel (pp566, pp611)

  @author Lutz Foucar
  */
  class pp566 : public PostprocessorBackend
  {
  public:
    /*! Constructor for Ratio of the reconstructed Hits vs MCP Hits*/
    pp566(PostProcessors::histograms_t&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp566();
    /*! Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /*! load the histogram settings from file*/
    virtual void loadParameters(size_t);

  protected:
    /*! The detector we are there for*/
    size_t _detector;

    /*! The Histogram storing the info*/
    Histogram1DFloat  *_ratio;
  };

}//end cass

#endif
