// Copyright (C) 2016 Lutz Foucar

/**
 * @file lcls_key.hpp contains the lcls keys
 *
 * @author Lutz Foucar
 */

#include "pdsdata/xtc/TypeId.hh"
#include "pdsdata/xtc/DetInfo.hh"

namespace cass
{
namespace lclsid
{
/** Key for IdConversion Map
 *
 * This will take the different information from the xtc and transform it to
 * a unique key by having a comparison smaller than applied.
 *
 * Unfortunately the xtc differentiats between the detector and the detectorId
 * the detector is an enum described in Pds::DetInfo::Detector and the detector
 * id is different. The same is true for the device. Both, the detector ids and
 * the device ids are encoded in a single unsigned 32 bit integer. To identify
 * a single detector it is therefore necessary to give all the information.
 *
 * @author Lutz Foucar
 */
class Key
{
public:
  /** constructor
   *
   * will create the physical from the detector and device inputs. The
   * detectorType can just be copied.
   *
   * @param detectorType the type of data contained in the xtc
   * @param det enum of the detector itself
   * @param detId the id of the detector
   * @param dev enum of the device of the data
   * @param devId the id of the device
   */
  Key(const Pds::TypeId::Type& detectorType,
      const Pds::DetInfo::Detector& det, uint32_t detId,
      const Pds::DetInfo::Device& dev, uint32_t devId)
    :_detectorType(detectorType),
     _detector(((det&0xff)<<24) | ((detId&0xff)<<16) | ((dev&0xff)<<8) |(devId&0xff))
  {}

  /** constructor
   *
   * one can retrieve the physical id and the type id directly from the xtc.
   * Therefore a direct assigmnet is possible.
   *
   * @param detectorType the type of data contained in the xtc
   * @param physicalId the id that has the info about device and detector encoded
   */
  Key(const Pds::TypeId::Type& detectorType, uint32_t physicalId)
    :_detectorType(detectorType),_detector(physicalId)
  {}

  /** check whether this is less than other
   *
   * will compare for less first the _detectorType. If this is the same it will
   * compare for less the _detector value. This makes sure that the lcls Id is
   * unique.
   *
   * @param other the other key that one compares this key to
   */
  bool operator <(const Key& other) const
  {
    if (_detectorType != other._detectorType)
      return _detectorType < other._detectorType;
    return _detector < other._detector;
  }

private:
  /** type of the detector contained in the xtc */
  uint32_t _detectorType;

  /** the physical value of the detectors data. */
  uint32_t _detector;
};
}//end namespace Id
}//end cass
