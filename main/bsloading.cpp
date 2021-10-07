#include "bsloading.h"

namespace BailiSoft {

BsLoading::BsLoading(const QString &waitHint)
    : QDialog(0), m_return_error(false), m_text(waitHint), m_step(0), m_timer_id(0)
{
    setFixedSize(500, 75);
}

void BsLoading::setVisible(bool visible)
{
    QDialog::setVisible(visible);
    if ( visible ) {
        m_return_error = false;
        m_timer_id = startTimer(100);
    }
    else if ( m_timer_id ) {
        killTimer(m_timer_id);
        m_timer_id = 0;
    }
}

void BsLoading::setError(const QString &err)
{
    m_return_error = true;

    m_text = err;

    if ( m_timer_id ) {
        killTimer(m_timer_id);
        m_timer_id = 0;
    }

    update();
}

void BsLoading::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);

    if ( m_return_error ) {
        reject();
    }
}

void BsLoading::timerEvent(QTimerEvent *)
{
    m_step++;

    if ( m_step > 20 )
        m_step = 0;

    update();
}

void BsLoading::paintEvent(QPaintEvent *)
{
    const int grd_height = 9;
    const int border_width = 5;

    QPainter p(this);

    p.setBrush(Qt::white);
    p.setPen(Qt::NoPen);
    p.drawRect(-1, -1, width() + 1, height() + 1);

    QLinearGradient grd(0, 0, 2 * width(), 0);
    grd.setColorAt(0,               Qt::lightGray);
    grd.setColorAt(m_step / 20.0,   Qt::white);
    grd.setColorAt(1,               Qt::lightGray);

    if ( m_timer_id ) {
        p.setBrush(grd);
    } else {
        p.setBrush(Qt::lightGray);
    }
    p.drawRect(0, height() - grd_height - border_width, width(), height());

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(Qt::darkGreen), border_width));
    p.drawRect(-1, -1, width() + 1, height() + 1);

    QFont f(font());
    if ( m_timer_id ) {
        f.setPointSize( 3 * f.pointSize() / 2 );
        f.setBold(true);
    }
    p.setFont(f);
    p.drawText(rect().adjusted(8, 0, -8, -grd_height), Qt::AlignCenter | Qt::TextWordWrap, m_text);
}

}
