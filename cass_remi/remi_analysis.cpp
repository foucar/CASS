#include "remi_analysis.h"
#include "remi_event.h"
#include "cass_event.h"

void loadSignalParameter(cass::REMI::Signal& s, const char * groupName, cass::REMI::Parameter* p)
{
    p->beginGroup(groupName);
        s.polarity(p->value("Polarity",cass::REMI::Peak::kNegative).toInt());
        s.chanNbr(p->value("ChannelNumber",0).toInt());
        s.trLow(p->value("LowerTimeRangeLimit",0.).toDouble());
        s.trHigh(p->value("UpperTimeRangeLimit",20000.).toDouble());
    p->endGroup();
}

void saveSignalParameter(const cass::REMI::Signal& s, const char * groupName, cass::REMI::Parameter* p)
{
    p->beginGroup(groupName);
        p->setValue("Polarity",static_cast<int>(s.polarity()));
        p->setValue("ChannelNumber",static_cast<int>(s.chanNbr()));
        p->setValue("LowerTimeRangeLimit",s.trLow());
        p->setValue("UpperTimeRangeLimit",s.trHigh());
    p->endGroup();
}

void loadAnodeParameter(cass::REMI::AnodeLayer& a, const char * groupName, cass::REMI::Parameter* p)
{
    p->beginGroup(groupName);
        a.tsLow(p->value("LowerTimeSumLimit",0.).toDouble());
        a.tsHigh(p->value("UpperTimeSumLimit",20000.).toDouble());
        a.sf(p->value("Scalefactor",0.5).toDouble());
        loadSignalParameter(a.one(),"One",p);
        loadSignalParameter(a.two(),"Two",p);
    p->endGroup();
}

void saveAnodeParameter(const cass::REMI::AnodeLayer& a, const char * groupName, cass::REMI::Parameter* p)
{
    p->beginGroup(groupName);
        p->setValue("LowerTimeSumLimit",a.tsLow());
        p->setValue("UpperTimeSumLimit",a.tsHigh());
        p->setValue("Scalefactor",a.sf());
        saveSignalParameter(a.one(),"One",p);
        saveSignalParameter(a.two(),"Two",p);
    p->endGroup();
}

void cass::REMI::Parameter::load()
{
    //the signal analyzer relevant stuff// 
    beginGroup("SignalAnalyzer");
        fPeakfindingMethod = value("Method",SignalAnalyzer::kCoM).toInt();
    endGroup(); //Signalanalyzer

    //the channel parameters//
    beginGroup("ChannelContainer");
        //delete the previous channel parameters//
        fChannelParameters.clear();
        for (size_t i = 0; i < value("size",16).toUInt();++i)
        {
            beginGroup(QString(static_cast<int>(i)));
                //add a channel//
                fChannelParameters.push_back(ChannelParameter());
                //load the parameters of the channel//
                fChannelParameters[i].fOffset    = value("Offset",0.).toDouble();
                fChannelParameters[i].fBacksize  = value("Backsize",30).toInt();
                fChannelParameters[i].fStepsize  = value("Stepsize",50).toInt();
                fChannelParameters[i].fThreshold = value("Threshold",50.).toDouble();
                fChannelParameters[i].fDelay     = value("Delay",5).toInt();
                fChannelParameters[i].fFraction  = value("Fraction",0.6).toDouble();
                fChannelParameters[i].fWalk      = value("Walk",0.).toDouble();
            endGroup(); //QString
        }
    endGroup();//channelparameter

    //the detektor parameters//
    beginGroup("DetectorContainer");
        //delete the previous detector parameters//
        fDetectorParameters.clear();
        for (size_t i = 0; i < value("size",1).toUInt();++i)
        {
            beginGroup(QString(static_cast<int>(i)));
                // add a detectorparameter//
                fDetectorParameters.push_back(DetectorParameter());
                //load the parameters of the detector//
                fDetectorParameters[i].fRuntime      = value("Runtime",150).toDouble();
                fDetectorParameters[i].fWLayerOffset = value("WLayerOffset",0.).toDouble();
                fDetectorParameters[i].fMcpRadius    = value("McpRadius",66.).toDouble();
                fDetectorParameters[i].fDeadMcp      = value("DeadTimeMcp",10.).toDouble();
                fDetectorParameters[i].fDeadAnode    = value("DeadTimeAnode",10.).toDouble();
                fDetectorParameters[i].fSortMethod   = value("SortingMethod",DetektorHitSorter::kSimple).toInt();
                fDetectorParameters[i].fIsHex        = value("isHex",true).toBool();
                fDetectorParameters[i].fName         = value("Name","IonDetector").toString().toStdString();
                loadSignalParameter(fDetectorParameters[i].fMcp,"McpSignal",this);
                loadAnodeParameter(fDetectorParameters[i].fULayer,"ULayer",this);
                loadAnodeParameter(fDetectorParameters[i].fVLayer,"VLayer",this);
                loadAnodeParameter(fDetectorParameters[i].fWLayer,"WLayer",this);
            endGroup(); //QString(i)
        }
    endGroup();//detectorcontainer
}

