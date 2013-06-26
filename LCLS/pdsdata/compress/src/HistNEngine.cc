#include "pdsdata/compress/HistNEngine.hh"

#include <iomanip>
#include <sstream>
#include <string>
#include <stdint.h>
#include <string.h>

//#define DBUG

namespace {
  std::string format_addr( size_t addr )
  {
    std::ostringstream s;
    s << std::setw(6) << std::setfill('0') << addr;
    return s.str();
  }
}


namespace Pds {
  namespace Compress {

    class BitStream {
    public:
      BitStream(uint32_t* ptr) : _inptr(ptr), _outptr(ptr), _v(0), _b(0) {}
      ~BitStream() {}
    public:
      inline void push(uint64_t v, unsigned b) {  // assumes b < 32
        _v = (_v & ((1<<_b)-1)) | (v << _b);
        _b += b;
        if (_b >= 32) {
          *_outptr++ = _v&0xffffffff;
          _v >>= 32;
          _b -=  32;
        }
      }
      inline uint16_t pull(unsigned b) {   // assumes b < 32
        if (_b < b) {
          uint64_t v = *_outptr++;
          _v = (_v & ((1<<_b)-1)) | (v << _b);
          _b += 32;
        }
        _b -= b;
        uint16_t v = _v & ((1<<b)-1);
        _v >>= b;
        return v;
      }
      uint32_t* close() {
        if (_b)
          *_outptr++ = _v&0xffffffff;
        return _outptr;
      }
      unsigned nbytes() const { return (_outptr - _inptr)*sizeof(uint32_t); }
    private:
      uint32_t* _inptr;
      uint32_t* _outptr;
      uint64_t _v;
      unsigned _b;
    };

#pragma pack(4)
    class HistNEngine::Header {
    public:
      uint16_t compression_flag;
      uint16_t depth;
      uint32_t checksum;
      uint32_t data_size;
    };
    class HistNEngine::HeaderC {
    public:
      uint16_t unmap_bits;
      uint16_t map_bits;
      uint32_t map_offset;
      uint32_t comp_size;
    };
#pragma pack()

    template <class T>
    static void phaseI(const void* inbuf, size_t inbufsize, 
                       unsigned   base,
                       unsigned&  count,
                       unsigned   nbits, 
                       unsigned   step_nbits, 
                       unsigned   unmapBits,
                       uint16_t*& outbmapptr,
                       uint8_t*&  outptr);

    template <class T>
    static int copy(void* image, const uint8_t* data, size_t size, uint32_t checksum);

    template <class T>
    static int _ucomp(void*           image, 
                      size_t          imageSize,
                      const uint16_t* bitmap_ptr, 
                      const uint8_t*  data_ptr, 
                      const HistNEngine::Header&   hdr,
                      const HistNEngine::HeaderC&  hdrC);

    template <class T>
    static int _uucomp(void*           image, 
                       size_t          imageSize,
                       const uint16_t* bitmap_ptr, 
                       const uint8_t*  data_ptr, 
                       const HistNEngine::Header&   hdr,
                       const HistNEngine::HeaderC&  hdrC);


    HistNEngine::HistNEngine() :
      m_hist_channels      (new unsigned int[0x10000]),
      m_hist_channels_8bits(new unsigned int[0x100])
    {}

    HistNEngine::~HistNEngine()
    {
      delete [] m_hist_channels;
      delete [] m_hist_channels_8bits;
    }

