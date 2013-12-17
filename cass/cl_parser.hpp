//Copyright (C) 2013 Lutz Foucar

/**
 * @file statistics_calculator.hpp contains declarations of statistic calculators
 *
 * @author Lutz Foucar
 */

#ifndef _COMMANDLINEPARSER_
#define _COMMANDLINEPARSER_

#include <map>
#include <string>
#include <iostream>

#include <QtCore/QStringList>

namespace cass
{
/** command line argument parser
 *
 * object that will parse the command line parameters and set the switches and
 * retrieve the arguments
 *
 * @author Lutz Foucar
 */
class CommandlineArgumentParser
{
public:
  /** a container type for switches */
  typedef std::map<std::string,std::pair<bool*,std::string> >  switches_t;

  /** a container type for switches */
  typedef std::map<std::string,std::pair<int*,std::string> >  intarguments_t;

  /** a container type for switches */
  typedef std::map<std::string,std::pair<std::string*,std::string> >  stringarguments_t;

  /** output which commandline parameters are available */
  void usage()
  {
    using namespace std;
    switches_t::iterator boolarg(_switches.begin());
    switches_t::iterator boolargEnd(_switches.end());
    for (;boolarg != boolargEnd; ++boolarg)
    {
      cout << boolarg->first <<":"<<boolarg->second.second<<endl;
    }

    intarguments_t::iterator intarg(_intargs.begin());
    intarguments_t::iterator intargEnd(_intargs.end());
    for (;intarg != intargEnd; ++intarg)
    {
      cout << intarg->first <<":"<<intarg->second.second
           <<" Default value is '"<<*(intarg->second.first)<<"'"<<endl;
    }

    stringarguments_t::iterator strarg(_stringargs.begin());
    stringarguments_t::iterator strargEnd(_stringargs.end());
    for (;strarg != strargEnd; ++strarg)
    {
      cout << strarg->first <<":"<<strarg->second.second
           <<" Default value is '"<<*(strarg->second.first)<<"'"<<endl;
    }
  }

  /** operator to parse the argumetns
   *
   * the arguments are retrieved as a QStringList from Qt. Go through the list
   * and try to find the parameter in the containers. If it is the switches
   * container simply set the switch to true. Otherwise take the next parameter
   * that should be the argument of the preceding parameter.
   * Start at the 2nd argument of the list, since the first is just the
   * program name.
   *
   * @param argumentList the list of arguments
   */
  void operator()(const QStringList& argumentList)
  {
    QStringList::const_iterator argument(argumentList.constBegin()+1);
    for (; argument != argumentList.constEnd(); ++argument)
    {
      switches_t::iterator boolarg(_switches.find(argument->toStdString()));
      if (boolarg != _switches.end())
      {
        *(boolarg->second.first) = true;
        continue;
      }
      intarguments_t::iterator intarg(_intargs.find(argument->toStdString()));
      if (intarg != _intargs.end())
      {
        ++argument;
        *(intarg->second.first) = argument->toInt();
        continue;
      }
      stringarguments_t::iterator stringarg(_stringargs.find(argument->toStdString()));
      if (stringarg != _stringargs.end())
      {
        ++argument;
        *(stringarg->second.first) = argument->toStdString();
        continue;
      }

      std::cout << "CommandlineArgumentParser(): parameter '" << argument->toStdString()
           << "' is unknown. Possible values for this version of the program are: "
           << std::endl;
      usage();
      exit(2);
    }
  }

  /** add a switch to the switches container
   *
   * @param sw the name of the parameter to look for
   * @param desc the description of the parameter
   * @param val a reference to the value that should be changed.
   */
  void add(const std::string &sw, const std::string& desc, bool &val)
  {
    _switches[sw] = make_pair(&val,desc);
  }

  /** add a switch to the string container
   *
   * @param sw the name of the parameter to look for
   * @param desc the description of the parameter
   * @param val a reference to the value that should be changed.
   */
  void add(const std::string &sw, const std::string& desc, std::string &val)
  {
    _stringargs[sw] = make_pair(&val,desc);
  }

  /** add a switch to the int container
   *
   * @param sw the name of the parameter to look for
   * @param desc the description of the parameter
   * @param val a reference to the value that should be changed.
   */
  void add(const std::string &sw, const std::string& desc, int &val)
  {
    _intargs[sw] = make_pair(&val, desc);
  }

private:
  /** container for the switches */
  switches_t _switches;

  /** container for the string arguments */
  stringarguments_t _stringargs;

  /** container for the int arguments */
  intarguments_t _intargs;
};
}//end namspace cass

#endif
