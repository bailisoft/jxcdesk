#ifndef BSSETPASSWORD_H
#define BSSETPASSWORD_H

#include <QtWidgets>

namespace BailiSoft {

class BsSetPassword : public QDialog
{
    Q_OBJECT
public:
    BsSetPassword(const QString &prLoginer, const QString &prOldPwd, QWidget *parent = nullptr);
    QLineEdit   *mpNewWord;

private slots:
    void toggleChkShow(bool isChecked);
    void checkAccept();

private:
    QLineEdit   *mpOldValue;
    QLineEdit   *mpNewAgain;
    QCheckBox   *mpChkReadable;
    QPushButton *mpBtnOK;
    QPushButton *mpBtnCancel;

    QString     mOldWord;
};

}

#endif // BSSETPASSWORD_H

