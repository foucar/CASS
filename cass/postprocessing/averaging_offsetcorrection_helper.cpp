//Copyright (C) 2010 Lutz Foucar

#include <stdint.h>

#include "averaging_offsetcorrection_helper.h"



//initialize static members//
std::map<cass::PostProcessors::key_t,cass::HelperAveragingOffsetCorrection*>
    cass::HelperAveragingOffsetCorrection::_instances;
QMutex cass::HelperAveragingOffsetCorrection::_mutex;

cass::HelperAveragingOffsetCorrection* cass::HelperAveragingOffsetCorrection::instance(const cass::PostProcessors::key_t &key)
{
  //lock this//
  QMutexLocker lock(&_mutex);
  //check if an instance of the helper class already exists//
  //return it, otherwise create one and return it//
  if (0 == _instances[key])
  {
    VERBOSEOUT(std::cout << "creating an instance of the Averaging correction Helper for postprocessor "
               <<key
               <<std::endl);
    _instances[key] = new HelperAveragingOffsetCorrection();
  }
  return _instances[key];
}

void cass::HelperAveragingOffsetCorrection::destroy()
{
  //lock this//
  QMutexLocker lock(&_mutex);
  //delete all instances of the helper class//
  std::map<PostProcessors::key_t,HelperAveragingOffsetCorrection*>::iterator it(_instances.begin());
  for (;it != _instances.end(); ++it)
    delete it->second;
}






//members

namespace cass
{
  /** predicate class for find_if.
   * this helps finding the right key in the list of pairs
   * @see HelperAveragingOffsetCorrection::_frameList
   * @author Lutz Foucar
   */
  class IsKey
  {
  public:
    /** initialize the key in the constructor */
    IsKey(const uint64_t key):_key(key){}
    /** compares the first element of the pair to the key */
    bool operator()(const std::pair<uint64_t,PixelDetector::frame_t*>& p)const
    { return (p.first == _key); }
  private:
    /** the key that we will compare to in the operator */
    const uint64_t _key;
  };
}


cass::HelperAveragingOffsetCorrection::HelperAveragingOffsetCorrection()
{
  //create the detector list with twice the amount of elements than workers
  for (size_t i=0; i<NbrOfWorkers*2;++i)
    _frameList.push_front(std::make_pair(0,new PixelDetector::frame_t));
}

cass::HelperAveragingOffsetCorrection::~HelperAveragingOffsetCorrection()
{
  //delete the detectorList
  frameList_t::iterator it(_frameList.begin());
  for (;it != _frameList.end();++it)
    delete it->second;
}

const cass::PixelDetector::frame_t&
cass::HelperAveragingOffsetCorrection::validate(uint64_t eventId,
                                                const cass::PixelDetector::frame_t &origFrame,
                                                const cass::HistogramFloatBase::storage_t &averagedFrame)
{
  //lock this so that only one helper will retrieve the detector at a time//
  QMutexLocker lock(&_helperMutex);
  //find the pair containing the detector//
  frameList_t::iterator it
      (std::find_if(_frameList.begin(), _frameList.end(), IsKey(eventId)));
  //check wether id is not already on the list//
  if(_frameList.end() == it)
  {
    //take the last element and get the the detector from it//
    PixelDetector::frame_t *frame (_frameList.back().second);
    //make the frame the same size as the orignalFrame//
    frame->resize(origFrame.size());
    //correct offset of the incomming frame using the averaged frame
    correct_offset(*frame, origFrame, averagedFrame);
    //create a new key from the id with the reloaded detector
    frameList_t::value_type newPair (std::make_pair(eventId,frame));
    //put it to the beginning of the list//
    _frameList.push_front(newPair);
    //erase the outdated element at the back//
    _frameList.pop_back();
    //make the iterator pointing to the just added element of the list//
    it = _frameList.begin();
  }
  return *(it->second);
}

void cass::HelperAveragingOffsetCorrection::correct_offset(PixelDetector::frame_t &result_frame,
                                                           const PixelDetector::frame_t & orig_frame,
                                                           const HistogramFloatBase::storage_t &aver_frame)
{
  float sumFrame(0);
  float sumAverage(0);
  PixelDetector::frame_t::iterator rIt(result_frame.begin());
  PixelDetector::frame_t::const_iterator oIt(orig_frame.begin());
  HistogramFloatBase::storage_t::const_iterator aIt(aver_frame.begin());

  while (oIt != orig_frame.end()) sumFrame   += *oIt++;
  while (aIt != aver_frame.end()) sumAverage += *aIt++;
  const float alpha = sumFrame / sumAverage;

  oIt = orig_frame.begin();
  aIt = aver_frame.begin();

  while(oIt != orig_frame.end())
    *rIt++ = *oIt++ - alpha * *aIt++ ;
}

