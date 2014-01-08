// Copyright (C) 2014 Lutz Foucar

/**
 * @file zero_d_viewer_data.h contains the wrapper of the data for the 0d viewer
 *
 * @author Lutz Foucar
 */

#include <QtGui/QLabel>

#include "zero_d_viewer_data.h"

#include "histogram.h"

using namespace jocassview;
using namespace cass;

ZeroDViewerData::ZeroDViewerData(QLabel *valuedisplay)
  : _hist(0),
    _value(valuedisplay)
{

}

ZeroDViewerData::~ZeroDViewerData()
{
  delete _hist;
}

void ZeroDViewerData::setResult(HistogramBackend *result)
{
  if (!result || dynamic_cast<Histogram0DFloat*>(result) == _hist)
    return;
  delete _hist;
  _hist = dynamic_cast<Histogram0DFloat*>(result);

  _value->setText(QString::number(_hist->getValue()));
}

HistogramBackend* ZeroDViewerData::result()
{
  return _hist;
}

