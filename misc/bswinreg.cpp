#include "bswinreg.h"
#include "main/bailicode.h"
#include "main/bsmain.h"
#include "main/bailiedit.h"
#include "main/bailigrid.h"
#include "main/bailidata.h"
#include "main/bssocket.h"

namespace BailiSoft {

BsWinReg::BsWinReg(QWidget *parent, const QString &tableName, const QStringList &fields)
    : QWidget(parent), mTable(tableName)
{
    //用于主窗口查找判断
    setProperty(BSWIN_TABLE, tableName);

    //主窗
    mppMain = qobject_cast<BsMain*>(parent);
    Q_ASSERT(mppMain);

    //中文名
    mCname = mapMsg.value(QStringLiteral("win_%1").arg(tableName)).split(QChar(9)).at(0);

    //字段表（继承类中可以用getFieldByName()获取字段指针以修改具体特别设置，比如不同的trader名称等）
    for ( int i = 0, iLen = fields.length(); i < iLen; ++i )
    {
        QString fld = fields.at(i);
        QStringList defs = mapMsg.value(QStringLiteral("fld_%1").arg(fld)).split(QChar(9));
        Q_ASSERT(defs.count() > 4);
        BsField *bsFld = new BsField(fld,
                                     defs.at(0),
                                     QString(defs.at(3)).toUInt(),
                                     QString(defs.at(4)).toInt(),
                                     defs.at(2));
        resetFieldDotsDefin(bsFld);
        mFields << bsFld;

        //用户定义名称
        QString defKey = QStringLiteral("%1_%2").arg(tableName).arg(fld);
        if ( mapFldUserSetName.contains(defKey) )
            bsFld->mFldCnName = mapFldUserSetName.value(defKey);
    }

    //抬头
    QLabel* lblTitle = new QLabel(QStringLiteral("登记%1").arg(mCname), this);
    lblTitle->setStyleSheet(QLatin1String("QLabel {font-size:32px; color:grey;}"));

    //编辑件
    for ( int i = 0, iLen = mFields.length(); i < iLen; ++i ) {
        BsField *field = mFields.at(i);
        BsAbstractModel *pickData = nullptr;
        if ( field->mFldName == QStringLiteral("colortype") ) {
            pickData = dsColorType;
            dsColorType->reload();
        }
        if ( field->mFldName == QStringLiteral("sizertype") ) {
            pickData = dsSizerType;
            dsSizerType->reload();
        }
        BsFldBox *edt = new BsFldBox(this, field, pickData, false);
        edt->setMinimumWidth(500);
        mEditors << edt;
    }

    //提交按钮
    mpBtnCommit = new QPushButton(QStringLiteral("提交"), this);
    mpBtnCommit->setFixedSize(90, 30);

    //布局
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(30, 15, 30, 30);
    lay->addWidget(lblTitle, 0, Qt::AlignCenter);
    for ( int i = 0, iLen = mEditors.length(); i < iLen; ++i ) {
        lay->addSpacing(10);
        lay->addWidget(mEditors.at(i));
    }
    lay->addSpacing(10);
    lay->addWidget(mpBtnCommit, 0, Qt::AlignCenter);

    //win
    setMinimumSize(sizeHint());
    setWindowTitle(mCname);
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);

    connect(mpBtnCommit, &QPushButton::clicked, this, &BsWinReg::onClickCommit);
    connect(netSocket, &BsSocket::requestOk, this, &BsWinReg::onRequestOk);
}

BsWinReg::~BsWinReg()
{
    qDeleteAll(mEditors);
    mEditors.clear();

    qDeleteAll(mFields);
    mFields.clear();
}

void BsWinReg::onClickCommit()
{   
    //准备参数
    QString kvalue;
    QString required;

    mFlds.clear();
    mDataVals.clear();
    mSqlVals.clear();

    for ( int i = 0, iLen = mEditors.length(); i < iLen; ++i ) {
        BsFldBox *editor = mEditors.at(i);

        QString strField = editor->mpEditor->mpField->mFldName.toLower();
        QString strDatav = editor->mpEditor->getDataValue();
        QString strSqlv = editor->mpEditor->getDataValueForSql();
        bool isKeyField = ( strField == QStringLiteral("hpcode") || strField == QStringLiteral("kname") );

        //必填检查
        if ( isKeyField ) {
            if ( strDatav.isEmpty() ) {
                required = editor->mpEditor->mpField->mFldCnName;
                break;
            }
        }

        if ( mTable == QStringLiteral("cargo") ) {
            if ( strField == QStringLiteral("setprice") && strDatav.toLongLong() == 0 ) {
                required = editor->mpEditor->mpField->mFldCnName;
                break;
            }
            if ( strField == QStringLiteral("sizertype") && !editor->mpEditor->inputIsValid() ) {
                required = editor->mpEditor->mpField->mFldCnName;
                break;
            }
        }

        //逗号处理
        if ( strField == QStringLiteral("colortype") ) {
            strDatav = strDatav.replace(QStringLiteral("，"), QStringLiteral(","));
        }

        //收集
        mFlds << strField;
        mDataVals << ((isKeyField) ? strDatav.toUpper() : strDatav);
        mSqlVals << ((isKeyField) ? strSqlv.toUpper() : strSqlv);

        //主键
        if ( strField == QStringLiteral("kname") ) {
            kvalue = editor->mpEditor->getDataValue();
        }
    }

    //提示
    if ( !required.isEmpty() ) {
        QMessageBox::information(this, QString(), QStringLiteral("“%1”未填或无效！").arg(required));
        return;
    }

    //提交
    QStringList params;

    if ( mTable == QStringLiteral("cargo") ) {
        params << QStringLiteral("REGCARGO");
        params << QString::number(QDateTime::currentMSecsSinceEpoch() * 1000);
        params << QString();
        params << mFlds.join(QChar('\t'));
        params << mDataVals.join(QChar('\t'));
    }
    else {
        params << QStringLiteral("REGINSERT");  //原协议名，命名不太准确，但手机端已经使用
        params << QString::number(QDateTime::currentMSecsSinceEpoch() * 1000);
        params << mTable;
        params << QStringLiteral("insert");
        params << kvalue;
        params << mFlds.join(QChar('\t'));
        params << mDataVals.join(QChar('\t'));
    }

    QString waitMsg = QStringLiteral("正在提交，请稍侯……");
    netSocket->netRequest(this, params, waitMsg);
}

void BsWinReg::onRequestOk(const QWidget *sender, const QStringList &fens)
{
    if ( sender != this )
        return;

    if ( fens.at(0) == QStringLiteral("REGCARGO") || fens.at(0) == QStringLiteral("REGINSERT") ) {

        QString sql = QStringLiteral("insert into %1(%2) values(%3);")
                .arg(mTable).arg(mFlds.join(QChar(','))).arg(mSqlVals.join(QChar(',')));

        QSqlDatabase db = QSqlDatabase::database();
        db.exec(sql);
        if ( db.lastError().isValid() ) {
            qDebug() << db.lastError().text();
            qDebug() << sql;
            return;
        }

        if ( mTable == QStringLiteral("cargo") ) {
            dsCargo->reload();
        }

        if ( mTable == QStringLiteral("supplier") ) {
            dsSupplier->reload();
        }

        if ( mTable == QStringLiteral("customer") ) {
            dsCustomer->reload();
        }

        for ( int i = 0, iLen = mEditors.length(); i < iLen; ++i ) {
            BsFldBox *editor = mEditors.at(i);
            editor->mpEditor->clear();
        }

        QMessageBox::information(this, QString(), QStringLiteral("提交成功！"));
    }
}

}
