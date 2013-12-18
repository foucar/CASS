// Copyright (C) 2013 Lutz Foucar

/**
 * @file status_led.hpp contains the main window of jocassview
 *
 * @author Lutz Foucar
 */

#ifndef _STATUS_LED_H_
#define _STATUS_LED_H_

#include <QtGui/QRadioButton>
#include <QtGui/QPushButton>
#include <QtGui/QBrush>
#include <QtGui/QColor>
#include <QtGui/QPalette>

#include <stdexcept>
#include <iostream>

namespace jocassview
{
/** a LED that should display the status
 *
 * @author unknown
 * @author Lutz Foucar
 */
class StatusLED : public QRadioButton
{
public:
  /** enum describing the possible states of the LED */
  enum{ok,bad,off};

  /** reaction to mouse press event
   *
   * overwrite the mousePressEvent() with nothing to disable the user to
   * interact with the button.
   *
   * @param event unused
   */
  void mousePressEvent(QMouseEvent */*event*/)
  {
  }

  /** set the status
   *
   * @throws invalid_argument in case the status is not one of the defined
   *
   * @param status the status that should be reflected by this
   */
  void setStatus(int status)
  {
    switch(status)
    {
    case ok:
    {
      setStyleSheet("color: green;");
      setChecked(true);
      break;
    }
    case bad:
    {
      setStyleSheet("color: red");
      setChecked(true);
      break;
    }
    case off:
      setChecked(false);
      break;
    default:
      throw std::invalid_argument("StatusLED::setStatus(): Unknown status");
    }
  }
};

}//end namespace jocassview

#endif
