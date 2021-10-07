#include "lxwelcome.h"
#include "main/bailifunc.h"

#define STARTING_NOW            "程序加载中，请稍候……"
#define UPGRADE_NOW             "现在升级"
#define IGNORE_FOR_NEXT         "下次"
#define RETRY_NET_LINE          "重试网络"
#define RETRY_UPGRAD_DOWN       "重试下载"
#define UPGRADE_CHANGE_DOC      "本次升级说明"
#define RETRY_NET_WAITING       "重新获取版本号，请稍候……"
#define GETVERNUM_FAILED        "网络故障，无法从www.bailisoft.com获取版本号。"
#define FOUND_NEW_VERSION       "本程序有更新版本，是否现在执行升级？"
#define WAIT_UPGRADING          "新版程序下载中，请稍候……"
#define DOWNLOAD_FILE_FAIL      "新版程序下载失败，请下次重试！"
#define CREATE_FILE_FORBIDDEN   "您没有改变程序的权限！"
#define NOTFOUND_LAST_UPGRADE   "本机中未发现有执行过升级的记录，撤消升级未进行！"
#define UPGRADEOK_NEED_RESTART  "升级成功！请重新运行本程序。"

namespace BailiSoft {

LxWelcome::LxWelcome() : QWidget(nullptr)
{
    mCanOver  = false;
    mNeedQuit = false;
    mCurrentReqType = bsverNum;
    mNetMajorNum = LXAPP_VERSION_MAJOR;
    mNetMinorNum = -1;
    mNetPatchNum = -1;
    mpFileDown = nullptr;
    mDownFileAborted = false;

    mNetDocFile = "https://www.bailisoft.com/download/bljydesk_version.txt";
#ifdef Q_OS_WIN
    mNetVerFile = "https://www.bailisoft.com/download/BR17DeskWinVer.txt";
    mNetAppFile = "https://www.bailisoft.com/download/BailiR17DeskWin";
#elif defined(Q_OS_MAC)
    mNetVerFile = "https://www.bailisoft.com/download/BR17DeskMacVer.txt";
    mNetAppFile = "https://www.bailisoft.com/download/BailiR17DeskMac";
#else
    mNetVerFile = "https://www.bailisoft.com/download/BR17DeskUnxVer.txt";
    mNetAppFile = "https://www.bailisoft.com/download/BailiR17DeskUnx";
#endif

    //网站放置升级程序文件后缀一律使用.exe，下载后更改。
    if ( sizeof(int *) == 4 )
        mNetAppFile += "32.exe";
    else
        mNetAppFile += "64.exe";

    //主图
    mpLblImg = new QLabel(this);
    mpLblImg->setPixmap(QPixmap(":/image/welcome.png"));
    mpLblImg->setAlignment(Qt::AlignCenter);
    mpLblImg->setFixedSize(640, 360);
    mpLblImg->setStyleSheet(" QLabel {background:white;}");

    //编译版本号
    mpBuildNum = new QLabel(this);
    mpBuildNum->setText(QStringLiteral("%1.%2.%3")
                        .arg(LXAPP_VERSION_MAJOR)
                        .arg(LXAPP_VERSION_MINOR)
                        .arg(LXAPP_VERSION_PATCH));
    mpBuildNum->setStyleSheet("QLabel {color:#ffffff;}");

    //进度等待条
    mpProgress = new LxProgressBar(this);

    //两按钮变化用途，分别用左右命名
    QMenu *pMenu = new QMenu(this);
    QAction *pAcUpgdoc = pMenu->addAction(QIcon(":/icon/help.png"), QStringLiteral(UPGRADE_CHANGE_DOC));
    connect(pAcUpgdoc, SIGNAL(triggered()), this, SLOT(doClickUpgradeDoc()));
    mpBtnLeft = new QToolButton(this);
    mpBtnLeft->setText(QStringLiteral(UPGRADE_NOW));
    mpBtnLeft->setToolButtonStyle(Qt::ToolButtonTextOnly);
    mpBtnLeft->setPopupMode(QToolButton::MenuButtonPopup);
    connect(mpBtnLeft, SIGNAL(clicked()), this, SLOT(doClickLeft()));
    mpBtnLeft->setMenu(pMenu);

    mpBtnRight  = new QToolButton(this);
    mpBtnRight->setText(QStringLiteral(IGNORE_FOR_NEXT));
    mpBtnRight->setIcon(QIcon(":/icon/go.png"));
    mpBtnRight->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    connect(mpBtnRight,  SIGNAL(clicked()), this, SLOT(doClickRight()));

    QWidget *pPnlBottom = new QWidget(this);
    QHBoxLayout *pLayBottom = new QHBoxLayout(pPnlBottom);
    pLayBottom->setSpacing(3);
    pLayBottom->addWidget(mpProgress);
    pLayBottom->addWidget(mpBtnLeft);
    pLayBottom->addWidget(mpBtnRight);
    pLayBottom->setContentsMargins(3, 3, 3, 3);
    pPnlBottom->setStyleSheet(".QWidget {background:#ccc;} ");

    //总布局
    QVBoxLayout *pLayAll = new QVBoxLayout(this);
    pLayAll->setSpacing(0);
    pLayAll->addWidget(mpLblImg);
    pLayAll->addWidget(pPnlBottom);
    pLayAll->setContentsMargins(1, 1, 1, 1);   //1为paintEvent中用灰fillRect(全部背景)后露出的边宽
    mpBtnLeft->hide();
    mpBtnRight->hide();

    //窗口属性
    setWindowFlags( Qt::SplashScreen | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    //启动检查，为显示节目，必须保证在窗口显示之后启动
    QTimer::singleShot(1001, this, SLOT(doNetCheckStart()));
}

void LxWelcome::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    QPainter p(this);
    p.fillRect(rect(), Qt::darkGray);
}

void LxWelcome::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    mpBuildNum->setGeometry(540, 200, 40, 20);
}

