#ifndef XTC_MONITOR_CLIENT_HH
#define XTC_MONITOR_CLIENT_HH


namespace Pds {

  class Dgram;

  class XtcMonitorClient {
    public:
      XtcMonitorClient() {};
      virtual ~XtcMonitorClient() {};

    public:
      int runMonitor(char * partitionTag);
      virtual void processDgram(Dgram*);
  };
}
#endif
