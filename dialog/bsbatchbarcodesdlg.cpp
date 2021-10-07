#include "bsbatchbarcodesdlg.h"
#include "main/bailicode.h"


namespace BailiSoft {

BsBatchBarcodesDlg::BsBatchBarcodesDlg(QWidget *parent, const QString &fileData) : QDialog(parent)
{
    mpText = new QTextEdit(this);
    mpText->setPlainText(fileData);
    mpText->setReadOnly(true);

    mpBarCol = new QLineEdit(this);
    mpBarCol->setText("1");

    mpQtyCol = new QLineEdit(this);
    mpQtyCol->setText("2");

    QGroupBox *form = new QGroupBox(this);
    form->setTitle(QStringLiteral("列位置"));
    QFormLayout *layForm = new QFormLayout(form);
    layForm->addRow(QStringLiteral("条码第几列："), mpBarCol);
    layForm->addRow(QStringLiteral("数量第几列："), mpQtyCol);

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
    lay->addWidget(mpText, 1);
    lay->addWidget(form, 0);
    lay->addWidget(pBox);
    setWindowTitle(QStringLiteral("确认条码文件格式"));
    setWindowFlags(windowFlags() &~ Qt::WindowContextHelpButtonHint);

    connect(mpBarCol, SIGNAL(editingFinished()), this, SLOT(fixupInteger()));
    connect(mpQtyCol, SIGNAL(editingFinished()), this, SLOT(fixupInteger()));
}

int BsBatchBarcodesDlg::textMaxColCount()
{
    QString txt = mpText->toPlainText();
    QChar splittor = (txt.indexOf(QChar(9)) > 0) ? QChar(9) : QChar(44);

    int count = 0;
    QStringList lines = mpText->toPlainText().split(QChar(10));
    for ( int i = 0, iLen = lines.length(); i < iLen; ++i )
    {
        int c = QString(lines.at(i)).split(splittor).length();
        if ( c > count )
            count = c;
    }
    return count;
}

void BsBatchBarcodesDlg::fixupInteger()
{
    //Qt'bug?————设了QIntValidator这个就不会触发editingFinished
    QLineEdit *edt = qobject_cast<QLineEdit*>(QObject::sender());
    if ( edt ) {
        if ( edt->text().toInt() < 1 )
            edt->setText("1");

        int maxCol = textMaxColCount();
        if ( edt->text().toInt() > maxCol )
            edt->setText(QString::number(maxCol));
    }
}

}