void LxWelcome::doNetCheckStart()
{
    //连网，从检测版本号开始
    mpProgress->setMinimum(0);
    mpProgress->setMaximum(0);

    mUsingUrl = mNetVerFile;
    mNetMajorNum = LXAPP_VERSION_MAJOR;
    mNetMinorNum = -1;
    mNetPatchNum = -1;
    mpFileDown = nullptr;

    startRequest(mUsingUrl, bsverNum);
}

void LxWelcome::doClickLeft()
{
    mpFileDown = nullptr;
    mDownFileAborted = false;
    mpBtnLeft->hide();

    //网络后通会到此
    if ( mNetMajorNum == LXAPP_VERSION_MAJOR && mNetMinorNum < 0 && mNetPatchNum < 0 ) {
        mpProgress->showMessage(QStringLiteral(RETRY_NET_WAITING), true);        
        mpBtnRight->hide();
        doNetCheckStart();
    }
    //请求下载
    else if ( needUpgradeNow() ) {
        mpProgress->showMessage(QStringLiteral(WAIT_UPGRADING), false);
        mpBtnRight->setText(QStringLiteral("取消"));
        mpBtnRight->setIcon(QIcon(":/icon/del.png"));
        mUsingUrl = mNetAppFile;
        startRequest(mUsingUrl, bsverDown);
    }
}

void LxWelcome::doClickRight()
{
    if ( mNetMajorNum == LXAPP_VERSION_MAJOR && mNetMinorNum >= 0 && mNetPatchNum >= 0
         && mpFileDown && mpFileDown->isOpen() ) {
        execUpgradeDownAbort();
    } else {
        mCanOver = true;
    }
}

void LxWelcome::doClickUpgradeDoc()
{
    mUsingUrl = mNetDocFile;
    startRequest(mUsingUrl, bsVerDoc);
}

void LxWelcome::doSimulateLoadOver()
{
    mCanOver = true;
}

void LxWelcome::httpReadyRead()
{
    switch (mCurrentReqType) {
    case bsverNum:
        mBytesVernum.append(mpNetReply->readAll());
        break;
    case bsVerDoc:
        mBytesUpgdoc.append(mpNetReply->readAll());
        break;
    default:
        mpFileDown->write(mpNetReply->readAll());
    }
}

void LxWelcome::httpReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if ( mCurrentReqType == bsverDown && needUpgradeNow() && !mDownFileAborted ) {
        mpProgress->setMaximum(int(totalBytes));
        mpProgress->setValue(int(bytesRead));
        mpBtnRight->hide();
    }
}

