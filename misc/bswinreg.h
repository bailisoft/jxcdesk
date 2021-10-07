#ifndef BSWINREG_H
#define BSWINREG_H

#include <QtWidgets>

namespace BailiSoft {

class BsMain;
class BsField;
class BsFldBox;

class BsWinReg : public QWidget
{
    Q_OBJECT
public:
    explicit BsWinReg(QWidget *parent, const QString &tableName, const QStringList &fields);
    ~BsWinReg();

    QString         mTable;
    QString         mCname;

private:
    void onClickCommit();
    void onRequestOk(const QWidget *sender, const QStringList &fens);

    BsMain*         mppMain;

    QList<BsField*>     mFields;
    QList<BsFldBox*>    mEditors;

    QPushButton*    mpBtnCommit;

    QStringList mFlds;
    QStringList mDataVals;
    QStringList mSqlVals;
};

}

#endif // BSWINREG_H
