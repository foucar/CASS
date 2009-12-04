//Author Lutz Foucar

#ifndef DIALOG_H
#define DIALOG_H

#include <QWidget>

class QLabel;
class QPushButton;

namespace cass
{
  class Window : public QWidget
  {
    Q_OBJECT

    public:
      Window();

    signals:
      void start();
      void load();
      void save();
      void quit();
        
    public slots:
      void updateInputRate(double);
      void updateProcessRate(double);

    private:
      QLabel      *statusLabel;
      QLabel      *inputRateLabel;
      QLabel      *processRateLabel;
      QPushButton *loadButton;
      QPushButton *saveButton;
      QPushButton *quitButton;
      QPushButton *startButton;
  };
}
#endif
