// Copyright (C) 2010 Uwe Hoppe

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <string>

#include <QtGui/QMainWindow>
#warning Fix includes
#include <QPrinter>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>
#include <QTime>

#include "ui_imageviewer.h"
#include "soapCASSsoapProxy.h"

#warning put everything in namespace jocassview
class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:

    ImageViewer(QWidget *parent = 0, Qt::WFlags flags = 0);

private slots:

    /** Open file.
      *
      * Load data from disc.
      * @todo Actually load from Disc
      * @todo Provide Control->getData for single shot data requests.
      */
    void on_open_triggered();
    void on_print_triggered();
    void on_zoomIn_triggered();
    void on_zoomOut_triggered();
    void on_normalSize_triggered();
    void on_fitToWindow_triggered();
    void on_readIni_triggered();
    void on_quitServer_triggered();
    void on_about_triggered();

    void updateServer();
    void zoomChanged(double);
    void running();

private:

    void closeEvent(QCloseEvent *event);

    void updateActions();

    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    void readIniStatusLED(int color, bool on);

    QLabel *imageLabel;
    QScrollArea *scrollArea;
    double _scaleFactor;

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

    CASSsoapProxy _cass;
    QLineEdit *_servername;
    QSpinBox *_serverport;
    QDoubleSpinBox *_period;
    QComboBox *_picturetype;
    QDoubleSpinBox *_zoom;
    QCheckBox *_running;
    QSpinBox *_attachId;
    QRadioButton *_ristatus;

    QTime _time;

    bool _ret;

    Ui::ImageViewer _ui;

    std::string _server;
};

#endif
