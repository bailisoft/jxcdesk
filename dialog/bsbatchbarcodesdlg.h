#ifndef BSBATCHBARCODESDLG_H
#define BSBATCHBARCODESDLG_H

#include <QDialog>
#include <QtWidgets>

namespace BailiSoft {

class BsBatchBarcodesDlg : public QDialog
{
    Q_OBJECT
public:
    BsBatchBarcodesDlg(QWidget *parent, const QString &fileData);
    ~BsBatchBarcodesDlg(){}
    int textMaxColCount();

    QTextEdit*  mpText;
    QLineEdit*  mpBarCol;
    QLineEdit*  mpQtyCol;

private slots:
    void fixupInteger();
};

}

#endif // BSBATCHBARCODESDLG_H