    int HistNEngine::compress (const void*        image,
                               unsigned           depth,
                               size_t             inDataSize,
                               void*              outData,
                               size_t&            outDataSize )
    {
      /* Evaluate input parameters and refuse to proceed if any obvious
       * problems were found.
       */
      if( 0 == image )         return ErrZeroImage;     // zero pointer instead of an input image

      if( inDataSize > 512*1024*1024 ) return ErrLargeImage;  // input image is too large

      /* The temporary variables for the input buffer. The variables
       * are expressed in terms of 16-bit unsigned integers.
       */

      if (depth < 1 || depth > 2) return ErrIllegalDepth;

      const size_t    inbufsize = inDataSize/depth;
 
      uint8_t* m_outbuf      = (uint8_t*)outData;

      size_t m_outbmapsize = inbufsize/ 16 + 1;
      uint16_t* m_outbmap     = new uint16_t [m_outbmapsize];

      uint16_t* m_outbmapc    = new uint16_t [m_outbmapsize * 2];

      /* Rotating checksum calculated on the original uncompressed data
       */
      uint32_t incs = 0;

      /* Evaluate the input buffer to see if it makes a sense to compress it.
       * If so then we'll also calculate a common base for the pixel values
       * as well as the original checksum on the image.
       * Otherwise just copy it "as is" into the output structure.
       */
      if (depth == 1) {
        memset( m_hist_channels_8bits, 0, sizeof(unsigned int)*0x100   );

        const uint8_t* inbuf = (uint8_t*)image;
        const uint8_t* end   = inbuf + inbufsize;
        for( const uint8_t* ptr = inbuf; ptr < end; ++ptr ) {
          m_hist_channels_8bits[*ptr]++;
          incs += *ptr;
        }
      }
      else {
        memset( m_hist_channels,       0, sizeof(unsigned int)*0x10000 );
        memset( m_hist_channels_8bits, 0, sizeof(unsigned int)*0x100   );

        const uint16_t* inbuf = (uint16_t*)image;
        const uint16_t* end   = inbuf + inbufsize;
        for( const uint16_t* ptr = inbuf; ptr < end; ++ptr ) {
          m_hist_channels[*ptr]++;
          m_hist_channels_8bits[(*ptr)>>8]++;
          incs += *ptr;
        }
      }

      // At the first pass we're going to see if there are at least 1/2
      // of elements in any two adjusent bins of the 8-bits/bin histogram.
      // If not then it won't make any sense to proceed with the compression
      // because the compression rate would be to low due to added overhead
      // of the bit-map index.
      //
      // This step (once we establish a case for the compression) may also
      // seegnificantly speedup the next step of locating a cluster of elements
      // to be compressed.
      //
      const unsigned int* start_8bits     = m_hist_channels_8bits + 0x100;
      unsigned int        uMax2BlockCount = 0;
      for( unsigned int* ptr = m_hist_channels_8bits;
           ptr < m_hist_channels_8bits + 0x100 - 1;
           ++ptr ) {
        unsigned int u2BlockCount = *(ptr+1) + *ptr;
        if (u2BlockCount >= uMax2BlockCount)
          {
            uMax2BlockCount = u2BlockCount;
            start_8bits = ptr;
          }
      }

      //
      //  Find the range of the raw data - 
      //    for using less than the max bits to represent unmapped values.
      //
      unsigned int  unmapBits;
      {
        unsigned int        uMaxBin;
        for(uMaxBin = 0x100 - 1;  m_hist_channels_8bits[uMaxBin] == 0; uMaxBin--) ;

        if (depth == 1) {
          for(unmapBits = 0; uMaxBin!=0; unmapBits++, uMaxBin >>= 1) ;
        }
        else {
          if (uMaxBin==0) {   // 8 or fewer bits
            for(uMaxBin = 0x100 - 1; m_hist_channels[uMaxBin] == 0; uMaxBin--) ;
            for(unmapBits = 0; uMaxBin!=0; unmapBits++, uMaxBin >>= 1) ;
          }
          else {
            for(unmapBits = 8; uMaxBin!=0; unmapBits++, uMaxBin >>= 1) ;
          }
        }
      }

      if( uMax2BlockCount < inbufsize / 2 ) {

        /* NO COMPRESSION: just fill in the header and copy the contents of
         *                 the input buffer into the output one and return
         *                 from the method.
         */
        uint8_t* ptr = m_outbuf;

        Header* hdr = (Header*)ptr;
        hdr->compression_flag = 0;
        hdr->depth            = depth;
        hdr->checksum         = incs;
        hdr->data_size        = inDataSize;

        ptr += sizeof(*hdr);

        memcpy((void*)ptr, image, inDataSize);
        ptr += inDataSize;

        outDataSize = ptr - m_outbuf;

        delete[] m_outbmap;
        delete[] m_outbmapc;

        return Success;
      }

      // The second pass has two stages. At the first one a sweep accross
      // the first 256 elements is made in order to calculate the sum of counters.
      // At the second stage the alforithm will be sliding the window through
      // the remaining 256 elements to determine the window (of 256 bins) where
      // the maximum number of counts is found. For optimization purposes, the
      // the second state algorithm will use the sum from each previous iteration.
      //
      //unsigned int count_8bits = 0;
      //unsigned int* start = m_hist_channels + ( start_8bits - m_hist_channels_8bits ) * 0x100;
      //for( unsigned int* ptr = start;
      //                   ptr < start + 0x100;
      //                 ++ptr ) count_8bits += *ptr;
    
      //
      //  
      //

      //
      //  Choose best packing by smallest compressed size (neglecting final run-length encoding of bit-map)
      //
      unsigned nbits = 0;
      unsigned size_nbits  = inDataSize*8;
      unsigned count_nbits = 0;
      unsigned step_nbits  = 0;
      unsigned int* start_nbits = 0;

      unsigned int* const start = (depth == 1) ? 
        m_hist_channels_8bits :
        m_hist_channels + ( start_8bits - m_hist_channels_8bits ) * 0x100;

      for(unsigned nb=0; nb<=8; nb++) {
        unsigned int* start4compression = start;
        const unsigned step = 1<<nb;
        unsigned int* const stop = (depth == 1) ? start + (0x100-step) : start + (0x200-step);
        int new_count = 0;
        for( unsigned int* ptr = start; ptr < start + step; ++ptr)
          new_count += *ptr;
        int count = new_count;
        for( unsigned int* ptr = start; ptr < stop; ++ptr) {
          new_count += *(ptr+step) - *(ptr);
          if( new_count > count ) {
            count = new_count;
            start4compression = ptr + 1;                        
          }
        }

        unsigned size = (count*nb + (inbufsize-count)*unmapBits + inbufsize*2 + 
                         sizeof(Header)+sizeof(HeaderC)+sizeof(uint32_t))/8;
#ifdef DBUG
        printf("    nb %d  count %d  base %x  size %d\n",
               nb,count,(depth==1) ? start4compression-m_hist_channels_8bits : start4compression-m_hist_channels,size);
#endif
        if (size < size_nbits) {
          nbits = nb;
          size_nbits  = size;
          count_nbits = count;
          step_nbits  = step;
          start_nbits = start4compression;
        }
      }

      /* Phase I: compress the elements, build the bit-map to indicate
       *          which elements have been compressed.
       */

      unsigned base, count;
      uint8_t*  outptr     = m_outbuf + sizeof(Header) + sizeof(HeaderC);
      uint16_t* outbmapptr = m_outbmap;
      
      if (depth == 1) {
        base = start_nbits - m_hist_channels_8bits;
        phaseI<uint8_t>(image, inDataSize, 
                        base, count,
                        nbits, step_nbits, unmapBits,
                        outbmapptr, outptr);
      }
      else {
        base = start_nbits - m_hist_channels;
        phaseI<uint16_t>(image, inDataSize, 
                         base, count,
                         nbits, step_nbits, unmapBits,
                         outbmapptr, outptr);
      }

      unsigned nbytes = (outptr - m_outbuf) - sizeof(Header) - sizeof(HeaderC);

      /* Phase II: Try to compress the bitmap using runlength compression
       *           algorithm (counting words with fully populated bits).
       *
       * ATTENTION: If the compressed bitmap will have the bigger size than
       *            the uncompressed one then the uncompressed bitmap will
       *            be transfered into the output buffer. The compression flag
       *            be also updated according to store this.
       */
      uint16_t* outbmapcomprptr = m_outbmapc;
      uint16_t  counter = 0;

      for( uint16_t* ptr = m_outbmap; ptr < outbmapptr; ++ptr ) {

        const uint16_t bmap = *ptr;

        if( 0xFFFF == bmap ) {
          if( 0xFFFF == counter ) {

            /* Flush to the storage as we've reached the representation range
             * limit (16-bits) for the counter.
             */
            *(outbmapcomprptr++) = counter;
            *(outbmapcomprptr++) = bmap;
            counter = 1;
          } else {
            ++counter;
          }
        } else {
          if( 0 != counter ) {

            // Flush the previously accumulated counter to the storage because
            // the previous sequence has just ended.
            //
            *(outbmapcomprptr++) = counter;
            *(outbmapcomprptr++) = 0xFFFF;
            counter = 0;
          }
          *(outbmapcomprptr++) = 1;
          *(outbmapcomprptr++) = bmap;
        }
      }
      if( 0 != counter ) {

        /* Flush the previously accumulated counter to the storage because
         * the previous sequence has just ended.
         */
        *(outbmapcomprptr++) = counter;
        *(outbmapcomprptr++) = 0xFFFF;
        counter = 0;
      }
      const size_t outbmapcsize = outbmapcomprptr - m_outbmapc;    
    
      /* COMPRESSION: fill in the header and compressed data (image itself
       * and (compressed or not) bitmap.
       */
      {
        const uint32_t  compression_flag = outbmapcsize < m_outbmapsize ? 3 : 1;
        const size_t    bitmapsize       = outbmapcsize < m_outbmapsize ? outbmapcsize : m_outbmapsize;
        const uint16_t* bitmapptr        = outbmapcsize < m_outbmapsize ? m_outbmapc   : m_outbmap;

        const size_t data_size               =
          sizeof(HeaderC)                  +
          nbytes                           +  // storage for the compressed image itself
          sizeof(uint32_t)                 +  // storage for the size of the (compressed or not) bitmap itself (shorts)
          bitmapsize * sizeof(uint16_t);      // storage for the (compressed or not) bitmap itself

        Header* hdr = (Header*)m_outbuf;
        hdr->compression_flag = compression_flag;
        hdr->depth            = depth;
        hdr->checksum         = incs;
        hdr->data_size        = data_size;

        HeaderC* hdr2 = (HeaderC*)(hdr+1);
        hdr2->unmap_bits       = unmapBits;
        hdr2->map_bits         = nbits;
        hdr2->map_offset       = base;
        hdr2->comp_size        = nbytes;

        /* Now set the pointer beyond the last byte of the compressed image
         * and add the bitmap size and the bitmap itself.
         */
        uint8_t* ptr = outptr;
        *(uint32_t*)ptr = bitmapsize; ptr += sizeof(uint32_t);
        
        memcpy((void*)ptr, bitmapptr, bitmapsize * sizeof(uint16_t));
        ptr +=  bitmapsize * sizeof(uint16_t);

        outDataSize = ptr -        m_outbuf;

#ifdef DBUG
        printf("  insize %d  outsize %d  dsize %d  msize %d"
               "  unmapb %d  mapb %d  base 0x%x  count %d\n",
               inDataSize, outDataSize, nbytes, bitmapsize*sizeof(uint16_t),
               unmapBits, nbits, base, count);
#endif
      }
        
      delete[] m_outbmap;
      delete[] m_outbmapc;

      return Success;
    }

