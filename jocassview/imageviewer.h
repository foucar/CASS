// Copyright (C) 2010 Uwe Hoppe

#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <string>

#include <QtCore/QTimer>
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

#include "cass/cass.h"
#include "cass/histogram.h"
#include "ui_imageviewer.h"
#include "soapCASSsoapProxy.h"
#include "plotwidget.h"

namespace jocassview
{

    class ImageLabel : public QLabel
    {
        Q_OBJECT

    protected:

        void mouseMoveEvent(QMouseEvent *event);

    signals:

        void newCursorPosition(int, int);
    };


    class getDataThread : public QThread
    {
        Q_OBJECT

    public:

        getDataThread();
        void setSoap(CASSsoapProxy* cassSoap);
        cass::PostProcessors::active_t getIdList(CASSsoapProxy *cass);
        std::string getMimeType(CASSsoapProxy *cass, int attachId);
        void getData(CASSsoapProxy *cass, int attachId, int useSpectrogram);
        void getImage(CASSsoapProxy *cass, cass::ImageFormat format, int attachId);
        void getHistogram0D(CASSsoapProxy *cass, int attachId);
        void getHistogram1D(CASSsoapProxy *cass, int attachId);
        void setImageFormat(cass::ImageFormat format) {_format=format;};

    signals:

        void newImage(const QImage *image);
        void newHistogram(cass::Histogram2DFloat*);
        void newHistogram(cass::Histogram1DFloat*);
        void newHistogram(cass::Histogram0DFloat*);
        void newNone();

    protected:

        /** Worker thread */
        void run();

        enum dataType {dat_Image=0, dat_Any, dat_2DHistogram, dat_1DHistogram, dat_0DHistogram, dat_COUNT};
        dataType _dataType;

        CASSsoapProxy *_cass;

        cass::ImageFormat _format;

        int _attachId;

        int _useSpectrogram;

    };


    class StatusLED : public QRadioButton
    {
    public:

        void setStatus(bool on, QColor color=Qt::red)
        {
            setChecked(on);
            if(!on)
                return;
            QPalette palette;
            QBrush brush(color, Qt::SolidPattern);
            palette.setBrush(QPalette::Active, QPalette::Text, brush);
            setPalette(palette);
        }
    };


/** Image viewer

@todo Fit to window needs to keep the aspect ratio
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

    /** Save Image file.
    *
    * Save Image to disc.
    */
    void on_save_image_triggered();

    /** quick/auto Save Image file.
    *
    * Save Image to disc with filename based on Date/Time.
    */
    void on_auto_save_image_triggered();

    /**
    @todo Use cass::imageformatName and such! */

    void useSpectrogram_stateChanged(int newstate);

    void on_getData_triggered();

    void on_getHistogram_triggered();

    void on_print_triggered();

    void on_zoomIn_triggered();

    void on_zoomOut_triggered();

    void on_normalSize_triggered();

    void on_fitToWindow_triggered();

    void on_readIni_triggered();

    void on_writeIni_triggered();

    void on_quitServer_triggered();

    void on_about_triggered();

    void updateServer();

    void updateStatusBar(int, int);

    void zoomChanged(double);

    void running();

    void updateNone();

    void updatePixmap(const QImage *image);

    void updateHistogram(cass::Histogram2DFloat* hist);
    
    void updateHistogram(cass::Histogram1DFloat* hist);

    void updateHistogram(cass::Histogram0DFloat* hist);


private:

    QDockWidget* _dock;

    QScrollArea* _imageScroller;

    QWidget* _imageWidget;

    QVBoxLayout* _imageLayout;

    QHBoxLayout* _imageValuesLayout;

    QComboBox* _imageMapping;

    QLineEdit* _imageMinValue;

    QLineEdit* _imageMaxValue;

    QLabel* _imageMinLabel;

    QLabel* _imageMaxLabel;

    ImageLabel *_imageLabel;

    plotWidget1D* _plotWidget1D;
    plotWidget0D* _plotWidget0D;
    spectrogramWidget* _spectrogramWidget;

    void closeEvent(QCloseEvent *event);

    virtual void resizeEvent(QResizeEvent *event);

    void updateActions();

    void scaleImage(double factor);

    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    void updateImageList(QComboBox* box);

    /** save Image
      * save helper function
      */
    void saveImage(QString fileName);

    virtual void showEvent(QShowEvent *);


    double _scaleFactor;

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

    CASSsoapProxy *_cass;

    QLineEdit *_servername;

    QSpinBox *_serverport;

    QDoubleSpinBox *_rate;

    QComboBox *_picturetype;

    QDoubleSpinBox *_zoom;

    QCheckBox *_running;

    QCheckBox *_chk_spectrogram;

    QComboBox*_attachId;

    QSize _imagesize;

    StatusLED *_statusLED;

    Ui::ImageViewer _ui;

    std::string _server;

    getDataThread _gdthread;

    /** internal image update timer */
    QTimer *_updater;

    bool _ready;

    cass::HistogramFloatBase* _lastHist;

    const QImage* _lastImage;

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
