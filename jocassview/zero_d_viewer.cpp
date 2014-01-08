// Copyright (C) 2013 Lutz Foucar

/**
 * @file zero_d_viewer.cpp contains viewer for 0d data
 *
 * @author Lutz Foucar
 */

#include <QtCore/QSettings>

#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QFont>
#include <QtGui/QMessageBox>

#include "zero_d_viewer.h"

#include "histogram.h"

using namespace jocassview;
using namespace cass;

ZeroDViewer::ZeroDViewer(QString title, QWidget *parent)
  : DataViewer(title,parent),
    _hist(0)
{
  QSettings settings;
  settings.beginGroup(windowTitle());

  // set the label to display the value as central widget
  _value = new QLabel(tr("Number"));
  _value->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  QFont font(_value->font());
  font.setPointSize(20);
  _value->setFont(font);
  setCentralWidget(_value);

  // Set the size and position of the window
  resize(settings.value("WindowSize",size()).toSize());
  move(settings.value("WindowPosition",pos()).toPoint());
}

ZeroDViewer::~ZeroDViewer()
{
  delete _hist;
}

void ZeroDViewer::setData(HistogramBackend *histogram)
{
  if (!histogram || histogram == _hist)
    return;
  delete _hist;

  _hist = dynamic_cast<Histogram0DFloat*>(histogram);
  _value->setText( QString::number(_hist->getValue()));
}

HistogramBackend* ZeroDViewer::data()
{
  return _hist;
}

QString ZeroDViewer::type() const
{
  return QString("0DViewer");
}

void ZeroDViewer::print()const
{
  QMessageBox::critical(0,tr("ZeroDViewer"),tr("Error: Can't print 0D data"));
}
