// Copyright (C) 2012 Lutz Foucar

/**
 * @file mapcreators_online.h contains correction map creators that work fast
 *                            easy for online purposes.
 *
 * @author Lutz Foucar
 */

#ifndef _MAPCREATORSONLINE_H_
#define _MAPCREATORSONLINE_H_

#include <tr1/memory>
#include <tr1/functional>
#include <vector>

#include "mapcreator_base.h"
#include "cass_pixeldetector.h"

namespace cass
{
class CASSSettings;

namespace pixeldetector
{
//forward declaration//
struct Frame;
class CommonData;

/** Creates the maps fast and simple
 *
 * details
 *
 * @author Lutz Foucar
 */
class OnlineFixedCreator : public MapCreatorBase
{
public:
  /** the operator
   *
   * details
   *
   * @param frame the frame to check for
   */
  void operator() (const Frame &/*frame*/);

  /** load the settings of this creator
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);

private:
  /** the special storage type of this class */
  typedef std::vector< std::vector<pixel_t>  > specialstorage_t;

  /** a function that just returns and does nothing
   *
   * @param unused not used
   * @param unused not used
   */
  void doNothing(const Frame& /*unused*/) {}

  /** build up storage and then calculate the maps
   *
   * @param frame the frame to build up the storage and to calc the maps from
   */
  void buildAndCalc(const Frame& frame);

  /** the container with all the maps */
  std::tr1::shared_ptr<CommonData> _commondata;

  /** the function object that will be called by the operator */
  std::tr1::function<void(const Frame&)> _createMap;

  /** storage where the pixels are already ordered */
  specialstorage_t _specialstorage;

  /** how many frames should be collected before the maps are calculated */
  size_t _nbrFrames;

  /** the multiplier to define the max noise before the pixel is considered to contain a photon */
  pixel_t _multiplier;

  /** flag wether the create maps should be saved to file or just used */
  bool _writeMaps;
};


} //end namespace pixeldetector
} //end namespace cass
#endif
