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
   * @cassttng PostProcessor/\%name\%/{HistName}\n
   *           Postprocessor name containing the histogram in which hits should be detected.
   * @cassttng PostProcessor/\%name\%/{threshold}\n
   *           Threshold for outlier detection based discrimination (mahalanobis distance to dataset) default = 1.0 .
   * @cassttng PostProcessor/\%name\%/{xstart}\n
   *           ROI for calculations. First pixel = 0 (default).
   * @cassttng PostProcessor/\%name\%/{ystart}\n
   *           ROI for calculations. First pixel = 0 (default).
   * @cassttng PostProcessor/\%name\%/{xend}\n
   *           ROI for calculations. Last pixel = -1 (default).
   * @cassttng PostProcessor/\%name\%/{yend}\n
   *           ROI for calculations. Last pixel = -1 (default).
   * 
   * (good ROI for single particle in pnCCD images:    xstart=402;xend=485; ystart=402;yend=485;
   *
   * @author Stephan Kassemeyer
   */

  class pp589 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp589(PostProcessors&, const PostProcessors::key_t&);

    /** destructor */
    virtual ~pp589();

   /** check if image specified in settings contains a single particle hit*/
    virtual void process(const CASSEvent&);

    /** load the settings for this pp */
    virtual void loadSettings(size_t);

  protected:
    /** xstart - ROI for calculations*/
    int _xstart;
    /** ystart - ROI for calculations*/
    int _ystart;
    /** xend - ROI for calculations*/
    int _xend;
    /** yend - ROI for calculations*/
    int _yend;

    /** Threshold for outlier detection based discrimination (mahalanobis distance to dataset). */
    float _threshold;

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
