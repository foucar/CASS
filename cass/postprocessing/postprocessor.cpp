// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#include <QtCore/QMutex>
#include <QtCore/QSettings>

#include <cassert>
#include <algorithm>
#include <iostream>

#include "acqiris_detectors.h"
#include "histogram.h"
#include "postprocessing/ccd.h"
#include "postprocessing/alignment.h"
#include "postprocessing/postprocessor.h"
#include "postprocessing/waveform.h"
#include "backend.h"


namespace cass
{


// ============define static members (do not touch)==============
PostProcessors *PostProcessors::_instance(0);
QMutex PostProcessors::_mutex;


// create an instance of the singleton
PostProcessors *PostProcessors::instance()
{
#ifdef VERBOSE
    static int n(0), create(0);
#endif
    VERBOSEOUT(std::cerr << "PostProcessors::instance -- call " << ++n << std::endl);
    QMutexLocker locker(&_mutex);
    if(0 == _instance) {
        VERBOSEOUT(std::cerr << "PostProcessors::instance -- create " << ++create << std::endl);
        _instance = new PostProcessors();
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





PostProcessors::PostProcessors()
{
    // set up list of all active postprocessors/histograms
    // and fill maps of histograms and postprocessors
    loadSettings(0);
    _IdList = new IdList();
}


void PostProcessors::process(CASSEvent& event)
{
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
    std::cout << "   Active postprocessor(s): ";
    for(std::list<id_t>::iterator iter = _active.begin(); iter != _active.end(); ++iter)
        std::cout << *iter << " ";
    setup();
}

IdList* PostProcessors::getIdList()
{
    _IdList->clear();
    _IdList->setList(_active);
    return _IdList;
}

std::string& PostProcessors::getMimeType(id_t type) {
    histograms_t::iterator it = _histograms.find(type);
    if (it!=_histograms.end())
      return it->second->mimeType();
std::cout << "PostProcessors::getMimeType id not found " << type <<std::endl;
    return _invalidMime;
}



void PostProcessors::_delete(id_t type)
{
    histograms_t::iterator iter(_histograms.find(type));
    HistogramBackend *hist(iter->second);
    _histograms.erase(iter);
    delete hist;
}



void PostProcessors::_replace(id_t type, HistogramBackend *hist)
{
    _delete(type);
    _histograms.insert(std::make_pair(type, hist));
}



void PostProcessors::setup()
{
    using namespace std;
    // check if PostProcessor is still on active list, if so load its settings
    // otherwise mark it to be deleted
    vector<id_t> delets;
    for(postprocessors_t::iterator iter = _postprocessors.begin(); iter != _postprocessors.end(); ++iter)
        if(_active.end() == find(_active.begin(), _active.end(), iter->first))
            delets.push_back(iter->first);
        else
            iter->second->loadSettings(0);

    //delete all postprocessors marked as to be deleted
    for (vector<id_t>::const_iterator it(delets.begin()); it != delets.end(); ++it)
        if (_postprocessors.end() != _postprocessors.find(*it)){
            PostprocessorBackend *pp(_postprocessors.find(*it)->second);
            delete pp;
            _postprocessors.erase(_postprocessors.find(*it));
        }

    // Add newly added PostProcessors -- for histograms we simply make sure the pointer is 0 and let
    // the postprocessor correctly initialize it whenever it wants to
    cout << "Postprocessor::setup(): add newly added postprocessors"<<endl;
    list<id_t>::iterator iter(_active.begin());
    while(iter != _active.end()) {
        cout << "Postprocessor::setup(): check that "<<*iter<<" is not implemented"<<endl;
        // check that the postprocessor is not already implemented
        if(_postprocessors.end() == _postprocessors.find(*iter)) {
            cout << "Postprocessor::setup(): did not find "<<*iter<<" in list creating it"<<endl;
            // create postprocessor
            _histograms[*iter] = 0;
            _postprocessors[*iter] = create(*iter);
            cout << "Postprocessor::setup(): done creating "<<*iter<<endl;
            // check for dependencies; if there are any open dependencies put all of them in front
            // of us
            bool update(false);
            list<id_t> deps(_postprocessors[*iter]->dependencies());
            for(list<id_t>::iterator d=deps.begin(); d!=deps.end(); ++d) {
                if(_postprocessors.end() != _postprocessors.find(*d)) {
                    _active.insert(iter, *d);
                    list<id_t>::iterator remove(find(iter, _active.end(), *d));
                    if(_active.end() != remove)
                        _active.erase(remove);
                    update = true;
                }
            }
            // if we have updated _active, start over again
            if(update) {
                // start over
                iter = _active.begin();
                continue;
            }
        }
        ++iter;
    }
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
    case PnccdFrontBinnedRunningAverage:
    case PnccdBackBinnedRunningAverage:
    case CommercialCCDBinnedRunningAverage:
    case Pnccd1BackgroundCorrectedBinnedRunnngAverage:
        processor = new pp101(*this, id);
        break;
    case VMIPhotonHits:
    case PnCCDFrontPhotonHits:
    case PnCCDBackPhotonHits:
        processor = new pp110(*this,id);
        break;
    case VMIPhotonHits1d:
    case PnCCDFrontPhotonHits1d:
    case PnCCDBackPhotonHits1d:
        processor = new pp113(*this,id);
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
