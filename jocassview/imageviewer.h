// Copyright (C) 2010 Uwe Hoppe

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>
#include <QPrinter>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QScrollArea>
#include <QScrollBar>
#include <QLabel>
#include <QTime>

#include "ui_imageviewer.h"
#include "soapCASSsoapProxy.h"


class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:

    ImageViewer(QWidget *parent = 0, Qt::WFlags flags = 0);

private slots:

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
    QComboBox *_pictureformat;
    QDoubleSpinBox *_zoom;
    QCheckBox *_running;

    QTime _time;

    Ui::ImageViewer _ui;

};

#endif
