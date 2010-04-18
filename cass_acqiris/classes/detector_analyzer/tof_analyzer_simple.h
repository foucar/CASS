//Copyright (C)  2010 Lutz Foucar

#ifndef __TOF_ANALYZER_SIMPLE_H_
#define __TOF_ANALYZER_SIMPLE_H_

#include "detector_analyzer_backend.h"
#include "cass_acqiris.h"
#include "tof_detector.h"

namespace cass
{
  namespace ACQIRIS
  {
    /** Simple Tof Analyzer.
     * will just take an event and feed it to the right
     * waveform analyzer
     * @note might not be needed anymore, once the list of peaks
     *       is created by the Signal itselve
     * @author Lutz Foucar
     */
    class CASS_ACQIRISSHARED_EXPORT ToFAnalyzerSimple
      : public DetectorAnalyzerBackend
    {
    public:
      /** constructor.
       * @param[in] waveformanalyzer reference to the waveformanalyzer
       *            container
       */
      ToFAnalyzerSimple(waveformanalyzers_t* waveformanalyzer)
        :DetectorAnalyzerBackend(waveformanalyzer)
      {}
      /** function that calles the right waveform analyzer*/
      virtual void operator()(DetectorBackend&,const Device&);
    };
  }
}

inline void cass::ACQIRIS::ToFAnalyzerSimple::operator ()(cass::ACQIRIS::DetectorBackend& det,const cass::ACQIRIS::Device& dev)
{
  //do a type conversion to have a delayline detector//
  TofDetector &d = dynamic_cast<TofDetector&>(detector);

  //extract the peaks for the signals of the detector from the channels//
  //check whether the requested channel does exist//
  //first retrieve the right Instruments / Channels for the signals
  Signal & MCP = d.mcp();
  const Instruments &MCPInstr = MCP.instrument();
  const size_t MCPChanNbr = MCP.channelNbr();
  const WaveformAnalyzers &MCPAnal= MCP.analyzerType();
  const Instrument::channels_t &MCPInstrChans = dev.instruments().find(MCPInstr)->second.channels();
  const Channel &MCPChan = MCPInstrChans[MCPChanNbr];

  //now extract values//
  if ((MCPChanNbr >= MCPInstrChans.size()))
  {
    std::cerr << "TofAnalzerSimple: the requested channel for mcp \""<<MCPChanNbr<<"\" is not present. We only have \""<<MCPInstrChans.size()<<"\" channels"<<std::endl;
    return;
  }
  (*(*_waveformanalyzer)[MCPAnal])(MCPChan,MCP);
}

#endif
