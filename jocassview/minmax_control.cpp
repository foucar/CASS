// Copyright (C) 2013 Lutz Foucar

/**
 * @file minmax_control.cpp contains a control over min and max values
 *
 * @author Lutz Foucar
 */

#include <QtCore/QSettings>

#include <QtGui/QHBoxLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QDoubleValidator>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>

#include "minmax_control.h"

using namespace jocassview;

MinMaxControl::MinMaxControl(QWidget *parent)
  : QWidget(parent)
{
  // the object to load the settings from
  QSettings settings;

  // generate a vertical layout that will hold the controls
  QHBoxLayout *layout(new QHBoxLayout);

  // creat a checkbox that toggles between log and linear scale
  _log = new QCheckBox("log",this);
  _log->setChecked(settings.value("Log",false).toBool());
  connect(_log,SIGNAL(stateChanged(int)),this,SLOT(on_changed()));
  layout->addWidget(_log);

  // generate the checkbox to enable manual control
  _manual = new QCheckBox("man control",this);
  _manual->setChecked(settings.value("Manual",false).toBool());
  connect(_manual,SIGNAL(stateChanged(int)),this,SLOT(on_changed()));
  layout->addWidget(_manual);

  // generate a validator to ensure that only numbers are entered in the input
  QDoubleValidator *doubleValidator(new QDoubleValidator(-2e12,2e12,3,this));

  // generate the min input
  QLabel *minlabel(new QLabel(tr("Min"),this));
  layout->addWidget(minlabel);
  _mininput = new QLineEdit(this);
  _mininput->setValidator(doubleValidator);
  _mininput->setText(settings.value("MinValue","0").toString());
  _mininput->setMaximumWidth(120);
  connect(_mininput,SIGNAL(textChanged(QString)),this,SLOT(on_changed()));
  layout->addWidget(_mininput);

  // generate the min input
  QLabel *maxlabel(new QLabel(tr("Max"),this));
  layout->addWidget(maxlabel);
  _maxinput = new QLineEdit(this);
  _maxinput->setValidator(doubleValidator);
  _maxinput->setText(settings.value("MaxValue","1").toString());
  _maxinput->setMaximumWidth(120);
  connect(_maxinput,SIGNAL(textChanged(QString)),this,SLOT(on_changed()));
  layout->addWidget(_maxinput);

  // set up the control
  on_changed();

  // set the layout of this widget
  setLayout(layout);
}

void MinMaxControl::on_changed()
{
  if(_maxinput->text().toDouble() < _mininput->text().toDouble())
  {
    _mininput->setStyleSheet("QLineEdit {color: blue; background-color: #FF0000}");
    _maxinput->setStyleSheet("QLineEdit {color: blue; background-color: #FF0000}");
  }
  else
  {
    _mininput->setStyleSheet("QLineEdit {color: black; background-color: #FFFFFF}");
    _maxinput->setStyleSheet("QLineEdit {color: black; background-color: #FFFFFF}");
  }

  if(_manual->isChecked())
    emit controls_changed();
}

bool MinMaxControl::manual() const
{
  return _manual->isChecked();
}

bool MinMaxControl::log() const
{
  return _log->isChecked();
}

double MinMaxControl::min() const
{
  return _mininput->text().toDouble();
}

double MinMaxControl::max() const
{
  return _maxinput->text().toDouble();
}
