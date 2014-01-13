// Copyright (C) 2013 Lutz Foucar

/**
 * @file minmax_control.cpp contains a control over min and max values
 *
 * @author Lutz Foucar
 */

#include <QtCore/QSettings>
#include <QtCore/QDebug>

#include <QtGui/QHBoxLayout>
#include <QtGui/QCheckBox>
#include <QtGui/QDoubleValidator>
#include <QtGui/QLineEdit>
#include <QtGui/QLabel>
#include <QtGui/QToolButton>
#include <QtGui/QToolBar>

#include "minmax_control.h"

using namespace jocassview;

MinMaxControl::MinMaxControl(QString title, QToolBar *parent)
  : QWidget(parent)
{
  // set the title of this widget
  setWindowTitle(title);

  // the object to load the settings from
  QSettings settings;
  settings.beginGroup(windowTitle());

  // generate a vertical layout that will hold the controls
  QHBoxLayout *layout(new QHBoxLayout);
  layout->setContentsMargins(0,0,0,0);

  // create a button that toggles between log and linear scale
  _log = new QToolButton(this);
  _log->setIcon(QIcon(":images/log.png"));
  _log->setCheckable(true);
  _log->setChecked(settings.value("LogScale",false).toBool());
  _log->setToolTip(title + tr(": Plot the axis in logrithmic scale"));
  _log->setToolButtonStyle(parent->toolButtonStyle());
  _log->setIconSize(parent->iconSize());
  _log->setAutoRaise(true);
  connect(_log,SIGNAL(toggled(bool)),this,SLOT(on_changed()));
  layout->addWidget(_log);

  // generate the checkbox to enable manual control
  _auto = new QToolButton(this);
  _auto->setIcon(QIcon(":images/autoscale.png"));
  _auto->setCheckable(true);
  _auto->setChecked(settings.value("AutoScale",true).toBool());
  _auto->setToolTip(title + tr(": Toggle manual setting the minimum and maximum value of the scale"));
  _auto->setToolButtonStyle(parent->toolButtonStyle());
  _auto->setIconSize(parent->iconSize());
  _auto->setAutoRaise(true);
  connect(_auto,SIGNAL(toggled(bool)),this,SLOT(on_changed()));
  layout->addWidget(_auto);

  // generate a validator to ensure that only numbers are entered in the input
  QDoubleValidator *doubleValidator(new QDoubleValidator(-2e12,2e12,3,this));

  // generate the min input
//  QLabel *minlabel(new QLabel(tr("Min"),this));
//  layout->addWidget(minlabel);
  _mininput = new QLineEdit(this);
  _mininput->setValidator(doubleValidator);
  _mininput->setText(settings.value("MinValue","0").toString());
  _mininput->setMaximumWidth(120);
  _mininput->setToolTip(title + tr(": Minimum axis value"));
  connect(_mininput,SIGNAL(textChanged(QString)),this,SLOT(on_changed()));
  layout->addWidget(_mininput);

  // generate the min input
//  QLabel *maxlabel(new QLabel(tr("Max"),this));
//  layout->addWidget(maxlabel);
  _maxinput = new QLineEdit(this);
  _maxinput->setValidator(doubleValidator);
  _maxinput->setText(settings.value("MaxValue","1").toString());
  _maxinput->setMaximumWidth(120);
  _maxinput->setToolTip(title + tr(": Maximum axis value"));
  connect(_maxinput,SIGNAL(textChanged(QString)),this,SLOT(on_changed()));
  layout->addWidget(_maxinput);

  // set up the control
  on_changed();

  // set the layout of this widget
  setLayout(layout);
}

void MinMaxControl::on_changed()
{
  if(max() <= min() || (log() && (!std::isfinite(std::log10(min())) ||
                                 !std::isfinite(std::log10(max())))))
  {
    _mininput->setStyleSheet("QLineEdit {color: blue; background-color: #FF0000}");
    _maxinput->setStyleSheet("QLineEdit {color: blue; background-color: #FF0000}");
  }
  else
  {
    _mininput->setStyleSheet("QLineEdit {color: black; background-color: #FFFFFF}");
    _maxinput->setStyleSheet("QLineEdit {color: black; background-color: #FFFFFF}");
  }


  // save the states
  QSettings settings;
  settings.beginGroup(windowTitle());
  settings.setValue("LogScale",log());
  settings.setValue("AutoScale",autoscale());
  settings.setValue("MinValue",min());
  settings.setValue("MaxValue",max());

  // tell others that something has changed
  emit controls_changed();
}

bool MinMaxControl::autoscale() const
{
  return _auto->isChecked();
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
