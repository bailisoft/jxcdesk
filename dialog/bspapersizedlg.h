#ifndef BSPAPERSIZEDLG_H
#define BSPAPERSIZEDLG_H

#include <QDialog>
#include <QtWidgets>

namespace BailiSoft {

class BsPaperSizeDlg : public QDialog
{
    Q_OBJECT
public:
    BsPaperSizeDlg(QWidget *parent);
    ~BsPaperSizeDlg() {}

    QLineEdit*  mpWidth;
    QLineEdit*  mpHeight;

private slots:
    void fixupInteger();

};

}

#endif // BSPAPERSIZEDLG_H
