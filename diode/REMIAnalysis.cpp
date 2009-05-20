/*
 *  REMIAnalysis.cpp
 *  diode
 *
 *  Created by Jochen Küpper on 20.05.09.
 *  Copyright 2009 Jochen Küpper. All rights reserved.
 *
 */

#include "REMIAnalysis.h"

using namespace std;



REMIAnalysis::REMIAnalysis(const REMIParameter& param)
{
    init(param);
}
    
void REMIAnalysis::init(const REMIParameter& param)
{
    // initialize analysis engine
}
    


vector<REMISignal> REMIAnalysis::operator()(const RawREMIData& data)
{
    vector<REMISignal> signals;
    // analyse data to return a vector of individual signals
    return signals;
}