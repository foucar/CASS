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
     * @param filename the file to parse
     * @param eventmap reference to the map of events
     * @param lock reference to the protector of the eventlist map
     */
    FileParser(const std::string &filename,
               eventmap_t &eventmap,
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
     * @param filename the file to parse
     * @param eventmap reference to the map of events
     * @param lock reference to the protector of the eventlist map
     */
    static shared_pointer instance(const std::string &filename,
                                   eventmap_t &eventmap,
                                   QReadWriteLock &lock);

  protected:
    /** reference to the map containing the beginnings of the event */
    eventmap_t &_eventmap;

    /** Lock that protects the map */
    QReadWriteLock &_lock;

    /** the file pointer */
    FilePointer _filepointer;
  };
}
#endif