    int HistNEngine::decompress (const void*  outData,
                                 size_t       outDataSize,
                                 void*        image,
                                 size_t       imageSize)
    {
      /* Evaluate input parameters and refuse to proceed if any obvious
       * problems were found.
       */
      if( 0 == outData ) return ErrZeroImage;  // zero pointer instead of a compressed image

      const size_t hdr_size_bytes = sizeof(Header);

      if( outDataSize < hdr_size_bytes )

        return ErrSmallImage;  // compressed image is too small

      Header* hdr = (Header*)outData;
      const uint32_t hdr_compression_flag  = hdr->compression_flag;
      const uint32_t hdr_original_checksum = hdr->checksum;

      if( hdr->depth < 1 || hdr->depth > 2 ) return ErrIllegalDepth;  // unsupported image depth

      const size_t hdr_compressed_size_bytes = hdr->data_size;

      if( hdr_size_bytes + hdr_compressed_size_bytes != outDataSize )

        return ErrBadHdr;  // inconsistent size of the compressed image
                           // and its payload.

      uint16_t* m_image = reinterpret_cast<uint16_t*>(image);
      unsigned m_imagesize = imageSize;

      if( 0 == hdr_compression_flag ) {

        /* NO COMPRESSION: of the original image. Just copy the image over and
         * recalculate the checksum to make sure nothing has been lost.
         */
        if( m_imagesize != hdr_compressed_size_bytes )

          return ErrBadImageData;  // inconsistent size of the uncompressed
        // data block.

        if (hdr->depth == 1) 
          return copy<uint8_t >(image, (const uint8_t*)(hdr+1), imageSize, hdr_original_checksum);
        else
          return copy<uint16_t>(image, (const uint8_t*)(hdr+1), imageSize, hdr_original_checksum);
      }

      /* COMPRESSED IMAGE: uncompressed using the attached bitmap.
       * If the last one is also compressed then uncompress it on the fly.
       */
      HeaderC* hdrp = (HeaderC*)(hdr+1);
      const size_t   data_size_bytes = hdrp->comp_size;

      if( hdr_compressed_size_bytes < sizeof(HeaderC) + data_size_bytes )
        return ErrBadImageData;  // inconsistent size of the compressed
                                 // data block.

      uint8_t*     data_ptr           = (uint8_t*)(hdrp+1);
      uint8_t*     bitmap_size_ptr    = data_ptr + data_size_bytes;
      const size_t bitmap_size_shorts = *(uint32_t*)bitmap_size_ptr; bitmap_size_ptr += sizeof(uint32_t);
      uint16_t*    bitmap_ptr         = (uint16_t*)bitmap_size_ptr;

#if 0
      printf(" flags : %08x\n"
             " cksum : %08x\n"
             " csize : %08x\n"
             " base  : %04x\n"
             " dsize : %08x\n"
             " bsize : %08x\n"
             " mbits : %d\n"
             " ubits : %d\n",
             hdr_compression_flag, hdr_original_checksum, hdr_compressed_size_bytes, base, data_size_bytes, bitmap_size_shorts,
             map_bits, unmap_bits);
#endif

      if( hdr_compressed_size_bytes != 
          sizeof(HeaderC)  +
          data_size_bytes  +       
          sizeof(uint32_t) +
          sizeof(uint16_t) * bitmap_size_shorts )

        return ErrBadImageBitMap;  // inconsistent size of the bitmap
      // data block.

      if( 3 == hdr_compression_flag ) {

        /* COMPRESSED BIT-MAP: make sure it's consistent with the compressed
         * image.
         */

        if (hdr->depth == 1)
          return _ucomp<uint8_t>(m_image, m_imagesize,
                                 bitmap_ptr, data_ptr, *hdr, *hdrp);
        else
          return _ucomp<uint16_t>(m_image, m_imagesize,
                                  bitmap_ptr, data_ptr, *hdr, *hdrp);

      } else if( 1 == hdr_compression_flag ) {

        /* NON-COMPRESSED BITMAP:
         */

        if (hdr->depth == 1)
          return _uucomp<uint8_t>(m_image, m_imagesize,
                                  bitmap_ptr, data_ptr, *hdr, *hdrp);
        else
          return _uucomp<uint16_t>(m_image, m_imagesize,
                                   bitmap_ptr, data_ptr, *hdr, *hdrp);

      } else {
        return ErrUnknownComprMethod;  // unsupported compresion algorithm
      }
    }

