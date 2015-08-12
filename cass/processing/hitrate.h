// Copyright (C) 2010 Stephan Kassemeyer

/**
 * @file hitrate.h processors for single particle hitfinding
 *
 * @author Stephan Kassemeyer
 */

#ifndef _HITRATE_H_
#define _HITRATE_H_

#include <math.h>
#include <vigra/linear_algebra.hxx>
#include <vigra/inspectimage.hxx>
#include <vigra/multi_pointoperators.hxx>
#include <vigra/separableconvolution.hxx>

#include "processor.h"

namespace cass
{
/** Single particle hit.
 *
 * @PPList "300": detect Single Particle hits.
 *
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{ImageName}\n
 *           processor name containing the histogram in which hits should be detected.
 * @cassttng Processor/\%name\%/{xstart}\n
 *           ROI for calculations. First pixel = 0 (default).
 * @cassttng Processor/\%name\%/{ystart}\n
 *           ROI for calculations. First pixel = 0 (default).
 * @cassttng Processor/\%name\%/{xend}\n
 *           ROI for calculations. Last pixel = -1 (default).
 * @cassttng Processor/\%name\%/{yend}\n
 *           ROI for calculations. Last pixel = -1 (default).
 * @cassttng Processor/\%name\%/{TrainingSetSize}\n
 *           How many images should be included in training phase. default = 200.
 * @cassttng Processor/\%name\%/{saveTraining}\n
 *           optional. If true (default), save Training matrices to
 *           files with automaic names in currend directory.
 * @cassttng Processor/\%name\%/{readTrainingFile}\n
 *           optional filename to read training matrices from.
 *           If setting doesn't exist, training is not read but
 *           calculated on "loadSettings".
 *
 * (good ROI for single particle in pnCCD images:    xstart=402;xend=485; ystart=402;yend=485;
 *
 * @author Stephan Kassemeyer
 */

class pp300 : public Processor
{
public:
  /** constructor */
  pp300(const name_t&);

  /** virtual destructor */
  virtual ~pp300();

  /** check if image specified in settings contains a single particle hit*/
  virtual void process(const CASSEvent&, result_t&);

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

  /** xstart - ROI for calculations*/
  int _xstart;

  /** ystart - ROI for calculations*/
  int _ystart;

  /** xend - ROI for calculations*/
  int _xend;

  /** yend - ROI for calculations*/
  int _yend;

  /** the histogram to work on */
  shared_pointer _pHist;

  // outlier detection processor:
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

  /** mutex to lock the processing part since only one can be processed at a time */
  QMutex _mutex;
};





/** convolute histogram with kernel
 *
 * @PPList "313": convolute histogram with kernel
 *
 * @see Processor for a list of all commonly available cass.ini
 *      settings.
 *
 * @cassttng Processor/\%name\%/{InputName} \n
 *           histogram name, that should be convoluted
 * @cassttng Processor/\%name\%/{Kernel} \n
 *           Comma separated list of values for the Kernel that should be
 *           convoluted to the histogram
 *
 * @author Lutz Foucar
 */
class pp313 : public Processor
{
public:
  /** constructor */
  pp313(const name_t &name);

  /** process event */
  virtual void process(const CASSEvent&, result_t&);

  /** load the settings of the pp */
  virtual void loadSettings(size_t);

protected:
  /** pp containing input histogram */
  shared_pointer _pHist;

  /** array containing the kernel */
  std::vector<float> _kernel;

  /** iterator to the center of the kernel */
  std::vector<float>::iterator _kernelCenter;

  /** element to the left of the kernel center */
  int _kleft;

  /** elements to the right of the kernel center */
  int _kright;
};




}
#endif
