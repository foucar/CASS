#include "pdsdata/app/XtcMonitorServer.hh"

#include "pdsdata/xtc/Dgram.hh"

#include <unistd.h>
#include <semaphore.h>
#ifdef _POSIX_MESSAGE_PASSING
#include <mqueue.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/prctl.h>
#include <sys/stat.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <list>

//#define DBUG

using std::queue;
using std::stack;

static const unsigned numberofTrBuffers=16;

//
//  Recover any shared memory buffer older than 10 seconds
//
static const unsigned TMO_SEC = 10;

#define PERMS (S_IRUSR|S_IRGRP|S_IROTH|S_IWUSR|S_IWGRP|S_IWOTH)
#define PERMS_IN (S_IRUSR|S_IRGRP|S_IROTH)
#define OFLAGS (O_CREAT|O_RDWR)

namespace Pds {
  class ShMsg {
  public:
    ShMsg() {}
    ShMsg(const XtcMonitorMsg&  m,
	  Dgram* dg) : _m(m), _dg(dg) {}
    ~ShMsg() {}
  public:
    const XtcMonitorMsg&  msg() const { return _m; }
    Dgram* dg () const { return _dg; }
  private:
    XtcMonitorMsg _m;
    Dgram*        _dg;
  };

  class TransitionCache {
  public:
    TransitionCache(char* p, size_t sz) : _pShm(p), _szShm(sz), _not_ready(0) 
    { 
      sem_init(&_sem, 0, 1);
      memset(&_allocated, 0, numberofTrBuffers*sizeof(unsigned)); 

      for(unsigned i=0; i<numberofTrBuffers; i++)
	_freeTr.push_back(i);
    }    
    ~TransitionCache() { sem_destroy(&_sem); }
  public:
    std::stack<int> current() {
      sem_wait(&_sem);
      std::stack<int> cached(_cachedTr);
      std::stack<int> tr;
      while(!cached.empty()) {
	tr.push(cached.top());
	cached.pop();
      }
      sem_post(&_sem);
      return tr;
    }
    int  allocate  (TransitionId::Value id) {
#ifdef DBUG
      printf("allocate(%s)\n",TransitionId::name(id));
      for(unsigned i=0; i<numberofTrBuffers; i++)
	printf("%08x ",_allocated[i]);
      printf("\n");
#endif
      sem_wait(&_sem);

      for(std::list<int>::iterator it=_freeTr.begin();
	  it!=_freeTr.end(); it++)
	if (_allocated[*it]==0) {
	  unsigned ibuffer = *it;
	  
	  //
	  //  Cache the transition for any clients 
	  //    which may not be listening (yet)
	  //
	  if ( _cachedTr.empty() ) {
	    if (id==TransitionId::Map) {
	      _freeTr.remove(ibuffer);
	      _cachedTr.push(ibuffer);
	    }
	  }
	  else {
	    const Dgram& odg = *reinterpret_cast<const Dgram*>(_pShm + _szShm*_cachedTr.top());
	    TransitionId::Value oid = odg.seq.service();
	    if (id == oid+2) {
	      _freeTr.remove(ibuffer);
	      _cachedTr.push(ibuffer);
	    }
	    else if (id == oid+1) {
	      int ib=_cachedTr.top();
	      _cachedTr.pop();
	      _freeTr.push_back(ib);
	    }
	    else {  // unexpected transition
	    }
	  }

	  if ((id&1)==0) {
	    unsigned not_ready=0;
	    for(unsigned itr=0; itr<numberofTrBuffers; itr++) {
	      if (itr==ibuffer) continue;
	      const Dgram& odg = *reinterpret_cast<const Dgram*>(_pShm + _szShm*itr);
	      if (odg.seq.service()==TransitionId::Enable)
		not_ready |= _allocated[itr];
	    }

	    if (not_ready &~_not_ready)
	      printf("Transition %s: not_ready %x -> %x\n",
		     TransitionId::name(id), _not_ready, _not_ready|not_ready);

	    _not_ready |= not_ready;
	  }

	  sem_post(&_sem);

#ifdef DBUG
	  printf("not_ready %08x\n",_not_ready);
#endif
	  return ibuffer;
	}
      return -1;
    }
    bool allocate  (int ibuffer, unsigned client) {
#ifdef DBUG
      printf("allocate[%d,%d] not_ready %08x\n",ibuffer,client,_not_ready);
#endif

      sem_wait(&_sem);

      if (_not_ready & (1<<client)) {
	TransitionId::Value last=TransitionId::NumberOf;
	for(unsigned i=0; i<numberofTrBuffers; i++)
	  if (_allocated[i] & (1<<client)) {
	    TransitionId::Value td = 
	      reinterpret_cast<const Dgram*>(_pShm + _szShm*i)->seq.service();
	    if ((td&1)==1 && td<last) last=td;
	  }
	
	TransitionId::Value id = 
	  reinterpret_cast<const Dgram*>(_pShm + _szShm*ibuffer)->seq.service();
	if (!((id&1)==1 && id<last)) {
	  sem_post(&_sem);
	  return false;
	}
      }
      _allocated[ibuffer] |= (1<<client);

      sem_post(&_sem);

#ifdef DBUG
      printf("_allocated[%d] = %08x\n",ibuffer,_allocated[ibuffer]);
#endif 
      return true;
    }
    bool deallocate(int ibuffer, unsigned client) {
      sem_wait(&_sem);
      bool result=false;
      { unsigned v = _allocated[ibuffer] & ~(1<<client);
#ifdef DBUG
	printf("_deallocate[%d,%d] %08x -> %08x\n",ibuffer,client,
	       _allocated[ibuffer],v);
#else
	if ( _allocated[ibuffer]==v )
	  printf("_deallocate[%d,%d] %08x no change\n",ibuffer,client,v);
#endif       
	_allocated[ibuffer]=v; }

      if (_not_ready & (1<<client)) {
	for(unsigned i=0; i<numberofTrBuffers; i++)
	  if (_allocated[i] & (1<<client)) {
	    sem_post(&_sem);
	    return false;
	  }
	printf("not_ready %x -> %x\n", _not_ready, _not_ready&~(1<<client));
	_not_ready &= ~(1<<client);
	result=true;
      }
      sem_post(&_sem);
      return result;
    }
    void deallocate(unsigned client) {
      sem_wait(&_sem);
      for(unsigned itr=0; itr<numberofTrBuffers; itr++)
	_allocated[itr] &= ~(1<<client);
      sem_post(&_sem);
#ifdef DBUG
      printf("deallocate %d\n",client);
      for(unsigned i=0; i<numberofTrBuffers; i++)
	printf("%08x ",_allocated[i]);
      printf("\n");
#endif
    }
    unsigned not_ready() const { return _not_ready; }
  private:
    sem_t            _sem;
    const char*      _pShm;
    size_t           _szShm;
    unsigned         _not_ready;
    unsigned         _allocated[numberofTrBuffers];
    std::stack <int> _cachedTr;
    std::list  <int> _freeTr;
  };
};

