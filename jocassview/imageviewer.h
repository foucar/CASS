// Copyright (C) 2010 Uwe Hoppe

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <string>

#include <QtCore/QTime>
#include <QtCore/QThread>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QPrinter>
#include <QtGui/QRadioButton>
#include <QtGui/QScrollArea>
#include <QtGui/QScrollBar>
#include <QtGui/QSpinBox>

#include "ui_imageviewer.h"
#include "soapCASSsoapProxy.h"

namespace jocassview
{

    class getImageThread : public QThread
    {
        Q_OBJECT

    public:

        getImageThread();

        void getImage();

    protected:

        /** Worker thread */
        void run();
    };


/** Image viewer

@todo Fit to window needs to keep the aspect ratio
@todo Separate getImage into its own thread
*/
class ImageViewer : public QMainWindow
{
    Q_OBJECT

    public:

    ImageViewer(QWidget *parent = 0, Qt::WFlags flags = 0);


private slots:

    /** Open file.
    *
    * Load Image from disc.
    */
    void on_open_triggered();

    /**
    @todo IMPORTANT: Put actual getImage into separate thread!
    @todo Use cass::imageformatName and such! */
    void on_getImage_triggered();

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

    virtual void showEvent(QShowEvent *);

    QLabel *imageLabel;

    QScrollArea *scrollArea;

    double _scaleFactor;

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

    CASSsoapProxy _cass;

    QLineEdit *_servername;

    QSpinBox *_serverport;

    QDoubleSpinBox *_rate;

    QComboBox *_picturetype;

    QDoubleSpinBox *_zoom;

    QCheckBox *_running;

    QSpinBox *_attachId;

    QRadioButton *_ristatus;

    QTime _time;

    bool _ret;

    Ui::ImageViewer _ui;

    std::string _server;
    
    getImageThread _githread;

    /** internal image update timer */
    QTimer *_updater;
};

} // end namespace jocassview

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
