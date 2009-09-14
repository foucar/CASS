// Copytight (C) 2009 Jochen Küpper

#include "analyzer.h"
#include "remi_analysis.h"
#include "vmi_analysis.h"
#include "pnccd_analysis.h"

cass::Analyzer::Analyzer()
{
    //create the parameters and load them//
    _parameter[REMI] = new cass::REMI::Parameter();
    _parameter[VMI] = new cass::VMI::Parameter();
    _parameter[pnCCD] = new cass::pnCCD::Parameter();

    //create the analyzers//
    _analyzer[REMI] = new cass::REMI::Analysis(_parameter[REMI]);
    _analyzer[VMI] = new cass::VMI::Analysis(_parameter[VMI]);
    _analyzer[pnCCD] = new cass::VMI::Analysis(_parameter[pnCCD]);
}

void cass::Analyzer::processEvent(cass::CASSEvent* cassevent)
{
    //use the analyzers to analyze the event//
    //iterate through all analyzers and send the cassevent to them//
    for (std::map<Analyzers,cass::AnalysisBackend*>::iterator it=_analyzer.begin() ; it != _analyzer.end(); ++it )
        (*(it->second))(cassevent);

    //once you are done emit the done singal//
    emit nextEvent(cassevent);
}
