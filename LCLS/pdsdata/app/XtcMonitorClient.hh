#ifndef XTC_MONITOR_CLIENT_HH
#define XTC_MONITOR_CLIENT_HH


namespace Pds {

  class Dgram;

  class XtcMonitorClient {
  public:
    XtcMonitorClient() {}
    virtual ~XtcMonitorClient() {};
    
  public:
    int run(const char * partitionTag, int index=0);
    virtual int processDgram(Dgram*);
    
  };
}
#endif
