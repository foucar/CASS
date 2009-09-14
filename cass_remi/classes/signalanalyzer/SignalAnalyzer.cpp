#include <iostream>
#include "SignalAnalyzer.h"

#include "SoftTDCCFD.h"
#include "SoftTDCCoM.h"


//_________________________________________________________________________________
cass::REMI::SignalAnalyzer::~SignalAnalyzer()
{
    //std::cout << "delete signalanalyzer"<<std::endl;
    delete fStdc;
    //std::cout << "done"<<std::endl;
}

//_________________________________________________________________________________
void cass::REMI::SignalAnalyzer::init(int analyzeMethod)
{
    //create an Analyzer according to the method chosen//
    delete fStdc;
    switch (analyzeMethod)
    {
    case(kCoM):         fStdc = new SoftTDCCoM16Bit(); break;
    case(kCfd):         fStdc = new SoftTDCCFD16Bit(); break;
    case(kDoNothing):   fStdc = new SoftTDCDoNothing();break;

    default: std::cerr << "No Analysis Method "<<analyzeMethod<<" found"<<std::endl; break;
    }
    fMethod = analyzeMethod;
}