using namespace Pds;

XtcMonitorServer::XtcMonitorServer(const char* tag,
				   unsigned sizeofBuffers, 
				   unsigned numberofEvBuffers,
				   unsigned numberofEvQueues) :
  _tag              (tag),
  _sizeOfBuffers    (sizeofBuffers),
  _numberOfEvBuffers(numberofEvBuffers),
  _numberOfEvQueues (numberofEvQueues),
  _myShm            (0),
  _discoveryQueue   (-1),
  _myInputEvQueue   (-1),
  _myOutputEvQueue  (new mqd_t[numberofEvQueues]),
  _myTrFd           (0),
  _msgDest          (numberofEvBuffers),
  _pfd              (new pollfd[32]),
  _nfd              (2),
  _ievt             (0) 
{
  _myMsg.numberOfBuffers(numberofEvBuffers+numberofTrBuffers);
  _myMsg.sizeOfBuffers  (sizeofBuffers);
  _myMsg.return_queue   (0);

  _tmo.tv_sec  = 0;
  _tmo.tv_nsec = 0;

  _init();
}

XtcMonitorServer::~XtcMonitorServer() 
{ 
  pthread_kill(_discThread, SIGTERM);
  pthread_kill(_taskThread  , SIGTERM);
  printf("Not Unlinking Shared Memory... \n");

  unlink();

  delete _transitionCache;
}

