#include "pdsdata/compress/CompressedPayload.hh"

#include "pdsdata/compress/Hist16Engine.hh"
#include "pdsdata/compress/HistNEngine.hh"

#include <string.h>
#include <stdio.h>

using namespace Pds;

CompressedPayload::CompressedPayload() {}

CompressedPayload::CompressedPayload(Engine   e,
                                     unsigned d,
                                     unsigned c) :
  _engine  (e),
  _reserved(0),
  _dsize   (d),
  _csize   (c)
{
}

CompressedPayload::Engine CompressedPayload::compressor() const { return Engine(_engine); }

unsigned CompressedPayload::dsize() const { return _dsize; }

unsigned CompressedPayload::csize() const { return _csize; }

const void* CompressedPayload::cdata() const { return (const void*)(this+1); }

bool CompressedPayload::uncompress(void* outbuf) const
{
  bool result;

  switch(compressor()) {
  case CompressedPayload::None:
    memcpy(outbuf, cdata(), csize());
    result = true;
    break;
  case CompressedPayload::Hist16: {
    Compress::Hist16Engine::ImageParams params;
    params.width  = dsize()/2;
    params.height = 1;
    params.depth  = 2;

    Compress::Hist16Engine e;
    int r = e.decompress(cdata(),
                         csize(),
                         params, 
                         outbuf);

    if (r == Compress::Hist16Engine::Success)
      result = true;
    else {
      printf("Hist16Engine::decompress failure %d : %s\n",r,e.err2str(r));
      result = false;
    }
    break; }
  case CompressedPayload::HistN: {
    Compress::HistNEngine e;
    int r = e.decompress(cdata(),
                         csize(),
                         outbuf,
                         dsize());

    if (r == Compress::HistNEngine::Success)
      result = true;
    else {
      printf("HistNEngine::decompress failure %d : %s\n",r,e.err2str(r));
      result = false;
    }
    break; }
  default:
    result = false;
    break;
  }
  return result;
}
