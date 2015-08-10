// Copyright (C) 2014 Lutz Foucar

/**
 * @file zero_d_viewer_data.cpp contains the wrapper of the data for the 0d viewer
 *
 * @author Lutz Foucar
 */

#include <QtGlobal>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QLabel>
#else
#include <QtGui/QLabel>
#endif

#include "zero_d_viewer_data.h"

#include "result.hpp"

using namespace jocassview;
using namespace cass;

ZeroDViewerData::ZeroDViewerData(QLabel *valuedisplay)
  : _value(valuedisplay)
{

}

ZeroDViewerData::~ZeroDViewerData()
{

}

void ZeroDViewerData::setResult(Data::result_t::shared_pointer result)
{
  if (!result || result->getValue() == _result->getValue())
    return;

  _value->setText(QString::number(_result->getValue()));
}

Data::result_t::shared_pointer ZeroDViewerData::result()
{
  return _result;
}

