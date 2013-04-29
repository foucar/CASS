/*
 * ProtectionSystem.hh
 *
 *  Created on: Jan 31, 2013
 *      Author: jackp
 */

#ifndef PROTECTIONSYSTEM_HH_
#define PROTECTIONSYSTEM_HH_

#pragma pack(4)

namespace Pds
{
  namespace CsPad2x2
  {
    class ProtectionSystemThreshold
    {
      public:
        ProtectionSystemThreshold() {};
        ~ProtectionSystemThreshold() {};

      public:
        uint32_t adcThreshold;
        uint32_t pixelCountThreshold;
    };
  } // namespace CsPad2x2

} // namespace Pds

#pragma pack()

#endif /* PROTECTIONSYSTEM_HH_ */