    const char* HistNEngine::err2str(int code)
    {
      switch(code) {
      case Success:               return "Success";
      case ErrZeroImage:          return "Zero pointer instead of a compressed or uncompressed image";
      case ErrIllegalDepth:       return "Illegal/unsupported image depth";
      case ErrLargeImage:         return "Image is is too large";
      case ErrSmallImage:         return "Image is is too small";
      case ErrBadHdr:             return "Bad image header";
      case ErrBadImageData:       return "Inconsistent size of the image data block";
      case ErrBadImageBitMap:     return "Inconsistent size of the bitmap data block";
      case ErrCSMissmatch:        return "Checksum doesn't match the original one";
      case ErrBitMapMissmatch:    return "Bit-map isn't consistent with the compressed image size";
      case ErrUnknownComprMethod: return "Unknown compresion method";
      }
      return "Unknown status code";
    }

    template <class T>
    void phaseI(const void* inbuf, 
                size_t     inbufsize, 
                unsigned   base,
                unsigned&  count,
                unsigned   nbits, 
                unsigned   step_nbits, 
                unsigned   unmapBits,
                uint16_t*& outbmapptr,
                uint8_t*&  outptr)
    {
      uint16_t bit  = 0x1;  // current bit in the bitmap
      uint16_t bmap = 0x0;  // current bitmap word
      
      BitStream bs((uint32_t*)outptr);
#ifdef DBUG
      count = 0;
#endif
      const T* end = (const T*)inbuf + inbufsize/sizeof(T);
      for( const T* ptr = (const T*)inbuf; ptr < end; ++ptr ) {

        if( *ptr >= base ) {
          const unsigned v_offset = *ptr - base;
          if( v_offset < step_nbits ) {
            bs.push(v_offset,nbits);
            bmap |= bit;
#ifdef DBUG
            count++;
#endif
          } else {
            bs.push(*ptr,unmapBits);
          }
        } else {
          bs.push(*ptr,unmapBits);
        }
        bit <<= 1;
        if( 0x0 == bit ) {
          *(outbmapptr++) = bmap;
          bit  = 0x1;
          bmap = 0x0;
        }
      }
      if( 0x1 != bit ) *(outbmapptr++) = bmap;    // Make sure the last bit map word is stored
      // if it was't complete within the loop.
      outptr = (uint8_t*)bs.close();
    }

