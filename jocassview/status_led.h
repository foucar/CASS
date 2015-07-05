// Copyright (C) 2013 Lutz Foucar

/**
 * @file status_led.h contains the status led class
 *
 * @author Lutz Foucar
 */

#ifndef _STATUS_LED_H_
#define _STATUS_LED_H_

#include "led.h"

namespace jocassview
{

/** a LED that should display the status
 *
 * @author unknown
 * @author Lutz Foucar
 */
class StatusLED : public LED
{
  Q_OBJECT

public:
  /** enum describing the possible states of the LED */
  enum{fail,ok,off,busy};

  /** constructor
   *
   * @param parent the parent of this widget
   */
  StatusLED(QWidget *parent=0);

public slots:
  /** set the status
   *
   * @param status the status that should be reflected by this
   */
  void setStatus(int status);

private slots:
  /** turn the led of */
  void turnOff();

private:
  /** the turn off timer */
  QTimer _offTimer;
};

}//end namespace jocassview

#endif
