// Copyright (C) 2011 Lutz Foucar

/**
 * @file mapcreators.h contains all correction map creators.
 *
 * @author Lutz Foucar
 */

#ifndef _MAPCREATORS_H_
#define _MAPCREATORS_H_

#include <tr1/memory>
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

/** Creates maps such that they will not do any correction
 *
 * @author Lutz Foucar
 */
class NonAlteringMaps : public MapCreatorBase
{
public:
  /** build map from frame
   *
   * take the input frame and use its data to build up the correction maps.
   *
   * @param frame the frame containing the data to build the maps from
   */
  void operator() (const Frame &frame);

  /** load the settings of this creator
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);
};


/** Creates maps from a fixed number of Frames
 *
 * @author Lutz Foucar
 */
class FixedMaps : public MapCreatorBase
{
public:
  /** build map from frame
   *
   * take the input frame and use its data to build up the correction maps.
   *
   * @param frame the frame containing the data to build the maps from
   */
  void operator() (const Frame &frame);

  /** load the settings of this creator
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);
};


/** Creates maps from the last number of maps
 *
 * @author Lutz Foucar
 */
class MovingMaps : public MapCreatorBase
{
public:
  /** build map from frame
   *
   * take the input frame and use its data to build up the correction maps.
   *
   * @param frame the frame containing the data to build the maps from
   */
  void operator() (const Frame &frame);

  /** load the settings of this creator
   *
   * @param s the CASSSettings object to read the information from
   */
  void loadSettings(CASSSettings &s);
};

} //end namespace pixeldetector
} //end namespace cass
#endif