    template <class T>
    int copy(void* image, const uint8_t* data, size_t size, uint32_t checksum)
    {
      uint32_t  cs        = 0;
      T* out_ptr   = (T*)image;
      const T* ptr_begin = (const T*)data;
      const T* const ptr_end   = ptr_begin + size/sizeof(T);

      for( const T* ptr = ptr_begin; ptr != ptr_end; ++ptr ) {
        *(out_ptr++) = *ptr;
        cs += *ptr;
      }
      return ( checksum != cs ) ? HistNEngine::ErrCSMissmatch : HistNEngine::Success;
    }

    template <class T>
    int _ucomp(void*           image, 
               size_t          imageSize,
               const uint16_t* bitmap_ptr, 
               const uint8_t*  data_ptr, 
               const HistNEngine::Header&   hdr,
               const HistNEngine::HeaderC&  hdrC)
    {
      size_t          data_size_bytes = hdrC.comp_size;
      unsigned        base            = hdrC.map_offset; 
      unsigned        map_bits        = hdrC.map_bits; 
      unsigned        unmap_bits      = hdrC.unmap_bits;

      bool done = false;

      T* outptr = (T*)image;
      T* const endptr = outptr + imageSize/sizeof(T);

      uint32_t cs = 0;

      BitStream bs((uint32_t*)data_ptr);
      
      for( const uint16_t* mptr = bitmap_ptr; !done; ) {

        const uint16_t counter = *(mptr++);
        const uint16_t bmap    = *(mptr++);

        /* TODO: consider optimizing the code for simple bitmaps (0x0 or 0xFFFF)
         */
        for( uint16_t i = 0; i < counter; ++i ) {

          uint16_t bit = 0x1;

          do {
            unsigned v;
            if( bmap & bit ) v = bs.pull(map_bits) + base;
            else             v = bs.pull(unmap_bits);
            *(outptr++) = v;
            cs += v;

            if( outptr >= endptr ) {
              
              if( bs.nbytes() != data_size_bytes )
                
                return HistNEngine::ErrBitMapMissmatch;  // the compressed bit-map isn't consistent with the compressed image size
              
              done = true;
              break;
            }
            bit <<= 1;
            
          } while( bit );
          
          if( done ) break;
        }
      }

      return ( hdr.checksum != cs ) ? HistNEngine::ErrCSMissmatch :  HistNEngine::Success;
    }


