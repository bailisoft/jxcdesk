#include "lxcalendar.h"
#include <QtWidgets>

//根本没用，这个捕捉不到具体日期（子控件）的事件！
void LxCalendar::keyPressEvent(QKeyEvent *e)
{
    QCalendarWidget::keyPressEvent(e);
    if ( e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return ) {
        emit clicked(selectedDate());
        qDebug() << "keyPressed!";
        hide();
    }
}
