// Copyright (C) 2009 Jochen Küpper

#ifndef DIODE_MAINWINDOW_H
#define DIODE_MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtGui/QTextEdit>
#include "ui_main_window.h"

#include "circ_root.h"


class ToolBar;
QT_FORWARD_DECLARE_CLASS(QMenu)
QT_FORWARD_DECLARE_CLASS(QSignalMapper)


/** @class Diode MainWindow

@author Jochen Küpper
@version 0.1
*/
class MainWindow : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent=0, Qt::WindowFlags flags=0);

public slots:

    void about();

    void aboutQt();

    void quit();

    void startrOotb();
    void startrOot(int argc, char**argv);

protected slots:

    void closeEvent(QCloseEvent *event);

};

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
