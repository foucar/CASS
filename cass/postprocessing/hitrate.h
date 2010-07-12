// Copyright (C) 2010 Stephan Kassemeyer

#ifndef _HITRATE_H_
#define _HITRATE_H_

#include <math.h>
#include <vigra/linear_algebra.hxx>
#include <vigra/inspectimage.hxx>
#include <vigra/multi_pointoperators.hxx> 

#include "backend.h"
#include "cass_event.h"
#include "cass_acqiris.h"

namespace cass
{

  // forward declaration
  class Histogram0DFloat;
  class Histogram1DFloat;
  class Histogram2DFloat;

  /** Single particle hit.
   *
   * detect Single Particle hits.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{HistName}\n
   *           Postprocessor name containing the histogram in which hits should be detected.
   * @cassttng PostProcessor/\%name\%/{xstart}\n
   *           ROI for calculations. First pixel = 0 (default).
   * @cassttng PostProcessor/\%name\%/{ystart}\n
   *           ROI for calculations. First pixel = 0 (default).
   * @cassttng PostProcessor/\%name\%/{xend}\n
   *           ROI for calculations. Last pixel = -1 (default).
   * @cassttng PostProcessor/\%name\%/{yend}\n
   *           ROI for calculations. Last pixel = -1 (default).
   * @cassttng PostProcessor/\%name\%/{TrainingSetSize}\n
   *           How many images should be included in training phase. default = 200.
   * @cassttng PostProcessor/\%name\%/{saveTraining}\n
   *           optional. If true (default), save Training matrices to
   *           files with automaic names in currend directory.
   * @cassttng PostProcessor/\%name\%/{readTrainingFile}\n
   *           optional filename to read training matrices from.
   *           If setting doesn't exist, training is not read but
   *           calculated on "loadSettings".
   * 
   * (good ROI for single particle in pnCCD images:    xstart=402;xend=485; ystart=402;yend=485;
   *
   * @author Stephan Kassemeyer
   */

  class pp300 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp300(PostProcessors&, const PostProcessors::key_t&);

    /** virtual destructor */
    virtual ~pp300();

   /** check if image specified in settings contains a single particle hit*/
    virtual void process(const CASSEvent&);

    /** load the settings for this pp */
    virtual void loadSettings(size_t);

  protected:

    /** */
    void trainingFinished();

    /** */
    void startNewTraining();
    
    /** */
    void readTrainingMatrices();

    /** */
    void saveTrainingMatrices();

    /** */
    void printTrainingMatrices();

    /** */
    virtual void clearHistogramEvent();

    /** */
    virtual void processCommand(std::string command);

    virtual void saveSettings(size_t);


    /** xstart - ROI for calculations*/
    int _xstart;

    /** ystart - ROI for calculations*/
    int _ystart;

    /** xend - ROI for calculations*/
    int _xend;

    /** yend - ROI for calculations*/
    int _yend;

    /** the histogram to work on */
    PostprocessorBackend* _pHist;

    /** storage for integralimage (2d integral of entire image) */
    Histogram2DFloat* _integralimg;

    /** storage for row sum (1d integral of image) */
    Histogram2DFloat* _rowsum;

    // outlier detection postprocessor:
    typedef vigra::Matrix<double> matrixType;
    
    int _nTrainingSetSize;
    int _nFeatures;

    /** user setting: do (default) or don't save training matrices to file */
    int _saveTraining;

    /** if filename is given in settings, load instead of calculate */
    int _readTraining;

    /** optional user-setting: filename to read training from*/
    std::string _trainingFile;

    matrixType _variationFeatures;
    matrixType _mean; // mean (one scalar per column or feature)
//    vigra::MultiArray<1,double> _mean; // mean (one scalar per column or feature)
    matrixType _cov;
    matrixType _covI;
    int _trainingSetsInserted; // counts how many training data items are already included in training set.
    int _reTrain; // manages retraining of mean and covariance matrix used to determine outliers.
  };


}
#endif
