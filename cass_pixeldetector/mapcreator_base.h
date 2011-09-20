// Copyright (C) 2011 Lutz Foucar

/**
 * @file mapcreator_base.h contains base class for all correction map creators.
 *
 * @author Lutz Foucar
 */

#ifndef _FRAMEPROCESSORBASE_H_
#define _FRAMEPROCESSORBASE_H_

#include <tr1/memory>
#include <vector>

#include "cass_pixeldetector.h"

namespace cass
{
class CASSSettings;

namespace pixeldetector
{
//forward declaration//
struct Frame;

/** base class for all frame processors
 *
 * a frame processor will process the pixels of a frame
 *
 * @author Lutz Foucar
 */
class MapCreatorBase
{
public:
  /** typedef the shared pointer of this */
  typedef std::tr1::shared_ptr<MapCreatorBase> shared_pointer;

  /** virtual destructor */
  virtual ~MapCreatorBase() {}

  /** create an instance of the requested functor
   *
   * @return a shared pointer to the requested type
   * @param type the reqested type
   */
  static shared_pointer instance(const std::string &type);

  /** build map from frame
   *
   * take the input frame and use its data to build up the correction maps.
   *
   * @param frame the frame containing the data to build the maps from
   */
  virtual void operator() (const Frame &frame)=0;

  /** load the settings of this creator
   *
   * @param s the CASSSettings object to read the information from
   */
  virtual void loadSettings(CASSSettings &s)=0;
};

} //end namespace pixeldetector
} //end namespace cass
#endif