void XtcMonitorServer::distribute(bool l)
{
  _myMsg.return_queue( l ? _numberOfEvQueues : 0 );

  _flushQueue(_myInputEvQueue);
  for(unsigned i=0; i<_numberOfEvBuffers; i++) {
    _myMsg.bufferIndex(i);
    mq_send(_myInputEvQueue, (const char*)&_myMsg, sizeof(_myMsg), 0);
  }
}

bool XtcMonitorServer::_send(Dgram* dg)
{
  //
  //  For reasons I don't yet understand, sometimes the message queues
  //  are opened in blocking mode.  So, I use mq_timedreceive 
  //  with a 0 timeout to avoid blocking.
  //
  const timespec no_wait={0,0};
  int r = mq_timedreceive(_myInputEvQueue, (char*)&_myMsg, sizeof(_myMsg), NULL,
                          &no_wait); 
  if (r>0)
    ;
  else {
    if (r<0) ; // perror("Error reading input event queue");
    for(unsigned i=0; i<_numberOfEvQueues; i++) {
      r=mq_timedreceive(_myOutputEvQueue[i], (char*)&_myMsg, sizeof(_myMsg), NULL,
                        &no_wait);
      if (r>0) break;
      if (r<0) ; // perror("Error reading output event queue");
    }
  }

  if (r>0) {
    _msgDest[_myMsg.bufferIndex()]=-1;
    ShMsg m(_myMsg, dg);
    if (mq_timedsend(_shuffleQueue, (const char*)&m, sizeof(m), 0, &_tmo)) {
      printf("ShuffleQ timedout\n");
      _deleteDatagram(dg);
    }
  }
  else {
    printf("No shared memory buffer found.  Dropping new event.\n");
    _deleteDatagram(dg);
  }
  return true;
}

XtcMonitorServer::Result XtcMonitorServer::events(Dgram* dg) 
{
  Dgram& dgrm = *dg;
  if (sizeof(dgrm)+dgrm.xtc.sizeofPayload() > _sizeOfBuffers) {
    printf("XtcMonitorServer skipping %s with payload size %d - too large\n",
	   TransitionId::name(dgrm.seq.service()), dgrm.xtc.sizeofPayload());
    //    return Handled;
    exit(1);
  }

  if (dgrm.seq.service() == TransitionId::L1Accept) {
    _send(dg);
    return Deferred;
  }
  else {

    TransitionId::Value trid = dgrm.seq.service();

    int itr = _transitionCache->allocate(trid);

    if (itr < 0) {
      printf("No buffers available for transition !\n");
      abort();
    }

    int ibuffer = itr + _numberOfEvBuffers;

    _myMsg.bufferIndex(ibuffer);
    _copyDatagram(dg, _myShm + _sizeOfBuffers*ibuffer);

    if (trid == TransitionId::Enable) {
      //
      //  Steal all event buffers from the clients
      //
      for(unsigned i=0; i<_numberOfEvQueues; i++)
        _moveQueue(_myOutputEvQueue[i], _myInputEvQueue);
    }

    //
    //  Broadcast the transition to all ready clients
    //
    for(unsigned i=0; i<_myTrFd.size(); i++) {
      int oq = _myTrFd[i];
      if (oq == -1 || !_transitionCache->allocate(itr,i))
	continue;
      if (::send(oq, (const char*)&_myMsg, sizeof(_myMsg), 0) < 0) {
	perror("Error sending transition");
	_transitionCache->deallocate(itr,i);
      }
    }

  }
  return Handled;
}

