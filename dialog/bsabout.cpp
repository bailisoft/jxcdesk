#include "bsabout.h"
#include "lxwelcome.h"
#include <QtWidgets>

BsAbout::BsAbout(QWidget *parent) : QLabel(parent)
{
    QLabel *pImg = new QLabel(this);
    pImg->setPixmap(QPixmap(":/image/about.png"));
    pImg->setAlignment(Qt::AlignCenter);
    pImg->setFixedSize(640, 360);
    pImg->setStyleSheet("QLabel {background:white; }");

    pBuildNo = new QLabel(this);
    pBuildNo->setText(QStringLiteral("%1.%2.%3")
                      .arg(LXAPP_VERSION_MAJOR)
                      .arg(LXAPP_VERSION_MINOR)
                      .arg(LXAPP_VERSION_PATCH));
    pBuildNo->setStyleSheet("QLabel {color:white; }");

    setFixedSize(640, 360);
}

void BsAbout::mouseReleaseEvent(QMouseEvent *e)
{
    QLabel::mouseReleaseEvent(e);
    close();
}

void BsAbout::resizeEvent(QResizeEvent *e)
{
    QLabel::resizeEvent(e);
    pBuildNo->setGeometry(550, 95, 40, 20);
}

