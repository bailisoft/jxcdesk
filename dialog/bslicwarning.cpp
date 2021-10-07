#include "bslicwarning.h"

namespace BailiSoft {

BsLicWarning::BsLicWarning(QWidget *parent) : QLabel(parent)
{
    setText(QStringLiteral("试用到期，请购买注册！"));
    setAlignment(Qt::AlignCenter);
    setStyleSheet("background-color:#ff0; color:red; font-size:24px;");
    setFixedSize(400, 60);
}

void BsLicWarning::popShow(QWidget *parent)
{
    BsLicWarning* w = new BsLicWarning(parent);
    w->setWindowFlags(Qt::SplashScreen | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    w->adjustSize();
    w->show();

    if ( parent ) {
        QPoint base = parent->mapToGlobal(QPoint(0, 0));
        w->setGeometry(base.x() + (parent->width() - w->width()) / 2,
                       base.y() + (parent->height() - w->height()) / 2,
                       w->width(),
                       w->height());
    }
    else {
        QRect rect = QGuiApplication::primaryScreen()->geometry();
        w->setGeometry((rect.width() - w->width()) / 2,
                       (rect.height() - w->height()) / 2,
                       w->width(),
                       w->height());
    }

    QTimer::singleShot(1000, qApp, SLOT(quit()));
}

}

