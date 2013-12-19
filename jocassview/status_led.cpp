// Copyright (C) 2013 Lutz Foucar

/**
 * @file status_led.cpp contains the status led class
 *
 * @author Lutz Foucar
 */

#include <QtGui/QMessageBox>
#include <QtGui/QColor>

#include "status_led.h"

using namespace jocassview;

StatusLED::StatusLED(QWidget *parent)
  : LED(parent)
{
  setDiameter(3.5);
}

void StatusLED::setStatus(int status)
{
  switch(status)
  {
  case ok:
    setColor(Qt::green);
    setState(true);
    break;
  case fail:
    setColor(Qt::red);
    setState(true);
    break;
  case off:
    setColor(Qt::white);
    setState(false);
    break;
  case buisy:
    setColor(QColor("#b0b000"));
    setState(true);
    break;
  default:
    QMessageBox::critical(this,tr("Error"),QString("StatusLED::setStatus(): Unknown dimension status"));
    break;
  }
}