void cass::REMI::Parameter::save()
{
    //the signal analyzer relevant stuff//
    beginGroup("SignalAnalyzer");
        setValue("Method",fPeakfindingMethod);
    endGroup(); //Signalanalyzer

    //the channel parameters//
    beginGroup("ChannelContainer");
        for (size_t i = 0; i < fChannelParameters.size();++i)
        {
            beginGroup(QString(static_cast<int>(i)));
                setValue("Offset",fChannelParameters[i].fOffset);
                setValue("Backsize",fChannelParameters[i].fBacksize);
                setValue("Stepsize",fChannelParameters[i].fStepsize);
                setValue("Threshold",fChannelParameters[i].fThreshold);
                setValue("Delay",fChannelParameters[i].fDelay);
                setValue("Fraction",fChannelParameters[i].fFraction);
                setValue("Walk",fChannelParameters[i].fWalk);
            endGroup(); //QString
        }
    endGroup();//channelparameter

    //the detektor parameters//
    beginGroup("DetectorContainer");
        for (size_t i = 0; i < fDetectorParameters.size();++i)
        {
            beginGroup(QString(static_cast<int>(i)));
                setValue("Runtime",fDetectorParameters[i].fRuntime);
                setValue("WLayerOffset",fDetectorParameters[i].fWLayerOffset);
                setValue("McpRadius",fDetectorParameters[i].fMcpRadius);
                setValue("DeadTimeMcp",fDetectorParameters[i].fDeadMcp);
                setValue("DeadTimeAnode",fDetectorParameters[i].fDeadAnode);
                setValue("SortingMethod",static_cast<int>(fDetectorParameters[i].fSortMethod));
                setValue("isHex",fDetectorParameters[i].fIsHex);
                setValue("Name",fDetectorParameters[i].fName.c_str());
                loadSignalParameter(fDetectorParameters[i].fMcp,"McpSignal",this);
                loadAnodeParameter(fDetectorParameters[i].fULayer,"ULayer",this);
                loadAnodeParameter(fDetectorParameters[i].fVLayer,"VLayer",this);
                loadAnodeParameter(fDetectorParameters[i].fWLayer,"WLayer",this);
            endGroup(); //QString(i)
        }
    endGroup();//detectorcontainer
}














void cass::REMI::Analysis::loadSettings()
{
    //we need to fill the parameters with some life first//
    fParam.load();

    //intitalize the signal analyzer//
    fSiganalyzer.init(fParam.fPeakfindingMethod);
    //initialize the Detectorhit sorter for each detector//
    fSorter.init(fParam);
}

void cass::REMI::Analysis::operator()(cass::CASSEvent* cassevent)
{
    //get the remievent from the cassevent//
    cass::REMI::REMIEvent& remievent = cassevent->REMIEvent();

    //ignore event if it is not initialized//
    if (remievent.isFilled() && remievent.isInitialized())
    {

        //copy the parameters to the event//
        remievent.CopyParameters(fParam);

        //find the Signals (peaks) of all waveforms in the channels//
        fSiganalyzer.findPeaksIn(remievent);

        //extract the peaks for the layers//
        //and sort the peaks for detektor hits//
        //fill the results in the Cass Event//
        //this has to be done for each detektor individually//
        fSorter.sort(remievent);
    }
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
