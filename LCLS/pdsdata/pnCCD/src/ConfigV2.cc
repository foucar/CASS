#include "pdsdata/pnCCD/ConfigV2.hh"

#include <stdio.h>
//#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

using namespace Pds;
using namespace PNCCD;
using std::string;

ConfigV2::ConfigV2() {}

ConfigV2::ConfigV2(uint32_t numLinks, uint32_t payloadSizePerLink, string sConfigFile) :
  _numLinks(numLinks), _payloadSizePerLink(payloadSizePerLink) {
  FILE* fp = ::fopen(sConfigFile.c_str(), "r");
  size_t ret;
  if (fp) {
    printf("Reading pnCCD config file\n");
    ret = fread(&_numChannels, sizeof(ConfigV2)-(2*sizeof(uint32_t)), 1, fp);
    if (ret != 1) printf("Error reading pnCCD config file\n");
  } else {
    printf("Could not open pnCCD file\n");
  }
}

uint32_t ConfigV2::numLinks()             const {return _numLinks;}
uint32_t ConfigV2::payloadSizePerLink()   const {return _payloadSizePerLink;} // bytes
uint32_t ConfigV2::numChannels()          const {return _numChannels;}
uint32_t ConfigV2::numRows()              const {return _numRows;}
uint32_t ConfigV2::numSubmoduleChannels() const {return _numSubmoduleChannels;}
uint32_t ConfigV2::numSubmoduleRows()     const {return _numSubmoduleRows;}
uint32_t ConfigV2::numSubmodules()        const {return _numSubmodules;}
uint32_t ConfigV2::camexMagic()           const {return _camexMagic;}
const char*    ConfigV2::info()                 const {return _info;}
const char*    ConfigV2::timingFName()          const {return _timingFName;}
unsigned ConfigV2::size()                 const {return sizeof(*this); }
