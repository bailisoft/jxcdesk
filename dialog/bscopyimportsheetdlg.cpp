#include "bscopyimportsheetdlg.h"
#include "main/bailicode.h"
#include "main/bailidata.h"

namespace BailiSoft {

BsCopyImportSheetDlg::BsCopyImportSheetDlg(QWidget *parent) : QDialog(parent)
{
    //单据选择
    mpCmbSheetName = new QComboBox(this);
    for ( int i = 0, iLen = lstSheetWinTableNames.length(); i < iLen; ++i ) {
        QString winTbl = lstSheetWinTableNames.at(i);
        QString winName = lstSheetWinTableCNames.at(i);
        if ( winTbl != QStringLiteral("szd") ) {
            if ( canDo(winTbl) )
                mpCmbSheetName->addItem(winName, winTbl);
        }
    }

    //单据号
    mpEdtSheetId = new QLineEdit(this);
    mpEdtSheetId->setMinimumWidth(200);

    //form
    QFormLayout* form = new QFormLayout;
    form->addRow(QStringLiteral("源单据："), mpCmbSheetName);
    form->addRow(QStringLiteral("单据号："), mpEdtSheetId);

    //确定取消
    QDialogButtonBox *pBox = new QDialogButtonBox(this);
    pBox->setOrientation(Qt::Horizontal);

    mpBtnOk = pBox->addButton(mapMsg.value("btn_ok"), QDialogButtonBox::AcceptRole);
    mpBtnOk->setIcon(QIcon(":/icon/ok.png"));
    mpBtnOk->setIconSize(QSize(20, 20));
    connect(mpBtnOk, SIGNAL(clicked()), this, SLOT(accept()));

    QPushButton *pBtnCancel = pBox->addButton(mapMsg.value("btn_cancel"), QDialogButtonBox::RejectRole);
    pBtnCancel->setIcon(QIcon(":/icon/cancel.png"));
    pBtnCancel->setIconSize(QSize(20, 20));
    connect(pBtnCancel, SIGNAL(clicked()), this, SLOT(reject()));

    //layout
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->addLayout(form, 1);
    lay->addWidget(pBox, 0, Qt::AlignCenter);
    setWindowTitle(QStringLiteral("复制导入单据"));
    setWindowFlags(windowFlags() &~ Qt::WindowContextHelpButtonHint);

    //init
    mpBtnOk->setEnabled(false);
    connect(mpCmbSheetName, SIGNAL(currentIndexChanged(int)), this, SLOT(checkReady()));
    connect(mpEdtSheetId, SIGNAL(textChanged(QString)), this, SLOT(checkReady()));
}

void BsCopyImportSheetDlg::checkReady()
{
    int sheetId = mpEdtSheetId->text().toInt();
    mpBtnOk->setEnabled(sheetId > 0 && mpCmbSheetName->currentIndex() >= 0);
}

}
