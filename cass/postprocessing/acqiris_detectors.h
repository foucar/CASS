//Copyright (C) 2010 Lutz Foucar

#ifndef _DELAYLINE_POSTPROCESSOR_H_
#define _DELAYLINE_POSTPROCESSOR_H_

#include "postprocessing/postprocessor.h"
#include "postprocessing/backend.h"
#include "cass_acqiris.h"

namespace cass
{
  //forward declarations//
  class Histogram1DFloat;
  class Histogram2DFloat;

  /** Number of Signals in MCP Waveform.
   * This postprocessor will output how many Signals have been found
   * in the acqiris channel for the mcp of the detector
   * (pp550, pp600, pp650, pp660, pp670, pp680).
   * @author Lutz Foucar
   */
  class pp550 : public PostprocessorBackend
  {
  public:
    /** Constructor for Number of Signals*/
    pp550(PostProcessors &, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp550();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadParameters(size_t);
  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;
    /** The Histogram storing the info*/
    Histogram1DFloat  *_nbrSignals;
  };



  /** Number of Signals in Anode Layers Waveform.
   * This postprocessor will output how many Signals have been found
   * in the acqiris channel for the layers (pp551-556 & pp601-604)
   * @author Lutz Foucar
   */
  class pp551 : public PostprocessorBackend
  {
  public:
    /** Constructor for Number of Signals*/
    pp551(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp551();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadParameters(size_t);
  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;
    /** The layer of the detector detector we are there for*/
    char _layer;
    /** The Signal of the layer detector we are there for*/
    char _signal;
    /** The Histogram storing the info*/
    Histogram1DFloat  *_nbrSignals;
  };



  /** Ratio of Peaks for two layers.
   * This postprocessor will output the Ratio of the Number
   * of Signals with Respect to the other layer
   * (pp577, pp560, pp563, pp605, pp608)
   * @author Lutz Foucar
   */
  class pp557 : public PostprocessorBackend
  {
  public:
    /** Constructor for Ratio of the Signals of two Layers*/
    pp557(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp557();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadParameters(size_t);
  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;
    /** The layer of the detector detector we are there for*/
    char _layer;
    /** The Histogram storing the info*/
    Histogram1DFloat  *_ratio;
  };



  /** Ratio of Signals for Signal vs MCP.
   * This postprocessor will output the Ratio of the Number
   * of Signals with Respect to the the number of Singals in
   * the MCP Channel
   * (pp558-559, pp561-562, pp564-565, pp606-507, pp609-610)
   * @author Lutz Foucar
   */
  class pp558 : public PostprocessorBackend
  {
  public:
    /** Constructor for Ratio of the Signals of Layers vs MCP*/
    pp558(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp558();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadParameters(size_t);
  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;
    /** The layer of the detector detector we are there for*/
    char _layer;
    /** The Signal of the layer detector we are there for*/
    char _wireend;
    /** The Histogram storing the info*/
    Histogram1DFloat  *_ratio;
  };




  /** Ratio of Signals for Rekonstukted Hits vs MCP Hits.
   * This postprocessor will output the Ratio of the Number
   * rekonstructed detector hits with respect to the the number
   * of Singals in the MCP Channel
   * (pp566, pp611)
   * @author Lutz Foucar
   */
  class pp566 : public PostprocessorBackend
  {
  public:
    /** Constructor for Ratio of the reconstructed Hits vs MCP Hits*/
    pp566(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp566();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadParameters(size_t);
  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;
    /** The Histogram storing the info*/
    Histogram1DFloat  *_ratio;
  };





  /** All Mcp Hits.
   * This postprocessor will output the times of all found Hits on the Mcp.
   * This is more or less a Time of Flight Spektrum
   * (pp567, pp612, pp651, pp661, pp671, pp681)
   * @author Lutz Foucar
   */
  class pp567 : public PostprocessorBackend
  {
  public:
    /** Constructor*/
    pp567(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp567();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadParameters(size_t);
  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;
    /** The Histogram storing the info*/
    Histogram1DFloat  *_tof;
  };






