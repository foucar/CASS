#include <map>
#include <string>

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ namespace cass;
#pragma link C++ namespace cass::MachineData;

#pragma link C++ class map<std::string,double>;

//#pragma link C++ nestedclasses;
 
//#pragma link C++ global gROOT;
//#pragma link C++ global gEnv;
 
//#pragma link C++ enum EMessageTypes;
#pragma link C++ defined_in "machine_event.h";

//#pragma link C++ class REMIEvent;
//#pragma link C++ class VMIEvent;
//#pragma link C++ class pnCCDEvent;
#pragma link C++ class cass::MachineData::MachineDataEvent;

#endif
