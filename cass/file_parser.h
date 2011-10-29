// Copyright (C) 2011 Lutz Foucar

/**
 * @file file_parser.h contains base class for all file parsers
 *
 * @author Lutz Foucar
 */

#ifndef _FILEPARSER_H_
#define _FILEPARSER_H_

#include <QtCore/QObject>
#include <QThread>
#include <QReadWriteLock>

#include <tr1/memory>
#include <fstream>
#include <string>

#include "cass.h"

namespace cass
{
/** base class for all file parsers
 *
 * @author Lutz Foucar
 */
class FileParser : public QThread
{
  Q_OBJECT
public:
  /** constructor
   *
   * @param readerpointerpair the filereader the will read the event from files
   * @param event2posreader reference to container that maps events to the
   *                        position in file, reader pair vector
   * @param lock reference to the protector of the eventlist map
   */
  FileParser(const filereaderpointerpair_t readerpointerpair,
             event2positionreaders_t &event2posreader,
             QReadWriteLock &lock);

  /** typedef the shared pointer of this */
  typedef std::tr1::shared_ptr<FileParser> shared_pointer;

  /** virtual destructor */
  virtual ~FileParser();

  /** create an instance of the requested type
   *
   * returns an instance of the right fileparser. To figure out the right
   * fileparser the filname is inspected and depending on the extension the
   * right file parser is created.
   *
   * @return a shared pointer to the requested type
   * @param type the type of the parser requested
   * @param readerpointerpair the filereader the will read the event from files
   * @param event2posreader reference to container that maps events to the
   *                        position in file, reader pair vector
   * @param lock reference to the protector of the eventlist map
   */
  static shared_pointer instance(const std::string type,
                                 const filereaderpointerpair_t readerpointerpair,
                                 event2positionreaders_t &event2posreader,
                                 QReadWriteLock &lock);

  /** @return the type of file parser */
  virtual const std::string type() {return "Unknown";}

protected:
  /** put current file position in the eventmap
   *
   * save the current position of the filestream in the filepointer and put
   * a copy of the file pointer in the eventmap.
   *
   * @param eventId the event id that should be associated with this position
   */
  void savePos(uint64_t eventId);

protected:
  /** the file pointer */
  filereaderpointerpair_t _readerpointerpair;

  /** reference to the map containing the beginnings of the event */
  event2positionreaders_t &_event2posreader;

  /** Lock that protects the map */
  QReadWriteLock &_lock;

};
}//end namespace cass
#endif
