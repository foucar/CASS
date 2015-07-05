// Copyright (C) 2013 Lutz Foucar

/**
 * @file status_led.cpp contains the status led class
 *
 * @author Lutz Foucar
 */

#include <stdexcept>
#include <iostream>

#include <QtCore/QDebug>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QPushButton>
#else
#include <QtGui/QMessageBox>
#include <QtGui/QRadioButton>
#include <QtGui/QPushButton>
#endif

#include <QtGui/QColor>
#include <QtGui/QBrush>
#include <QtGui/QPalette>

#include "status_led.h"

using namespace jocassview;

StatusLED::StatusLED(QWidget *parent)
  : LED(parent)
{
  setDiameter(3.5);
  _offTimer.setSingleShot(true);
  _offTimer.setInterval(60000);
  connect(&_offTimer,SIGNAL(timeout()),this,SLOT(turnOff()));
}

void StatusLED::setStatus(int status)
{
  switch(status)
  {
  case ok:
    //qDebug()<<"LED Status ok";
    _offTimer.start();
    setColor(Qt::green);
    setState(true);
    break;
  case fail:
    //qDebug()<<"LED Status fail";
    _offTimer.start();
    setColor(Qt::red);
    setState(true);
    break;
  case off:
    //qDebug()<<"LED Status off";
    setColor(Qt::black);
    setState(false);
    break;
  case busy:
    //qDebug()<<"LED Status busy";
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
