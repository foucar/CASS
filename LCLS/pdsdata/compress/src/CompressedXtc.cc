#include "pdsdata/compress/CompressedXtc.hh"
#include "pdsdata/compress/CompressedData.hh"
#include "pdsdata/compress/HistNEngine.hh"
#include "pdsdata/compress/Hist16Engine.hh"
#include "pdsdata/xtc/TypeId.hh"

#include <boost/shared_ptr.hpp>

#include <string.h>

using namespace Pds;

static const unsigned align_mask = sizeof(uint32_t)-1;

static void Destroy(Xtc* p) { delete[](char*)p; }

boost::shared_ptr<Xtc> CompressedXtc::uncompress(const Xtc& xtc)
{
  const char* end = reinterpret_cast<const char*>(xtc.next());
  char* payload = xtc.payload(); 

  //  Calculate the uncompressed size
  size_t sz = sizeof(xtc);
  while (payload<end) {
    const CompressedData& cd = *reinterpret_cast<const CompressedData*>(payload);
    sz += cd.headerSize() + cd.pd().dsize();
    payload += (sizeof(cd) + cd.headerSize() + sizeof(cd.pd()) + cd.pd().csize() + align_mask)&~align_mask;
  }

  char* p = new char[sz];
  Xtc* pxtc = new (p) Xtc( TypeId(xtc.contains.id(), xtc.contains.compressed_version()),
                           xtc.src, xtc.damage );
  payload = xtc.payload();
  while (payload < end) {
    const CompressedData& cd = *reinterpret_cast<const CompressedData*>(payload);
    memcpy( pxtc->alloc(cd.headerSize()), cd.header(), cd.headerSize());
    if (!cd.pd().uncompress( pxtc->alloc(cd.pd().dsize()) ) ) {
      delete[] p;
      p = 0;
      break;
    }
    payload += (sizeof(cd) + cd.headerSize() + sizeof(cd.pd()) + cd.pd().csize() + align_mask)&~align_mask;
  }
    
  boost::shared_ptr<Xtc> q(reinterpret_cast<Xtc*>(p),Destroy);
  return q;
}

static void compress_image(Xtc& xtc,
                           Xtc& oxtc,
                           const std::list<unsigned>& headerOffsets,
                           const std::list<unsigned>& headerSizes,
                           unsigned depth,
                           CompressedPayload::Engine engine ) 
{
  std::list<unsigned>::const_iterator it=headerOffsets.begin();
  std::list<unsigned>::const_iterator sz=headerSizes  .begin();
  while(it!=headerOffsets.end()) {

    unsigned hoff = *it;
    unsigned headerSize = *sz;
    char* payload = xtc.payload()+hoff;
    new (oxtc.alloc(sizeof(CompressedData))) CompressedData(headerSize);
    memcpy(oxtc.alloc(headerSize), payload, headerSize);

    char*    ibuff = payload+headerSize;
    char*    obuff = (char*)oxtc.next()+sizeof(CompressedPayload);
    unsigned dsize = (++it == headerOffsets.end()) ? (char*)xtc.next()-ibuff : (*it)-hoff-headerSize;
    Compress::Hist16Engine::ImageParams img;
    img.width  = dsize/depth;
    img.height = 1;
    img.depth  = depth;
    size_t   csize;

    if      (engine == CompressedPayload::HistN &&
             Compress::HistNEngine().compress(ibuff,depth,dsize,obuff,csize) == Compress::HistNEngine::Success)
      ;
    else if (engine == CompressedPayload::Hist16 &&
             Compress::Hist16Engine().compress(ibuff,img,obuff,csize) == Compress::Hist16Engine::Success)
      ;
    else {
      engine = CompressedPayload::None;
      memcpy(obuff, ibuff, csize=dsize);
    }

    new (oxtc.alloc(sizeof(CompressedPayload))) CompressedPayload(engine,dsize,csize);
    oxtc.alloc((csize+align_mask)&~align_mask);

    ++sz;
  }
}

CompressedXtc::CompressedXtc( Xtc&     xtc,
                              const std::list<unsigned>& headerOffsets,
                              unsigned headerSize,
                              unsigned depth,
                              CompressedPayload::Engine engine ) :
  Xtc( TypeId(xtc.contains.id(), xtc.contains.version(), true),
       xtc.src,
       xtc.damage )
{
  std::list<unsigned> headerSizes;
  for(std::list<unsigned>::const_iterator it=headerOffsets.begin(); it!=headerOffsets.end(); it++)
    headerSizes.push_back(headerSize);

  compress_image(xtc, *this, headerOffsets, headerSizes, depth, engine);
}

CompressedXtc::CompressedXtc( Xtc&     xtc,
                              const std::list<unsigned>& headerOffsets,
                              const std::list<unsigned>& headerSizes,
                              unsigned depth,
                              CompressedPayload::Engine engine ) :
  Xtc( TypeId(xtc.contains.id(), xtc.contains.version(), true),
       xtc.src,
       xtc.damage )
{
  compress_image(xtc, *this, headerOffsets, headerSizes, depth, engine);
}

