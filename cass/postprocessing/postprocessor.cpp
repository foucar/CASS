// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#include <QtCore/QMutex>
#include <QtCore/QSettings>

#include <cassert>
#include <algorithm>
#include <iostream>

#include "acqiris_detectors.h"
#include "histogram.h"
#include "ccd.h"
#include "alignment.h"
#include "postprocessor.h"
#include "waveform.h"
#include "hdf5dump.h"
#include "operations.h"
#include "imaging.h"
#include "machine_data.h"
#include "backend.h"


namespace cass
{


// ============define static members (do not touch)==============
PostProcessors *PostProcessors::_instance(0);
QMutex PostProcessors::_mutex;


// create an instance of the singleton
PostProcessors *PostProcessors::instance(std::string outputfilename)
{
#ifdef VERBOSE
    static int n(0), create(0);
#endif
    VERBOSEOUT(std::cerr << "PostProcessors::instance -- call " << ++n << std::endl);
    QMutexLocker locker(&_mutex);
    if(0 == _instance) {
        VERBOSEOUT(std::cerr << "PostProcessors::instance -- create " << ++create << std::endl);
        _instance = new PostProcessors(outputfilename);
    }
    return _instance;
}



// destroy the instance of the singleton
void PostProcessors::destroy()
{
    QMutexLocker locker(&_mutex);
    delete _instance;
    _instance = 0;
}
//===============================================================



/** Internal helper function to  convert QVariant to id_t */
static inline PostProcessors::id_t QVarianttoId_t(QVariant i)
{
    return PostProcessors::id_t(i.toInt());
}





PostProcessors::PostProcessors(std::string outputfilename)
  :_outputfilename(outputfilename),
   _IdList(new IdList())
{
    VERBOSEOUT(std::cout<<"Postprocessors::constructor: output Filename: "<<_outputfilename<<std::endl);
    // set up list of all active postprocessors/histograms
    // and fill maps of histograms and postprocessors
    /**
     * @todo maybe delay the call to load settings, so that in load settings
     *       one can call instance again.
     */
    loadSettings(0);
}


void PostProcessors::process(CASSEvent& event)
{
    /**
     * @todo catch when postprocessor throws an exeption and delete the
     *       postprocessor from the active list.
     *       - create a remove list with all postprocessors that depend on this
     *       - go through that list and fill all pp that depend on the ones in
     *         the list recursivly.
     *       - remove all pp that made it on the removelist
     */
    for(std::list<id_t>::iterator iter(_active.begin()); iter != _active.end(); ++iter)
        (*(_postprocessors[*iter]))(event);
}


void PostProcessors::loadSettings(size_t)
{
    VERBOSEOUT(std::cout << "Postprocessor::loadSettings" << std::endl);
    QSettings settings;
    settings.beginGroup("PostProcessor");
    QVariantList list(settings.value("active").toList());
    _active.resize(list.size());
    std::transform(list.begin(), list.end(), _active.begin(), QVarianttoId_t);
    // remove duplicates (keep first occurence)
    _active.unique();
    std::cout << "   Number of unique postprocessor activations: " << _active.size() << std::endl;
    setup();
    std::cout << "   Active postprocessor(s): ";
    for(std::list<id_t>::iterator iter = _active.begin(); iter != _active.end(); ++iter)
        std::cout << *iter << " ";
}

void PostProcessors::clear(size_t id)
{
    id_t ppid (static_cast<id_t>(id));
    try
    {
      validate(ppid);
    }
    catch (InvalidHistogramError&)
    {
      return;
    }
    histograms_checkout().find(ppid)->second->clear();
    histograms_release();
}

IdList* PostProcessors::getIdList()
{
    _IdList->clear();
    _IdList->setList(_active);
    return _IdList;
}

std::string& PostProcessors::getMimeType(id_t type)
{
    histograms_t::iterator it = _histograms.find(type);
    if (it!=_histograms.end())
        return it->second->mimeType();
    VERBOSEOUT(std::cout << "PostProcessors::getMimeType id not found " << type <<std::endl);
    return _invalidMime;
}

void PostProcessors::_delete(id_t type)
{
    histograms_t::iterator iter(_histograms.find(type));
    if (iter == _histograms.end())
      return;
    HistogramBackend *hist(iter->second);
    _histograms.erase(iter);
    delete hist;
}

void PostProcessors::_replace(id_t type, HistogramBackend *hist)
{
    _delete(type);
    hist->setId(type);
    _histograms.insert(std::make_pair(type, hist));
}

void PostProcessors::setup()
{
    using namespace std;

    // for the time beeing delete all existing postprocessors
    for(postprocessors_t::iterator iter = _postprocessors.begin(); iter != _postprocessors.end(); ++iter)
      delete iter->second;
    _postprocessors.clear();

    // Add all PostProcessors on active list -- for histograms we simply make sure the pointer is 0 and let
    // the postprocessor correctly initialize it whenever it wants to.
    // When the PostProcessor has a dependency resolve it
    VERBOSEOUT(cout << "Postprocessor::setup(): add postprocessors to list"<<endl);
    list<id_t>::iterator iter(_active.begin());
    while(iter != _active.end())
    {
        VERBOSEOUT(cout << "Postprocessor::setup(): check that "<<*iter<<" is not implemented"<<endl);
        // check that the postprocessor is not already implemented
        if(_postprocessors.end() == _postprocessors.find(*iter))
        {
            VERBOSEOUT(cout << "Postprocessor::setup(): did not find "<<*iter<<" in histogram container => creating it"<<endl);
            // create postprocessor
            _histograms[*iter] = 0;
            _postprocessors[*iter] = create(*iter);
            VERBOSEOUT(cout << "Postprocessor::setup(): done creating "<<*iter<<" Now checking its dependecies."<<endl);
            // check for dependencies; if there are any open dependencies put all of them in front
            // of us
            bool update(false);
            list<id_t> deps(_postprocessors[*iter]->dependencies());
            for(list<id_t>::iterator d=deps.begin(); d!=deps.end(); ++d)
            {
                VERBOSEOUT(cout << "Postprocessor::setup(): "<<*iter<<" depends on "<<*d
                           <<" checking whether dependecy is already there"<<endl);
                if(_postprocessors.end() == _postprocessors.find(*d))
                {
                    VERBOSEOUT(cout << "Postprocessor::setup(): "<<*d<<" is not in postprocessor container"  <<" inserting it into the active list before "<<*iter<<endl);
                    _active.insert(iter, *d);
                    list<id_t>::iterator remove(find(iter, _active.end(), *d));
                    VERBOSEOUT(cout << "Postprocessor::setup(): check whether dependency "<<*d
                               <<" was on the active list, but not at the right position"<<endl);
                    if(_active.end() != remove)
                    {
                         VERBOSEOUT(cout << "Postprocessor::setup(): dependency "<<*d <<" was on list"
                                    <<" removing the later double entry."<<endl);
                        _active.erase(remove);
                    }
                    update = true;
                }
            }
            // if we have updated _active, start over again
            if(update) {
                // start over
                VERBOSEOUT(cout << "Postprocessor::setup(): start over again."<<endl);
                iter = _active.begin();
                continue;
            }
        }
        ++iter;
    }

    for(postprocessors_t::iterator iter = _postprocessors.begin(); iter != _postprocessors.end(); ++iter)
    {
      VERBOSEOUT(std::cout << "PostProcessor::setup(): 2nd loading: load settings of "<< iter->first<<std::endl);
      iter->second->loadSettings(0);
    }

#ifdef VERBOSE
    VERBOSEOUT( cout << "active postprocessors processing order: ");
    for(std::list<id_t>::iterator iter(_active.begin()); iter != _active.end(); ++iter)
        VERBOSEOUT(cout << *iter << ", ");
    VERBOSEOUT( cout << endl);
#endif
}


PostprocessorBackend * PostProcessors::create(id_t id)
{
    PostprocessorBackend * processor(0);
    switch(id) {
    case Pnccd1LastImage:
    case Pnccd2LastImage:
    case VmiCcdLastImage:
        processor = new pp1(*this, id);
        break;
    case FirstPnccdFrontBinnedConditionalRunningAverage:
    case SecondPnccdFrontBinnedConditionalRunningAverage:
    case FirstPnccdBackBinnedConditionalRunningAverage:
    case SecondPnccdBackBinnedConditionalRunningAverage:
    case FirstCommercialCCDBinnedConditionalRunningAverage:
    case SecondCommercialCCDBinnedConditionalRunningAverage:
        processor = new pp101(*this, id);
        break;
    case FirstImageSubstraction:
    case SecondImageSubstraction:
        processor = new pp106(*this, id);
        break;
    case VMIPhotonHits:
    case VMIPhotonHitsTwo:
    case PnCCDFrontPhotonHits:
    case PnCCDBackPhotonHits:
        processor = new pp110(*this,id);
        break;
    case VMIPhotonHits1d:
    case PnCCDFrontPhotonHits1d:
    case PnCCDBackPhotonHits1d:
        processor = new pp113(*this,id);
        break;
    case VMIPhotonHitseV1d:
    case PnCCDFrontPhotonHitseV1d:
    case PnCCDBackPhotonHitseV1d:
        processor = new pp116(*this,id);
        break;
    case CampChannel00LastWaveform:
    case CampChannel01LastWaveform:
    case CampChannel02LastWaveform:
    case CampChannel03LastWaveform:
    case CampChannel04LastWaveform:
    case CampChannel05LastWaveform:
    case CampChannel06LastWaveform:
    case CampChannel07LastWaveform:
    case CampChannel08LastWaveform:
    case CampChannel09LastWaveform:
    case CampChannel10LastWaveform:
    case CampChannel11LastWaveform:
    case CampChannel12LastWaveform:
    case CampChannel13LastWaveform:
    case CampChannel14LastWaveform:
    case CampChannel15LastWaveform:
    case CampChannel16LastWaveform:
    case CampChannel17LastWaveform:
    case CampChannel18LastWaveform:
    case CampChannel19LastWaveform:
    case ITofChannel00LastWaveform:
    case ITofChannel01LastWaveform:
    case ITofChannel02LastWaveform:
    case ITofChannel03LastWaveform:
        processor = new pp4(*this,id);
        break;
    case CampChannel00AveragedWaveform:
    case CampChannel01AveragedWaveform:
    case CampChannel02AveragedWaveform:
    case CampChannel03AveragedWaveform:
    case CampChannel04AveragedWaveform:
    case CampChannel05AveragedWaveform:
    case CampChannel06AveragedWaveform:
    case CampChannel07AveragedWaveform:
    case CampChannel08AveragedWaveform:
    case CampChannel09AveragedWaveform:
    case CampChannel10AveragedWaveform:
    case CampChannel11AveragedWaveform:
    case CampChannel12AveragedWaveform:
    case CampChannel13AveragedWaveform:
    case CampChannel14AveragedWaveform:
    case CampChannel15AveragedWaveform:
    case CampChannel16AveragedWaveform:
    case CampChannel17AveragedWaveform:
    case CampChannel18AveragedWaveform:
    case CampChannel19AveragedWaveform:
    case ITofChannel00AveragedWaveform:
    case ITofChannel01AveragedWaveform:
    case ITofChannel02AveragedWaveform:
    case ITofChannel03AveragedWaveform:
        processor = new pp500(*this,id);
        break;
    case HexMCPNbrSignals:
    case QuadMCPNbrSignals:
    case VMIMcpNbrSignals:
    case FELBeamMonitorNbrSignals:
    case YAGPhotodiodeNbrSignals:
    case FsPhotodiodeNbrSignals:
        processor = new pp550(*this, id);
        break;
    case HexU1NbrSignals:
    case HexU2NbrSignals:
    case HexV1NbrSignals:
    case HexV2NbrSignals:
    case HexW1NbrSignals:
    case HexW2NbrSignals:
    case QuadX1NbrSignals:
    case QuadX2NbrSignals:
    case QuadY1NbrSignals:
    case QuadY2NbrSignals:
        processor = new pp551(*this, id);
        break;
    case HexU1U2Ratio:
    case HexV1V2Ratio:
    case HexW1W2Ratio:
    case QuadX1X2Ratio:
    case QuadY1Y2Ratio:
        processor = new pp557(*this, id);
        break;
    case HexU1McpRatio:
    case HexU2McpRatio:
    case HexV1McpRatio:
    case HexV2McpRatio:
    case HexW1McpRatio:
    case HexW2McpRatio:
    case QuadX1McpRatio:
    case QuadX2McpRatio:
    case QuadY1McpRatio:
    case QuadY2McpRatio:
        processor = new pp558(*this, id);
        break;
    case HexRekMcpRatio:
    case QuadRekMcpRatio:
        processor = new pp566(*this, id);
        break;
    case HexAllMcp:
    case QuadAllMcp:
    case VMIMcpAllMcp:
    case FELBeamMonitorAllMcp:
    case YAGPhotodiodeAllMcp:
    case FsPhotodiodeAllMcp:
        processor = new pp567(*this, id);
        break;
    case HexTimesumU:
    case HexTimesumV:
    case HexTimesumW:
    case QuadTimesumX:
    case QuadTimesumY:
        processor = new pp568(*this, id);
        break;
    case HexTimesumUvsU:
    case HexTimesumVvsV:
    case HexTimesumWvsW:
    case QuadTimesumXvsX:
    case QuadTimesumYvsY:
        processor = new pp571(*this, id);
        break;
    case HexFirstUV:
    case HexFirstUW:
    case HexFirstVW:
    case QuadFirstXY:
        processor = new pp574(*this, id);
        break;
    case HexXY:
    case HexXT:
    case HexYT:
    case QuadXY:
    case QuadXT:
    case QuadYT:
        processor = new pp578(*this, id);
        break;
    case HexHeightvsFwhmMcp:
    case QuadHeightvsFwhmMcp:
    case VMIMcpHeightvsFwhmMcp:
    case FELBeamMonitorHeightvsFwhmMcp:
    case YAGPhotodiodeHeightvsFwhmMcp:
    case FsPhotodiodeHeightvsFwhmMcp:
        processor = new pp581(*this, id);
        break;
    case HexHeightvsFwhmU1:
    case HexHeightvsFwhmU2:
    case HexHeightvsFwhmV1:
    case HexHeightvsFwhmV2:
    case HexHeightvsFwhmW1:
    case HexHeightvsFwhmW2:
    case QuadHeightvsFwhmX1:
    case QuadHeightvsFwhmX2:
    case QuadHeightvsFwhmY1:
    case QuadHeightvsFwhmY2:
        processor = new pp582(*this, id);
        break;
    case HexPIPICO:
    case HexQuadPIPICO:
        processor = new pp700(*this,id);
        break;
    case VmiFixedCos2Theta:
        processor = new pp150(*this,id);
        break;
    case AdvancedPhotonFinderFrontPnCCD:
    case AdvancedPhotonFinderFrontPnCCDTwo:
    case AdvancedPhotonFinderBackPnCCD:
    case AdvancedPhotonFinderBackPnCCDTwo:
    case AdvancedPhotonFinderCommercialCCD:
    case AdvancedPhotonFinderCommercialCCDTwo:
        processor = new pp160(*this,id);
        break;
    case AdvancedPhotonFinderFrontPnCCD1dHist:
    case AdvancedPhotonFinderFrontPnCCDTwo1dHist:
    case AdvancedPhotonFinderBackPnCCD1dHist:
    case AdvancedPhotonFinderBackPnCCDTwo1dHist:
    case AdvancedPhotonFinderCommercialCCD1dHist:
    case AdvancedPhotonFinderCommercialCCDTwo1dHist:
        processor = new pp166(*this,id);
        break;
    case PhotonEnergy:
        processor = new pp852(*this,id);
        break;
    case Project2d:
        processor = new pp806(*this,id);
        break;
    case Project2rho:
        processor = new pp807(*this,id);
        break;
    case Project2phi:
        processor = new pp808(*this,id);
        break;

#ifdef HDF5
    case PnccdHDF5:
        processor = new pp1001(*this,id);
        break;
#endif

#ifdef CERNROOT
    case ROOTDump:
        processor = new pp2000(*this,id,_outputfilename);
        break;
#endif

    default:
        throw std::invalid_argument(QString("Postprocessor %1 not available").arg(id).toStdString());
    }
    return processor;
}

} // end namespace cass



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
