#ifndef CASS_RATEMETER_H
#define CASS_RATEMETER_H

#include <vector>
#include <QtCore/QObject>
#include <QtCore/QTime>
#include <QtCore/QTimer>

namespace cass
{
  class Ratemeter : public QObject
  {
    Q_OBJECT;

  public:
    Ratemeter();
    ~Ratemeter();

  signals:
    void rate(double);

  private slots:
    void calculateRate();

  public slots:
    void count();

  private:
    QTime              *_time;
    QTimer             *_timer;
    std::vector<double> _counter;
    std::vector<double> _times;
    size_t              _idx;
  };
}

#endif // RATEMETER_H
