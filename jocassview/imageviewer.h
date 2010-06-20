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
#include "cass/postprocessing/convenience_functions.h"
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
        ~getDataThread();
        std::string getMimeType(const std::string& attachId);
        void getData(const std::string& attachId, int useSpectrogram);
        void getImage(cass::ImageFormat format, const std::string& attachId);
        void getHistogram0D(const std::string& attachId);
        void getHistogram1D(const std::string& attachId);
        void setImageFormat(cass::ImageFormat format) {_format=format;};
        void updateServer(std::string server);

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
        CASSsoapProxy *_cassCmd;

        cass::ImageFormat _format;

        const std::string* _attachId;
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

    ~ImageViewer();

    /** load Data
      * loads data (image, 0d 1d 2d hist) into viewer.
      */
    void loadData(QString fileName, bool overlay);


private slots:

    /** Open file.
    *
    * Load Image from disc.
    */
    void on_open_triggered();

    /** Open file.
    *
    * Load Image from disc.
    */
    void on_overlay_data_triggered();

    /** Save Image file.
    *
    * Save Image to disc.
    */
    void on_save_data_triggered();

    /** quick/auto Save Image file.
    *
    * Save Image to disc with filename based on Date/Time.
    */
    void on_auto_save_data_triggered();

    /**
    @todo Use cass::imageformatName and such! */

    void pictureTypeChanged(int newstate);

    void on_getData_triggered();

    void on_getHistogram_triggered();

    void on_clearHistogram_triggered();

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

    cass::PostProcessors::keyList_t getIdList();

    void updateImageList(QComboBox* box);

    bool eventFilter(QObject *obj, QEvent *ev);

    /** save Image
      * save helper function
      */
    void saveImage(QString fileName);

    /** save 1D data to file
    *
    * save helper function
    *
    * @param fileName Name of file to save data to
    */
    void save1dAscii(QString fileName);

    /** save 2D data to file
    *
    * save helper function
    *
    * @param fileName Name of file to save data to
    */
    void save2dAscii(QString fileName);

    /** save Histogram
      * save helper function
      */
    void saveHistogram(QString fileName);

    virtual void showEvent(QShowEvent *);


    double _scaleFactor;

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

    QReadWriteLock _histogramlock;

    CASSsoapProxy *_cass;

    QLineEdit *_servername;

    QSpinBox *_serverport;

    QDoubleSpinBox *_rate;

    QComboBox *_picturetype;

    QDoubleSpinBox *_zoom;

    QCheckBox *_running;

    QComboBox*_attachId;

    QSize _imagesize;

    StatusLED *_statusLED;

    Ui::ImageViewer _ui;

    std::string _server;

    getDataThread _gdthread;

    /** internal image update timer */
    QTimer *_updater;

    bool _ready;

    bool _useSpectrogram;

    cass::HistogramFloatBase* _lastHist;

    const QImage* _lastImage;


protected:

    /** Rate of recent updates of displayed data */
    float _updaterate;
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
