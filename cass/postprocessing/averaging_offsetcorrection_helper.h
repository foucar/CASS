//Copyright (C) 2010 Lutz Foucar

#ifndef _PHOTONHIT_HELPER_H_
#define _PHOTONHIT_HELPER_H_

#include <stdint.h>
#include <utility>
#include <algorithm>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include "cass_event.h"
#include "pixel_detector.h"
#include "postprocessor.h"
#include "histogram.h"

namespace cass
{

  /** Offset Correction Helper.
   *
   * This class will return the requested detector, which signals are going to
   * a Acqiris Instrument. It is implemented as a singleton such that every postprocessor
   * can call it without knowing about it.
   *
   * @author Lutz Foucar
   */
  class HelperAveragingOffsetCorrection
  {
  public:
    /** static function creating instance of this.
     * create an instance of an helper for the requested average.
     */
    static HelperAveragingOffsetCorrection * instance(const PostProcessors::key_t&);

    /** destroy the whole helper*/
    static void destroy();

    /** retrieve the offset corrected image.
     *
     * first validate it
     *
     * @return reference to the offset corrected frame
     * @param[in] id the id of the event. Needed for validation.
     * @param[in] f the frame that we want to correct
     * @param[in] a the averaged frame that we use for correcting
     */
    const PixelDetector::frame_t &correctFrame(uint64_t id, const PixelDetector::frame_t & f, HistogramFloatBase::storage_t a)
    {return validate(id,f,a);}

  protected:
    /** typdef defining the list of frames for more readable code*/
    typedef std::list<std::pair<uint64_t, PixelDetector::frame_t*> > frameList_t;

    /** validation of event.
     *
     * validate whether we have already seen this event. If not than offsetcorrect
     * the frame inside this event using the average and return the corrected frame.
     *
     * @return reference to the offset corrected frame
     * @param[in] id the id of the event. Needed for validation.
     * @param[in] f the frame that we want to correct
     * @param[in] a the averaged frame that we use for correcting
     */
    const PixelDetector::frame_t& validate(uint64_t id, const PixelDetector::frame_t &f, const HistogramFloatBase::storage_t &a);

    /** the offset correction.
     *
     * @param[out] result_frame the result of the offsetcorrection
     * @param[in] orig_frame the frame from the current event
     * @param[in] aver_frame the averaged frame that we substract
     */
    void correct_offset(PixelDetector::frame_t &result_frame,
                        const PixelDetector::frame_t & orig_frame,
                        const HistogramFloatBase::storage_t &aver_frame);

    /** list of frames for this helper */
    frameList_t _frameList;

    /** id of the postprocessor */
    PostProcessors::key_t _key;

    /** the device that contains the frame that this helper is for*/
    CASSEvent::Device _device;

    /** the detector id within the device that this helper instance is for */
    size_t _detector;

  private:
    /** private constructor.*/
    HelperAveragingOffsetCorrection();

    /** prevent copy-construction*/
    HelperAveragingOffsetCorrection(const HelperAveragingOffsetCorrection&);

    /** private desctuctor.
     * prevent destruction other than trhough destroy(),
     * delete the detector and the detectorlist for this instance
     */
    ~HelperAveragingOffsetCorrection();

    /** prevent assingment */
    HelperAveragingOffsetCorrection& operator=(const HelperAveragingOffsetCorrection&);

    /** the helperclass instances.
     *
     * the instances of this class put into map. One instance for each available detector
     */
    static std::map<PostProcessors::key_t,HelperAveragingOffsetCorrection*> _instances;

    /** Singleton Mutex to lock write operations*/
    static QMutex _mutex;

    /** Mutex for each helper*/
    QMutex _helperMutex;
  };
}

#endif
