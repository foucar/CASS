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

void cass::Ratemeter::nextEvent()
{
//    ++counter;
//    if (counter == 32)
//    {
       std::cout<< "it took  "<< time->elapsed() <<" ms to analyze event"<<std::endl;
//       counter = 0;
       time->restart();
//    }
}
