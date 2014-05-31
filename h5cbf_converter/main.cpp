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

#include <cstdlib>
#include <vector>
#include <string>
#include <map>

#include "cl_parser.hpp"
#include "hdf5_handle.hpp"
#include "cbf_handle.hpp"

std::map<std::string,float> parseCBFHeader(const std::string &header)
{
  using namespace std;

  map<string,float> values;
  istringstream s(header);
  string line;
  while(getline(s,line))
  {
//    cout << line<<endl;
    sscanf(line.c_str(),"# Pixel_size %f m x %f m", &values["PixelsizeX_m"],
                                                    &values["PixelsizeY_m"]);
    sscanf(line.c_str(),"# Exposure_time %f s", &values["ExposureTime_s"]);
    sscanf(line.c_str(),"# Exposure_period %f s", &values["ExposurePeriod_s"]);
    sscanf(line.c_str(),"# Tau = %f s", &values["Tau_s"]);
    sscanf(line.c_str(),"# Count_cutoff %f counts", &values["CountCutoff_counts"]);
    sscanf(line.c_str(),"# Threshold_setting: %f eV", &values["ThresholdSettings_eV"]);
    sscanf(line.c_str(),"# Wavelength %f", &values["Wavelength_A"]);
    sscanf(line.c_str(),"# Detector_distance %f", &values["DetectorDistance_m"]);
    sscanf(line.c_str(),"# Beam_xy (%f, %f) pixels", &values["BeamcenterX_pix"],
                                                     &values["BeamcenterY_pix"]);
    sscanf(line.c_str(),"# Flux %f", &values["Flux"]);
    sscanf(line.c_str(),"# Filter_transmission %f", &values["FilterTransmission"]);
    sscanf(line.c_str(),"# Start_angle %f deg.", &values["StartAngle_deg"]);
    sscanf(line.c_str(),"# Angle_increment %f deg.", &values["AngleIncrement_deg"]);
    sscanf(line.c_str(),"# Phi %f deg.", &values["Phi_deg"]);
    sscanf(line.c_str(),"# Chi %f deg.", &values["Chi_deg"]);
  }
//  for (map<string,float>::const_iterator it(values.begin()); it != values.end(); ++it)
//    cout << it->first << " = " << it->second<<endl;

  return values;
}

int main(int argc, char *argv[])
{
  /** parse command line parameters and if first parameter is given open file */
  QCoreApplication app(argc, argv);
  cass::CommandlineArgumentParser parser;
  std::string filename("nofile");
  parser.add("-f","filename of file that needs to be converted",filename);
  std::string outfile("nofile");
  parser.add("-o","output filename of file that it will be converted to",outfile);
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

  /** convert from cbf to h5 */
  if (QFileInfo(fname).suffix() == "cbf")
  {
    /** read the data from the cbf file */
    std::vector<float> matrix;
    std::pair<int,int> shape;
    std::string head;
    cass::CBF::read(filename, head, matrix, shape);
    std::map<std::string,float> headervals(parseCBFHeader(head));

    /** write the h5 file */
    QString outfilename = QString::fromStdString(outfile);
    if (outfilename == "nofile")
    {
      outfilename = QString(QFileInfo(fname).absolutePath() + '/' +
                            QFileInfo(fname).baseName() + ".h5");
    }
    else if (!outfilename.contains("h5"))
    {
      qDebug()<< " the outputfile should be a hdf5 file";
      exit(0);
    }
    if (key == "empty")
      key = "detector_data";
    hdf5::Handler h5handle(outfilename.toStdString(),"w");
    h5handle.writeMatrix(matrix,shape,key,0);
    h5handle.writeString(head,"cbf_header");
    for (std::map<std::string,float>::const_iterator it(headervals.begin()); it != headervals.end(); ++it)
      h5handle.writeScalar(it->second, std::string("cbf_header_vals/" + it->first));
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
      break;
    }
    default:
      qDebug()<<" key points to data with unknown dimension";
      return 0;
    }

    /** write data to cbf file */
    QString outfilename = QString::fromStdString(outfile);
    if (outfilename == "nofile")
    {
      outfilename = QString(QFileInfo(fname).absolutePath() + '/' +
                            QFileInfo(fname).baseName() + ".cbf");
    }
    else if (!outfilename.contains("cbf"))
    {
      qDebug()<< " the outputfile should be a cbf file";
      exit(0);
    }

    std::cout<<"converting "<< filename << " to "<<outfilename.toStdString()<<std::endl;
    cass::CBF::write(outfilename.toStdString(), matrix, shape);
  }

}
