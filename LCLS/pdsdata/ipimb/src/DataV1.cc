#include "pdsdata/ipimb/DataV1.hh"

using namespace Pds;
using namespace Ipimb;

static const unsigned Version=1;

Pds::TypeId DataV1::typeId() {
  return Pds::TypeId(Pds::TypeId::Id_IpimbData,Version);
}

uint64_t DataV1::triggerCounter() const {
  return _triggerCounter;
}

uint16_t DataV1::config0() const {
  return _config0;
}

uint16_t DataV1::config1() const {
  return _config1;
}

uint16_t DataV1::config2() const {
  return _config2;
}

uint16_t DataV1::channel0() const {
  return _channel0;
}

uint16_t DataV1::channel1() const {
  return _channel1;
}

uint16_t DataV1::channel2() const {
  return _channel2;
}

uint16_t DataV1::channel3() const {
  return _channel3;
}

uint16_t DataV1::checksum() const {
  return _checksum;
}
