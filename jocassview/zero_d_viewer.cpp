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

ZeroDViewer::ZeroDViewer(QWidget *parent)
  : QWidget(parent)
{
  QVBoxLayout *layout(new QVBoxLayout);
  _value = new QLabel(tr("Number"));
  _value->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
  QFont font(_value->font());
  font.setPointSize(20);
  _value->setFont(font);
  layout->addWidget(_value);
  setLayout(layout);
}

void ZeroDViewer::setData(cass::Histogram0DFloat *histogram)
{
  _value->setText( QString::number(histogram->getValue()));
}

void ZeroDViewer::setData(float value)
{
  _value->setText( QString::number(value));
}
