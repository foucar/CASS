// Copyright (C) 2013 Lutz Foucar

/**
 * @file track_zoomer_2d.cpp contains zoomer for a 2d plot with tracking information
 *
 * @author Lutz Foucar
 */
#include <QtCore/QDebug>
#include <QtCore/QPoint>

#include <qwt_raster_data.h>

#include "track_zoomer_2d.h"

using namespace jocassview;

TrackZoomer2D::TrackZoomer2D(QWidget *canvas)
  : ScrollZoomer(canvas),
    _data(0)
{
  setTrackerMode(AlwaysOn);
}

QwtText TrackZoomer2D::trackerTextF(const QPointF &pos) const
{
  QString text_string;
  if (_data)
    text_string = "x:" + QString::number(pos.x()) + " , " +
        "y:" + QString::number(pos.y()) + " , " +
        "z:" + QString::number(_data->value(pos.x(),pos.y()));

  QColor bg(Qt::white);
  bg.setAlpha(200);

  QwtText text(text_string);
  text.setBackgroundBrush( QBrush( bg ));
  return text;
}

void TrackZoomer2D::setData(QwtRasterData *data)
{
  _data = data;
}
