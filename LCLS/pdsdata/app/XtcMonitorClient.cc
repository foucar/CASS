#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#ifdef _POSIX_MESSAGE_PASSING
#include <mqueue.h>
#endif

#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/Dgram.hh"
#include "XtcMonitorClient.hh"

#include <poll.h>

namespace Pds {
  class Msg {
    public:
      Msg() {}; 
      ~Msg() {}; 
      int bufferIndex() const {return _bufferIndex;}
      int numberOfBuffers() const {return _numberOfBuffers;}
      int sizeOfBuffers() const {return _sizeOfBuffers;}
      void bufferIndex(int i) { _bufferIndex = i; }
    private:
      int _bufferIndex;
      int _numberOfBuffers;
      unsigned _sizeOfBuffers;
  };
}

using namespace Pds;

int XtcMonitorClient::processDgram(Dgram* dg) {
  printf("%s transition: time 0x%x/0x%x, payloadSize 0x%x\n",TransitionId::name(dg->seq.service()),
       dg->seq.stamp().fiducials(),dg->seq.stamp().ticks(),dg->xtc.sizeofPayload());
  return 0;
}

enum {PERMS_IN  = S_IRUSR|S_IRGRP|S_IROTH};
enum {PERMS_OUT  = S_IWUSR|S_IWGRP|S_IWOTH};
enum {OFLAGS = O_RDONLY};

static mqd_t _openQueue(const char* name, unsigned flags, unsigned perms)
{
  struct mq_attr mymq_attr;
  mqd_t queue = mq_open(name, flags, perms, &mymq_attr);
  if (queue == (mqd_t)-1) {
    char b[128];
    sprintf(b,"mq_open %s",name);
    perror(b);
    sleep(1);
  }
  else
    printf("Opened queue %s (%d)\n",name,queue);

  return queue;
}

static bool _handleDg(XtcMonitorClient& client, mqd_t iq, mqd_t oq, char*& myShm)
{
  Msg myMsg;
  unsigned priority;
  if (mq_receive(iq, (char*)&myMsg, sizeof(myMsg), &priority) < 0) {
    perror("mq_receive buffer");
    return false;
  } else {
    
    int i = myMsg.bufferIndex();
    if ( (i>=0) && (i<myMsg.numberOfBuffers()))
    {
      Dgram* dg = (Dgram*) (myShm + (myMsg.sizeOfBuffers() * i));
      int error = client.processDgram(dg);
      if (oq != -1 && mq_send(oq, (const char *)&myMsg, sizeof(myMsg), priority))
      {
        perror("mq_send back buffer");
        return false;
      }
      if (error)
        return false;
    }
    else {
      fprintf(stderr, "ILLEGAL BUFFER INDEX %d\n", i);
      return false;
    }
  }
  return true;
}

static void _flushQueue(mqd_t q)
{
  // flush the queues just to be sure they are empty.
  Msg m;
  unsigned priority;
  struct mq_attr mymq_attr;
  do {
      mq_getattr(q, &mymq_attr);
      if (mymq_attr.mq_curmsgs)
	mq_receive(q, (char*)&m, sizeof(m), &priority);
  } while (mymq_attr.mq_curmsgs);
}