void XtcMonitorServer::discover()
{
  int fd;
  if ( (fd = ::socket(AF_INET, SOCK_STREAM, 0))<0 ) {
    perror("Create discovery socket failed");
    exit(1);
  }

  //  assign an ephemeral port
  unsigned port = 32768;
  sockaddr_in saddr;
  saddr.sin_family      = AF_INET;
  saddr.sin_addr.s_addr = htonl(0x7f000001);
  while(1) {
    saddr.sin_port        = htons(port);
#ifdef DBUG
    printf("Trying to bind to port %d\n",port);
#endif
    if (::bind(fd,(sockaddr*)&saddr,sizeof(saddr))>=0)
      break;
#ifdef DBUG
    else
      perror("bind failed");
#endif
    port++;
  }
  printf("Awaiting clients on port %d\n",port);

  const char* p = _tag;
  char* fromQname  = new char[128];
  XtcMonitorMsg::discoveryQueue(p, fromQname);
  struct mq_attr q_attr;
  q_attr.mq_maxmsg  = 2;
  q_attr.mq_msgsize = (long int)sizeof(XtcMonitorMsg);
  q_attr.mq_flags   = O_NONBLOCK;
  _discoveryQueue  = _openQueue(fromQname,q_attr);
  delete[] fromQname;

  if (::listen(fd,10)<0)
    printf("ConnectionManager listen failed\n");
  else {
    while(1) {
      XtcMonitorMsg m(port);
      if (mq_send(_discoveryQueue,(const char*)&m,sizeof(m),0)<0) {
	perror("Error advertising discovery port");
	abort();
      }

      sockaddr_in client;
      socklen_t   len = sizeof(sockaddr_in);
      int s = ::accept(fd,(sockaddr*)&client,&len);
      if (s<0) {
        perror("XtcMonitorServer discovery accept failed");
        abort();
      }

#ifdef DBUG
      printf("Accepted connection from %x.%d on socket %d\n",
	     ntohl(client.sin_addr.s_addr),ntohs(client.sin_port),s);
      printf("Writing to %d\n",_initFd);
#endif

      //  Post connection request to taskThread
      ::write(_initFd,&s,sizeof(s));
    }
  }
}

void XtcMonitorServer::routine()
{
  while(1) {
#if 0
    printf("Enter poll with {");
    for(int i=0; i<_nfd; i++)
      printf("%d,",_pfd[i].fd);
    printf("}\n");
#endif
    if (::poll(_pfd,_nfd,-1) > 0) {

#if 0
      printf("Return poll with {");
      for(int i=0; i<_nfd; i++)
	printf("%x,",_pfd[i].revents);
      printf("}\n");
#endif

      if (_pfd[0].revents & POLLIN) 
	_initialize_client();

      //
      //  Handle events ready for distribution
      //
      if (_pfd[1].revents & POLLIN) {
        ShMsg m;
        if (mq_receive(_shuffleQueue, (char*)&m, sizeof(m), NULL) < 0)
          perror("mq_receive");

        _copyDatagram(m.dg(),_myShm+_sizeOfBuffers*m.msg().bufferIndex());
        _deleteDatagram(m.dg());

	if (m.msg().serial()) {
	  //
	  //  Send this event to the first available client
	  //
	  for(unsigned i=0; i<=_numberOfEvQueues; i++)
	    if (mq_timedsend(_myOutputEvQueue[i], (const char*)&m.msg(), sizeof(m.msg()), 0, &_tmo))
	      ; //	    printf("outputEv timedout to client %d\n",i);
	    else {
              _msgDest[m.msg().bufferIndex()]=i;
	      break;
            }
	}
	else {
	  //
	  //  Send this event to the next client around (round-robin)
	  //
	  bool lsent=false;
	  for(unsigned i=0; i<_numberOfEvQueues; i++) {
            int oc = _ievt++%_numberOfEvQueues;
	    int oq = _myOutputEvQueue[oc];
	    if (mq_timedsend(oq, (const char*)&m.msg(), sizeof(m.msg()), 0, &_tmo))
	      ;
	    else {
              _msgDest[m.msg().bufferIndex()]=oc;
	      lsent=true;
	      break;
	    }
	  }
	  if (!lsent)
	    if (mq_send(_myInputEvQueue, (const char*)&m.msg(), sizeof(m.msg()), 0))
	      perror("Unable to distribute or reclaim event");
	}
      }

      //
      //  Receive transitions back from clients
      //  or handle client disconnects
      //
      for(int i=2; i<_nfd; i++) {
	if (_pfd[i].revents & POLLIN) {
	  int r;
	  while((r=::recv(_pfd[i].fd, (char*)&_myMsg, sizeof(_myMsg), MSG_DONTWAIT))>=0) {
	    for(unsigned q=0; q<_myTrFd.size(); q++)
	      if (_myTrFd[q]==_pfd[i].fd) {
		if (r > 0) {
		  int itr=_myMsg.bufferIndex()-_numberOfEvBuffers;
		  if (_transitionCache->deallocate(itr,q))
		    _update(q,reinterpret_cast<Dgram*>(_myShm+_sizeOfBuffers*_myMsg.bufferIndex())->seq.service());
		}
		else { // retire client
		  printf("Retiring client %d [%d]\n",q,_pfd[i].fd);
                  //  Recover buffers last sent to this client
                  //  First, account for the ones waiting in our input queue
                  const timespec no_wait={0,0};
                  while(mq_timedreceive(_myInputEvQueue, (char*)&_myMsg, 
                                        sizeof(_myMsg), NULL, &no_wait)>0) {
                    mq_send(_myInputEvQueue, (char*)&_myMsg,sizeof(_myMsg),0);
                    if (_msgDest[_myMsg.bufferIndex()]>=0)
                      _msgDest[_myMsg.bufferIndex()]=-1;
                    else
                      break;
                  }
                  //  Recover the buffers still queued to the retired client
                  _moveQueue(_myOutputEvQueue[q], _myInputEvQueue);
                  //  Force recovery of those still outstanding to the retired client
                  for(int j=0; j<_msgDest.size(); j++)
                    if (_msgDest[j]==q) {
                      printf("Recovering buffer %d\n",j);
                      _myMsg.bufferIndex(j);
                      mq_send(_myInputEvQueue, (const char*)&_myMsg, sizeof(_myMsg), 0);
                    }
		  _myTrFd[q]=-1;
                  //  Clear the transition tracking for this client
		  _transitionCache->deallocate(q);
		  _nfd--;
		  for(int j=i; j<_nfd; j++)
		    _pfd[j] = _pfd[j+1];
		  i--;
		}
		break;
	      }
	  }
	}
      }
    }
  }
}

