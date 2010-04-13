#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#ifdef _POSIX_MESSAGE_PASSING
#include <mqueue.h>
#endif
#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/Dgram.hh"

#include <poll.h>
#include <queue>
#include <stack>

using std::queue;
using std::stack;

#define PERMS (S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH)
#define OFLAGS (O_CREAT|O_RDWR)

using namespace Pds;

static char* dgramBuffer;

class Msg {
  public:
    Msg() {}
    Msg(int bufferIndex) {_bufferIndex = bufferIndex;}
    ~Msg() {}; 
    int bufferIndex() const {return _bufferIndex;}
    int numberOfBuffers() const { return _numberOfBuffers; }
    int sizeOfBuffers() const { return _sizeOfBuffers; }
    Msg* bufferIndex(int b) {_bufferIndex=b; return this;}
    void numberOfBuffers(int n) {_numberOfBuffers = n;} 
    void sizeOfBuffers(int s) {_sizeOfBuffers = s;} 
  private:
    int _bufferIndex;
    int _numberOfBuffers;
    unsigned _sizeOfBuffers;
};

class ShMsg {
public:
  ShMsg() {}
  ShMsg(const Msg&  m,
	Dgram* dg) : _m(m), _dg(dg) {}
  ~ShMsg() {}
public:
  const Msg&  msg() const { return _m; }
  Dgram* dg () const { return _dg; }
private:
  Msg _m;
  Dgram* _dg;
};

class XtcMonServer {
public:
  enum { numberofTrBuffers=8 };
public:
  XtcMonServer(unsigned sizeofBuffers, int numberofEvBuffers, unsigned numberofClients) : 
    _sizeOfBuffers(sizeofBuffers),
    _numberOfEvBuffers(numberofEvBuffers),
    _numberOfClients  (numberofClients),
    _priority(0)
  {
    _myMsg.numberOfBuffers(numberofEvBuffers+numberofTrBuffers);
    _myMsg.sizeOfBuffers(sizeofBuffers);

    _tmo.tv_sec  = 0;
    _tmo.tv_nsec = 0;
  }
  ~XtcMonServer() 
  { 
    printf("Not Unlinking ... \n");
  }

public:
  void events     (Dgram* dg) 
  {
    Dgram& dgrm = *dg;
    if (dgrm.seq.service() == TransitionId::L1Accept) {
      mq_getattr(_myInputEvQueue, &_mymq_attr);
      if (_mymq_attr.mq_curmsgs) {
	if (mq_receive(_myInputEvQueue, (char*)&_myMsg, sizeof(_myMsg), &_priority) < 0) 
	  perror("mq_receive");
	
	ShMsg m(_myMsg, dg);
	if (mq_timedsend(_shuffleQueue, (const char*)&m, sizeof(m), 0, &_tmo)) {
	  printf("ShuffleQ timedout\n");
	}
      }
    }
    else {

      if (_freeTr.empty()) {
	printf("No buffers available for transition!\n");
	abort();
      }

      int ibuffer = _freeTr.front(); _freeTr.pop();

      _myMsg.bufferIndex(ibuffer);
      _copyDgram(dg, ibuffer);

      if (unsigned(dgrm.seq.service())%2) {
	_pop_transition();
	_freeTr.push(ibuffer);
      }
      else 
	_push_transition(ibuffer);
      
      for(unsigned i=0; i<_numberOfClients; i++) {
	if (mq_timedsend(_myOutputTrQueue[i], (const char*)&_myMsg, sizeof(_myMsg), 0, &_tmo))
	  ;  // best effort
      }

      _moveQueue(_myOutputEvQueue, _myInputEvQueue);
    }
  }

  void routine()
  {
      if (::poll(_pfd,2,0) > 0) {
	if (_pfd[0].revents & POLLIN)
	  _initialize_client();

	if (_pfd[1].revents & POLLIN) {
	  ShMsg m;
	  if (mq_receive(_shuffleQueue, (char*)&m, sizeof(m), &_priority) < 0) 
	    perror("mq_receive");

	  _copyDgram(m.dg(), m.msg().bufferIndex());

	  if (mq_timedsend(_myOutputEvQueue, (const char*)&m.msg(), sizeof(m.msg()), 0, &_tmo)) {
	    printf("outputEv timedout\n");
	  }
	}
      }
  }

