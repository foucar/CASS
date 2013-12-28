// Copyright (C) 2013 Lutz Foucar

/**
 * @file zero_d_viewer.cpp contains viewer for 0d data
 *
 * @author Lutz Foucar
 */

#include <QtGui/QBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QFont>

#include "zero_d_viewer.h"

#include "histogram.h"

using namespace jocassview;

ZeroDViewer::ZeroDViewer(QString title, QWidget *parent)
  : DataViewer(title,parent)
{
  QSettings settings;
  settings.beginGroup(windowTitle());

  QVBoxLayout *layout(new QVBoxLayout);
  _value = new QLabel(tr("Number"));
  _value->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  QFont font(_value->font());
  font.setPointSize(20);
  _value->setFont(font);
  layout->addWidget(_value);
  setLayout(layout);

  resize(settings.value("WindowSize",size()).toSize());
  move(settings.value("WindowPosition",pos()).toPoint());

  show();
}

ZeroDViewer::~ZeroDViewer()
{

}

void ZeroDViewer::setData(cass::HistogramBackend *histogram)
{
  _value->setText( QString::number(dynamic_cast<cass::Histogram0DFloat*>(histogram)->getValue()));
}

cass::HistogramBackend* ZeroDViewer::data()
{
  return 0;
}

void ZeroDViewer::setData(float value)
{
  _value->setText( QString::number(value));
}
