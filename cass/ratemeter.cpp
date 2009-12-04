#include <iostream>

#include "ratemeter.h"

cass::Ratemeter::Ratemeter():
        QObject(),
        counter(0)
{
  time = new QTime();
  time->start();
}

cass::Ratemeter::~Ratemeter()
{
  delete time;
}

void cass::Ratemeter::count()
{
  ++counter;
  if (counter%10 == 0 && counter != 0)
  {
//    std::cout<< "it took  "<< time->elapsed() <<" ms to analyze 10 events"<<std::endl;
    double r = 10000. / static_cast<double>(time->elapsed());
    emit rate(r);
    time->restart();
  }
}
