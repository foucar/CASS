/*
 * FrameV1.hh
 *
 *  Created on: Nov 6, 2009
 *      Author: jackp
 */

#ifndef FRAMEV1_HH_
#define FRAMEV1_HH_

#include <stdint.h>

namespace Pds {
  namespace PNCCD {

    class ConfigV1;
    class FrameV1 {
      public:
        enum {Version=1};
        FrameV1(const uint32_t specialWord,
                const uint32_t frameNumber,
                const uint32_t timeStampHi,
                const uint32_t timeStampLo):
                _specialWord(specialWord),
                _frameNumber(frameNumber),
                _timeStampHi(timeStampHi),
                _timeStampLo(timeStampLo)
                {}

        uint32_t specialWord() const;
        uint32_t frameNumber() const;
        uint32_t timeStampHi() const;
        uint32_t timeStampLo() const;

        const FrameV1* next(const ConfigV1& cfg) const;
        const uint16_t* data()             const;
        unsigned sizeofData(const ConfigV1& cfg) const;

      private:
        const uint32_t    _specialWord;
        const uint32_t    _frameNumber;
        const uint32_t    _timeStampHi;
        const uint32_t    _timeStampLo;
    };
  }
}

#endif /* FRAMEV1_HH_ */
