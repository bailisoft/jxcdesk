#ifndef LXCALENDAR_H
#define LXCALENDAR_H

#include <QCalendarWidget>
//#include <QVariant>

class QVBoxLayout;
class QLabel;
class QCalendarWidget;
class QLineEdit;
class QToolButton;
class LxMain;

class LxCalendar : public QCalendarWidget
{
    Q_OBJECT
public:
    explicit LxCalendar(QWidget *parent = Q_NULLPTR) : QCalendarWidget(parent) {}

protected:
    void keyPressEvent(QKeyEvent *e);   //根本没用，这个捕捉不到具体日期（子控件）的事件！
};

#endif // LXCALENDAR_H