int XtcMonitorClient::run(const char * tag, int index) {
  int error = 0;
  char* shmName           = new char[128];
  char* toServerEvQname   = new char[128];
  char* fromServerEvQname = new char[128];
  char* discoveryQname    = new char[128];
  char* fromServerTrQname = new char[128];

  sprintf(shmName          ,"/PdsMonitorSharedMemory_%s",tag);
  sprintf(toServerEvQname  ,"/PdsFromMonitorEvQueue_%s",tag);
  sprintf(fromServerEvQname,"/PdsToMonitorEvQueue_%s",tag);
  sprintf(discoveryQname   ,"/PdsFromMonitorDiscovery_%s",tag);
  sprintf(fromServerTrQname,"/PdsToMonitorTrQueue_%s_%d",tag,index);

  char* myShm = shmName;
  unsigned priority = 0;
  Msg myMsg;

  mqd_t myOutputEvQueue;
  do {  // make a few tries to open the first queue
    myOutputEvQueue = _openQueue(toServerEvQname, O_WRONLY, PERMS_OUT);
    if (myOutputEvQueue == (mqd_t)-1) 
      error++;
    else
      error = 0;
  } while (error && (error < 4));

  mqd_t myInputEvQueue = _openQueue(fromServerEvQname, O_RDONLY, PERMS_IN);
  if (myInputEvQueue == (mqd_t)-1)
    error++;

  mqd_t discoveryQueue = _openQueue(discoveryQname, O_WRONLY, PERMS_OUT);
  if (discoveryQueue == (mqd_t)-1)
    error++;

  mqd_t myInputTrQueue = _openQueue(fromServerTrQname, O_RDONLY, PERMS_IN);
  if (myInputTrQueue == (mqd_t)-1)
    error++;
  _flushQueue(myInputTrQueue);

  if (error) {
    fprintf(stderr, "Could not open at least one message queue!\n");
    fprintf(stderr, "FromServerEvQueue: %s %d\n", fromServerEvQname, myInputEvQueue);
    fprintf(stderr, "FromServerTrQueue: %s %d\n", fromServerTrQname, myInputTrQueue);
    fprintf(stderr, "TrServerEvQueue  : %s %d\n", toServerEvQname, myOutputEvQueue);
    fprintf(stderr, "DiscoverQueue    : %s %d\n", discoveryQname, discoveryQueue);
    return error;
  }

  //
  //  Request initialization
  //
  myMsg.bufferIndex(index);
  if (mq_send(discoveryQueue, (const char *)&myMsg, sizeof(myMsg), priority)) {
    perror("mq_send tr queue");
    error++;
  }

  //
  //  Initialize shared memory from first message
  //  Seek the Map transition
  //
  bool init=false;
  do {
    Msg myMsg;
    unsigned priority;
    if (mq_receive(myInputTrQueue, (char*)&myMsg, sizeof(myMsg), &priority) < 0) {
      perror("mq_receive buffer");
      return ++error;
    } else {
      if (!init) {
	init = true;
	unsigned sizeOfShm = myMsg.numberOfBuffers() * myMsg.sizeOfBuffers();
	unsigned pageSize  = (unsigned)sysconf(_SC_PAGESIZE);
	unsigned remainder = sizeOfShm % pageSize;
	if (remainder)
	  sizeOfShm += pageSize - remainder;

	printf("Opening shared memory %s of size 0x%x (0x%x * 0x%x)\n",
	       myShm,sizeOfShm,myMsg.numberOfBuffers(),myMsg.sizeOfBuffers());

	int shm = shm_open(myShm, OFLAGS, PERMS_IN);
	if (shm < 0) perror("shm_open");
	myShm = (char*)mmap(NULL, sizeOfShm, PROT_READ, MAP_SHARED, shm, 0);
	if (myShm == MAP_FAILED) perror("mmap");
	else printf("Shared memory at %p\n", (void*)myShm);
      }

      int i = myMsg.bufferIndex();
      if ( (i>=0) && (i<myMsg.numberOfBuffers())) {
	Dgram* dg = (Dgram*) (myShm + (myMsg.sizeOfBuffers() * i));
	if (dg->seq.service()==TransitionId::Map)
	  if (!processDgram(dg))
	    break;
      }
    }
  } while(1);

  //
  //  Handle all transitions first, then events
  //
  pollfd pfd[2];
  pfd[0].fd      = myInputTrQueue;
  pfd[0].events  = POLLIN | POLLERR;
  pfd[0].revents = 0;
  pfd[1].fd      = myInputEvQueue;
  pfd[1].events  = POLLIN | POLLERR;
  pfd[1].revents = 0;
  int nfd = 2;
    
  while (!error) {
    if (::poll(pfd, nfd, -1) > 0) {
      if (pfd[0].revents & POLLIN) { // Transition
	if (!_handleDg(*this,myInputTrQueue,-1,myShm)) error++;
      }
      else if (pfd[1].revents & POLLIN) { // Event
	if (!_handleDg(*this,myInputEvQueue,myOutputEvQueue,myShm)) error++;
      }
    }
  }
  return error;
}