  int init(char *p) 
  { 
    char* shmName    = new char[128];
    char* toQname    = new char[128];
    char* fromQname  = new char[128];

    sprintf(shmName  , "/PdsMonitorSharedMemory_%s",p);
    sprintf(toQname  , "/PdsToMonitorEvQueue_%s",p);
    sprintf(fromQname, "/PdsFromMonitorEvQueue_%s",p);
    _pageSize = (unsigned)sysconf(_SC_PAGESIZE);
  
    int ret = 0;
    _sizeOfShm = (_numberOfEvBuffers + numberofTrBuffers) * _sizeOfBuffers;
    unsigned remainder = _sizeOfShm%_pageSize;
    if (remainder) _sizeOfShm += _pageSize - remainder;

    _mymq_attr.mq_maxmsg  = _numberOfEvBuffers;
    _mymq_attr.mq_msgsize = (long int)sizeof(Msg);
    _mymq_attr.mq_flags   = O_NONBLOCK;

    umask(1);  // try to enable world members to open these devices.

    int shm = shm_open(shmName, OFLAGS, PERMS);
    if (shm < 0) {ret++; perror("shm_open");}

    if ((ftruncate(shm, _sizeOfShm))<0) {ret++; perror("ftruncate");}

    _myShm = (char*)mmap(NULL, _sizeOfShm, PROT_READ|PROT_WRITE, MAP_SHARED, shm, 0);
    if (_myShm == MAP_FAILED) {ret++; perror("mmap");}

    _flushQueue(_myOutputEvQueue = _openQueue(toQname));

    _flushQueue(_myInputEvQueue  = _openQueue(fromQname));

    sprintf(fromQname, "/PdsFromMonitorDiscovery_%s",p);
    _pfd[0].fd      = _discoveryQueue  = _openQueue(fromQname);
    _pfd[0].events  = POLLIN;
    _pfd[0].revents = 0;
    
    _myOutputTrQueue = new mqd_t[_numberOfClients];
    for(unsigned i=0; i<_numberOfClients; i++) {
      sprintf(toQname  , "/PdsToMonitorTrQueue_%s_%d",p,i);
      _flushQueue(_myOutputTrQueue[i] = _openQueue(toQname));
    }

    struct mq_attr shq_attr;
    shq_attr.mq_maxmsg  = _numberOfEvBuffers;
    shq_attr.mq_msgsize = (long int)sizeof(ShMsg);
    shq_attr.mq_flags   = O_NONBLOCK;
    sprintf(toQname, "/PdsShuffleQueue_%s",p);
    _shuffleQueue = _openQueue(toQname, shq_attr);
    { ShMsg m; _flushQueue(_shuffleQueue,(char*)&m, sizeof(m)); }

    _pfd[1].fd = _shuffleQueue;
    _pfd[1].events  = POLLIN;
    _pfd[1].revents = 0;
      
    // prestuff the input queue which doubles as the free list
    for (int i=0; i<_numberOfEvBuffers; i++) {
      if (mq_send(_myInputEvQueue, (const char *)_myMsg.bufferIndex(i), sizeof(Msg), 0)) 
      { perror("mq_send inQueueStuffing");
	delete this;
	exit(EXIT_FAILURE);
      }
    }

    for(int i=0; i<numberofTrBuffers; i++)
      _freeTr.push(i+_numberOfEvBuffers);

    delete[] shmName;
    delete[] toQname;
    delete[] fromQname;

    return ret;
  }

private:  
  void _initialize_client()
  {
    Msg msg;
    if (mq_receive(_discoveryQueue, (char*)&msg, sizeof(msg), &_priority) < 0) 
      perror("mq_receive");

    unsigned iclient = msg.bufferIndex();
    printf("_initialize_client %d\n",iclient);

    std::stack<int> tr;
    while(!_cachedTr.empty()) {
      tr.push(_cachedTr.top());
      _cachedTr.pop();
    }
    while(!tr.empty()) {
      int ibuffer = tr.top(); tr.pop();
      _myMsg.bufferIndex(ibuffer);
      
      { Dgram& dgrm = *reinterpret_cast<Dgram*>(_myShm + _sizeOfBuffers * _myMsg.bufferIndex());
	printf("Sending tr %s to mq %d\nmsg %x/%x/%x\n",
	       TransitionId::name(dgrm.seq.service()), 
	       _myOutputTrQueue[iclient],
	       _myMsg.bufferIndex(),
	       _myMsg.numberOfBuffers(),
	       _myMsg.sizeOfBuffers()); }

      if (mq_send(_myOutputTrQueue[iclient], (const char*)&_myMsg, sizeof(_myMsg), 0)) 
	;   // best effort only
      _cachedTr.push(ibuffer);
    }
  }

