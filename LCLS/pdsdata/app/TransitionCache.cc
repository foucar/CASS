#include "pdsdata/app/TransitionCache.hh"
#include "pdsdata/xtc/Dgram.hh"

using namespace Pds;

TransitionCache::TransitionCache(char* p, size_t sz, unsigned nbuff) : 
  _pShm(p), 
  _szShm(sz), 
  _numberofTrBuffers(nbuff),
  _not_ready(0),
  _allocated(new unsigned[nbuff])
{ 
  sem_init(&_sem, 0, 1);
  memset(_allocated, 0, _numberofTrBuffers*sizeof(unsigned)); 

  for(unsigned i=0; i<_numberofTrBuffers; i++) {
    Dgram* dg = new (p + _szShm*i) Dgram;
    dg->seq = Sequence(Sequence::Event, TransitionId::Reset,
                       ClockTime(0),
                       TimeStamp(0,0,0));
    dg->env = 0;
    _freeTr.push_back(i);
  }
}    

TransitionCache::~TransitionCache() 
{
  sem_destroy(&_sem); 
  delete[] _allocated;
}

void TransitionCache::dump() const {
  printf("---TransitionCache---\n");
  printf("\tBuffers:\n");
  for(unsigned i=0; i<_numberofTrBuffers; i++) {
    const Dgram& odg = *reinterpret_cast<const Dgram*>(_pShm + _szShm*i);
    time_t t=odg.seq.clock().seconds();
    char cbuf[64]; ctime_r(&t,cbuf); strtok(cbuf,"\n");
    printf ("%15.15s : %s : %08x\n", 
            TransitionId::name(odg.seq.service()),
            cbuf,
            _allocated[i]);
  }
  std::stack<int> cached(_cachedTr);
  printf("\tCached: ");
  while(!cached.empty()) {
    printf("%d ",cached.top());
    cached.pop();
  }
  printf("\n\tFree: ");
  for(std::list<int>::const_iterator it=_freeTr.begin();
      it!=_freeTr.end(); it++)
    printf("%d ",*it);
  printf("\n");
}

std::stack<int> TransitionCache::current() {
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

//
//  Find a free buffer for the next transition
//
int  TransitionCache::allocate  (TransitionId::Value id) {
  int result = -1;
#ifdef DBUG
  printf("allocate(%s)\n",TransitionId::name(id));
  for(unsigned i=0; i<_numberofTrBuffers; i++)
    printf("%08x ",_allocated[i]);
  printf("\n");
#endif
  bool lbegin = ((id&1)==0);
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
        else {
          printf("Unexpected state for TransitionCache: _cachedTr empty but tr[%s]!=Map\n",
                 TransitionId::name(id));
          //dump();
          //abort();
        }
      }
      else {
        const Dgram& odg = *reinterpret_cast<const Dgram*>(_pShm + _szShm*_cachedTr.top());
        TransitionId::Value oid = odg.seq.service();
        if (id == oid+2) {       // Next begin transition
          _freeTr.remove(ibuffer);
          _cachedTr.push(ibuffer);
        }
        else if (id == oid+1) {  // Matching end transition
          int ib=_cachedTr.top();
          _cachedTr.pop();
          _freeTr.push_back(ib);
        }
        else {  // unexpected transition
          printf("Unexpected transition for TransitionCache: tr[%s]!=[%s] or [%s]\n",
                 TransitionId::name(id), 
                 TransitionId::name(TransitionId::Value(oid+2)),
                 TransitionId::name(TransitionId::Value(oid+1)));
          if (lbegin) { // Begin transition
            if (id > oid) {  // Missed a begin transition leading up to it
              printf("Irrecoverable.\n");
              dump();
              abort();
            }
            else {
              printf("Recover by rolling back.\n");
              do {
                int ib=_cachedTr.top();
                _freeTr.push_back(ib);
                oid = reinterpret_cast<const Dgram*>(_pShm + _szShm*ib)->seq.service();
                _cachedTr.pop();
              } while(oid > id);
              _freeTr.remove(ibuffer);
              _cachedTr.push(ibuffer);
            }
          }
          else { // End transition
            printf("Recover by rolling back.\n");
            while( id < oid+3 ) {
              int ib=_cachedTr.top();
              _freeTr.push_back(ib);
              _cachedTr.pop();
              if (_cachedTr.empty()) break;
              oid = reinterpret_cast<const Dgram*>(_pShm + _szShm*_cachedTr.top())
                ->seq.service();
            }
          }
        }
      }

      if (lbegin) {
        unsigned not_ready=0;
        for(unsigned itr=0; itr<_numberofTrBuffers; itr++) {
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

#ifdef DBUG
      printf("not_ready %08x\n",_not_ready);
#endif
      result = ibuffer;
      break;
    }

  sem_post(&_sem);
      
  return result;
}

//
//  Queue this transition for a client
//
bool TransitionCache::allocate  (int ibuffer, unsigned client) {

  bool result = true;
#ifdef DBUG
  printf("allocate[%d,%d] not_ready %08x\n",ibuffer,client,_not_ready);
#endif

  sem_wait(&_sem);

  if (_not_ready & (1<<client)) {
    TransitionId::Value last=TransitionId::NumberOf;
    for(unsigned i=0; i<_numberofTrBuffers; i++)
      if (_allocated[i] & (1<<client)) {
        TransitionId::Value td = 
          reinterpret_cast<const Dgram*>(_pShm + _szShm*i)->seq.service();
        if ((td&1)==1 && td<last) last=td;
      }
	
    TransitionId::Value id = 
      reinterpret_cast<const Dgram*>(_pShm + _szShm*ibuffer)->seq.service();
    if (!((id&1)==1 && id<last))
      result=false;
  }

  if (result)
    _allocated[ibuffer] |= (1<<client);

  sem_post(&_sem);

#ifdef DBUG
  printf("_allocated[%d] = %08x\n",ibuffer,_allocated[ibuffer]);
#endif 
  return result;
}

//
//  Client has completed this transition.
//  Remove client from _allocated list for this buffer.
//  Return true if client was previously "not ready" but now is "ready"
bool TransitionCache::deallocate(int ibuffer, unsigned client) {
  bool result=false;
  sem_wait(&_sem);
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
    for(unsigned i=0; i<_numberofTrBuffers; i++)
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

//
//  Retire this client.
//  Remove the client from the _allocated list for all buffers.
//
void TransitionCache::deallocate(unsigned client) {
  sem_wait(&_sem);
  _not_ready &= ~(1<<client);
  for(unsigned itr=0; itr<_numberofTrBuffers; itr++)
    _allocated[itr] &= ~(1<<client);
  sem_post(&_sem);
#ifdef DBUG
  printf("deallocate %d\n",client);
  for(unsigned i=0; i<_numberofTrBuffers; i++)
    printf("%08x ",_allocated[i]);
  printf("\n");
#endif
}
