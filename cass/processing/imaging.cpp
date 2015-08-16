// Copyright (C) 2010-2013 Lutz Foucar

/**
 * @file imaging.cpp processors to generate a test image
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */

#include <QtCore/QString>

#include "imaging.h"

#include "cass_settings.h"
#include "log.h"


using namespace cass;
using namespace std;
using namespace std::tr1;

// *** test image ***

pp240::pp240(const name_t &name)
  : Processor(name)
{
  loadSettings(0);
}

void pp240::loadSettings(size_t)
{
  CASSSettings s;
  s.beginGroup("Processor");
  s.beginGroup(QString::fromStdString(name()));
  _result = result_t::shared_pointer(new result_t(s.value("sizeX", 1024).toInt(),
                                                  s.value("sizeY", 1024).toInt()));
  for (int xx=0; xx<s.value("sizeX", 1024).toInt(); ++xx)
    for (int yy=0; yy<s.value("sizeY", 1024).toInt(); ++yy)
      (*_result)[histogramming::bin(_result->axis(result_t::xAxis),
                                    _result->axis(result_t::yAxis),
                                    make_pair(yy,xx))] =
        s.value("FixedValue",false).toBool() ? s.value("Value",0).toFloat() : xx*yy;

  Log::add(Log::INFO,"processor " + name() +": creates test image with shape '" +
           toString(s.value("sizeX", 1024).toInt()) + "x" +
           toString(s.value("sizeY", 1024).toInt()) + "'");
}
