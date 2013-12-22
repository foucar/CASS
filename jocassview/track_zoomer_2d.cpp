// Copyright (C) 2013 Lutz Foucar

/**
 * @file track_zoomer_2d.cpp contains zoomer for a 2d plot with tracking information
 *
 * @author Lutz Foucar
 */

#include <qwt_raster_data.h>

#include "track_zoomer_2d.h"

using namespace jocassview;

TrackZoomer2d::TrackZoomer2d(QwtPlotCanvas *canvas)
  : ScrollZoomer(canvas),
    _data(0)
{

}

QwtText TrackZoomer2d::trackerText(const QwtDoublePoint & pos) const
{
    QColor bg(Qt::white);
    bg.setAlpha(200);

    QwtText text = QwtPlotZoomer::trackerText(pos);
    QString text_string(text.text());
    if (_data)
      text_string = text_string + " : " + QString::number( _data->value(pos.x(), pos.y()) );
    text.setText(text_string);
    text.setBackgroundBrush( QBrush( bg ));
    return text;

}

void TrackZoomer2d::setData(QwtRasterData *data)
{
  _data = data;
}