  /** Timesum of Delayline.
   * This postprocessor will output Timesum of a Delayline Anode for
   * the first hit in a selectable good range
   * (pp568-570, pp613-614)
   * @author Lutz Foucar
   */
  class pp568 : public PostprocessorBackend
  {
  public:
    /** Constructor*/
    pp568(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp568();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadParameters(size_t);
  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;
    /** The layer of the detector detector we are there for*/
    char _layer;
    /** The Histogram storing the info*/
    Histogram1DFloat  *_timesum;
  };





  /** Timesum of Delayline Anode vs Position of Anode.
   * This postprocessor will output Timesum of a Delayline Anode
   * versus the position of the delayline. This is used to know the value
   * for extracting the detectorhits.
   * (pp571-573, pp615-616)
   * @author Lutz Foucar
   */
  class pp571 : public PostprocessorBackend
  {
  public:
    /** Constructor */
    pp571(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp571();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadParameters(size_t);
  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;
    /** The layer of the detector detector we are there for*/
    char _layer;
    /** The Histogram storing the info*/
    Histogram2DFloat  *_timesumvsPos;
  };






  /** Detector Picture of First Hit.
   * This postprocessor will output the Detector picture of the first Hit
   * in the selectable good range. The added Hit fullfilles the timesum.
   * (pp574-577, pp617)
   * @author Lutz Foucar
   */
  class pp574 : public PostprocessorBackend
  {
  public:
    /** Constructor */
    pp574(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp574();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadParameters(size_t);
  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;
    /** The first layer of the detector for the position */
    char _first;
    /** The second layer of the detector for the position */
    char _second;
    /** The Histogram storing the info*/
    Histogram2DFloat  *_pos;
  };






  /** Detector Hits Values.
   *  This postprocessor will output the Detector Hit values reqeuested.
   * depending on the postprocessor id, it will histogram 2 of the 3 values
   * of an detectorhit.
   * (pp578-580, pp618-620)
   * @author Lutz Foucar
   */
  class pp578 : public PostprocessorBackend
  {
  public:
    /** Constructor */
    pp578(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp578();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadParameters(size_t);
  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;
    /** The first value of the detector hit */
    char _first;
    /** The second value of the detector */
    char _second;
    /** The Histogram storing the info*/
    Histogram2DFloat  *_hist;
  };


  /** FWHM vs. Height of MCP Signals.
   * This postprocessor will make a histogram of the fwhm and height of
   * all MCP Signals in a detector.
   * (pp581, pp621, pp652, pp662, pp672, pp682)
   * @author Lutz Foucar
   */
  class pp581 : public PostprocessorBackend
  {
  public:
    /** Constructor*/
    pp581(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp581();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadParameters(size_t);
  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;
    /** The Histogram storing the info*/
    Histogram2DFloat  *_sigprop;
  };



  /** FWHM vs. Height of Wireend Signals.
   * This postprocessor will make a histogram of the fwhm and height of
   * all Wireend Signals in a delaylinedetector.
   * (pp582-587, pp622-625)
   * @author Lutz Foucar
   */
  class pp582 : public PostprocessorBackend
  {
  public:
    /** Constructor for Number of Signals*/
    pp582(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp582();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadParameters(size_t);
  protected:
    /** The detector we are there for*/
    ACQIRIS::Detectors _detector;
    /** The layer of the detector detector we are there for*/
    char _layer;
    /** The Signal of the layer detector we are there for*/
    char _signal;
    /** The Histogram storing the info*/
    Histogram2DFloat  *_sigprop;
  };




  /** Pipico spectra.
   * This postprocessor will create Photo Ion Photo Ion Coincidence Spectra
   * (pp700)
   * @author Lutz Foucar
   */
  class pp700 : public PostprocessorBackend
  {
  public:
    /** Constructor for Number of Signals*/
    pp700(PostProcessors&, PostProcessors::id_t);
    /** Free _image space */
    virtual ~pp700();
    /** Retrieve the number of Signals and histogram it */
    virtual void operator()(const CASSEvent&);
    /** load the histogram settings from file*/
    virtual void loadParameters(size_t);
  protected:
    /** The first detector of the cooincdence*/
    ACQIRIS::Detectors _detector01;
    /** The second detector of the cooincdence*/
    ACQIRIS::Detectors _detector02;
    /** The Histogram storing the info*/
    Histogram2DFloat  *_pipico;
  };



}//end cass

#endif
