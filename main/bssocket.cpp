#include "bssocket.h"
#include "bailicode.h"
#include "bailifunc.h"
#include "bailidata.h"
#include "bsloading.h"
#include "third/tinyAES/aes.hpp"
#include <QCryptographicHash>
#include <QDateTime>

namespace BailiSoft {

BsLoading* netLoading = nullptr;
BsSocket*  netSocket = nullptr;

BsSocket::BsSocket(QObject *parent)
    : QAbstractSocket(QAbstractSocket::TcpSocket, parent)
{
    mBailiSiteUrl = QUrl(QStringLiteral("https://www.bailisoft.com"));

    mBeator.setInterval(5000);
    mBeator.setSingleShot(false);
    mBeator.stop();
    connect(&mBeator, &QTimer::timeout, this, &BsSocket::onBeatTimer);

    connect(this, &BsSocket::connected, this, &BsSocket::onConnected);
    connect(this, &BsSocket::disconnected, this, &BsSocket::onDisconnected);
    connect(this, &BsSocket::readyRead, this, &BsSocket::onReadyRead);
    //connect(this, &BsSocket::stateChanged, this, &BsSocket::onStateChanged);

    connect(this, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            [=](QAbstractSocket::SocketError socketError) {
        mBeator.stop();
        QString reason;
        if ( mLogined ) {
            reason = QStringLiteral("掉线");
        } else {
            reason = (socketError == QAbstractSocket::RemoteHostClosedError)
                    ? QStringLiteral("连接设置无效")
                    : QStringLiteral("后台故障");
        }
        qDebug() << "socket error: " << socketError;
        checkShowError(QStringLiteral("网络错误，错误码：%1").arg(socketError));
        emit netOfflined(reason);
    });
}

QByteArray BsSocket::calcShortMd5(const QString &text)
{
    return QCryptographicHash::hash(text.toUtf8(), QCryptographicHash::Md5).toHex().mid(8, 16);
}

//  1
void BsSocket::connectStart(const QString &backer, const QString &aesKey,
                            const QString &fronter, const QString &password)
{
    mBacker = backer;
    mAesKey = aesKey;
    mFronter = fronter;
    mPassword = password;

    mOnlining = false;
    mBailiSiteUrl = QStringLiteral("https://www.bailisoft.com/cmd/mids?backer=%1").arg(backer);
    QNetworkRequest request(mBailiSiteUrl);
    QSslConfiguration sslConf = QSslConfiguration::defaultConfiguration();
    sslConf.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConf.setProtocol(QSsl::TlsV1SslV3);
    request.setSslConfiguration(sslConf);

    netManager.clearAccessCache();
    QNetworkReply *netReply = netManager.get(request);
    connect(netReply, &QNetworkReply::finished, this, &BsSocket::onHttpLookupAddressFinished);
}

//工作请求公用
void BsSocket::netRequest(QWidget *sender, const QStringList &params, const QString &waitHint)
{
    mSender = sender;
    if ( sender != nullptr && !mOnlining ) {
        connectStart(mBacker, mAesKey, mFronter, mPassword);
        return;
    }

    mPrevReqTime = QDateTime::currentMSecsSinceEpoch();
    mThisReqTime = mPrevReqTime;
    QTimer::singleShot(15000, [&]{
        if ( mThisReqTime == mPrevReqTime ) {
            checkCloseLoading();
            QMessageBox::information(mSender, QString(), QStringLiteral("网络超时，请重新启动系统，然后再尝试！"));
            qApp->quit();
        }
    });

    QByteArray dataBody = upflowConvert(QString(params.join(QChar('\f'))));
    QByteArray lenHeaderBytes = QByteArray(4, '\0');
    qToBigEndian<quint32>(dataBody.length(), lenHeaderBytes.data());
    dataBody.prepend(lenHeaderBytes);
    writeWhole(dataBody);
    //qDebug() << "netRequest net hex:" << dataBody.toHex() << dataBody.length() << "bytes";

    if ( !waitHint.isEmpty() ) {
        netLoading = new BsLoading(waitHint);
        netLoading->setWindowFlags(netLoading->windowFlags() | Qt::FramelessWindowHint);
        netLoading->setAttribute(Qt::WA_DeleteOnClose);
        netLoading->exec();
    }
}

void BsSocket::requestLogin()
{
    qint64 reqId = QDateTime::currentMSecsSinceEpoch() * 1000;
    qint64 reqTm = (mLogined) ? QDateTime::currentMSecsSinceEpoch() : 0;
    QStringList params;
    params << QStringLiteral("LOGIN") << QString::number(reqId) << QString::number(reqTm) << QStringLiteral("desk");

    QString waitHint = (mLogined) ? QString() : QStringLiteral("登录中，请稍候……");

    netRequest(nullptr, params, waitHint);
}

void BsSocket::exitLogin()
{
    mBeator.stop();
    mLogined = false;
    disconnectFromHost();
}

//  2   -> connectToHost(公服中转上线)
void BsSocket::onHttpLookupAddressFinished()
{
    QNetworkReply *netReply = qobject_cast<QNetworkReply*>(sender());
    Q_ASSERT(netReply);
    netReply->deleteLater();

    //获取失败
    if ( netReply->error() ) {
        mTransferHost.clear();
        mTransferPort = 0;
    }
    else {
        //检查地址重定向
        QVariant redirectionTarget = netReply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        //无重定向
        if ( redirectionTarget.isNull() ) {
            QStringList flds = QString::fromLatin1(netReply->readAll()).split(QChar(':'));
            if ( flds.length() >= 2 ) {

                //连接公服，并设置网络触发事件
                mTransferHost = flds.at(0);
                mTransferPort = QString(flds.at(1)).toUShort();
                connectToHost(mTransferHost, mTransferPort);
                //qDebug() << "connectToHost " << mTransferHost << ":" << mTransferPort;
            }
            else {
                mTransferHost.clear();
                mTransferPort = 0;
            }
        }
        //有重定向，需重新请求
        else {
            mBailiSiteUrl = mBailiSiteUrl.resolved(redirectionTarget.toUrl());
            QNetworkRequest request(mBailiSiteUrl);
            QSslConfiguration sslConf = QSslConfiguration::defaultConfiguration();
            sslConf.setPeerVerifyMode(QSslSocket::VerifyNone);
            sslConf.setProtocol(QSsl::TlsV1SslV3);
            request.setSslConfiguration(sslConf);
            netManager.clearAccessCache();
            netReply = netManager.get(request);
            connect(netReply, &QNetworkReply::finished, this, &BsSocket::onHttpLookupAddressFinished);
            //qDebug() << "https redirected";
        }
    }
}

//  3   -> write(私服后台验证包)
void BsSocket::onConnected()
{
    //清空缓存
    mReadLen = 0;
    mReading.clear();

    //构造验证协议包
    QByteArray epochBytes = QByteArray(8, '\0');
    qint64 epochValue = QDateTime::currentMSecsSinceEpoch();
    qToBigEndian<qint64>(epochValue, epochBytes.data());
    QByteArray randomToken = generateRandomAsciiBytes(64);
    QByteArray backerHash = calcShortMd5(mBacker);
    QByteArray fronterHash = calcShortMd5(mFronter);

    QString vplain = fronterHash + backerHash + QString::number(epochValue) + mPassword.toUtf8() + randomToken;
    QByteArray vhash = QCryptographicHash::hash(vplain.toLatin1(), QCryptographicHash::Sha256).toHex();
    QByteArray req = fronterHash + backerHash + epochBytes + randomToken + vhash;

    //请求验证身份
    writeWhole(req);
}

void BsSocket::onDisconnected()
{
    qDebug() << "socket disconnected";
    mBeator.stop();
    mOnlining = false;
    emit netOfflined(QString());
}

//  4  -> 接收并解析数据
void BsSocket::onReadyRead()
{
    //读取数据
    mReading += readAll();    

    //理论上有可能包括多个任务数据，所以要用while
    while ( mReading.length() >= mReadLen + 4 ) {

        //取长度值
        mReadLen = qFromBigEndian<quint32>(mReading.left(4).constData());

        //收齐触发
        if ( mReading.length() >= mReadLen + 4 ) {

            //标记
            mPrevReqTime = 0;

            //取出数据
            QByteArray readyData = mReading.mid(4, mReadLen);
            mReading = mReading.mid(mReadLen + 4);  //待下一循环处理后续数据
            mReadLen = 0;

            //公服不返回错误，有错误服务器会主动Close同时以日志方式记录错误；这里则会有SocketError触发结束。
            //qDebug() << "read hex:" << readyData.toHex() << " string:" << QString::fromUtf8(readyData);

            //上线，非请求
            if ( readyData.startsWith("ON") ) {
                exitLogin();
                checkShowError(QStringLiteral("已有用户登录!"));
                return;
            }

            //上线，非请求
            if ( readyData.startsWith("OK") ) {
                if ( readyData.length() > 2 ) {
                    saveLostMessage(downflowConvert(readyData.mid(2)));
                }
                QTimer::singleShot(0, [=] { requestLogin(); });
                return;
            }

            //普通数据流程
            QString bodyText = downflowConvert(readyData.mid(1));
            QStringList fens = bodyText.split(QChar('\f'));
            QString reqName = fens.at(0);
            bool reqLogining = (reqName == QStringLiteral("LOGIN"));

            //qDebug() << "fens:" << (reqLogining ? (QStringList()<<"[LOGINDATA]") : fens);

            //群或消息没有OK结尾，先处理
            if ( readyData.startsWith('G') ) {
                checkCloseLoading();
                emit meetingEvent(fens);
                return;
            }

            if ( readyData.startsWith('M') ) {
                checkCloseLoading();
                emit msgArraried(fens);
                return;
            }

            //后台权限等原因拒绝
            QString result = fens.at(fens.length() - 1);
            if ( result != QStringLiteral("OK") ) {
                if ( reqLogining ) {
                    exitLogin();
                }
                if ( result.trimmed().isEmpty() ) {
                    checkCloseLoading();
                    //后台改动权限，但没有重新启动服务时，如果前端也不重新登录，则请求结果downFlowConvert
                    //会发生qUncompress异常后为空，暂时找不到原因，只好提示重新登录。
                    QMessageBox::information(mSender, QString(),
                                             QStringLiteral("后台配置变动，请重新登录！"));
                    qApp->quit();
                } else {
                    checkShowError(result);
                }
                return;
            }

            //以下逐命令全部处理

            if ( reqLogining ) {
                saveLoginData(fens.mid(2));
                mLogined = true;    //登录成功后，掉线不改变
                mOnlining = true;   //后期掉线，则恢复为false
                mBeator.start();    //启动心跳
            }

            if ( reqName == QStringLiteral("BIZOPEN") ) saveBizOpen(fens);
            if ( reqName.startsWith(QStringLiteral("QRY")) ) saveQueryDataset(fens);

            //关闭进度条
            checkCloseLoading();

            //通知
            emit requestOk(mSender, fens);
        }
    }
}

void BsSocket::onBeatTimer()
{
    if ( mOnlining && state() == QAbstractSocket::ConnectedState && bytesToWrite() == 0 ) {
        writeWhole(QByteArray("\0\0\0\0"));
    }
    else if ( state() == QAbstractSocket::UnconnectedState ) {
        mBeator.stop();
        mOnlining = false;
        emit netOfflined(QStringLiteral("掉线"));
    }
}

void BsSocket::checkCloseLoading()
{
    BsLoading *dlg = qobject_cast<BsLoading*>(netLoading);
    if ( dlg ) {
        dlg->accept();
        netLoading = nullptr;
    }
}

void BsSocket::checkShowError(const QString &errText)
{
    BsLoading *dlg = qobject_cast<BsLoading*>(netLoading);
    if ( dlg ) {
        dlg->setError(errText);
        netLoading = nullptr;
    }
}

//sheetid为0表示查询结果
QString BsSocket::createSaveTable(const QString &tblName, const QString &fen, const int sheetid)
{
    QStringList lines = fen.split(QChar('\n'));
    QStringList flds = QString(lines.at(0)).split(QChar('\t'));
    QStringList fldDefs;
    QStringList places;
    for ( int i = 0, iLen = flds.length(); i < iLen; ++i ) {
        QString fname = QString(flds.at(i)).toLower();
        QStringList defs = mapMsg.value(QStringLiteral("fld_%1").arg(fname)).split(QChar('\t'));
        Q_ASSERT(defs.length() > 1);
        fldDefs << QStringLiteral("%1 %2").arg(fname).arg(defs.at(1));
        places << QStringLiteral("?");
    }

    QSqlDatabase db = QSqlDatabase::database();

    //单据
    if ( sheetid > 0 ) {
        QString tkey = (tblName.endsWith(QStringLiteral("dtl")))
                ? QStringLiteral("parentid")
                : QStringLiteral("sheetid");
        db.exec(QStringLiteral("delete from %1 where %2=%3;").arg(tblName).arg(tkey).arg(sheetid));
        if ( db.lastError().isValid() ) return db.lastError().text();
    }
    //查询
    else {
        db.exec(QStringLiteral("drop table if exists %1;").arg(tblName));
        if ( db.lastError().isValid() ) return db.lastError().text();

        db.exec(QStringLiteral("create table if not exists %1(%2);").arg(tblName).arg(fldDefs.join(QChar(','))));
        if ( db.lastError().isValid() ) return db.lastError().text();
    }

    QSqlQuery qry(db);
    qry.prepare(QStringLiteral("insert into %1(%2) values(%3);")
                .arg(tblName)
                .arg(flds.join(QChar(',')))
                .arg(places.join(QChar(','))));

    for ( int i = 0, iLen = flds.length(); i < iLen; ++i ) {
        QVariantList vals;
        for ( int j = 1, jLen = lines.length(); j < jLen; ++j ) {
            QStringList cols = QString(lines.at(j)).split(QChar('\t'));
            if ( cols.length() == flds.length() ) {
                QString val = cols.at(i);
                if ( flds.at(i) == QStringLiteral("sizers") ) {
                    if ( ! val.startsWith(QChar('\r')) && sheetid == 0 ) {
                        //单据明细表名没有tmp开头，而所有查询临时表都以tmp开头
                        val.prepend("\r\v");  //reqQryPick返回结果已经sum过并且没有正负头
                    }
                    val.replace(QChar(';'), QChar('\n')).replace(QChar(':'), QChar('\t'));
                }
                vals << val;
            }
        }
        qry.addBindValue(vals);
    }
    if ( !qry.execBatch() ) return qry.lastError().text();

    return QString();
}

void BsSocket::saveLostMessage(const QString &msg)
{
    QStringList fens = msg.split(QChar('\f'));
    if ( fens.length() < 6 ) return;

    qint64 msgId = QString(fens.at(1)).toLongLong();
    QString senderId = fens.at(2);
    QString senderName = fens.at(3);
    QString receiverId = fens.at(4);
    QString msgText = fens.at(5);

    QString prepare = QStringLiteral("insert into msglog(msgid, senderid, sendername, "
                                     "receiverid, receivername, content) "
                                     "values(?, ?, ?, ?, '', ?);");
    QSqlQuery qry;
    qry.prepare(prepare);
    qry.bindValue(0, msgId);
    qry.bindValue(1, senderId);
    qry.bindValue(2, senderName);
    qry.bindValue(3, receiverId);
    qry.bindValue(4, msgText);
    qry.exec();
    if ( qry.lastError().isValid() ) qDebug() << qry.lastError() << "\t" << prepare;
}

//  5  -> 保存登录数据
void BsSocket::saveLoginData(const QStringList &fens)
{
    QSqlDatabase db = QSqlDatabase::database();
    if ( !db.isOpen() ) db.open();
    QSqlQuery qry(db);

    //加载入sqlite
    for ( int i = 0, iLen = fens.length(); i < iLen; ++i ) {
        QStringList lines = QString(fens.at(i)).split(QChar('\n'));

        //barcodeRules
        if (i == 0 && lines.length() > 1) {
            qry.prepare(QStringLiteral("insert or replace into barcodeRule"
                                       "(barcodexp,sizermiddlee,barcodemark)"
                                       "values(?, ?, ?);"));
            QVariantList barcodexps;
            QVariantList sizermiddlees;
            QVariantList barcodemarks;
            for ( int j = 1, jLen = lines.length(); j < jLen; ++j ) {
                QStringList cols = QString(lines.at(j)).split(QChar('\t'));
                if ( cols.length() >= 3 ) {
                    barcodexps << cols.at(0);
                    sizermiddlees << QString(cols.at(1)).toInt();
                    barcodemarks << cols.at(2);
                }
            }
            qry.addBindValue(barcodexps);
            qry.addBindValue(sizermiddlees);
            qry.addBindValue(barcodemarks);
            if (!qry.execBatch()) qDebug() << "execBatch barcodeRules:" << qry.lastError();
        }

        //sizertype
        if (i == 1 && lines.length() > 1) {
            qry.prepare(QStringLiteral("insert or replace into sizertype"
                                       "(tname,namelist,codelist)"
                                       "values(?, ?, ?);"));
            QVariantList tnames;
            QVariantList namelists;
            QVariantList codelists;
            for ( int j = 1, jLen = lines.length(); j < jLen; ++j ) {
                QStringList cols = QString(lines.at(j)).split(QChar('\t'));
                if ( cols.length() >= 3 ) {
                    tnames << cols.at(0);
                    namelists << cols.at(1);
                    codelists << cols.at(2);
                }
            }
            qry.addBindValue(tnames);
            qry.addBindValue(namelists);
            qry.addBindValue(codelists);
            if (!qry.execBatch()) qDebug() << "execBatch sizertype:" << qry.lastError();
        }

        //colortype
        if (i == 2 && lines.length() > 1) {
            qry.prepare(QStringLiteral("insert or replace into colortype"
                                       "(tname,namelist,codelist)"
                                       "values(?, ?, ?);"));
            QVariantList tnames;
            QVariantList namelists;
            QVariantList codelists;
            for ( int j = 1, jLen = lines.length(); j < jLen; ++j ) {
                QStringList cols = QString(lines.at(j)).split(QChar('\t'));
                if ( cols.length() >= 3 ) {
                    tnames << cols.at(0);
                    namelists << cols.at(1);
                    codelists << cols.at(2);
                }
            }
            qry.addBindValue(tnames);
            qry.addBindValue(namelists);
            qry.addBindValue(codelists);
            if (!qry.execBatch()) qDebug() << "execBatch colortype:" << qry.lastError();
        }

        //cargo
        if (i == 3 && lines.length() > 1) {
            qry.prepare(QStringLiteral("insert or replace into cargo"
                                       "(hpcode,hpname,sizertype,colortype,unit,setprice,buyprice,lotprice,retprice)"
                                       "values(?, ?, ?, ?, ?, ?, ?, ?, ?);"));
            QVariantList hpcodes;
            QVariantList hpnames;
            QVariantList sizertypes;
            QVariantList colortypes;
            QVariantList units;
            QVariantList setprices;
            QVariantList buyprices;
            QVariantList lotprices;
            QVariantList retprices;
            for ( int j = 1, jLen = lines.length(); j < jLen; ++j ) {
                QStringList cols = QString(lines.at(j)).split(QChar('\t'));
                if ( cols.length() >= 9 ) {
                    hpcodes << cols.at(0);
                    hpnames << cols.at(1);
                    sizertypes << cols.at(2);
                    colortypes << cols.at(3);
                    units << cols.at(4);
                    setprices << QString(cols.at(5)).toLongLong();
                    buyprices << QString(cols.at(6)).toLongLong();
                    lotprices << QString(cols.at(7)).toLongLong();
                    retprices << QString(cols.at(8)).toLongLong();
                }
            }
            qry.addBindValue(hpcodes);
            qry.addBindValue(hpnames);
            qry.addBindValue(sizertypes);
            qry.addBindValue(colortypes);
            qry.addBindValue(units);
            qry.addBindValue(setprices);
            qry.addBindValue(buyprices);
            qry.addBindValue(lotprices);
            qry.addBindValue(retprices);
            if (!qry.execBatch()) qDebug() << "execBatch cargo:" << qry.lastError();
        }

        //shop
        if (i == 4 && lines.length() > 1) {
            qry.prepare(QStringLiteral("insert or replace into shop"
                                       "(kname,regdis,regman,regaddr,regtele)"
                                       "values(?, ?, ?, ?, ?);"));
            QVariantList knames;
            QVariantList regdiss;
            QVariantList regmans;
            QVariantList regaddrs;
            QVariantList regteles;
            for ( int j = 1, jLen = lines.length(); j < jLen; ++j ) {
                QStringList cols = QString(lines.at(j)).split(QChar('\t'));
                if ( cols.length() >= 5 ) {
                    knames << cols.at(0);
                    regdiss << QString(cols.at(1)).toLongLong();
                    regmans << cols.at(2);
                    regaddrs << cols.at(3);
                    regteles << cols.at(4);
                }
            }
            qry.addBindValue(knames);
            qry.addBindValue(regdiss);
            qry.addBindValue(regmans);
            qry.addBindValue(regaddrs);
            qry.addBindValue(regteles);
            if (!qry.execBatch()) qDebug() << "execBatch shop:" << qry.lastError();
        }

        //customer
        if (i == 5 && lines.length() > 1) {
            qry.prepare(QStringLiteral("insert or replace into customer"
                                       "(kname,regdis,regman,regaddr,regtele)"
                                       "values(?, ?, ?, ?, ?);"));
            QVariantList knames;
            QVariantList regdiss;
            QVariantList regmans;
            QVariantList regaddrs;
            QVariantList regteles;
            for ( int j = 1, jLen = lines.length(); j < jLen; ++j ) {
                QStringList cols = QString(lines.at(j)).split(QChar('\t'));
                if ( cols.length() >= 5 ) {
                    knames << cols.at(0);
                    regdiss << QString(cols.at(1)).toLongLong();
                    regmans << cols.at(2);
                    regaddrs << cols.at(3);
                    regteles << cols.at(4);
                }
            }
            qry.addBindValue(knames);
            qry.addBindValue(regdiss);
            qry.addBindValue(regmans);
            qry.addBindValue(regaddrs);
            qry.addBindValue(regteles);
            if (!qry.execBatch()) qDebug() << "execBatch customer:" << qry.lastError();
        }

        //supplier
        if (i == 6 && lines.length() > 1) {
            qry.prepare(QStringLiteral("insert or replace into supplier"
                                       "(kname,regdis,regman,regaddr,regtele)"
                                       "values(?, ?, ?, ?, ?);"));
            QVariantList knames;
            QVariantList regdiss;
            QVariantList regmans;
            QVariantList regaddrs;
            QVariantList regteles;
            for ( int j = 1, jLen = lines.length(); j < jLen; ++j ) {
                QStringList cols = QString(lines.at(j)).split(QChar('\t'));
                if ( cols.length() >= 5 ) {
                    knames << cols.at(0);
                    regdiss << QString(cols.at(1)).toLongLong();
                    regmans << cols.at(2);
                    regaddrs << cols.at(3);
                    regteles << cols.at(4);
                }
            }
            qry.addBindValue(knames);
            qry.addBindValue(regdiss);
            qry.addBindValue(regmans);
            qry.addBindValue(regaddrs);
            qry.addBindValue(regteles);
            if (!qry.execBatch()) qDebug() << "execBatch supplier:" << qry.lastError();
        }

        //staff
        if (i == 7 && lines.length() > 1) {
            qry.prepare(QStringLiteral("insert or replace into staff"
                                       "(kname)"
                                       "values(?);"));
            QVariantList knames;
            for ( int j = 1, jLen = lines.length(); j < jLen; ++j ) {
                QStringList cols = QString(lines.at(j)).split(QChar('\t'));
                if ( cols.length() >= 1 ) {
                    knames << cols.at(0);
                }
            }
            qry.addBindValue(knames);
            if (!qry.execBatch()) qDebug() << "execBatch staff:" << qry.lastError();
        }

        //subject
        if ( i == 8 && lines.length() > 1 ) {
            qry.prepare(QStringLiteral("insert or replace into subject"
                                       "(kname)"
                                       "values(?);"));
            QVariantList knames;
            for ( int j = 1, jLen = lines.length(); j < jLen; ++j ) {
                QStringList cols = QString(lines.at(j)).split(QChar('\t'));
                if ( cols.length() >= 1 ) {
                    knames << cols.at(0);
                }
            }
            qry.addBindValue(knames);
            if (!qry.execBatch()) qDebug() << "execBatch subject:" << qry.lastError();
        }

        //bailioptions - stypes
        if ( i == 9 && lines.length() == 11 ) {
            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='stypes_cgd';"));
            qry.bindValue(0, lines.at(1));
            if (!qry.exec()) qDebug() << "exec bailioption_cgd:" << qry.lastError();

            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='stypes_cgj';"));
            qry.bindValue(0, lines.at(2));
            if (!qry.exec()) qDebug() << "exec bailioption_cgj:" << qry.lastError();

            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='stypes_cgt';"));
            qry.bindValue(0, lines.at(3));
            if (!qry.exec()) qDebug() << "exec bailioption_cgt:" << qry.lastError();

            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='stypes_dbd';"));
            qry.bindValue(0, lines.at(4));
            if (!qry.exec()) qDebug() << "exec bailioption_dbd:" << qry.lastError();

            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='stypes_syd';"));
            qry.bindValue(0, lines.at(5));
            if (!qry.exec()) qDebug() << "exec bailioption_syd:" << qry.lastError();

            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='stypes_pfd';"));
            qry.bindValue(0, lines.at(6));
            if (!qry.exec()) qDebug() << "exec bailioption_pfd:" << qry.lastError();

            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='stypes_pff';"));
            qry.bindValue(0, lines.at(7));
            if (!qry.exec()) qDebug() << "exec bailioption_pff:" << qry.lastError();

            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='stypes_pft';"));
            qry.bindValue(0, lines.at(8));
            if (!qry.exec()) qDebug() << "exec bailioption_pft:" << qry.lastError();

            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='stypes_lsd';"));
            qry.bindValue(0, lines.at(9));
            if (!qry.exec()) qDebug() << "exec bailioption_lsd:" << qry.lastError();

            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='stypes_szd';"));
            qry.bindValue(0, lines.at(10));
            if (!qry.exec()) qDebug() << "exec bailioption_szd:" << qry.lastError();
        }

        //bailioptions - other
        if (i == 10 && lines.length() == 8) {
            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='dots_of_qty';"));
            qry.bindValue(0, QString(lines.at(1)).split(QChar('\t')).at(1));
            if (!qry.exec()) qDebug() << "exec bailioption_qtydots:" << qry.lastError();

            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='dots_of_price';"));
            qry.bindValue(0, QString(lines.at(2)).split(QChar('\t')).at(1));
            if (!qry.exec()) qDebug() << "exec bailioption_pricedots:" << qry.lastError();

            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='dots_of_money';"));
            qry.bindValue(0, QString(lines.at(3)).split(QChar('\t')).at(1));
            if (!qry.exec()) qDebug() << "exec bailioption_mnydots:" << qry.lastError();

            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='dots_of_discount';"));
            qry.bindValue(0, QString(lines.at(4)).split(QChar('\t')).at(1));
            if (!qry.exec()) qDebug() << "exec bailioption_disdots:" << qry.lastError();

            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='app_company_name';"));
            qry.bindValue(0, QString(lines.at(5)).split(QChar('\t')).at(1));
            if (!qry.exec()) qDebug() << "exec bailioption_comname:" << qry.lastError();

            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='app_company_pcolor';"));
            qry.bindValue(0, QString(lines.at(6)).split(QChar('\t')).at(1));
            if (!qry.exec()) qDebug() << "exec bailioption_comcolor:" << qry.lastError();

            qry.prepare(QStringLiteral("update bailioption set vsetting=? where optcode='app_company_plogo';"));
            qry.bindValue(0, QString(lines.at(7)).split(QChar('\t')).at(1));
            if (!qry.exec()) qDebug() << "exec bailioption_comlogo:" << qry.lastError();
        }

        //自身权限
        if (i == 11 && lines.length() > 1) {

            QStringList flds = QString(lines.at(0)).split(QChar('\t'));
            QStringList cols = QString(lines.at(1)).split(QChar('\t'));

            const int right_fields = 39; //注意与后台协议列数及顺序
            if (flds.length() == right_fields && cols.length() == right_fields) {
                //因为bindShop、bindCustomer、bindSupplier只会有一个非空
                loginTrader = QString(cols.at(0)) + QString(cols.at(1)) + QString(cols.at(2));
                QStringList places;
                for ( int j = 0; j < right_fields; ++j ) {
                    places << QStringLiteral("?");
                }
                QString prepare = QStringLiteral("insert or replace into baililoginer"
                                                 "(%1) values(%2);")
                                  .arg(flds.join(QChar(',')))
                                  .arg(places.join(QChar(',')));
                qry.prepare(prepare);
                for ( int j = 0; j < right_fields; ++j ) {
                    if ( j < 6 ) {
                        qry.bindValue(j, cols.at(j));
                    } else {
                        qry.bindValue(j, QString(cols.at(j)).toLongLong());
                    }
                }
                if (!qry.exec()) qDebug() << "exec self_right:" << qry.lastError();
            }
        }

        //群资料
        if (i == 12 && lines.length() > 1) {
            qry.prepare(QStringLiteral("insert or replace into meeting"
                                       "(meetid, meetname) values(?, ?);"));
            QVariantList meetids;
            QVariantList meetnames;
            for ( int j = 1, jLen = lines.length(); j < jLen; ++j ) {
                QStringList cols = QString(lines.at(j)).split(QChar('\t'));
                if ( cols.length() >= 2 ) {
                    meetids << cols.at(0);
                    meetnames << cols.at(1);
                }
            }
            qry.addBindValue(meetids);
            qry.addBindValue(meetnames);
            if (!qry.execBatch()) qDebug() << "execBatch meeting:" << qry.lastError();
        }

        //单聊人（i == 13 忽略，因为桌面端没有总经理登录）

        //离线消息，先取为后台离线部分，后面再加载本地及公服最早一条
        if (i == 14 && lines.length() > 1) {
            qry.prepare(QStringLiteral("insert or replace into msglog"
                                       "(msgid, senderid, sendername, receiverid, receivername, content)"
                                       "values(?, ?, ?, ?, '', ?);"));
            QVariantList msgids;
            QVariantList senderids;
            QVariantList sendernames;
            QVariantList receiverids;
            QVariantList contents;
            for ( int j = 1, jLen = lines.length(); j < jLen; ++j ) {
                QStringList cols = QString(lines.at(j)).split(QChar('\t'));
                if ( cols.length() >= 5 ) {
                    msgids << QString(cols.at(0)).toLongLong();
                    senderids << cols.at(1);
                    sendernames << cols.at(2);
                    receiverids << cols.at(3);
                    contents << cols.at(4);
                }
            }
            qry.addBindValue(msgids);
            qry.addBindValue(senderids);
            qry.addBindValue(sendernames);
            qry.addBindValue(receiverids);
            qry.addBindValue(contents);
            if (!qry.execBatch()) qDebug() << "execBatch msglog:" << qry.lastError();
        }

        //价格政策（迭代增加内容在此以后）
        if ( i == 15 && lines.length() > 1 ) {

            qDeleteAll(pricePolicies);
            pricePolicies.clear();

            for ( int j = 1, jLen = lines.length(); j < jLen; ++j ) {
                QStringList cols = QString(lines.at(j)).split(QChar('\t'));
                if ( cols.length() >= 6 ) {
                    QDateTime dtStart = QDateTime::fromSecsSinceEpoch(QString(cols.at(4)).toLongLong());
                    QDateTime dtEnd   = QDateTime::fromSecsSinceEpoch(QString(cols.at(5)).toLongLong());
                    pricePolicies << new BsPolicy(
                                         cols.at(0),
                                         cols.at(1),
                                         QString(cols.at(2)).toLongLong() / 10000.0,
                                         dtStart.date(),
                                         dtEnd.date()
                                         );
                }
            }
        }

        //总经理账号
        if ( i == 16 ) {
            bossAccount = QString(fens.at(i));
        }
    }
}

void BsSocket::saveQueryDataset(const QStringList &fens)
{
    Q_ASSERT(fens.length() >= 4);
    QString err = createSaveTable(QStringLiteral("tmp_query_dataset"), fens.at(2), 0);
    if ( !err.isEmpty() ) qDebug() << err;
}

void BsSocket::saveBizOpen(const QStringList &fens)
{
    Q_ASSERT(fens.length() >= 7);

    int sheetid = QString(fens.at(3)).trimmed().toInt();

    QString tname = fens.at(2);
    QString err = createSaveTable(tname, fens.at(4), sheetid);
    if ( !err.isEmpty() ) qDebug() << err;

    err = createSaveTable(QStringLiteral("%1dtl").arg(tname), fens.at(5), sheetid);
    if ( !err.isEmpty() ) qDebug() << err;
}

QByteArray BsSocket::upflowConvert(const QString &data)
{
    return dataEncrypt(dataDozip(data.toUtf8()));
}

QString BsSocket::downflowConvert(const QByteArray &data)
{
    if ( data.isEmpty() ) return data;
    return dataUnzip(dataDecrypt(data));
}

QByteArray BsSocket::dataDecrypt(const QByteArray &data)
{
    if ( mAesKey.isEmpty() )
        return data;

    if ( data.length() <= AES_BLOCKLEN || (data.length() % AES_BLOCKLEN) )
        return QByteArray();

    QByteArray aesKeyBytes = QCryptographicHash::hash(mAesKey.toLatin1(), QCryptographicHash::Md5);

    //Header block is IV data, so buffLen minus one block size.
    size_t buffLen = size_t(data.length()) - AES_BLOCKLEN;
    uint8_t iv[ AES_BLOCKLEN ];
    memcpy(iv, data.left(AES_BLOCKLEN).constData(), AES_BLOCKLEN);
    struct AES_ctx ctx;

    //malloc memory
    void *heap = malloc(buffLen);
    uint8_t *buff = reinterpret_cast<uint8_t *>(heap);
    memcpy(buff, data.mid(AES_BLOCKLEN).constData(), buffLen);

    //tinyAES does not throw exception, but return empty result.
    try {
        //Flutter's encrypt package uses PKCS7 padding which just as tinyAES.
        AES_init_ctx_iv(&ctx, (uint8_t *)(aesKeyBytes.constData()), iv);
        AES_CBC_decrypt_buffer(&ctx, buff, uint32_t(buffLen));
    }
    catch (...) {
        buff[0] = 0;
    }

    //deep copy then free
    QByteArray result = QByteArray(reinterpret_cast<char*>(buff), int(buffLen));
    free(heap);
    heap = nullptr;
    return result;
}

QByteArray BsSocket::dataEncrypt(const QByteArray &data)
{
    if ( mAesKey.isEmpty() )
        return data;

    if ( data.isEmpty() )
        return QByteArray();

    QByteArray aesKeyBytes = QCryptographicHash::hash(mAesKey.toLatin1(), QCryptographicHash::Md5);

    int dataLen = data.length();
    int padding = AES_BLOCKLEN - (dataLen % AES_BLOCKLEN);
    int buffLen = dataLen + padding;
    struct AES_ctx ctx;

    //malloc memory
    void *heap = malloc(AES_BLOCKLEN + size_t(buffLen));                     //Prepending header block of IV data
    uint8_t *buff = reinterpret_cast<uint8_t *>(heap);

    memset(buff + AES_BLOCKLEN + dataLen, padding, size_t(padding));            //PKCS7 padding
    memcpy(buff, generateRandomBytes(AES_BLOCKLEN).constData(), AES_BLOCKLEN);  //prepend IV header
    memcpy(buff + AES_BLOCKLEN, data.constData(), size_t(dataLen));             //data

    AES_init_ctx_iv(&ctx, (uint8_t *)(aesKeyBytes.constData()), buff);
    AES_CBC_encrypt_buffer(&ctx, buff + AES_BLOCKLEN, uint32_t(buffLen));       //第三参数应为buffLen，而不是dataLen！

    //deep copy then free
    QByteArray result = QByteArray(reinterpret_cast<char*>(buff), AES_BLOCKLEN + buffLen);
    free(heap);
    heap = nullptr;

    //return
    return result;
}

QByteArray BsSocket::dataDozip(const QByteArray &data)
{
    return qCompress(data);
}

QByteArray BsSocket::dataUnzip(const QByteArray &data)
{
    QByteArray result;
    try {
        result = qUncompress(data);
    }
    catch (...) {
        result = QByteArray();
    }
    return result;
}

void BsSocket::writeWhole(const QByteArray &byteArray)
{
    const int total = byteArray.length();
    int sends = 0;
    while ( sends < total ) {
        sends += write(byteArray.mid(sends));
    }
}

}
