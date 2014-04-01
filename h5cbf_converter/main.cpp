// Copyright (C) 2014 Lutz Foucar

/**
 * @file main.cpp main file for the converter
 *
 * @author Lutz Foucar
 */

#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QDebug>

#include <vector>

#include "cl_parser.hpp"
#include "hdf5_handle.hpp"
#include "cbf_handle.hpp"

int main(int argc, char *argv[])
{
  /** parse command line parameters and if first parameter is given open file */
  QCoreApplication app(argc, argv);
  cass::CommandlineArgumentParser parser;
  std::string filename("nofile");
  parser.add("-f","filename of file that needs to be converted",filename);
  std::string key("empty");
  parser.add("--h5key","key of the datafield in the hdf5 that conains or should contain the detector data",key);
  bool showUsage(false);
  parser.add("-h","show this help",showUsage);

  parser(QCoreApplication::arguments());

  /** show help and exit if requested */
  if (showUsage)
  {
    parser.usage();
    exit(0);
  }

  QString fname(QString::fromStdString(filename));
  if (QFileInfo(fname).suffix() == "cbf")
  {
    /** convert from cbf to h5 */
  }
  /** convert from h5 to cbf */
  else if (QFileInfo(fname).suffix() == "h5")
  {
    /** read data from h5 file */
    hdf5::Handler h5handle(filename,"r");

    std::vector<float> matrix;
    std::pair<size_t,size_t> shape;

    switch (h5handle.dimension(key))
    {
    case (0):
    {
      qDebug()<< " provided key points to a scalar value";
      break;
    }
    case (1):
    {
      qDebug()<< " provided key points to a array value";
      break;
    }
    case (2):
    {
      h5handle.readMatrix(matrix,shape,key);
//      float xlow,xup,ylow,yup;
//      try { xlow = h5handle.readScalarAttribute<float>("xLow",key.toStdString()); }
//      catch(const invalid_argument & what) { xlow = 0; }
//      try { xup = h5handle.readScalarAttribute<float>("xUp",key.toStdString()); }
//      catch(const invalid_argument & what) { xup = shape.first; }
//      try { ylow = h5handle.readScalarAttribute<float>("yLow",key.toStdString()); }
//      catch(const invalid_argument & what) { ylow = 0; }
//      try { yup = h5handle.readScalarAttribute<float>("yUp",key.toStdString()); }
//      catch(const invalid_argument & what) { yup = shape.second; }
//      cass::Histogram2DFloat * hist(new cass::Histogram2DFloat(shape.first,xlow,xup,
//                                                               shape.second,ylow,yup));
//      copy(matrix.begin(),matrix.end(),hist->memory().begin());
//      hist->key() = key.toStdString();
//      return hist;
      break;
    }
    default:
      qDebug()<<" key points to data with unknown dimension";
      return 0;
    }

    /** write data to cbf file */
    QString outfilename = QString(QFileInfo(fname).absolutePath() + '/' +
        QFileInfo(fname).baseName() + ".cbf");
    cass::CBF::write(outfilename.toStdString(), matrix, shape);
  }

}