  void _copyDgram(Dgram* dg, unsigned index) 
  {
    _bufferP = _myShm + (_sizeOfBuffers * index);
    memcpy((char*)_bufferP, dg, sizeof(Dgram)+dg->xtc.sizeofPayload());
  }

  mqd_t _openQueue(const char* name) { return _openQueue(name,_mymq_attr); }

  mqd_t _openQueue(const char* name, mq_attr& attr) {
    mqd_t q = mq_open(name,  O_CREAT|O_RDWR, PERMS, &attr);
    if (q == (mqd_t)-1) {
      perror("mq_open output");
      printf("mq_attr:\n\tmq_flags 0x%0lx\n\tmq_maxmsg 0x%0lx\n\tmq_msgsize 0x%0lx\n\t mq_curmsgs 0x%0lx\n",
	     attr.mq_flags, attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs );
      fprintf(stderr, "Initializing XTC monitor server encountered an error!\n");
      delete this;
      exit(EXIT_FAILURE);
    }
    else {
      printf("Opened queue %s (%d)\n",name,q);
    }
    return q;
  }

  void _flushQueue(mqd_t q) { Msg m; _flushQueue(q,(char*)&m,sizeof(m)); }

  void _flushQueue(mqd_t q, char* m, unsigned sz) {
    // flush the queues just to be sure they are empty.
    struct mq_attr attr;
    do {
      mq_getattr(q, &attr);
      if (attr.mq_curmsgs)
	mq_receive(q, m, sz, &_priority);
     } while (attr.mq_curmsgs);
  }

  void _moveQueue(mqd_t iq, mqd_t oq) {
    Msg m;
    struct mq_attr attr;
    do {
      mq_getattr(iq, &attr);
      if (attr.mq_curmsgs) {
	if (mq_receive(iq, (char*)&m, sizeof(m), &_priority) == -1)
	  perror("moveQueue: mq_receive");
	if (mq_send   (oq, (char*)&m, sizeof(m), 0) == -1) {
	  printf("Failed to reclaim buffer %i : %s\n",
		 m.bufferIndex(), strerror(errno));
	}
      }
     } while (attr.mq_curmsgs);
  }

  void _push_transition(int ibuffer)
  {
    _cachedTr.push(ibuffer);
  }

  void _pop_transition()
  {
    _freeTr.push(_cachedTr.top());
    _cachedTr.pop();
  }

public:
  Dgram* next(int fd) {
    Dgram& dg = *(Dgram*)dgramBuffer;
    unsigned header = sizeof(dg);
    int sz;
    if ((sz=::read(fd, dgramBuffer, header)) != int(header))
      return 0;

    unsigned payloadSize = dg.xtc.sizeofPayload();
    if ((payloadSize+header)>unsigned(_myMsg.sizeOfBuffers())) {
      printf("Dgram size 0x%x larger than maximum: 0x%x\n",
	     (unsigned)payloadSize+(unsigned)sizeof(dg), 
	     _myMsg.sizeOfBuffers()); 
      return 0;
    }
    
    if ((sz=::read(fd, dgramBuffer+header, payloadSize)) != int(payloadSize)) {
      printf("Read payload found %d/%d bytes\n",sz,payloadSize);
      return 0;
    }

    events(&dg);
    return &dg;
  }

  //
  //  Insert a simulated transition
  //
  void insert(TransitionId::Value tr) {
    Dgram* dg = (Dgram*)dgramBuffer;
    new((void*)&dg->seq) Sequence(Sequence::Event, tr, ClockTime(0,0), TimeStamp(0,0,0,0));
    new((char*)&dg->xtc) Xtc(TypeId(TypeId::Id_Xtc,0),ProcInfo(Level::Event,0,0));
    events(dg);
    printf("%s transition: time 0x%x/0x%x, payloadSize 0x%x\n",
	   TransitionId::name(dg->seq.service()),
	   dg->seq.stamp().fiducials(),dg->seq.stamp().ticks(),
	   dg->xtc.sizeofPayload());
  }
      
private:
  unsigned _sizeOfBuffers;
  int      _numberOfEvBuffers;
  unsigned _numberOfClients;
  unsigned _sizeOfShm;
  char*    _bufferP;   //  pointer to the shared memory area being used
  char*    _myShm; // the pointer to start of shared memory
  mqd_t    _myOutputEvQueue;
  mqd_t    _myInputEvQueue;
  unsigned _priority;
  unsigned _pageSize;
  struct mq_attr _mymq_attr;
  Msg _myMsg;
  mqd_t*   _myOutputTrQueue;
  mqd_t    _discoveryQueue;
  std::stack<int> _cachedTr;
  std::queue<int> _freeTr;
  pollfd     _pfd[2];
  mqd_t          _shuffleQueue;
  timespec       _tmo;
};