static void* DiscRoutine(void* task)
{
  XtcMonitorServer* srv = (XtcMonitorServer*)task;
  srv->discover();
  return srv;
}

static void* TaskRoutine(void* task)
{
  XtcMonitorServer* srv = (XtcMonitorServer*)task;
  srv->routine();
  return srv;
}

int XtcMonitorServer::_init() 
{ 
  const char* p = _tag;
  char* shmName    = new char[128];
  char* toQname    = new char[128];
  char* fromQname  = new char[128];

  sprintf(shmName  , "/PdsMonitorSharedMemory_%s",p);
  unsigned pageSize = (unsigned)sysconf(_SC_PAGESIZE);

  int ret = 0;
  unsigned sizeOfShm = (_numberOfEvBuffers + numberofTrBuffers) * _sizeOfBuffers;
  unsigned remainder = sizeOfShm%pageSize;
  if (remainder) sizeOfShm += pageSize - remainder;

  umask(1);  // try to enable world members to open these devices.

  int shm = shm_open(shmName, OFLAGS, PERMS);
  if (shm < 0) {ret++; perror("shm_open");}

  if ((ftruncate(shm, sizeOfShm))<0) {ret++; perror("ftruncate");}

  _myShm = (char*)mmap(NULL, sizeOfShm, PROT_READ|PROT_WRITE, MAP_SHARED, shm, 0);
  if (_myShm == MAP_FAILED) {ret++; perror("mmap");}

  _transitionCache = new TransitionCache(_myShm+_numberOfEvBuffers*_sizeOfBuffers,
					 _sizeOfBuffers);

  mq_attr q_attr;
  q_attr.mq_maxmsg  = _numberOfEvBuffers;
  q_attr.mq_msgsize = (long int)sizeof(XtcMonitorMsg);
  q_attr.mq_flags   = O_NONBLOCK;

  XtcMonitorMsg::eventOutputQueue(p,_numberOfEvQueues-1,toQname);
  _flushQueue(_myInputEvQueue  = _openQueue(toQname,q_attr));
  for(unsigned i=0; i<_numberOfEvBuffers; i++) {
    _myMsg.bufferIndex(i);
    _msgDest[i]=-1;
    mq_send(_myInputEvQueue, (const char*)&_myMsg, sizeof(_myMsg), 0);
  }

  q_attr.mq_maxmsg  = _numberOfEvBuffers / _numberOfEvQueues;
  q_attr.mq_msgsize = (long int)sizeof(XtcMonitorMsg);
  q_attr.mq_flags   = O_NONBLOCK;

  for(unsigned i=0; i<_numberOfEvQueues; i++) {
    XtcMonitorMsg::eventInputQueue(p,i,toQname);
    _flushQueue(_myOutputEvQueue[i] = _openQueue(toQname,q_attr));
  }

  { int pfd[2];
    if(::pipe(pfd)<0)
      perror("Opening pipe");
    
    _initFd         = pfd[1];
    _pfd[0].fd      = pfd[0];
    _pfd[0].events  = POLLIN;
    _pfd[0].revents = 0;
  }

  q_attr.mq_maxmsg  = _numberOfEvBuffers;
  q_attr.mq_msgsize = (long int)sizeof(ShMsg);
  q_attr.mq_flags   = O_NONBLOCK;

  sprintf(toQname, "/PdsShuffleQueue_%s",p);
  _shuffleQueue = _openQueue(toQname, q_attr);
  { ShMsg m; _flushQueue(_shuffleQueue,(char*)&m, sizeof(m)); }

  _pfd[1].fd = _shuffleQueue;
  _pfd[1].events  = POLLIN;
  _pfd[1].revents = 0;

  // create the listening threads
  pthread_create(&_taskThread,NULL,TaskRoutine,this);
  pthread_create(&_discThread,NULL,DiscRoutine,this);

  delete[] shmName;
  delete[] toQname;
  delete[] fromQname;

  return ret;
}

