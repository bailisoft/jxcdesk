#ifndef BSCOPYIMPORTSHEETDLG_H
#define BSCOPYIMPORTSHEETDLG_H

#include <QDialog>
#include <QtWidgets>

namespace BailiSoft {

class BsCopyImportSheetDlg : public QDialog
{
    Q_OBJECT
public:
    BsCopyImportSheetDlg(QWidget *parent);

    QComboBox*  mpCmbSheetName;
    QLineEdit*  mpEdtSheetId;

private slots:
    void checkReady();

private:
    QPushButton*    mpBtnOk;

};

}

#endif // BSCOPYIMPORTSHEETDLG_H
