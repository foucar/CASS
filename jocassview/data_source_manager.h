// Copyright (C) 2013, 2014 Lutz Foucar

/**
 * @file data_source_manager.h contains singleton class to manage the data sources
 *
 * @author Lutz Foucar
 */

#ifndef _DATASOURCEMANAGER_
#define _DATASOURCEMANAGER_

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QString>

class QMenu;
class QActionGroup;
class QSignalMapper;

namespace jocassview
{
class DataSource;

/** singleton class to manage the available data sources
 *
 * @author Lutz Foucar
 */
class DataSourceManager : public QObject
{
  Q_OBJECT

public:
  /** generate an instance of this, if not already existent
   *
   * @return the singleton instance of this
   */
  static DataSourceManager* instance();

  /** retrieve an available source
   *
   * if no name of the source is given the current active one will be returned
   * If no are available or the requested doesn't exist a 0 pointer will be
   * returned
   *
   * @return pointer to the requested source
   * @param sourcename the name of the requested source
   */
  static DataSource* source(const QString & sourcename=QString());

  /** return the current source name
   *
   * @return name of current active source
   */
  static QString currentSourceName();

  /** retrieve the names of all current active sources
   *
   * @return list of names of the sources
   */
  static QStringList sourceNames();

  /** set the Menu where the data sources will be displayed in
   *
   * @param menue the menu that displays the available data sources
   */
  static void setMenu(QMenu* menu);

  /** add a data source
   *
   * the added source will be set as the current active source
   *
   * @param sourcename the name of the data source that should be added
   * @param source the data source that should be added
   * @param setActive Set added source as the current active one and emit the
   *                  signal that a new source has been
   */
  static void addSource(const QString &sourcename, DataSource * source, bool setActive=true);

signals:
  /** signal when a new source was chosen
   *
   * @param newSource the name of the new source
   */
  void sourceChanged(QString newSource);

private:
  /** constructor */
  DataSourceManager();

  /** copy constructor */
  DataSourceManager(const DataSourceManager&);

  /** self assignment */
  DataSourceManager& operator=(const DataSourceManager&);

private:
  /** an instane of this */
  static DataSourceManager *_instance;

  /** container for all data sources */
  QMap<QString,DataSource*> _sources;

  /** the source menu */
  QMenu *_sourceMenu;

  /** the action group where the menue items will be grouped */
  QActionGroup *_actionGroup;

  /** mapper for the signals of the signal group */
  QSignalMapper *_mapper;
};
}//end namespace jocassview

#endif
