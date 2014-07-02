// Copyright (C) 2013 Lutz Foucar

/**
 * @file track_zoomer_2d.cpp contains zoomer for a 2d plot with tracking information
 *
 * @author Lutz Foucar
 */
#include <QtCore/QDebug>
#include <QtCore/QPoint>

#include <QtGui/QStatusBar>

#include "track_zoomer_2d.h"

#include "two_d_viewer_data.h"

using namespace jocassview;

TrackZoomer2D::TrackZoomer2D(QWidget *canvas)
  : ScrollZoomer(canvas),
    _data(0),
    _statusbar(0)
{
  setTrackerMode(AlwaysOn);
}

QwtText TrackZoomer2D::trackerTextF(const QPointF &pos) const
{
  QString text_string;
  if (_data)
  {
    text_string = "x:" + QString::number(pos.x()) + " , " +
        "y:" + QString::number(pos.y()) + " , " +
        "z:" + QString::number(_data->value(pos.x(),pos.y()));
    if (!qFuzzyIsNull(_wavelength_A) &&
        !qFuzzyIsNull(_cameraDistance_cm) &&
        !qFuzzyIsNull(_pixelsize_um))
    {
      const double x_cm = _pixelsize_um * 1e-4 * pos.x();
      const double y_cm = _pixelsize_um * 1e-4 * pos.y();
      const double radius_cm = sqrt(x_cm*x_cm + y_cm*y_cm);
      const double Q =
          2. / _wavelength_A * sin(0.5*atan(radius_cm/_cameraDistance_cm));
      const double d = 1. / Q;
      text_string.append(" , D:" + QString::number(d) + "A");
    }
  }

  if (_statusbar)
    _statusbar->showMessage(text_string);

  QColor bg(Qt::white);
  bg.setAlpha(200);

  QwtText text(text_string);
  text.setBackgroundBrush( QBrush( bg ));
  return text;
}

void TrackZoomer2D::setData(TwoDViewerData *data)
{
  _data = data;
}

void TrackZoomer2D::setWavelength_A(double wavelength_A)
{
  _wavelength_A = wavelength_A;
}

void TrackZoomer2D::setCameraDistance_cm(double cameradistance_cm)
{
  _cameraDistance_cm = cameradistance_cm;
}


void TrackZoomer2D::setPixelSize_um(double pixelsize_um)
{
  _pixelsize_um = pixelsize_um;
}

void TrackZoomer2D::setStatusBar(QStatusBar *statusbar)
{
  _statusbar = statusbar;
}
