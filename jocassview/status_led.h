// Copyright (C) 2013 Lutz Foucar

/**
 * @file status_led.h contains the status led class
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
  enum{ok,fail,off,busy};

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
};

}//end namespace jocassview

#endif