void LxWelcome::httpFinished()
{
    //用户中断结束
    if (mDownFileAborted) {
        execUpgradeDownFail("");
        mCanOver = true;    //结束停留
        return;
    }

    //正常关闭结束
    if ( mpFileDown ) {
        mpFileDown->flush();
        mpFileDown->close();
    }

    //检测网络应答，分三种情况
    QVariant redirectionTarget = mpNetReply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    int iNetErrNo = mpNetReply->error();
    //获取失败
    if ( iNetErrNo ) {
        switch (mCurrentReqType) {
        case bsverNum:
            execCheckVersionFail();
            break;
        case bsVerDoc:
            execGetUpgradeDocFail();
            break;
        default:
            execUpgradeDownFail(QStringLiteral(DOWNLOAD_FILE_FAIL));
        }
    }
    //地址有重定向，需要重新进行网络请求
    else if (!redirectionTarget.isNull()) {
        mUsingUrl = mUsingUrl.resolved(redirectionTarget.toUrl());
        startRequest(mUsingUrl, mCurrentReqType);
        return;
    }
    //成功完成
    else {
        switch (mCurrentReqType) {
        case bsverNum:
            execCheckVersionSuccess();
            break;
        case bsVerDoc:
            execGetUpgradeDocSuccess();
            break;
        default:
            execUpgradeDownSuccess();
        }
    }
}

void LxWelcome::httpError(QNetworkReply::NetworkError code)
{
    qDebug() << "httpError:" << code;
}

void LxWelcome::httpSslErrors(const QList<QSslError> &errors)
{
    for ( int i = 0, iLen = errors.length(); i < iLen; ++i ) {
        qDebug() << "httpSslErrors " << i << ":" << errors;
    }
}

void LxWelcome::startRequest(QUrl prUrl, const bsVersionReqType reqType)
{
    mCurrentReqType = reqType;

    //准备接受数据变量
    switch (reqType) {
    case bsverNum:
        mBytesVernum.clear();
        mTimeStart = QDateTime::currentMSecsSinceEpoch() / 1000;
        break;
    case bsVerDoc:
        mBytesUpgdoc.clear();
        mTimeStart = QDateTime::currentMSecsSinceEpoch() / 1000;
        break;
    default:
        if ( ! mpFileDown ) {
            mpFileDown = new QFile(qApp->arguments().at(0) + ".tmp");
            if ( mpFileDown->exists() ) mpFileDown->remove();
            if ( !mpFileDown->open(QIODevice::WriteOnly) ) {
                delete mpFileDown;
                mpFileDown = nullptr;
                execUpgradeDownFail(CREATE_FILE_FORBIDDEN);
                return;
            }
        }
        else {
            if ( !mpFileDown->isOpen() ) mpFileDown->open(QIODevice::WriteOnly);
            mpFileDown->resize(0);
        }
        mTimeStart = 0;
    }

    //开始读请求
    mDownFileAborted = false;
    QNetworkRequest request(prUrl);
    QSslConfiguration sslConf = QSslConfiguration::defaultConfiguration();
    sslConf.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConf.setProtocol(QSsl::TlsV1SslV3);
    request.setSslConfiguration(sslConf);

    BailiSoft::netManager.clearAccessCache();
    mpNetReply = BailiSoft::netManager.get(request);
    connect(mpNetReply, SIGNAL(readyRead()), this, SLOT(httpReadyRead()));
    connect(mpNetReply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(httpReadProgress(qint64,qint64)));
    connect(mpNetReply, SIGNAL(finished()), this, SLOT(httpFinished()));
    connect(mpNetReply, SIGNAL(finished()), mpNetReply, SLOT(deleteLater()));
    connect(mpNetReply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(httpError(QNetworkReply::NetworkError)));
    connect(mpNetReply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(httpSslErrors(QList<QSslError>)));
}

void LxWelcome::execCheckVersionSuccess()
{
    QStringList numPair = QString(mBytesVernum).split(QChar('.'));
    if ( numPair.length() == 3 ) {
        mNetMajorNum = QString(numPair.at(0)).toInt();
        mNetMinorNum = QString(numPair.at(1)).toInt();
        mNetPatchNum = QString(numPair.at(2)).toInt();
    } else {
        mNetMajorNum = LXAPP_VERSION_MAJOR;
        mNetMinorNum = 999;
        mNetPatchNum = 999;
    }

    if ( needUpgradeNow() ) {
        mpProgress->setRange(0, 99);
        mpProgress->reset();
        mpProgress->showMessage(QStringLiteral(FOUND_NEW_VERSION), true);
        mpBtnLeft->setText(QStringLiteral(UPGRADE_NOW));
        mpBtnRight->setText(QStringLiteral(IGNORE_FOR_NEXT));

        mpBtnLeft->show();
        mpBtnRight->show();
    }
    else {
        qint64 timeNow = QDateTime::currentMSecsSinceEpoch() / 1000;
        qint64 secsUsed = timeNow - mTimeStart;
        if ( secsUsed > 2 ) {
            mCanOver = true;
        } else {
            QTimer::singleShot( (4 - int(secsUsed)) * 1000, this, SLOT(doSimulateLoadOver()) );
        }
    }
}

