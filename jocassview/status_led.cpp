// Copyright (C) 2013 Lutz Foucar

/**
 * @file status_led.cpp contains the status led class
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDebug>

#include <QtGui/QMessageBox>
#include <QtGui/QColor>

#include "status_led.h"

using namespace jocassview;

StatusLED::StatusLED(QWidget *parent)
  : LED(parent)
{
  setDiameter(3.5);
  _offTimer.setSingleShot(true);
  _offTimer.setInterval(5000);
  connect(&_offTimer,SIGNAL(timeout()),this,SLOT(turnOff()));
}

void StatusLED::setStatus(int status)
{
  switch(status)
  {
  case ok:
    qDebug()<<"LED Status ok";
    _offTimer.start();
    setColor(Qt::darkGreen);
    setState(true);
    break;
  case fail:
    qDebug()<<"LED Status fail";
    _offTimer.start();
    setColor(Qt::darkRed);
    setState(true);
    break;
  case off:
    qDebug()<<"LED Status off";
    setColor(Qt::black);
    setState(false);
    break;
  case busy:
    qDebug()<<"LED Status busy";
    setColor(QColor("#b0b000"));
    setState(true);
    break;
  default:
    QMessageBox::critical(this,tr("Error"),
                          QString("StatusLED::setStatus(): Unknown dimension status"));
    break;
  }
}

void StatusLED::turnOff()
{
  setStatus(off);
}