    template <class T>
    int _uucomp(void*           image, 
                size_t          imageSize,
                const uint16_t* bitmap_ptr, 
                const uint8_t*  data_ptr, 
                const HistNEngine::Header&   hdr,
                const HistNEngine::HeaderC&  hdrC)
    {
      size_t          data_size_bytes = hdrC.comp_size;
      unsigned        base            = hdrC.map_offset; 
      unsigned        map_bits        = hdrC.map_bits; 
      unsigned        unmap_bits      = hdrC.unmap_bits;

      bool done = false;

      T* outptr = (T*)image;
      T* const endptr = outptr + imageSize/sizeof(T);

      uint32_t cs = 0;

      BitStream bs((uint32_t*)data_ptr);

      for( const uint16_t* mptr = bitmap_ptr; !done; ++mptr ) {

        const uint16_t bmap = *mptr;

        /* TODO: consider optimizing the code for simple bitmaps (0x0 or 0xFFFF)
         */
        uint16_t bit = 0x1;

        do {
          unsigned v;
          if( bmap & bit ) v = bs.pull(map_bits) + base;
          else             v = bs.pull(unmap_bits);
          *(outptr++) = v;
          cs += v;
            
          if( outptr >= endptr ) {

            if( bs.nbytes() != data_size_bytes )
                
              return HistNEngine::ErrBitMapMissmatch;  // the compressed bit-map isn't consistent with the compressed

            done = true;
            break;
          }
          bit <<= 1;

        } while( bit );

        if( done ) break;
      }

      return ( hdr.checksum != cs ) ? HistNEngine::ErrCSMissmatch : HistNEngine::Success;
    }

  }  // namespace Compress
}  // namespace Pds