void LxWelcome::execCheckVersionFail()
{
    mpProgress->setRange(0, 99);
    mpProgress->reset();
    mpProgress->showMessage(QStringLiteral(GETVERNUM_FAILED), true);

    mpBtnLeft->setText(QStringLiteral(RETRY_NET_LINE));
    mpBtnRight->setText(QStringLiteral(IGNORE_FOR_NEXT));

    mpBtnLeft->show();
    mpBtnRight->show();

}

void LxWelcome::execGetUpgradeDocSuccess()
{
    QTextEdit *note = new QTextEdit(this);
    note->setReadOnly(true);
    note->setText(mBytesUpgdoc);
    note->setWindowTitle(QStringLiteral("升级说明"));
    note->setWindowFlags(Qt::Drawer);
    note->show();
    note->setGeometry(pos().x(), pos().y(), width(), height());
}

void LxWelcome::execGetUpgradeDocFail()
{
    QMessageBox::information(this, "", QStringLiteral("网络获取信息不成功。"));
}

void LxWelcome::execUpgradeDownAbort()
{
    mpProgress->setRange(0, 99);
    mpProgress->reset();

    mDownFileAborted = true;
    mpNetReply->abort();        //等待异步finish处理
}

void LxWelcome::execUpgradeDownSuccess()
{
    //检查删除bak文件
    QFile fileMainBak(qApp->arguments().at(0) + ".bak");
    if ( fileMainBak.exists() ) fileMainBak.remove();

    //然后将主程序文件改为此名
    QFile fileMain(qApp->arguments().at(0));
    fileMain.rename(qApp->arguments().at(0) + ".bak");

    //将下载文件改名为程序文件名
    QFile fileDown(qApp->arguments().at(0) + ".tmp");
    fileDown.rename(qApp->arguments().at(0));

    //提示需要退出程序后，结束程序
    QMessageBox::information(this, QString(), QStringLiteral(UPGRADEOK_NEED_RESTART));
    mCanOver  = true;
    mNeedQuit = true;
}

void LxWelcome::execUpgradeDownFail(const QString &prErr)
{
    if ( mpFileDown ) {
        if ( mpFileDown->isOpen() ) mpFileDown->close();
        if ( mpFileDown->exists() ) mpFileDown->remove();
        delete mpFileDown;
        mpFileDown = nullptr;
    }

    mpProgress->setRange(0, 99);
    mpProgress->reset();

    if ( !prErr.isEmpty() ) {
        mpProgress->showMessage(prErr, true);
        mpBtnLeft->setText(QStringLiteral(RETRY_UPGRAD_DOWN));
        mpBtnRight->setText(QStringLiteral(IGNORE_FOR_NEXT));
        mpBtnLeft->show();
        mpBtnRight->show();
    }
}

bool LxWelcome::needUpgradeNow()
{
    return ( mNetMajorNum == LXAPP_VERSION_MAJOR ) && ( mNetMinorNum > LXAPP_VERSION_MINOR ||
             (mNetMinorNum == LXAPP_VERSION_MINOR && mNetPatchNum > LXAPP_VERSION_PATCH) );
}

////////////////////////////////////////////////////////////////////////

LxProgressBar::LxProgressBar(QWidget *parent) : QProgressBar(parent)
{
    mShowMsg = QStringLiteral(STARTING_NOW);
    setTextVisible(false);
    setStyleSheet("LxProgressBar{background:rgba(0,0,0,0); border:none;}");
}

void LxProgressBar::showMessage(const QString &prMsg, const bool prFlatt)
{
    mShowMsg = prMsg;

    if ( prFlatt )
        setStyleSheet("LxProgressBar{background:rgba(0,0,0,0); border:none;}");
    else
        setStyleSheet("");

    repaint();
}

void LxProgressBar::paintEvent(QPaintEvent *e)
{
    QProgressBar::paintEvent(e);
    if ( !mShowMsg.isEmpty() ) {
        QPainter p(this);
        p.setPen(QColor(88, 88, 88));
        p.drawText(rect().adjusted(3, 0, 0, 0), Qt::AlignVCenter, mShowMsg);
    }
}

}
