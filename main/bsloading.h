#ifndef BSLOADING_H
#define BSLOADING_H

#include <QtWidgets>

namespace BailiSoft {

class BsLoading : public QDialog
{
    Q_OBJECT
public:
    explicit BsLoading(const QString &waitHint);
    void setVisible(bool visible);
    void setError(const QString &err);

protected:
    void mousePressEvent(QMouseEvent *e);
    void timerEvent(QTimerEvent*);
    void paintEvent(QPaintEvent*);
private:
    bool        m_return_error;
    QString     m_text;
    int         m_step;
    int         m_timer_id;
};

}

#endif // BSLOADING_H
