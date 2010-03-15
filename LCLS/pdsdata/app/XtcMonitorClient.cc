#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#if (_POSIX_MESSAGE_PASSING > 0)
#include <mqueue.h>
#endif

#include "pdsdata/xtc/DetInfo.hh"
#include "pdsdata/xtc/ProcInfo.hh"
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/Dgram.hh"
#include "XtcMonitorClient.hh"

namespace Pds
{
  class Msg 
  {
  public:
    Msg() {}
    ~Msg() {}
    int bufferIndex()     {return _bufferIndex;}
    int numberOfBuffers() {return _numberOfBuffers;}
    int sizeOfBuffers()   {return _sizeOfBuffers;}
  private:
    int _bufferIndex;
    int _numberOfBuffers;
    unsigned _sizeOfBuffers;
  };
}

using namespace Pds;

int XtcMonitorClient::processDgram(Dgram* dg)
{
  printf("%s transition: time 0x%x/0x%x, payloadSize 0x%x\n",TransitionId::name(dg->seq.service()),
         dg->seq.stamp().fiducials(),dg->seq.stamp().ticks(),dg->xtc.sizeofPayload());
  return 0;
}

int XtcMonitorClient::run(char * tag)
{
  int error = 0;
  unsigned sizeOfShm = 0;
  char toServerQname[128] = "/PdsFromMonitorMsgQueue_";
  char fromServerQname[128] = "/PdsToMonitorMsgQueue_";
  char shmName[128] = "/PdsMonitorSharedMemory_";
  strcat(shmName, tag);
  strcat(toServerQname, tag);
  strcat(fromServerQname, tag);
  char* myShm = NULL;
  unsigned pageSize = (unsigned)sysconf(_SC_PAGESIZE);
  unsigned priority = 0;
  struct mq_attr mymq_attr;
  Dgram* dg = NULL;
  Msg myMsg;
  enum {PERMS_IN  = S_IRUSR|S_IRGRP|S_IROTH};
  enum {PERMS_OUT  = S_IWUSR|S_IWGRP|S_IWOTH};
  enum {OFLAGS = O_RDONLY};

  mqd_t myOutputQueue;
  do
  {  // make a few tries to open the first queue
    myOutputQueue = mq_open(toServerQname, O_WRONLY, PERMS_OUT, &mymq_attr);
    if (myOutputQueue == (mqd_t)-1)
    {
      perror("mq_open output");
      error+=1 ;
      sleep(1);
    }
    else error = 0;
  } while (error && (error < 4));

  mqd_t myInputQueue = mq_open(fromServerQname, O_RDONLY, PERMS_IN, &mymq_attr);
  if (myInputQueue == (mqd_t)-1)
  {
    perror("mq_open input");
    error+=1;
    sleep(1);
  };

  if (error)
  {
    fprintf(stderr, "Could not open at least one message queue!\n");
    fprintf(stderr, "To Server:%d, From Server:%d\n", myOutputQueue, myInputQueue);
    fprintf(stderr, "To Server:%s, From Server:%s\n", toServerQname, fromServerQname);
    error++;
  }

  while (!error) //eventloop
  {
    if (mq_receive(myInputQueue, (char*)&myMsg, sizeof(myMsg), &priority) < 0)
    {
      perror("mq_receive buffer");
      error++;
    }
    else
    {
      if (!sizeOfShm) //is this the first time? -> initialize the shared memory
      {
        sizeOfShm = myMsg.numberOfBuffers() * myMsg.sizeOfBuffers();
        unsigned remainder = sizeOfShm % pageSize;
        if (remainder)
        {
          printf("sizeOfShm changed from 0x%x", sizeOfShm);
          sizeOfShm += pageSize - remainder;
          printf(" to 0x%x\n", sizeOfShm);
        }
        int shm = shm_open(shmName, OFLAGS, PERMS_IN);
        if (shm < 0) perror("shm_open");
        myShm = (char*)mmap(NULL, sizeOfShm, PROT_READ, MAP_SHARED, shm, 0);
        if (myShm == MAP_FAILED) perror("mmap");
        else printf("Shared memory at %p\n", (void*)myShm);
      }
      int i = myMsg.bufferIndex();
      if ( (i>=0) && (i<myMsg.numberOfBuffers())) {
	dg = (Dgram*) (myShm + (myMsg.sizeOfBuffers() * i));
	error += this->processDgram(dg);
	if (mq_send(myOutputQueue, (const char *)&myMsg, sizeof(myMsg), priority)) {
	  perror("mq_send back buffer");
	  error++;
	}
      } else {
        fprintf(stderr, "ILLEGAL BUFFER INDEX %d\n", i);
	error++;
      }
    }
  }
  return error;
}
