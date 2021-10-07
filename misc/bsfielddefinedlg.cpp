#include "bsfielddefinedlg.h"
#include "main/bailicode.h"
#include "main/bailigrid.h"

namespace BailiSoft {

BsFieldDefineDlg::BsFieldDefineDlg(QWidget *parent, const QString &tbl, const QList<BsField *> &flds)
    : QDialog(parent), mTable(tbl), mFields(flds)
{
    mpForm = new QFormLayout;

    for ( int i = 0, iLen = flds.length(); i < iLen; ++i ) {
        BsField *fld = flds.at(i);
        QLineEdit *edt = new QLineEdit(this);
        edt->setMaxLength(10);
        edt->setText(fld->mFldCnName);
        edt->setProperty("field_name", fld->mFldName);
        edt->setProperty("field_value", fld->mFldCnName);
        mpForm->addRow(fld->mFldCnName, edt);
    }

    //确定取消
    mpBox = new QDialogButtonBox(this);
    mpBox->setOrientation(Qt::Horizontal);
    mpBox->setCenterButtons(true);

    QPushButton *pBtnOk = mpBox->addButton(mapMsg.value("btn_ok"), QDialogButtonBox::AcceptRole);
    pBtnOk->setIcon(QIcon(":/icon/ok.png"));
    pBtnOk->setIconSize(QSize(20, 20));
    connect(pBtnOk, SIGNAL(clicked()), this, SLOT(accept()));

    QPushButton *pBtnCancel = mpBox->addButton(mapMsg.value("btn_cancel"), QDialogButtonBox::RejectRole);
    pBtnCancel->setIcon(QIcon(":/icon/cancel.png"));
    pBtnCancel->setIconSize(QSize(20, 20));
    connect(pBtnCancel, SIGNAL(clicked()), this, SLOT(reject()));

    //layout
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->addLayout(mpForm, 1);
    lay->addWidget(mpBox);
    setWindowFlags(windowFlags() &~ Qt::WindowContextHelpButtonHint);
}

void BsFieldDefineDlg::keyPressEvent(QKeyEvent *e)
{
    if ( e->key() != Qt::Key_Enter && e->key() != Qt::Key_Return && e->key() != Qt::Key_Escape )
        QDialog::keyPressEvent(e);
}

}
