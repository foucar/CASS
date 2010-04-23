// Copyright (C) 2010 Uwe Hoppe

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>
#include <QPrinter>
#include <QSpinBox>

class QScrollArea;
class QScrollBar;
#include <QLabel>

#include "ui_imageviewer.h"

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
    void on_about_triggered();

private:

    void updateActions();
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    QLabel *imageLabel;
    QScrollArea *scrollArea;
    double scaleFactor;

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

    QLineEdit *_servername;
    QSpinBox *_serverport;

    Ui::ImageViewer _ui;

};

#endif
