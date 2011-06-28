#include "map"
#include "string"
#include "vector"
#ifdef __CINT__
#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedefs;
#pragma link C++ class map<string,vector<map<string,double> > >+;
#pragma link C++ class map<string,vector<map<string,double> > >::*;
#pragma link C++ operators map<string,vector<map<string,double> > >::iterator;
#pragma link C++ operators map<string,vector<map<string,double> > >::const_iterator;
#pragma link C++ operators map<string,vector<map<string,double> > >::reverse_iterator;
#endif
