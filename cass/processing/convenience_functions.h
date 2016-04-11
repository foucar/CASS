// Copyright (C) 2010,2013 Lutz Foucar

/**
 * @file convenience_functions.h file contains declaration of classes and
 *                               functions that help other processors to do
 *                               their job.
 *
 * @author Lutz Foucar
 */

#ifndef __CONVENIENCE_FUNCTIONS_H__
#define __CONVENIENCE_FUNCTIONS_H__

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QString>

#include "cass.h"
#include "result.hpp"
#include "processor.h"

namespace cass
{
class CASSSettings;

namespace ACQIRIS
{
class DetectorBackend;

/** load detector from file
 *
 * after loading check whether it is a delayline detector, if not throw
 * invalid_argument exception.
 *
 * @return key containing detector name
 * @param s CASSSettings object to read the info from
 * @param ppNbr the Postprocessor number of the processor calling this
 *              function
 * @param key the key of the processor calling this function
 *
 * @author Lutz Foucar
 */
std::string loadDelayDet(CASSSettings &s,
                         int ppNbr,
                         const std::string& key);

/** load particle for a specific detector
 *
 * after loading check whether it is a delayline detector, if not throw
 * invalid_argument exception.
 *
 * @return key containing detector name
 * @param s CASSSettings object to read the info from
 * @param detector the name of the detector that contains the layer
 * @param ppNbr the Postprocessor number of the processor calling this
 *              function
 * @param key the key of the processor calling this function
 *
 * @author Lutz Foucar
 */
std::string loadParticle(CASSSettings &s,
                         const std::string &detector,
                         int ppNbr,
                         const std::string& key);
}//end namespace acqiris

/** Binary function for thresholding
 *
 * Returns the value if it is below threshold.  Otherwise, returns 0.
 *
 * @author Thomas White
 */
class threshold : public std::binary_function<float, float, float>
{
public:
  /** operator */
  float operator() (float value, float thresh)const
  {
    return (value > thresh) ? value : 0.0;
  }
};

/** binary function for weighted subtraction.
 *
 * @author Lutz Foucar
 */
class weighted_minus : std::binary_function<float, float, float>
{
public:
  /** constructor.
   *
   * @param first_weight the weight value of the first histogram
   * @param second_weight the weight value of the second histogram
   */
  weighted_minus(float first_weight, float second_weight)
    :_first_weight(first_weight),_second_weight(second_weight)
  {}
  /** operator */
  float operator() (const float first, const float second)
  { return first * _first_weight - second * _second_weight;}
protected:
  float _first_weight, _second_weight;
};


/** binary function for averaging.
 *
 * this operator performs a moving sum
 *
 * @author Nicola Coppola
 */
class TimeAverage : std::binary_function<float,float,float>
{
public:
  /** constructor.
   *
   * initializes the nEvents value
   *
   * @param nEvents The number of Events used up to now
   */
  explicit TimeAverage(float nEvents)
    :_nEvents(nEvents)
  {}

  /** the operator calculates the average over the last _nEvents */
  float operator()(float currentValue, float Average_Nm1)
  {
    if(_nEvents!=0)
      return ( Average_Nm1 * (_nEvents-1) + currentValue ) /_nEvents;
    else
      return currentValue;
  }

protected:
  /** nEvents for the average calculation */
  float _nEvents;
};


/** function to set the 1d histogram properties from the ini file.
 *
 * @param[in] name the name of the processor too look up in cass.ini
 *
 * @author Lutz Foucar
 */
Processor::result_t::shared_pointer set1DHist(const Processor::name_t &name);


/** function to set the 2d histogram properties from the ini file.
 *
 * @param[in] name the name of the processor too look up in cass.ini
 *
 * @author Lutz Foucar
 */
Processor::result_t::shared_pointer set2DHist(const Processor::name_t &name);

/** an alphabetical counter extension
 *
 * changes dirs by appending a subdir with an alphabetically increasing counter
 *
 * @author Lutz Foucar
 */
class AlphaCounter
{
public:
  /** initialize the directory
   *
   * add an alphabtically subdir to the dir of the filename
   *
   * @return the filename containing the new subdir
   * @param fname the filenname whos dir should be modified
   */
  static std::string intializeDir(const std::string &fname)
  {
    QFileInfo fInfo(QString::fromStdString(fname));
    QString path(fInfo.path());
    QString filename(fInfo.fileName());
    path += "/aa/";
    QDir dir(path);
    if (!dir.exists())
      dir.mkpath(".");
    const std::string newfilename(path.toStdString() + filename.toStdString());
    return newfilename;
  }