void XtcMonitorServer::_initialize_client()
{
#ifdef DBUG
  printf("Reading from %d\n",_pfd[0].fd);
#endif

  int s;
  if (::read(_pfd[0].fd,&s,sizeof(s))<0) {
    perror("Error reading client socket");
    abort();
  }

#ifdef DBUG
  printf("initialize client socket %d [%d]\n",s,_nfd);
#endif

  _pfd[_nfd].fd = s;
  _pfd[_nfd].events  = POLLIN;
  _pfd[_nfd].revents = 0;
  _nfd++;

  int iclient=-1;
  for(unsigned i=0; i<_myTrFd.size(); i++) {
    if (_myTrFd[i] == -1) {
      iclient = i;
      break;
    }
  }
  if (iclient == -1) {
    iclient = _myTrFd.size();
    _myTrFd.push_back(-1);
  }

  _myTrFd[iclient] = s;
  printf("_initialize_client %d [socket %d]\n",iclient,s);
  
  _myMsg.bufferIndex(iclient);
  
  if (::send(_myTrFd[iclient], (const char*)&_myMsg, sizeof(_myMsg), 0)<0) {
    perror("first send to client");
    abort();
  }

  _update(iclient,TransitionId::Unmap);
}

void XtcMonitorServer::_update(int iclient,
			       TransitionId::Value last)
{
  TransitionId::Value next = TransitionId::Value(last^1);
  std::stack<int> tr(_transitionCache->current());
  while(!tr.empty()) {
    int itr = tr.top(); tr.pop();
    int ib  = itr+_numberOfEvBuffers;
    if (reinterpret_cast<const Dgram*>(_myShm+_sizeOfBuffers*ib)->seq.service()>=next) {
      _myMsg.bufferIndex(ib);

      _transitionCache->allocate(itr,iclient);
      if (::send(_myTrFd[iclient], (const char*)&_myMsg, sizeof(_myMsg), 0)<0) 
	perror("Error sending current");
    }
  }
}

void XtcMonitorServer::_copyDatagram(Dgram* p, char* b) 
{
  Dgram* dg = (Dgram*)p;
  memcpy((char*)b, dg, sizeof(Dgram)+dg->xtc.sizeofPayload());
}

