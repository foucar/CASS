/* hack to get control PV into hdf5 dump 
 *
 * @author Mirko Scholz
 */
#include <iostream>

#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/Xtc.hh"
#include "calibcycle.h"

using namespace cass;

int CalibCycleIterator::process(Pds::Xtc* xtc)
{
  //if it is another xtc, then iterate through it//
  if (xtc->contains.id() == Pds::TypeId::Id_Xtc)
    iterate(xtc);
  //otherwise use the responsible format converter for this xtc//
  else
  {
    //first check whether datagram is damaged or the xtc id is bigger than//
    //we expect when it contains an incomplete contribution stop iterating//
    //and return that it is not good//
    uint32_t damage = xtc->damage.value();
    if (xtc->contains.id() >= Pds::TypeId::NumberOf)
    {
      VERBOSEOUT(std::cout << xtc->contains.id()
          <<" is an unkown xtc id. Skipping Event"
          <<std::endl);
      return Stop;
    }
    else if (damage)
    {
      VERBOSEOUT(std::cout <<std::hex<<Pds::TypeId::name(xtc->contains.id())
          << " is damaged: 0x" <<xtc->damage.value()
          <<std::dec<<std::endl);
      if (damage & ( 0x1 << Pds::Damage::IncompleteContribution))
      {
        VERBOSEOUT(std::cout <<std::hex<<"0x"<<xtc->damage.value()
            <<" is incomplete Contribution. Skipping Event"
            <<std::dec<<std::endl);
        return Stop;
      }
    }
    else
    {
      //std::cout<<"XtcIterator::process(): found:"
      //    << Pds::TypeId::name(xtc->contains.id())<<std::endl;
      //use the converter that is good for this xtc type//
        if (xtc->contains.id() == Pds::TypeId::Id_ControlConfig) {
          process_ControlConfig(*reinterpret_cast<const Pds::ControlData::ConfigV1*>(xtc->payload()));
        }
    }
  }
  return Continue;
}

void CalibCycleIterator::process_ControlConfig(const Pds::ControlData::ConfigV1& config) 
{
  if (_pvControlSet) return;
  _pvNum = config.npvControls();
  if (_pvNum > FormatConverter::pvNumMax) throw "increase pvNumMax";
  for (unsigned int i = 0; i < _pvNum; i++)
  {
    const Pds::ControlData::PVControl& pvControlCur = config.pvControl(i);
    //std::cout << pvControlCur.name() << pvControlCur.value() << std::endl;
    _pvControlName[i] = pvControlCur.name();
    _pvControlValue[i] = pvControlCur.value();
  } 
  _pvControlSet = true;
}

