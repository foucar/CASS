/* hack to get control PV into hdf5 dump 
 *
 * @author Mirko Scholz
 */
#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/control/ConfigV1.hh"
#include "pdsdata/control/PVControl.hh"
#include "format_converter.h"

namespace cass 
{

  class CalibCycleIterator : public Pds::XtcIterator
  {

  public:

    enum {Stop, Continue};


    CalibCycleIterator(Xtc* root,
        unsigned int &pvNum,
        double pvControlValue[],
        std::string pvControlName[]
        ) 
      :Pds::XtcIterator(root),
      _pvNum(pvNum),
      _pvControlValue(pvControlValue),
      _pvControlName(pvControlName)
    {
      _pvControlSet = false;
    }

    /** @overload
     * function that is called for each xtc found in the xtc
     *
     * will check whether its an id or another xtc. if its another xtc it will
     * call this with increased recursion depth, otherwise it will call the
     * format converter for the id.
     */
    int process(Pds::Xtc* xtc);
  private:
    bool _pvControlSet;
    void process_ControlConfig(const Pds::ControlData::ConfigV1& config);

    unsigned int &_pvNum;
    double *_pvControlValue;
    std::string  *_pvControlName;
  };
}
