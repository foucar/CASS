#ifndef CASS_RATEMETER_H
#define CASS_RATEMETER_H

#include <QtCore/QObject>
#include <QtCore/QTime>

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

    public slots:
      void count();

    private:
      QTime* time;
      size_t counter;
  };
}

#endif // RATEMETER_H
