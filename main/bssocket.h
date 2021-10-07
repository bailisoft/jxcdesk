#ifndef BSSOCKET_H
#define BSSOCKET_H

#include <QtCore>
#include <QtNetwork>
#include <QtSql>
#include <QTimer>

namespace BailiSoft {

class BsSocket : public QAbstractSocket
{
    Q_OBJECT
public:
    explicit BsSocket(QObject *parent);

    static QByteArray calcShortMd5(const QString &text);
    void connectStart(const QString &backer, const QString &aesKey,
                      const QString &fronter, const QString &password);
    void netRequest(QWidget *sender, const QStringList &params, const QString &waitHint);
    void requestLogin();
    void exitLogin();

signals:
    void netOfflined(const QString &reason);
    void msgArraried(const QStringList &fens);
    void meetingEvent(const QStringList &fens);
    void requestOk(const QWidget*sender, const QStringList &fens);

private:
    void onHttpLookupAddressFinished();
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onBeatTimer();

    void checkCloseLoading();
    void checkShowError(const QString &errText);
    QString createSaveTable(const QString &tblName, const QString &fen, const int sheetid = 0);

    void saveLostMessage(const QString &msg);
    void saveLoginData(const QStringList &fens);
    void saveQueryDataset(const QStringList &fens);
    void saveBizOpen(const QStringList &fens);

    QByteArray upflowConvert(const QString &data);
    QString downflowConvert(const QByteArray &data);

    QByteArray dataDecrypt(const QByteArray &data);
    QByteArray dataEncrypt(const QByteArray &data);
    QByteArray dataDozip(const QByteArray &data);
    QByteArray dataUnzip(const QByteArray &data);

    void writeWhole(const QByteArray &byteArray);

    QTimer          mBeator;

    QUrl            mBailiSiteUrl;
    QString         mTransferHost;
    quint16         mTransferPort;

    QString         mBacker;
    QString         mAesKey;
    QString         mFronter;
    QString         mPassword;

    int             mReadLen;   //数据长度头
    QByteArray      mReading;   //读缓存buffer
    QByteArray      mLostMsg;

    bool            mLogined = false;       //登录成功后，掉线不改变
    bool            mOnlining = false;      //后期掉线，则恢复为false
    qint64          mPrevReqTime = 0;
    qint64          mThisReqTime = 0;

    QWidget*        mSender = nullptr;

};

extern BsSocket*  netSocket;

}

#endif // BSSOCKET_H
