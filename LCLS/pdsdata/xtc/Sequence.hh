#ifndef PDS_SEQUENCE
#define PDS_SEQUENCE

#include "ClockTime.hh"
#include "TransitionId.hh"

namespace Pds {

  class Sequence 
  {
  public:
    enum Type    {Event = 0, Occurrence = 1, Marker = 2};
    enum         {NumberOfTypes = 3};
  public:
    Sequence() {}
    Sequence(const Sequence&);
    Sequence(const Sequence&, unsigned modify);
    Sequence(Type, TransitionId::Value, const ClockTime&, unsigned low, unsigned high);
  public:
    Type     type()         const;
    TransitionId::Value  service()      const;
    unsigned isExtended()   const;
    unsigned notEvent()     const;
    unsigned low()          const;  // 119MHz clocks since fiducial
    unsigned high()         const;  // pulseId (fiducial count)
    unsigned highAll()      const;
    const ClockTime& clock() const;
  public:
    const Sequence& operator=(const Sequence&);
    int operator==(const Sequence&)  const;
    int operator<=(const Sequence&)  const;
    int operator>=(const Sequence&)  const;
    int operator> (const Sequence&)   const;

    int isSameEvent(const Sequence&) const;
  private:

    // For performance reasons, EventPool::deque() makes an assumption about
    // the position of the type information.

    enum {v_cntrl    = 24, k_cntrl    = 8, m_cntrl    = ((1 << k_cntrl)-1)};
    enum {v_function = 24, k_function = 7, m_function = ((1 << k_function)-1)};
    enum {v_service  = 27, k_service  = 4, m_service  = ((1 << k_service)-1)};
    enum {v_seqType  = 31, k_seqType  = 0, m_seqType  = ((1 << k_seqType)-1)};
    enum {v_extended = 31, k_extended = 1, m_extended = ((1 << k_extended)-1)};
  public:
    enum {Extended = (m_extended << v_extended)};
    enum {Function = (m_function << v_function)};
    enum {EventM   = (TransitionId::L1Accept << v_service)};
    enum {Control  = (m_cntrl    << v_cntrl)};
    enum {NumFiducialBits = 17};
  private:
    ClockTime _clock;
    unsigned  _low;
    unsigned  _high;
  };
}

inline Pds::Sequence::Sequence(const Pds::Sequence& input) :
  _clock(input._clock),
  _low(input._low),
  _high(input._high)
  {
  }

inline Pds::Sequence::Sequence(const Pds::Sequence& input, unsigned modify) :
  _clock(input._clock),
  _low(input._low),
  _high(input._high | modify)
  {
  }

inline Pds::Sequence::Sequence(Type type,
			       TransitionId::Value service,
			       const ClockTime& input,
			       unsigned low, 
			       unsigned high) :
  _clock(input),
  _low(low),
  _high(((unsigned)type << v_seqType) | ((unsigned)service << v_service) | 
	(high & (unsigned)~(m_cntrl << v_cntrl)))
  {
  }

inline Pds::Sequence::Type Pds::Sequence::type() const
  {
  return (Type)((_high >> v_seqType) & (unsigned)m_seqType);
  }

inline Pds::TransitionId::Value Pds::Sequence::service() const
  {
  return (Pds::TransitionId::Value)((_high >> v_service) & (unsigned)m_service);
  }

inline unsigned Pds::Sequence::isExtended() const
  {
  return _high & (unsigned)Extended;
  }

inline unsigned Pds::Sequence::notEvent() const
  {
  return (unsigned)(_high & (unsigned)Function) ^ (unsigned)EventM;
  }

inline unsigned Pds::Sequence::low() const
  {
  return _low;
  }

inline unsigned Pds::Sequence::high() const
  {
  return _high & (unsigned)~(m_cntrl << v_cntrl);
  }

inline unsigned Pds::Sequence::highAll() const
  {
  return _high;
  }

inline const Pds::Sequence& Pds::Sequence::operator=(const Pds::Sequence& input)
  {
  _clock = input._clock;
  _low = input._low;
  _high = input._high;
  return *this;
  }

inline int Pds::Sequence::operator==(const Pds::Sequence& input) const
  {
  unsigned high_input = input._high & ~((unsigned) Extended);
  unsigned high       = _high       & ~((unsigned) Extended);

  return (high == high_input);
  }

inline int Pds::Sequence::operator>=(const Pds::Sequence& input) const
  {
  unsigned input_high = input._high & ~((unsigned) Control);
  unsigned high       = _high       & ~((unsigned) Control);

  return (high >= input_high);
  }

inline int Pds::Sequence::operator>(const Pds::Sequence& input) const
  {
  unsigned input_high = input._high & ~((unsigned) Control);
  unsigned high       = _high       & ~((unsigned) Control);

  return (high > input_high);
  }

inline int Pds::Sequence::operator<=(const Pds::Sequence& input) const
  {
  unsigned input_high = input._high & ~((unsigned) Control);
  unsigned high       = _high       & ~((unsigned) Control);

  return (high <= input_high);
  }

inline const Pds::ClockTime& Pds::Sequence::clock() const
{
  return _clock;
}

#endif
