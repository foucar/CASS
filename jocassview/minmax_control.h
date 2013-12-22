// Copyright (C) 2013 Lutz Foucar

/**
 * @file minmax_control.h contains a control over min and max values
 *
 * @author Lutz Foucar
 */

#ifndef _MINMAXCONTROL_
#define _MINMAXCONTROL_

#include <QtGui/QWidget>

class QCheckBox;
class QLineEdit;

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
   * @param parent the parent of this
   */
  MinMaxControl(QWidget *parent=0);

  /** return whether the manual control is activated
   *
   * @return true when manual control is activated
   */
  bool manual() const;

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
  QCheckBox *_manual;

  /** the minimum value input */
  QLineEdit *_mininput;

  /** the maximum value input */
  QLineEdit *_maxinput;
};
}//end namespace jocassview
#endif
