#include "bspapersizedlg.h"
#include "main/bailicode.h"

namespace BailiSoft {

BsPaperSizeDlg::BsPaperSizeDlg(QWidget *parent) : QDialog(parent)
{
    mpWidth = new QLineEdit(this);
//    mpWidth->setValidator(new QIntValidator(99, 999));    //设了这个就不会触发editingFinished
    mpWidth->setText("200");

    mpHeight = new QLineEdit(this);
//    mpHeight->setValidator(new QIntValidator(99, 999));
    mpHeight->setText("160");

    QGroupBox *form = new QGroupBox(this);
    form->setTitle(QStringLiteral("纸张尺寸"));
    QFormLayout *layForm = new QFormLayout(form);
    layForm->addRow(QStringLiteral("宽(mm)："), mpWidth);
    layForm->addRow(QStringLiteral("高(mm)："), mpHeight);

    //确定取消
    QDialogButtonBox *pBox = new QDialogButtonBox(this);
    pBox->setOrientation(Qt::Horizontal);

    QPushButton *pBtnOk = pBox->addButton(mapMsg.value("btn_ok"), QDialogButtonBox::AcceptRole);
    pBtnOk->setIcon(QIcon(":/icon/ok.png"));
    pBtnOk->setIconSize(QSize(20, 20));
    connect(pBtnOk, SIGNAL(clicked()), this, SLOT(accept()));

    QPushButton *pBtnCancel = pBox->addButton(mapMsg.value("btn_cancel"), QDialogButtonBox::RejectRole);
    pBtnCancel->setIcon(QIcon(":/icon/cancel.png"));
    pBtnCancel->setIconSize(QSize(20, 20));
    connect(pBtnCancel, SIGNAL(clicked()), this, SLOT(reject()));

    //layout
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->addWidget(form, 1);
    lay->addWidget(pBox);
    setWindowTitle(QStringLiteral("添加纸张规格"));
    setWindowFlags(windowFlags() &~ Qt::WindowContextHelpButtonHint);

    connect(mpWidth, SIGNAL(editingFinished()), this, SLOT(fixupInteger()));
    connect(mpHeight, SIGNAL(editingFinished()), this, SLOT(fixupInteger()));
}

void BsPaperSizeDlg::fixupInteger()
{
    //Qt'bug?————设了QIntValidator这个就不会触发editingFinished
    QLineEdit *edt = qobject_cast<QLineEdit*>(QObject::sender());
    if ( edt ) {
        if ( edt->text().toInt() < 99 )
            edt->setText("99");

        if ( edt->text().toInt() > 999 )
            edt->setText("999");
    }
}

}
