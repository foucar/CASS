// Copyright (C) 2013 Lutz Foucar

/**
 * @file minmax_control.h contains a control over min and max values
 *
 * @author Lutz Foucar
 */

#ifndef _MINMAXCONTROL_
#define _MINMAXCONTROL_

#include <QtCore/QString>

#include <QtGui/QWidget>
#include <QtGui/QWidgetAction>

class QCheckBox;
class QToolButton;
class QLineEdit;
class QToolBar;

namespace jocassview
{

/** widget to control the min and max values
 *
 * @author Lutz Foucar
 */
class MinMaxControl : public QWidget
{
  Q_OBJECT
public:
  /** constructor
   *
   * @param title the title of the widget
   * @param parent the parent of this
   */
  MinMaxControl(QString title, QToolBar *parent);

  /** return whether the plot should be autoscaled
   *
   * @return true when plot should be autoscaled
   */
  bool autoscale() const;

  /** return wether log is enabled
   *
   * @return true when log is enabled
   */
  bool log() const;

  /** retrieve the minimum value
   *
   * @return the minimum value
   */
  double min() const;

  /** retieve the maximum value
   *
   * @return the maximum value
   */
  double max() const;

private slots:
  /** react on when one of the inputs changed
   *
   * details
   */
  void on_changed();

signals:
  /** signal is emitted when one of the controls have changed */
  void controls_changed();

private:
  /** select manual input */
  QToolButton *_log;

  /** select manual input */
  QToolButton *_auto;

  /** the minimum value input */
  QLineEdit *_mininput;

  /** the maximum value input */
  QLineEdit *_maxinput;
};
}//end namespace jocassview
#endif