static XtcMonServer* apps;

long long int timeDiff(struct timespec* end, struct timespec* start) {
  long long int diff;
  diff =  (end->tv_sec - start->tv_sec) * 1000000000;
  diff += end->tv_nsec;
  diff -= start->tv_nsec;
  return diff;
}

void usage(char* progname) {
  fprintf(stderr,"Usage: %s -f <filename> -n <numberOfBuffers> -s <sizeOfBuffers> [-r <ratePerSec>] [-p <partitionTag>] [-l] [-h]\n", progname);
}

void sigfunc(int sig_no) {
   delete apps;
   exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
  int c;
  char* xtcname=0;
  char* partitionTag = 0;
  int rate = 1;
  unsigned nclients = 1;
  bool loop = false;
  bool verbose = false;
  int numberOfBuffers = 0;
  unsigned sizeOfBuffers = 0;
  struct timespec start, now, sleepTime;
  (void) signal(SIGINT, sigfunc);

  while ((c = getopt(argc, argv, "hf:r:n:s:p:lvc:")) != -1) {
    switch (c) {
    case 'h':
      usage(argv[0]);
      exit(0);
    case 'f':
      xtcname = optarg;
      break;
    case 'r':
      sscanf(optarg, "%d", &rate);
      break;
    case 'n':
      sscanf(optarg, "%d", &numberOfBuffers);
      break;
    case 's':
      sizeOfBuffers = (unsigned) strtoul(optarg, NULL, 0);
      break;
    case 'p':
      partitionTag = optarg;
      break;
    case 'c':
      nclients = strtoul(optarg, NULL, 0);
      break;
    case 'l':
      loop = true;
      printf("Enabling infinite looping\n");
      break;
    case 'v':
      verbose = true;
      break;
    default:
      fprintf(stderr, "I don't understand %c!\n", c);
      usage(argv[0]);
      exit(0);
    }
  }
  
  if (!xtcname || !sizeOfBuffers || !numberOfBuffers) {
    usage(argv[0]);
    printf("rate %d, numb %d, size %d, partition %s\n", rate, numberOfBuffers, sizeOfBuffers, partitionTag);
    exit(2);
  }

  dgramBuffer = new char[sizeOfBuffers];

  int fd = ::open(xtcname,O_LARGEFILE,O_RDONLY);
  if (fd == -1) {
    char s[120];
    sprintf(s, "Unable to open file %s ", xtcname);
    perror(s);
    exit(2);
  }

  long long int period = 1000000000 / rate;
  sleepTime.tv_sec = 0;
  long long int busyTime = period;

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

  apps = new XtcMonServer(sizeOfBuffers, 
			  numberOfBuffers, 
			  nclients);
  apps->init(partitionTag);
  
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
  printf("Opening shared memory took %lld nanonseconds.\n", timeDiff(&now, &start));

  do {

    apps->insert(TransitionId::Map);

    Dgram* dg;

    do {
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

      if ((dg = apps->next(fd))) {
	apps->routine();
	if (dg->seq.service() != TransitionId::L1Accept || verbose)
    printf("\r%s transition: time 0x%x/0x%x, payloadSize 0x%x, spareTime %lld ns, rate %lf Hz",
		 TransitionId::name(dg->seq.service()),
		 dg->seq.stamp().fiducials(),dg->seq.stamp().ticks(),
     dg->xtc.sizeofPayload(), period - busyTime, 1./(busyTime*1e-9));
      }
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
      busyTime = timeDiff(&now, &start);
      if (period > busyTime) {
	sleepTime.tv_nsec = period - busyTime;
	if (nanosleep(&sleepTime, &now)<0) perror("nanosleep");
      }
    } while (dg);

    apps->insert(TransitionId::Unconfigure);
    apps->insert(TransitionId::Unmap);

    if (loop) {
      ::close(fd);
      fd = ::open(xtcname,O_LARGEFILE,O_RDONLY);
      if (fd == -1) {
	char s[120];
	sprintf(s, "Unable to open file %s ", xtcname);
	perror(s);
	exit(2);
      }
    }

  } while(loop);

  ::close(fd);
  sigfunc(0);

  delete[] dgramBuffer;

  return 0;
}
