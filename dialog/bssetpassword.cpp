#include "bssetpassword.h"
#include "main/bailicode.h"
#include "main/bailifunc.h"

//具体窗口————更换密码窗口
#define LXWT_MANPASS                   "更换设置登录密码"
#define LXL_MANPASS_OLD_PASSWORD       "原密码"
#define LXL_MANPASS_NEW_PASSWORD       "新密码"
#define LXL_MANPASS_NEW_PASSWORDAGAIN  "确认新密码"
#define LXL_MANPASS_SHOW_PLAIN_TEXT    "显示密码"
#define LXL_MANPASS_OLD_INVALID        "原密码不对，禁止更改。"
#define LXL_MANPASS_NEW_NONULL         "新密码不能为空。"
#define LXL_MANPASS_NEW_DIFF           "新密码两遍输入不一致。"

#define LXAPP_MAX_PASSWORD_LEN          16

namespace BailiSoft {

BsSetPassword::BsSetPassword(const QString &prLoginer, const QString &prOldPwd, QWidget *parent) :
    QDialog(parent), mOldWord(prOldPwd)
{
    setMinimumSize(300, 150);
    setWindowTitle(QStringLiteral(LXWT_MANPASS));
    setWindowFlags(windowFlags() &~ Qt::WindowContextHelpButtonHint);

    QLineEdit *loginerName = new QLineEdit(this);
    loginerName->setText(prLoginer);
    loginerName->setEnabled(false);

    mpOldValue   = new QLineEdit(this);
    disableEditInputMethod(mpOldValue);
    mpOldValue->setEchoMode(QLineEdit::Password);
    mpOldValue->setMaxLength(LXAPP_MAX_PASSWORD_LEN);

    mpNewWord   = new QLineEdit(this);
    disableEditInputMethod(mpNewWord);
    mpNewWord->setEchoMode(QLineEdit::Password);
    mpNewWord->setMaxLength(LXAPP_MAX_PASSWORD_LEN);
    mpNewWord->setMinimumWidth(220);

    mpNewAgain  = new QLineEdit(this);
    disableEditInputMethod(mpNewAgain);
    mpNewAgain->setEchoMode(QLineEdit::Password);
    mpNewAgain->setMaxLength(LXAPP_MAX_PASSWORD_LEN);

    mpChkReadable = new QCheckBox(this);
    mpChkReadable->setText(QStringLiteral(LXL_MANPASS_SHOW_PLAIN_TEXT));

    mpBtnOK      = new QPushButton(this);
    mpBtnOK->setText(mapMsg.value("btn_ok"));

    mpBtnCancel  = new QPushButton(this);
    mpBtnCancel->setText(mapMsg.value("btn_cancel"));

    QFormLayout *pLayForm = new QFormLayout;
    pLayForm->addRow(QStringLiteral("用户名"), loginerName);
    pLayForm->addRow(QStringLiteral(LXL_MANPASS_OLD_PASSWORD), mpOldValue);
    pLayForm->addRow(QStringLiteral(LXL_MANPASS_NEW_PASSWORD), mpNewWord);
    pLayForm->addRow(QStringLiteral(LXL_MANPASS_NEW_PASSWORDAGAIN), mpNewAgain);
    pLayForm->addRow(QString(), mpChkReadable);

    QVBoxLayout *pLayRight  = new QVBoxLayout;
    pLayRight->addWidget(mpBtnOK);
    pLayRight->addWidget(mpBtnCancel);
    pLayRight->addStretch();

    QHBoxLayout *pLayMain   = new QHBoxLayout;
    pLayMain->addLayout(pLayForm);
    pLayMain->addLayout(pLayRight);

    this->setLayout(pLayMain);

    connect(mpBtnOK, SIGNAL(clicked()), this, SLOT(checkAccept()));
    connect(mpBtnCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(mpChkReadable, SIGNAL(toggled(bool)), this, SLOT(toggleChkShow(bool)));

    mpOldValue->setFocus();
}

void BsSetPassword::toggleChkShow(bool isChecked)
{
    if (isChecked) {
        mpOldValue->setEchoMode(QLineEdit::Normal);
        mpNewWord->setEchoMode(QLineEdit::Normal);
        mpNewAgain->setEchoMode(QLineEdit::Normal);
    } else {
        mpOldValue->setEchoMode(QLineEdit::Password);
        mpNewWord->setEchoMode(QLineEdit::Password);
        mpNewAgain->setEchoMode(QLineEdit::Password);
    }
}

void BsSetPassword::checkAccept()
{
    if (mpOldValue->text() != mOldWord) {
        QMessageBox::information(this, QStringLiteral(LXWT_MANPASS), QStringLiteral(LXL_MANPASS_OLD_INVALID));
        return;
    }

    if (mpNewWord->text().isEmpty()) {
        QMessageBox::information(this, QStringLiteral(LXWT_MANPASS), QStringLiteral(LXL_MANPASS_NEW_NONULL));
        return;
    }

    if (mpNewWord->text() != mpNewAgain->text()) {
        QMessageBox::information(this, QStringLiteral(LXWT_MANPASS), QStringLiteral(LXL_MANPASS_NEW_DIFF));
        return;
    }

    accept();
}

}
