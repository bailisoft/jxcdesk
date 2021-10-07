#include "bspickdatedlg.h"
#include "main/bailicode.h"
#include "main/bailiedit.h"
#include "main/bailigrid.h"
#include "main/bailiwins.h"

namespace BailiSoft {

BsPickDateDlg::BsPickDateDlg(QWidget *parent) : QDialog(parent)
{
    BsWin *mppParent = qobject_cast<BsWin*>(parent);
    Q_ASSERT(mppParent);
    QString viName = mppParent->mMainTable;

    //明细类型
    mpTypeSpec = new QRadioButton(QStringLiteral("色码明细"), this);
    mpTypeLeft = new QRadioButton(QStringLiteral("历史明细"), this);

    mpHisType = new QGroupBox(QStringLiteral("核对类型"), this);
    QHBoxLayout *layType = new QHBoxLayout(mpHisType);
    layType->addStretch(1);
    layType->addWidget(mpTypeSpec, 1);
    layType->addSpacing(30);
    layType->addWidget(mpTypeLeft, 1);
    layType->addStretch(1);

    //提示文字
    mpLabel = new QLabel(this);
    mpLabel->setText(QStringLiteral("期初日期"));
    mpLabel->setAlignment(Qt::AlignCenter);
    mpLabel->setStyleSheet("font-weight:900;");

    //日历
    QStringList defs = mapMsg.value(QStringLiteral("fld_dated")).split(QChar(9));
    Q_ASSERT(defs.count() > 4);
    mFldDate = new BsField("dated",
                           defs.at(0),
                           QString(defs.at(3)).toUInt(),
                           QString(defs.at(4)).toInt(),
                           defs.at(2));
    mpEdtDate = new BsFldEditor(this, mFldDate, nullptr);

    //确定取消
    QDialogButtonBox *pBox = new QDialogButtonBox(this);
    pBox->setOrientation(Qt::Horizontal);
    pBox->setCenterButtons(true);

    QPushButton *pBtnOk = pBox->addButton(mapMsg.value("btn_ok"), QDialogButtonBox::AcceptRole);
    pBtnOk->setIcon(QIcon(":/icon/ok.png"));
    pBtnOk->setIconSize(QSize(20, 20));
    connect(pBtnOk, SIGNAL(clicked()), this, SLOT(accept()));

    QPushButton *pBtnCancel = pBox->addButton(mapMsg.value("btn_cancel"), QDialogButtonBox::RejectRole);
    pBtnCancel->setIcon(QIcon(":/icon/cancel.png"));
    pBtnCancel->setIconSize(QSize(20, 20));
    connect(pBtnCancel, SIGNAL(clicked()), this, SLOT(reject()));

    //布局
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(40, 10, 40, 20);
    lay->addWidget(mpHisType);
    lay->addWidget(mpLabel);
    lay->addWidget(mpEdtDate);
    lay->addSpacing(5);
    lay->addWidget(pBox);
    setWindowFlags(windowFlags() &~ Qt::WindowContextHelpButtonHint);

    if ( viName.contains("cash") ) {
        mpTypeLeft->setChecked(true);
        mpHisType->hide();
    }
    else {
        mpTypeSpec->setChecked(true);
        mpLabel->hide();
        mpEdtDate->hide();
    }
    connect(mpTypeLeft, SIGNAL(toggled(bool)), this, SLOT(typeLeftToggled(bool)));
}

BsPickDateDlg::~BsPickDateDlg()
{
    delete mFldDate;
}

void BsPickDateDlg::keyPressEvent(QKeyEvent *e)
{
    if ( e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return )
        return;
    QDialog::keyPressEvent(e);
}

void BsPickDateDlg::typeLeftToggled(bool checked)
{
    mpEdtDate->setVisible(checked);
    mpLabel->setVisible(checked);
    adjustSize();
}

}
