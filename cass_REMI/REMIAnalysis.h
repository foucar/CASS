// Copyright (C) 2009 Jochen Küpper


#ifndef REMIANALYSIS_H
#define REMIANALYSIS_H

#include <map>
#include <string>
#include <vector>
#include "cass_remi.h"
#include "AnalysisBackend.h"
#include "Config.h"
#include "DataDescriptor.h"
#include "Classes/SignalAnalyzer/SignalAnalyzer.h"
#include "Classes/DetektorHitSorter/DetektorHitSorter.h"

namespace cass {
namespace REMI {

/** @class REMI backend parameter sets

@author Jochen Küpper
@version 0.1
*/

class Layerproperty
{
public:
	//the properties of one layer of the delayline-anode//
	long			_polarity;			//the Polarity the Signal have
	double			_tsLow;				//lower edge of the timesum
	double			_tsHeigh;			//upper edge of the timesum
	double			_trLow;				//lower edge of the timerange events can happen in
	double			_trHigh;			//upper edge of the timerange events can happen in
	double			_sf;				//scalefactor for layer
	int				_chNbrFiEnd;		//the channel that we will find the Signals of the first end of the Anode
	int				_chNbrSeEnd;		//the channel that we will find the Signals of the second end of the Anode
};

class Detector
{
public:
	//the settings for one detector//
	double			_Runtime;			//the runtime over the anode
	double			_wOff;				//the offset of w-layer towards u and v-layer
	double			_mcpRadius;			//the radius of the MCP in mm
	double			_deadMCP;			//the Deadtime between to Signals on the MCP
	double			_deadAnode;			//the Deadtime between to Signals on the Layers
	int				_chNbrMcp;			//the Channel that we will find the mcp signals in
	Layerproperty	_uLayer;			//the properties of the U-Layer
	Layerproperty	_vLayer;			//the properties of the V-Layer
	Layerproperty	_wLayer;			//the properties of the W-Layer
	const char *	_name;				//a name for this detector (ie. Electrondetector, Iondetector)
};

class Channel
{

public:
	//the settings for one channel//
	double			_thresh;			//the threshold of the channel
	double			_offset;			//the offset
	int				_back;				//the backsize
	int				_stepsize;			//the stepsize
	int				_delay;				//the delay of the cfd
	double			_fraction;			//the fraction of the cfd
	double			_walk;				//the walk of the cfd
};


class Parameter : cass::BackendParameter 
{
public:
    //a dictionary of all user settings

    //The following entries (keys) must be present:
	std::map<Detector,int>	_detectors;	//we have the option to have 1 or 2 Detectors
	std::map<Channel,int>	_channels;	//settings to extract peaks of the channels
	int						_sortMethod;//way how peaks are sorted for detectorhits
	int						_anaMethod;	//way how peaks are identified

    //The following entries (keys) are used if available:
    //<none>

    std::map<std::string, double> _settings;
};



/** REMI data container */
class RawData 
{
public:
	const Pds::Acqiris::ConfigV1&	config()const	{return _config;}
	const Pds::Acqiris::DataDescV1&	data()const		{return _data;}

private:
     Pds::Acqiris::ConfigV1&	_config;
     Pds::Acqiris::DataDescV1&	_data;
};



/** @class REMI analysis signal

@author Jochen Küpper
@version 0.1
//*/
//class Particle {
//public:
//    double _posx, _posy, _tof;
//};



//enum HistogramType {
//    FWHM_U1, FWHM_U2, FWHM_U3
//};
//
//
//
///* @class histogram container */
//class Histogram {
//};



/** @class REMI analysis backend

@author Jochen Küpper
@version 0.1
*/
class CASS_REMISHARED_EXPORT Analysis : cass::AnalysisBackend
{
public:

	Analysis(const Parameter& param)		{init(param);}

    /** initialize AnalysisBackend with new set of parameters */
    virtual void init(const Parameter& param);

    /* analyse dataset
    @param data Raw data to be analysed
    @return analysed data
    */
	virtual operator()(const RawData& data, cass::Event &cassevent);

    /* provide analysis histogram

    Return the specified histogram from the last processed event.

    @param type Which histogram do we want
    @return histogram data
    */
    //Histogram histogram(HistogramType type);

private:
	SignalAnalyzer		siganalyzer;
	DetektorHitSorter	sorter;
	Parameter			_param;
};


} //end namespace REMI
} //end namespace CASS

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