void XtcMonitorServer::_deleteDatagram(Dgram* p) {}

mqd_t XtcMonitorServer::_openQueue(const char* name, mq_attr& attr) 
{
  mqd_t q = mq_open(name,  O_CREAT|O_RDWR, PERMS, &attr);
  if (q == (mqd_t)-1) {
    perror("mq_open output");
    printf("mq_attr:\n\tmq_flags 0x%0lx\n\tmq_maxmsg 0x%0lx\n\tmq_msgsize 0x%0lx\n\t mq_curmsgs 0x%0lx\n",
        attr.mq_flags, attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs );
    fprintf(stderr, "Initializing XTC monitor server encountered an error!\n");
    delete this;
    exit(EXIT_FAILURE);
  }
  else {  // Open twice to set all of the attributes
    printf("Opened queue %s (%d)\n",name,q);
  }

  mq_attr r_attr;
  mq_getattr(q,&r_attr);
  if (r_attr.mq_maxmsg != attr.mq_maxmsg ||
      r_attr.mq_msgsize!= attr.mq_msgsize) {

    printf("Failed to set queue attributes the first time.\n");
    mq_close(q);

    mqd_t q = mq_open(name,  O_CREAT|O_RDWR, PERMS, &attr);
    mq_getattr(q,&r_attr);

    if (r_attr.mq_maxmsg != attr.mq_maxmsg ||
	r_attr.mq_msgsize!= attr.mq_msgsize) {
      printf("Failed to set queue attributes the second time.\n");
      printf("open attr  %lx %lx %lx  read attr %lx %lx %lx\n",
	     attr.mq_flags, attr.mq_maxmsg, attr.mq_msgsize,
	     r_attr.mq_flags, r_attr.mq_maxmsg, r_attr.mq_msgsize);
    }
  }

  return q;
}

void XtcMonitorServer::_flushQueue(mqd_t q)
{
  XtcMonitorMsg m; 
  _flushQueue(q,(char*)&m,sizeof(m)); 
}

void XtcMonitorServer::_flushQueue(mqd_t q, char* m, unsigned sz) 
{
  // flush the queues just to be sure they are empty.
  struct mq_attr attr;
  do {
    mq_getattr(q, &attr);
    if (attr.mq_curmsgs)
      mq_timedreceive(q, m, sz, NULL, &_tmo);
  } while (attr.mq_curmsgs);
}

void XtcMonitorServer::_moveQueue(mqd_t iq, mqd_t oq) 
{
  XtcMonitorMsg m;
  struct mq_attr attr;
  do {
    mq_getattr(iq, &attr);
    if (attr.mq_curmsgs) {
      if (mq_timedreceive(iq, (char*)&m, sizeof(m), NULL, &_tmo) == -1)
        perror("moveQueue: mq_timedreceive");
      else if (mq_send   (oq, (char*)&m, sizeof(m), 0) == -1) {
        printf("Failed to reclaim buffer %i : %s\n",
	       m.bufferIndex(), strerror(errno));
      }
      else
        _msgDest[m.bufferIndex()]=-1;
    }
  } while (attr.mq_curmsgs);
}

void XtcMonitorServer::unlink() 
{ 
  printf("Unlinking Message Queues... \n");
  mq_close(_myInputEvQueue);

  for(unsigned i=0; i<_numberOfEvQueues; i++) {
    mq_close(_myOutputEvQueue[i]);
  }
  mq_close(_shuffleQueue);
  mq_close(_discoveryQueue);

  char* qname = new char[128];
  for(unsigned i=0; i<_numberOfEvQueues; i++) {
    XtcMonitorMsg::eventInputQueue     (_tag,i,qname); mq_unlink(qname);
  }
  XtcMonitorMsg::eventInputQueue     (_tag,_numberOfEvQueues,qname); mq_unlink(qname);
  sprintf(qname, "/PdsShuffleQueue_%s",_tag);  mq_unlink(qname);
  XtcMonitorMsg::discoveryQueue      (_tag,qname); mq_unlink(qname);
  delete[] qname;
}

