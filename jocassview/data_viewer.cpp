// Copyright (C) 2014 Lutz Foucar

/**
 * @file data_viewer.cpp contains the base class for all data viewers
 *
 * @author Lutz Foucar
 */

#include <QtCore/QSettings>

#if QT_VERSION >= 0x050000
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrinter>
#else
#include <QtGui/QPrintDialog>
#include <QtGui/QPrinter>
#endif
#include <QtGui/QCloseEvent>

#include <qwt_plot_renderer.h>
#include <qwt_plot.h>

#include "data_viewer.h"

using namespace jocassview;

DataViewer::DataViewer(QString title, QWidget *parent)
  : QMainWindow(parent),
    _plot(0)
{
  setWindowTitle(title);
  setAttribute(Qt::WA_DeleteOnClose);
}

DataViewer::~DataViewer()
{

}

void DataViewer::print()const
{
  QPrinter printer( QPrinter::HighResolution );
  printer.setDocName(windowTitle());
  printer.setCreator(windowTitle());
  printer.setOrientation( QPrinter::Landscape );

  QPrintDialog dialog(&printer);
  if ( dialog.exec() == QDialog::Accepted)
  {
    QwtPlotRenderer renderer;
    if ( printer.colorMode() == QPrinter::GrayScale )
    {
      renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground );
      renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground );
      renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame );
      renderer.setLayoutFlag( QwtPlotRenderer::FrameWithScales );
    }
    renderer.renderTo( _plot, printer );
  }
}

void DataViewer::dataChanged()
{

}

void DataViewer::closeEvent(QCloseEvent *event)
{
  emit viewerClosed(this);
  event->accept();
}

void DataViewer::moveEvent(QMoveEvent *event)
{
  QMainWindow::moveEvent(event);
  QSettings settings;
  settings.beginGroup(windowTitle());
  settings.setValue("WindowPosition",frameGeometry().topLeft());
  settings.endGroup();
}

void DataViewer::resizeEvent(QResizeEvent *event)
{
  QSettings settings;
  settings.beginGroup(windowTitle());
  settings.setValue("WindowSize",event->size());
  settings.endGroup();
}
