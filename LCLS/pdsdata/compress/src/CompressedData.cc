#include "pdsdata/compress/CompressedData.hh"
#include "pdsdata/compress/CompressedPayload.hh"

using namespace Pds;

CompressedData::CompressedData(unsigned size) :
  _headerSize(size)
{
}

unsigned                 CompressedData::headerSize() const 
{
  return _headerSize; 
}

const void*              CompressedData::header    () const 
{
  return (void*)(this+1); 
}

const CompressedPayload& CompressedData::pd        () const 
{
  return *reinterpret_cast<const CompressedPayload*>((char*)(this+1)+_headerSize);
}
