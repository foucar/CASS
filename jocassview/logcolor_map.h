// Copyright (C) 2014 Lutz Foucar

/**
 * @file logcolor_map.h contains a logarithmic color map.
 *
 * @author Lutz Foucar
 */

#ifndef _LOGCOLORMAP_
#define _LOGCOLORMAP_

#include <qwt_color_map.h>

namespace jocassview
{
/** a logarithmic color map
 *
 * based upon code found here
 * https://stackoverflow.com/questions/7745506/qwtplotspectrogram-using-a-logarithmic-color-scale
 *
 */
class LogColorMap : public QwtLinearColorMap
{
public:
  /** constructor
   *
   * @param from the first color stop
   * @param to the last color stop
   */
  LogColorMap(const QColor &from, const QColor &to);

  /** return the rgb value for a given value
   *
   * @return the rgb value
   * @param interval
   * @param value
   */
  QRgb rgb(const QwtInterval &interval, double value) const;
};

}//end namespace jocassview
#endif
