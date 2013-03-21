#ifndef Pds_XtcMonitorMsg_hh
#define Pds_XtcMonitorMsg_hh

#include <stdint.h>

namespace Pds {
  class XtcMonitorMsg {
    enum { SizeMask   = 0x0fffffff };
    enum { SerialShift = 28 };
  public:
    XtcMonitorMsg() : _reserved(0) {}
    XtcMonitorMsg(int bufferIndex) : _bufferIndex(bufferIndex), _reserved(0) {}
    ~XtcMonitorMsg() {}; 
  public:
    int bufferIndex     () const {return _bufferIndex;}
    int numberOfBuffers () const { return _numberOfBuffers; }
    int sizeOfBuffers   () const { return _sizeOfBuffers&SizeMask; }
    bool serial         () const { return (_sizeOfBuffers>>SerialShift)==0; }
    int return_queue    () const { return (_sizeOfBuffers>>SerialShift); }
  public:
    XtcMonitorMsg* bufferIndex(int b) {_bufferIndex=b; return this;}
    void numberOfBuffers      (int n) {_numberOfBuffers = n;} 
    void sizeOfBuffers        (int s) {_sizeOfBuffers = (_sizeOfBuffers&~SizeMask) | (s&SizeMask);}
    void return_queue         (int q) {_sizeOfBuffers = (_sizeOfBuffers&SizeMask) | (q<<SerialShift);}
  public:
    static void sharedMemoryName     (const char* tag, char* buffer);
    static void eventInputQueue      (const char* tag, unsigned client, char* buffer);
    static void eventOutputQueue     (const char* tag, unsigned client, char* buffer);
    static void transitionInputQueue (const char* tag, unsigned client, char* buffer);
    static void discoveryQueue       (const char* tag, char* buffer);
    static void registerQueue        (const char* tag, char* buffer, int id);
  private:
    int32_t  _bufferIndex;
    int32_t  _numberOfBuffers;
    uint32_t _sizeOfBuffers;
    uint32_t _reserved;
  };
};

#endif
