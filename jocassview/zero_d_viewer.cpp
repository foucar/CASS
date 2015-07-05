// Copyright (C) 2013 Lutz Foucar

/**
 * @file zero_d_viewer.cpp contains viewer for 0d data
 *
 * @author Lutz Foucar
 */

#include <QtCore/QSettings>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QBoxLayout>
#else
#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QBoxLayout>
#endif

#include <QtGui/QFont>

#include "zero_d_viewer.h"

#include "histogram.h"
#include "zero_d_viewer_data.h"
#include "file_handler.h"

using namespace jocassview;
using namespace cass;

ZeroDViewer::ZeroDViewer(QString title, QWidget *parent)
  : DataViewer(title,parent)
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

  // generate the data that will handle the display//
  _data = new ZeroDViewerData(_value);

  // Set the size and position of the window
  resize(settings.value("WindowSize",size()).toSize());
  move(settings.value("WindowPosition",pos()).toPoint());

  settings.endGroup();
}

ZeroDViewer::~ZeroDViewer()
{

}

QList<Data*> ZeroDViewer::data()
{
  QList<Data*> dlist;
  dlist.append(_data);
  return dlist;
}

QString ZeroDViewer::type() const
{
  return QString("0DViewer");
}

void ZeroDViewer::print()const
{
  QMessageBox::critical(0,tr("ZeroDViewer"),tr("Error: Can't print 0D data"));
}

void ZeroDViewer::saveData(const QString &filename)
{
  if (data().isEmpty())
    return;
  FileHandler::saveData(filename,data().front()->result());
}

QStringList ZeroDViewer::dataFileSuffixes() const
{
  QStringList list;
  list << "h5";
  return list;
}