  /** initialize the filename
   *
   * add an alphabtically ending to the filename
   *
   * @return the filename containing the new counter ending
   * @param fname the filenname with appended counter ending
   */
  static std::string intializeFile(const std::string &fname)
  {
    QFileInfo fInfo(QString::fromStdString(fname));
    if (fInfo.suffix().isEmpty())
      return (fname + "_aa.h5");
    QString filename(fInfo.baseName() + "__aa");
    QString newfilename(fInfo.path() + "/" + filename + "." + fInfo.suffix());
    return newfilename.toStdString();
  }

  /** remove the alpha counter subdir from filename
   *
   * @return filename without the alphacounter subdir
   * @param fname The filename who's subdir should be removed
   */
  static std::string removeAlphaSubdir(const std::string &fname)
  {
    QFileInfo fInfo(QString::fromStdString(fname));
    QString path(fInfo.path());
    QString filename(fInfo.fileName());
    QStringList dirs = path.split("/");
    dirs.removeLast();
    QString newPath(dirs.join("/"));
    newPath.append("/");
    const std::string newfilename(newPath.toStdString() + filename.toStdString());
    return newfilename;

  }

  /** increase the alpha counter
   *
   * @return filename with alphabetically increased subdir
   * @param fname the Filename whos subdir should be increased
   */
  static std::string increaseDirCounter(const std::string &fname)
  {
    QFileInfo fInfo(QString::fromStdString(fname));
    QString path(fInfo.path());
    QString filename(fInfo.fileName());
    QStringList dirs = path.split("/");
    QString subdir = dirs.last();
    QByteArray alphaCounter = subdir.toLatin1();
    if (alphaCounter[1] == 'z')
    {
      const char first(alphaCounter[0]);
      alphaCounter[0] = first + 1;
      alphaCounter[1] = 'a';
    }
    else
    {
      const char second(alphaCounter[1]);
      alphaCounter[1] = second + 1;
    }
    QString newSubdir(QString::fromLatin1(alphaCounter));
    dirs.removeLast();
    dirs.append(newSubdir);
    QString newPath(dirs.join("/"));
    newPath.append("/");
    QDir dir(newPath);
    if (!dir.exists())
      dir.mkpath(".");
    const std::string newfilename(newPath.toStdString() + filename.toStdString());
    return newfilename;
  }

  /** increase the alpha counter in the file name
   *
   * @return filename with alphabetically increased subdir
   * @param fname the Filename whos subdir should be increased
   */
  static std::string increaseFileCounter(const std::string &fname)
  {
    QFileInfo fInfo(QString::fromStdString(fname));
    QString filename(fInfo.baseName());
    QStringList filenameparts(filename.split("__"));
    QByteArray counter(filenameparts.last().toLatin1());
    if (counter[1] == 'z')
    {
      const char first(counter[0]);
      counter[0] = first + 1;
      counter[1] = 'a';
    }
    else
    {
      const char second(counter[1]);
      counter[1] = second + 1;
    }
    filenameparts.last() = QString::fromLatin1(counter);
    QString newfilename(fInfo.path() + "/" + filenameparts.join("__") + "." + fInfo.suffix());
    return newfilename.toStdString();
  }
};



/** Helper function to delete duplicates from a std::list
 *
 * This keeps the earliest entry in the list and removes all later ones
 *
 * @param l List to remove duplicates from.
 */
template<typename T>
inline void unique(std::list<T>& l)
{
  // shorten list by removing consecutive duplicates
  l.unique();
  // now remove remaining (harder) duplicates
  for(typename std::list<T>::iterator i1 = l.begin();
      i1 != l.end();
      ++i1) {
    typename std::list<T>::iterator i2(i1);
    ++i2;
    while(l.end() != (i2 = find(i2, l.end(), *i1)))
      l.erase(i2);
  }
}

/** fuzzy compare two floating point variables
 *
 * @tparam the type that one want to compare
 * @return true when the two values are equal within the precision, false otherwise
 * @param first the first value for the equal comparison
 * @param second the second value for the equal comparison
 */
template <typename T>
bool fuzzycompare(const T& first, const T& second)
{
  return (abs(first-second) < sqrt(std::numeric_limits<T>::epsilon()));
}

/** fuzzy compare a floating point number to 0
 *
 * @tparam the type that one want to compare
 * @param val the value for the comparison
 */
template <typename T>
bool fuzzyIsNull(const T& val)
{
  return (val < sqrt(std::numeric_limits<T>::epsilon()));
}
}//end namespace cass

#endif
