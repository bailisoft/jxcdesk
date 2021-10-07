#ifndef LXWELCOME_H
#define LXWELCOME_H

#include <QtWidgets>
#include <QtNetwork>

#define LXAPP_VERSION_MAJOR     17
#define LXAPP_VERSION_MINOR     1
#define LXAPP_VERSION_PATCH     20

/*
18.前端登记检查、前端登记后及时生效
18.前端零库存
18.前端图片
20...配合后端总经理账号名自由设置
20.OK网络换协议https以及更换中服寻址服务
*/


namespace BailiSoft {

enum bsVersionReqType { bsverNum, bsVerDoc, bsverDown };

class LxProgressBar;

class LxWelcome : public QWidget
{
    Q_OBJECT
public:
    explicit LxWelcome();
    bool    mCanOver;
    bool    mNeedQuit;

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

private slots:
    void doNetCheckStart();
    void doClickLeft();
    void doClickRight();
    void doClickUpgradeDoc();
    void doSimulateLoadOver();

    void httpReadyRead();
    void httpReadProgress(qint64 bytesRead, qint64 totalBytes);
    void httpFinished();
    void httpError(QNetworkReply::NetworkError code);
    void httpSslErrors(const QList<QSslError> &errors);

private:
    void startRequest(QUrl prUrl, const bsVersionReqType reqType);

    void execCheckVersionSuccess();
    void execCheckVersionFail();

    void execGetUpgradeDocSuccess();
    void execGetUpgradeDocFail();

    void execUpgradeDownAbort();
    void execUpgradeDownSuccess();
    void execUpgradeDownFail(const QString &prErr);

    bool needUpgradeNow();

    bsVersionReqType    mCurrentReqType;
    int                 mNetMajorNum;
    int                 mNetMinorNum;
    int                 mNetPatchNum;
    QString             mNetVerFile;
    QString             mNetDocFile;
    QString             mNetAppFile;

    QLabel          *mpLblImg;
    QLabel          *mpBuildNum;
    LxProgressBar   *mpProgress;
    QToolButton     *mpBtnLeft;
    QToolButton     *mpBtnRight;

    qint64          mTimeStart; //记录网络版本成功获取时间（如果时间太短，使用模拟加载等待3秒钟效果）

    QUrl                    mUsingUrl;
    QNetworkReply          *mpNetReply;
    QByteArray              mBytesVernum;
    QByteArray              mBytesUpgdoc;
    QFile                  *mpFileDown;
    bool                    mDownFileAborted;
};

class LxProgressBar : public QProgressBar
{
    Q_OBJECT
public:
    explicit LxProgressBar(QWidget *parent);

    void showMessage(const QString &prMsg, const bool prFlatt);

protected:
    void paintEvent(QPaintEvent *e);

private:
    QString     mShowMsg;

};

}

#endif // LXWELCOME_H
