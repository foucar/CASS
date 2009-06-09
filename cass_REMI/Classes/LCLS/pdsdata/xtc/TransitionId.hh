#ifndef Pds_TransitionId_hh
#define Pds_TransitionId_hh

namespace Pds {

  class TransitionId {
  public:
    enum Value {
      Unknown, Reset,
      Map, Unmap,
      Configure, Unconfigure,
      BeginRun, EndRun,
      Pause, Resume,
      Enable, Disable,
      L1Accept,
      NumberOf };
    static const char* name(TransitionId::Value id);
  };
}

#endif
