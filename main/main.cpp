#include "bailicode.h"
#include "bailidata.h"
#include "bsmain.h"
#include "dialog/lxwelcome.h"
//#include "misc/bsdebug.h"       //release调试结束后应注释掉

#include <QApplication>

int main(int argc, char *argv[])
{
    //重定向debug输出到文件，release调试结束后应注释掉，
    //pro文件中 DEFINES += QT_MESSAGELOGCONTEXT 也应注释掉
    //qInstallMessageHandler(myMsgOutput);

    QApplication a(argc, argv);
    qsrand(uint(QDateTime::currentMSecsSinceEpoch()));
#ifdef Q_OS_WIN
    QFont f(qApp->font());
    f.setFamily(QStringLiteral("微软雅黑"));
    qApp->setFont(f);
#endif

    //Global settings    
    a.setApplicationName(QStringLiteral("JXCR17Desk"));                     //for QSettings
    a.setWindowIcon(QIcon(QStringLiteral(":/icon/applogo.png")));
    a.setOrganizationName(QStringLiteral("BailiSoft"));                     //for QSettings
    a.setOrganizationDomain(QStringLiteral("bailisoft.com"));               //for QSettings

    //基本变量与字典初始化
    BailiSoft::initWinTableNames();
    BailiSoft::initMapMsg();

    //授权检测
    qsrand(uint(QTime(0,0,0).secsTo(QTime::currentTime())));

#ifndef QT_DEBUG
    //SplashScreen
    BailiSoft::LxWelcome *splash = new BailiSoft::LxWelcome();
#ifdef Q_OS_UNIX
    splash->setWindowFlags(splash->windowFlags() |= Qt::X11BypassWindowManagerHint);
#endif
    splash->show();
    splash->move((qApp->desktop()->width() - splash->width()) / 2,
                  (qApp->desktop()->height() - splash->height()) / 2);
    while ( !splash->mCanOver ) a.processEvents();
    if ( splash->mNeedQuit ) return 0;
#endif

    //根据授权，显示版本名称
    a.setApplicationDisplayName(BailiSoft::mapMsg.value("app_name"));

    //检查数据目录
    int iRet = BailiSoft::checkDataDir();
    if ( iRet ) return iRet;

    //打开默认DB连接
    iRet = BailiSoft::openDefaultSqliteConn();
    if ( iRet ) return iRet;

    //主窗口
    BailiSoft::BsMain w;
#ifdef QT_DEBUG
    w.showNormal();
    w.setGeometry(0, 30, w.sizeHint().width(), w.sizeHint().height());
#else
    w.showMaximized();
#endif
    w.setWindowIcon(QIcon(QStringLiteral(":/icon/applogo.png")));

#ifndef QT_DEBUG
    //删除splash
    delete splash;
#endif

    //启动运行循环
    return a.exec();
}
