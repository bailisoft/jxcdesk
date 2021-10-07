#include "bailicode.h"
#include "bailidata.h"
#include "bailisql.h"
#include "bailiwins.h"
#include "bailigrid.h"
#include "bailifunc.h"
#include "bssocket.h"
#include "bsmain.h"
#include "dialog/bsloginguide.h"
#include "dialog/bssetpassword.h"
#include "dialog/bsabout.h"
#include "misc/bswinreg.h"
#include "misc/bschat.h"
#include "tools/bsbarcodemaker.h"


namespace BailiSoft {

BsMain::BsMain(QWidget *parent) : QMainWindow(parent)
{
    //self
    setMinimumSize(1000, 700);

    //UI
    QMenuBar *mnbar = menuBar();

    QMenu *mnReg = mnbar->addMenu(mapMsg.value("main_register"));
    mpMenuCargo = mnReg->addAction(mapMsg.value("win_cargo").split(QChar(9)).at(0), this, SLOT(openRegCargo()));
    mnReg->addSeparator();
    mpMenuSupplier = mnReg->addAction(mapMsg.value("win_supplier").split(QChar(9)).at(0), this, SLOT(openRegSupplier()));
    mpMenuCustomer = mnReg->addAction(mapMsg.value("win_customer").split(QChar(9)).at(0), this, SLOT(openRegCustomer()));

    QMenu *mnInput = mnbar->addMenu(mapMsg.value("main_input"));
    mpMenuSheetCGD  = mnInput->addAction(mapMsg.value("win_cgd").split(QChar(9)).at(0), this, SLOT(openSheetCGD()));
    mpMenuSheetCGJ  = mnInput->addAction(mapMsg.value("win_cgj").split(QChar(9)).at(0), this, SLOT(openSheetCGJ()));
    mpMenuSheetCGT  = mnInput->addAction(mapMsg.value("win_cgt").split(QChar(9)).at(0), this, SLOT(openSheetCGT()));
    mnInput->addSeparator();
    mpMenuSheetPFD  = mnInput->addAction(mapMsg.value("win_pfd").split(QChar(9)).at(0), this, SLOT(openSheetPFD()));
    mpMenuSheetPFF  = mnInput->addAction(mapMsg.value("win_pff").split(QChar(9)).at(0), this, SLOT(openSheetPFF()));
    mpMenuSheetPFT  = mnInput->addAction(mapMsg.value("win_pft").split(QChar(9)).at(0), this, SLOT(openSheetPFT()));
    mnInput->addSeparator();
    mpMenuSheetLSD  = mnInput->addAction(mapMsg.value("win_lsd").split(QChar(9)).at(0), this, SLOT(openSheetLSD()));
    mnInput->addSeparator();
    mpMenuSheetDBD  = mnInput->addAction(mapMsg.value("win_dbd").split(QChar(9)).at(0), this, SLOT(openSheetDBD()));
    mnInput->addSeparator();
    mpMenuSheetSYD  = mnInput->addAction(mapMsg.value("win_syd").split(QChar(9)).at(0), this, SLOT(openSheetSYD()));

    QMenu *mnSummary = mnbar->addMenu(mapMsg.value("main_summary"));
    mpMenuQryVICGD  = mnSummary->addAction(mapMsg.value("win_vi_cgd").split(QChar(9)).at(0), this, SLOT(openQryVICGD()));
    mpMenuQryVICGJ  = mnSummary->addAction(mapMsg.value("win_vi_cgj").split(QChar(9)).at(0), this, SLOT(openQryVICGJ()));
    mpMenuQryVICGT  = mnSummary->addAction(mapMsg.value("win_vi_cgt").split(QChar(9)).at(0), this, SLOT(openQryVICGT()));
    mpMenuQryVIJCG  = mnSummary->addAction(mapMsg.value("win_vi_cg").split(QChar(9)).at(0), this, SLOT(openQryViCgNeat()));
    mnSummary->addSeparator();
    mpMenuQryVIPFD  = mnSummary->addAction(mapMsg.value("win_vi_pfd").split(QChar(9)).at(0), this, SLOT(openQryVIPFD()));
    mpMenuQryVIPFF  = mnSummary->addAction(mapMsg.value("win_vi_pff").split(QChar(9)).at(0), this, SLOT(openQryVIPFF()));
    mpMenuQryVIPFT  = mnSummary->addAction(mapMsg.value("win_vi_pft").split(QChar(9)).at(0), this, SLOT(openQryVIPFT()));
    mpMenuQryVIJPF  = mnSummary->addAction(mapMsg.value("win_vi_pf").split(QChar(9)).at(0), this, SLOT(openQryViPfNeat()));
    mnSummary->addSeparator();
    mpMenuQryVILSD  = mnSummary->addAction(mapMsg.value("win_vi_lsd").split(QChar(9)).at(0), this, SLOT(openQryVILSD()));
    mpMenuQryVIJXS  = mnSummary->addAction(mapMsg.value("win_vi_xs").split(QChar(9)).at(0), this, SLOT(openQryViXsNeat()));
    mnSummary->addSeparator();
    mpMenuQryVIDBD  = mnSummary->addAction(mapMsg.value("win_vi_dbd").split(QChar(9)).at(0), this, SLOT(openQryVIDBD()));
    mnSummary->addSeparator();
    mpMenuQryVISYD  = mnSummary->addAction(mapMsg.value("win_vi_syd").split(QChar(9)).at(0), this, SLOT(openQryVISYD()));

    QMenu *mnBalance = mnbar->addMenu(mapMsg.value("main_balance"));
    mpMenuQryVIYCG  = mnBalance->addAction(mapMsg.value("win_vi_cg_rest").split(QChar(9)).at(0), this, SLOT(openQryViCgRest()));
    mpMenuQryVIJCGM = mnBalance->addAction(mapMsg.value("win_vi_cg_cash").split(QChar(9)).at(0), this, SLOT(openQryViCgCash()));
    mnBalance->addSeparator();
    mpMenuQryVIYPF  = mnBalance->addAction(mapMsg.value("win_vi_pf_rest").split(QChar(9)).at(0), this, SLOT(openQryViPfRest()));
    mpMenuQryVIJPFM = mnBalance->addAction(mapMsg.value("win_vi_pf_cash").split(QChar(9)).at(0), this, SLOT(openQryViPfCash()));
    mpMenuQryVIJXSM = mnBalance->addAction(mapMsg.value("win_vi_xs_cash").split(QChar(9)).at(0), this, SLOT(openQryViXsCash()));
    mnBalance->addSeparator();
    mpMenuQryVIYKC  = mnBalance->addAction(mapMsg.value("win_vi_stock").split(QChar(9)).at(0), this, SLOT(openQryViStock()));
    mpMenuQryVIALL  = mnBalance->addAction(mapMsg.value("win_vi_all").split(QChar(9)).at(0), this, SLOT(openQryViewAll()));

    QMenu *mnTool = mnbar->addMenu(mapMsg.value("main_tool"));
    mpMenuToolBarcodeMaker = mnTool->addAction(mapMsg.value("menu_barcode_maker"), this, SLOT(openToolBarcodeMaker()));
    mnTool->addSeparator();
    mnTool->addAction(mapMsg.value("menu_custom"), this, SLOT(openHelpSite()));

    QMenu* mnHelp = mnbar->addMenu(mapMsg.value("main_help"));
    mnHelp->addAction(QStringLiteral("使用手册"), this, SLOT(openHelpManual()));
    mnHelp->addAction(QStringLiteral("官方网站"), this, SLOT(openHelpSite()));
    mnHelp->addAction(QStringLiteral("关于软件"), this, SLOT(openHelpAbout()));

    //状态栏
    QStatusBar *stbar = statusBar();

    stbar->insertWidget(0, new QLabel(QStringLiteral("当前连接")));
    mpLoginedFile = new QLabel(QStringLiteral("未连接"));
    mpLoginedFile->setStyleSheet(QLatin1String("color:#090;padding-right:20px;"));
    mpLoginedFile->setMinimumWidth(160);
    stbar->insertWidget(1, mpLoginedFile);

    stbar->insertWidget(2, new QLabel(QStringLiteral("当前用户")));
    mpLoginedUser = new QLabel(QStringLiteral("未登录"));
    mpLoginedUser->setStyleSheet(QLatin1String("color:#090;padding-right:20px;"));
    mpLoginedUser->setMinimumWidth(160);
    stbar->insertWidget(3, mpLoginedUser);

    stbar->insertWidget(4, new QLabel(QStringLiteral("权限角色")));
    mpLoginedRole = new QLabel(QStringLiteral("未知"));
    mpLoginedRole->setStyleSheet(QLatin1String("color:#090;padding-right:20px;"));
    stbar->insertWidget(5, mpLoginedRole);

    mpBtnNetMessage = new QToolButton(this);
    mpBtnNetMessage->setText(QStringLiteral("沟通报告"));
    mpBtnNetMessage->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mpBtnNetMessage->setIcon(QIcon(":/icon/man.png"));
    mpBtnNetMessage->setStyleSheet(QLatin1String("border:none;"));
    connect(mpBtnNetMessage, &QToolButton::clicked, this, &BsMain::onClickMsgCheckButton);

    mpBtnNetStatus = new QToolButton(this);
    mpBtnNetStatus->setText(QString());
    mpBtnNetStatus->setToolTip(QString());
    mpBtnNetStatus->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mpBtnNetStatus->setIcon(QIcon(":/icon/serveron.png"));
    mpBtnNetStatus->setStyleSheet(QLatin1String("border:none;color:#f00;"));
    connect(mpBtnNetStatus, &QToolButton::clicked, this, &BsMain::onClickNetStatus);

    QWidget* pnlCorner = new QWidget(this);
    QHBoxLayout *layCorner = new QHBoxLayout(pnlCorner);
    layCorner->setContentsMargins(0, 0, 0, 0);
    layCorner->addStretch();
    layCorner->addWidget(mpBtnNetMessage);
    layCorner->addStretch();
    layCorner->addWidget(mpBtnNetStatus);
    stbar->addPermanentWidget(pnlCorner, 999);

    //主容器
    mpMdi = new BsMdiArea(this);
    mpMdi->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mpMdi->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mpMdi->show();
    setCentralWidget(mpMdi);

    //消息显示浮窗
    mpMeetMenu = new BsMeetMenu(this);
    mpMeetMenu->hide();
    connect(mpMeetMenu, &BsMeetMenu::meetClicked, this, &BsMain::onMeetClicked);

    //准备网络接口
    netSocket = new BsSocket(this);
    connect(netSocket, &BsSocket::netOfflined, this, &BsMain::onNetOffline);
    connect(netSocket, &BsSocket::msgArraried, this, &BsMain::onMsgArrived);
    connect(netSocket, &BsSocket::meetingEvent, this, &BsMain::onMeetingEvent);
    connect(netSocket, &BsSocket::requestOk, this, &BsMain::onSocketRequestOk);

    //开启登录向导
    QTimer::singleShot(100, this, SLOT(openLoginGuide()));
}

bool BsMain::eventFilter(QObject *watched, QEvent *event)
{
    if ( event->type() == QEvent::StatusTip ) {
        BsWin *w = qobject_cast<BsWin*>(watched);
        QStatusTipEvent *e = static_cast<QStatusTipEvent*>(event);
        if ( w && e ) {
            w->displayGuideTip(e->tip());
            return true;
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void BsMain::closeEvent(QCloseEvent *e)
{
    netSocket->exitLogin();
    QMainWindow::closeEvent(e);
}

void BsMain::openLoginGuide()
{
    BsLoginGuide dlg(this);
    dlg.adjustSize();
    if ( QDialog::Accepted == dlg.exec() )
    {
        //加载配置
        loginLoadOptions(); //必须最先

        //加载结构
        loginLoadRegis();

        //加载权限
        loginLoadRights();

        //设置菜单禁用
        setMenuAllowable();

        //状态栏
        loginLink = mapOption.value("app_company_name");
        QString loginedRole = (loginTrader.isEmpty())
                ? QStringLiteral("总部管理")
                : QStringLiteral("绑定%1").arg(loginTrader);
        mpLoginedFile->setText(loginLink);
        mpLoginedUser->setText(loginer);
        mpLoginedRole->setText(loginedRole);

        //记录登录连接参数
        QSettings settings;
        settings.beginGroup(BSR17LinkList);
        settings.beginGroup(loginLink);

        settings.setValue("id", dlg.mTryingBackerId);
        settings.setValue("aes", dlg.mTryingBackerAes);

        QStringList existsUsers = settings.value("users").toStringList();
        if ( !existsUsers.contains(dlg.mTryingUserName) ) existsUsers << dlg.mTryingUserName;
        settings.setValue("users", existsUsers);

        settings.endGroup();
        settings.endGroup();

        //大数据集重置更新时间
        dsCargo->switchBookLogin();
        dsSubject->switchBookLogin();
        dsShop->switchBookLogin();
        dsSupplier->switchBookLogin();
        dsCustomer->switchBookLogin();

        //加载数据集
        dsSizerType->reload();
        dsSizerList->reload();
        dsColorType->reload();
        dsColorList->reload();
        dsCargo->reload();
        dsSubject->reload();
        dsShop->reload();
        dsSupplier->reload();
        dsCustomer->reload();
    }
    else if ( loginer.isEmpty() ) {
        qApp->quit();   //保证菜单能有效打开
    }
}

void BsMain::openRegCargo()
{
    if ( ! checkRaiseSubWin("cargo") ) {
        QStringList flds;
        flds << QStringLiteral("hpcode")
             << QStringLiteral("hpname")
             << QStringLiteral("sizertype")
             << QStringLiteral("colortype")
             << QStringLiteral("setprice");
        addNewSubWin(new BsWinReg(this, QStringLiteral("cargo"), flds));
    }
}

void BsMain::openRegSupplier()
{
    if ( ! checkRaiseSubWin("supplier") ) {
        QStringList flds;
        flds << QStringLiteral("kname")
             << QStringLiteral("regman")
             << QStringLiteral("regtele")
             << QStringLiteral("regaddr")
             << QStringLiteral("regmark");
        addNewSubWin(new BsWinReg(this, QStringLiteral("supplier"), flds));
    }
}

void BsMain::openRegCustomer()
{
    if ( ! checkRaiseSubWin("customer") ) {
        QStringList flds;
        flds << QStringLiteral("kname")
             << QStringLiteral("regman")
             << QStringLiteral("regtele")
             << QStringLiteral("regaddr")
             << QStringLiteral("regmark");
        addNewSubWin(new BsWinReg(this, QStringLiteral("customer"), flds));
    }
}

void BsMain::openSheetCGD()
{
    if ( ! checkRaiseSubWin("cgd") ) {
        addNewSubWin(new BsSheetCargoWin(this, QStringLiteral("cgd"), cargoSheetCommonFields));
    }
}

void BsMain::openSheetCGJ()
{
    if ( ! checkRaiseSubWin("cgj") ) {
        addNewSubWin(new BsSheetCargoWin(this, QStringLiteral("cgj"), cargoSheetCommonFields));
    }
}

void BsMain::openSheetCGT()
{
    if ( ! checkRaiseSubWin("cgt") ) {
        addNewSubWin(new BsSheetCargoWin(this, QStringLiteral("cgt"), cargoSheetCommonFields));
    }
}

void BsMain::openQryVICGD()
{
    if ( ! checkRaiseSubWin("vi_cgd") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_cgd"), cargoQueryCommonFields, bsqtSumSheet | bsqtSumOrder));
    }
}

void BsMain::openQryVICGJ()
{
    if ( ! checkRaiseSubWin("vi_cgj") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_cgj"), cargoQueryCommonFields, bsqtSumSheet));
    }
}

void BsMain::openQryVICGT()
{
    if ( ! checkRaiseSubWin("vi_cgt") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_cgt"), cargoQueryCommonFields, bsqtSumSheet));
    }
}

void BsMain::openQryViCgRest()
{
    if ( ! checkRaiseSubWin("vi_cg_rest") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_cg_rest"), cargoQueryCommonFields, bsqtSumOrder | bsqtSumMinus | bsqtSumRest));
    }
}

void BsMain::openQryViCgNeat()
{
    if ( ! checkRaiseSubWin("vi_cg") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_cg"), cargoQueryCommonFields, bsqtSumMinus));
    }
}

void BsMain::openQryViCgCash()
{
    if ( ! checkRaiseSubWin("vi_cg_cash") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_cg_cash"), cargoQueryCommonFields, bsqtSumCash | bsqtSumMinus));
    }
}

void BsMain::openSheetPFD()
{
    if ( ! checkRaiseSubWin("pfd") ) {
        addNewSubWin(new BsSheetCargoWin(this, QStringLiteral("pfd"), cargoSheetCommonFields));
    }
}

void BsMain::openSheetPFF()
{
    if ( ! checkRaiseSubWin("pff") ) {
        addNewSubWin(new BsSheetCargoWin(this, QStringLiteral("pff"), cargoSheetCommonFields));
    }
}

void BsMain::openSheetPFT()
{
    if ( ! checkRaiseSubWin("pft") ) {
        addNewSubWin(new BsSheetCargoWin(this, QStringLiteral("pft"), cargoSheetCommonFields));
    }
}

void BsMain::openSheetLSD()
{
    if ( ! checkRaiseSubWin("lsd") ) {
        addNewSubWin(new BsSheetCargoWin(this, QStringLiteral("lsd"), cargoSheetCommonFields));
    }
}

void BsMain::openQryVIPFD()
{
    if ( ! checkRaiseSubWin("vi_pfd") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_pfd"), cargoQueryCommonFields, bsqtSumSheet | bsqtSumOrder));
    }
}

void BsMain::openQryVIPFF()
{
    if ( ! checkRaiseSubWin("vi_pff") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_pff"), cargoQueryCommonFields, bsqtSumSheet));
    }
}

void BsMain::openQryVIPFT()
{
    if ( ! checkRaiseSubWin("vi_pft") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_pft"), cargoQueryCommonFields, bsqtSumSheet));
    }
}

void BsMain::openQryVILSD()
{
    if ( ! checkRaiseSubWin("vi_lsd") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_lsd"), cargoQueryCommonFields, bsqtSumSheet));
    }
}

void BsMain::openQryViPfRest()
{
    if ( ! checkRaiseSubWin("vi_pf_rest") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_pf_rest"), cargoQueryCommonFields, bsqtSumMinus | bsqtSumOrder | bsqtSumRest));
    }
}

void BsMain::openQryViPfNeat()
{
    if ( ! checkRaiseSubWin("vi_pf") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_pf"), cargoQueryCommonFields, bsqtSumMinus));
    }
}

void BsMain::openQryViPfCash()
{
    if ( ! checkRaiseSubWin("vi_pf_cash") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_pf_cash"), cargoQueryCommonFields, bsqtSumCash | bsqtSumMinus));
    }
}

void BsMain::openQryViXsNeat()
{
    if ( ! checkRaiseSubWin("vi_xs") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_xs"), cargoQueryCommonFields, bsqtSumMinus));
    }
}

void BsMain::openQryViXsCash()
{
    if ( ! checkRaiseSubWin("vi_xs_cash") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_xs_cash"), cargoQueryCommonFields, bsqtSumCash | bsqtSumMinus));
    }
}

void BsMain::openSheetDBD()
{
    if ( ! checkRaiseSubWin("dbd") ) {
        addNewSubWin(new BsSheetCargoWin(this, QStringLiteral("dbd"), cargoSheetCommonFields));
    }
}

void BsMain::openSheetSYD()
{
    if ( ! checkRaiseSubWin("syd") ) {
        addNewSubWin(new BsSheetCargoWin(this, QStringLiteral("syd"), cargoSheetCommonFields));
    }
}

void BsMain::openQryVIDBD()
{
    if ( ! checkRaiseSubWin("vi_dbd") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_dbd"), cargoQueryCommonFields, bsqtSumSheet));
    }
}

void BsMain::openQryVISYD()
{
    if ( ! checkRaiseSubWin("vi_syd") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_syd"), cargoQueryCommonFields, bsqtSumSheet));
    }
}

void BsMain::openQryViStock()
{
    if ( ! checkRaiseSubWin("vi_stock") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_stock"), cargoQueryCommonFields, bsqtSumMinus | bsqtSumRest | bsqtSumStock));
    }
}

void BsMain::openQryViewAll()
{
    if ( ! checkRaiseSubWin("vi_all") ) {
        addNewSubWin(new BsQryWin(this, QStringLiteral("vi_all"), cargoQueryCommonFields, bsqtSumSheet | bsqtSumStock));
    }
}

void BsMain::openToolBarcodeMaker()
{
    if ( ! checkRaiseSubWin("tool_barcodemaker") ) {
        addNewSubWin(new BsBarcodeMaker(this, QStringLiteral("tool_barcodemaker")));
    }
}

void BsMain::openHelpManual()
{
    QDesktopServices::openUrl(QUrl("https://www.bailisoft.com/passage/jyb_index.html"));
}

void BsMain::openHelpSite()
{
    QDesktopServices::openUrl(QUrl("https://www.bailisoft.com/"));
}

void BsMain::openHelpAbout()
{
    BsAbout *w = new BsAbout(this);
    w->show();
    w->setGeometry((mpMdi->width() - w->width()) / 2, (mpMdi->height() - w->height()) / 2, w->width(), w->height());
}

void BsMain::onMeetClicked(const QString &meetid, const QString &meetname)
{
    QDockWidget *dockWidget = nullptr;
    QList<QDockWidget *> docks = findChildren<QDockWidget *>();
    for ( int i = 0, iLen = docks.length(); i < iLen; ++i ) {
        QDockWidget *dock = docks.at(i);
        BsChat *chat = qobject_cast<BsChat*>(dock->widget());
        if ( chat && chat->mMeetId == meetid ) {
            dockWidget = dock;
            break;
        }
    }

    if ( !dockWidget ) {
        BsChat *chat = new BsChat(this, meetid, meetname);
        dockWidget = new QDockWidget(meetname, this);
        dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        dockWidget->setWidget(chat);
        addDockWidget(Qt::RightDockWidgetArea, dockWidget);
    }

    dockWidget->setFloating(true);
    int w = dockWidget->widget()->sizeHint().width() + 10;
    dockWidget->resize(w, mpMdi->height() / 2);
    QPoint pt = mpMdi->mapToGlobal(QPoint(0, 0));
    dockWidget->setGeometry(pt.x() + (mpMdi->width() - w) / 2,
                            pt.y() + mpMdi->height() / 4,
                            w,
                            mpMdi->height() / 2);
    dockWidget->show();

}

void BsMain::onClickMsgCheckButton()
{
    QPoint pt = mpBtnNetMessage->mapToGlobal(QPoint(0, 0));
    mpMeetMenu->show();  //因为showEvent中有加载决定尺寸，故要在setGeometry之前
    mpMeetMenu->setGeometry(pt.x(), pt.y() - mpMeetMenu->height(), mpMeetMenu->width(), mpMeetMenu->height());

    mpBtnNetMessage->setText(QStringLiteral("沟通报告"));
    mpBtnNetMessage->setStyleSheet(QLatin1String("border:none;"));
}

void BsMain::onClickNetStatus()
{
    if ( mOnlinee ) {
        netSocket->requestLogin();
    }
    else {
        QSettings settings;
        settings.beginGroup(BSR17LinkList);
        settings.beginGroup(loginLink);
        QString backerId = settings.value("id").toString();
        QString backerAesKey = settings.value("aes").toString();
        settings.endGroup();
        settings.endGroup();
        QString strErr = sqlInitBook(getDataPathFile(backerId), backerId);
        if ( strErr.isEmpty() ) {
            netSocket->connectStart(backerId, backerAesKey, loginer, loginPassword);
        } else {
            QMessageBox::information(this, QString(), strErr);
        }
    }
}

void BsMain::onNetOffline(const QString &reason)
{
    if ( !reason.isEmpty() ) {
        mOnlinee = false;
        mpBtnNetStatus->setText(QStringLiteral("恢复上线"));
        mpBtnNetStatus->setToolTip(reason);
        mpBtnNetStatus->setIcon(QIcon(":/icon/serveron.png"));
        mpBtnNetStatus->setStyleSheet(QLatin1String("border:none;color:#f00;"));
    }
}

void BsMain::onMsgArrived(const QStringList &fens)
{
    //解析
    qint64 msgId = QString(fens.at(1)).toLongLong();
    QString senderId = fens.at(2);
    QString senderName = fens.at(3);
    QString receiverId = fens.at(4);
    QString msgText = fens.at(5);
    QString receiverName;

    //存储
    QSqlDatabase db = QSqlDatabase::database();
    if ( receiverId.length() == 16 ) {
        receiverName = loginer;
    } else {
        QSqlQuery qry(db);
        qry.setForwardOnly(true);
        qry.exec(QStringLiteral("select meetname from meeting where meetid=%1;").arg(receiverId));
        if ( qry.next() ) {
            receiverName = qry.value(0).toString();
        } else {
            if ( qry.lastError().isValid() ) qDebug() << "lastError: " << qry.lastError();
        }
        qry.finish();
    }

    QString prepare = QStringLiteral("insert into msglog(msgid, senderid, sendername, "
                                     "receiverid, receivername, content) "
                                     "values(?, ?, ?, ?, ?, ?);");
    QSqlQuery qry;
    qry.prepare(prepare);
    qry.bindValue(0, msgId);
    qry.bindValue(1, senderId);
    qry.bindValue(2, senderName);
    qry.bindValue(3, receiverId);
    qry.bindValue(4, receiverName);
    qry.bindValue(5, msgText);
    qry.exec();
    if ( qry.lastError().isValid() ) qDebug() << qry.lastError() << "\t" << prepare;

    //查找添加到已打开聊天窗口
    bool chatOpened = false;
    QList<QDockWidget *> docks = findChildren<QDockWidget *>();
    for ( int i = 0, iLen = docks.length(); i < iLen; ++i ) {
        QDockWidget *dock = docks.at(i);
        BsChat *chat = qobject_cast<BsChat*>(dock->widget());
        if ( chat && chat->mMeetId == receiverId ) {
            chat->appendMessage(msgId, senderName, msgId / 1000000, false, msgText);
            chatOpened = true;
            QTimer::singleShot(30, [=]{chat->scrollToBottom();});
            break;
        }
    }

    //界面通知
    if ( !chatOpened ) {
        mpBtnNetMessage->setText(QStringLiteral("有新消息！"));
        mpBtnNetMessage->setStyleSheet(QLatin1String("border:none;color:#f00;"));
    }
}

void BsMain::onMeetingEvent(const QStringList &fens)
{
    QSqlDatabase db = QSqlDatabase::database();

    QString cmd = fens.at(0);
    if ( cmd == QStringLiteral("GRPCREATE") ) {
        qint64 meetId = QString(fens.at(2)).toLongLong();
        QString meetName = fens.at(3);
        QString sql = QStringLiteral("insert or ignore into meeting(meetid, meetname) "
                                     "values(%1, '%2');").arg(meetId).arg(meetName);
        db.exec(sql);
        if ( db.lastError().isValid() ) qDebug() << db.lastError() << "\t" << sql;
    }

    if ( cmd == QStringLiteral("GRPINVITE") ) {
        qint64 meetId = QString(fens.at(2)).toLongLong();
        QString meetName = fens.at(3);
        QStringList adds = QString(fens.at(5)).split(QChar('\t'));
        if ( adds.contains(loginer) ) {
            QString sql = QStringLiteral("insert or ignore into meeting(meetid, meetname) "
                                         "values(%1, '%2');").arg(meetId).arg(meetName);
            db.exec(sql);
            if ( db.lastError().isValid() ) qDebug() << db.lastError() << "\t" << sql;
        }
    }

    if ( cmd == QStringLiteral("GRPKICKOFF") ) {
        qint64 meetId = QString(fens.at(2)).toLongLong();
        QStringList kicks = QString(fens.at(3)).split(QChar('\t'));
        if ( kicks.contains(loginer) ) {
            QString sql = QStringLiteral("delete from meeting where meetid=%1;").arg(meetId);
            db.exec(sql);
            if ( db.lastError().isValid() ) qDebug() << db.lastError() << "\t" << sql;
        }
    }

    if ( cmd == QStringLiteral("GRPDISMISS") ) {
        qint64 meetId = QString(fens.at(2)).toLongLong();
        QString sql = QStringLiteral("delete from meeting where meetid=%1;").arg(meetId);
        db.exec(sql);
        if ( db.lastError().isValid() ) qDebug() << db.lastError() << "\t" << sql;
    }

    if ( cmd == QStringLiteral("GRPRENAME") ) {
        qint64 meetId = QString(fens.at(2)).toLongLong();
        QString meetName = fens.at(3);
        QString sql = QStringLiteral("update meeting set meetname='%1' where meetid=%2;")
                .arg(meetName).arg(meetId);
        db.exec(sql);
        if ( db.lastError().isValid() ) qDebug() << db.lastError() << "\t" << sql;
    }
}

void BsMain::onSocketRequestOk(const QWidget *sender, const QStringList &retList)
{
    QString senderClass = (sender)
            ? QString(sender->metaObject()->className())
            : QString();

    if ( sender != nullptr && sender != this && senderClass.contains("BsQryWin") )
        return;

    QString cmd = retList.at(0);
    QString ret = retList.at(retList.length() - 1);
    if ( cmd == QStringLiteral("LOGIN") ) {
        if ( ret == QStringLiteral("OK") ) {
            mOnlinee = true;
            mpBtnNetStatus->setText(QStringLiteral("在线"));
            mpBtnNetStatus->setToolTip(QStringLiteral("刷新同步"));
            mpBtnNetStatus->setIcon(QIcon(":/icon/reload.png"));
            mpBtnNetStatus->setStyleSheet(QLatin1String("border:none;color:#090;"));
        }
        else {
            mOnlinee = false;
            mpBtnNetStatus->setText(QStringLiteral("登录上线"));
            mpBtnNetStatus->setToolTip(ret);
            mpBtnNetStatus->setIcon(QIcon(":/icon/serveron.png"));
            mpBtnNetStatus->setStyleSheet(QLatin1String("border:none;color:#f00;"));
        }
    }

    if ( cmd == QStringLiteral("GETIMAGE") && retList.length() >= 5 ) {

        if ( ret == QStringLiteral("OK") ) {

            QString cargo = retList.at(2);
            QString base64 = retList.at(3);

            QImage img = QImage::fromData(QByteArray::fromBase64(base64.toLatin1()));

            QLabel* lbl = new QLabel(this);
            lbl->setPixmap(QPixmap::fromImage(img));
            lbl->setWindowTitle(cargo);

            QMdiSubWindow* sub = mpMdi->addSubWindow(lbl);
            sub->setWindowFlags(sub->windowFlags()&~Qt::WindowMaximizeButtonHint&~Qt::WindowMinimizeButtonHint);
            lbl->show();
        }
    }
}

bool BsMain::checkRaiseSubWin(const QString &winTable)
{
    for ( int i = 0, iLen = mpMdi->subWindowList().size(); i < iLen; ++i ) {
        QMdiSubWindow *sw = mpMdi->subWindowList().at(i);
        if ( mpMdi->subWindowList().at(i)->widget()->property(BSWIN_TABLE).toString() == winTable ) {
            sw->raise();
            sw->setFocus();
            return true;
        }
    }
    return false;
}

void BsMain::addNewSubWin(QWidget *win, const bool setCenterr)
{
    win->installEventFilter(this);
    QMdiSubWindow *subWin = mpMdi->addSubWindow(win);
    win->showNormal();

    if ( setCenterr ) {
        int w = subWin->width();
        int h = subWin->height();
        int ww = width();
        int wh = height();
        subWin->setGeometry((ww - w) / 2, (wh - h) / 2, w, h);
    }
}

void BsMain::closeAllSubWin()
{
    for ( int i = mpMdi->subWindowList().size() - 1; i >= 0; --i )
    {
        QWidget *w = qobject_cast<QWidget *>(mpMdi->subWindowList().at(i)->widget());
        if ( w ) {
            w->close();
            mpMdi->subWindowList().at(i)->close();
        }
    }
}

bool BsMain::questionCloseAllSubWin()
{
    bool allClean = true;
    for ( int i = mpMdi->subWindowList().size() - 1; i >= 0; --i )
    {
        QWidget *w = qobject_cast<QWidget *>(mpMdi->subWindowList().at(i)->widget());
        if ( w ) {
            BsWin *bsWin = qobject_cast<BsWin*>(w);
            if ( bsWin ) {
                if ( bsWin->isEditing() ) {
                    allClean = false;
                    break;
                }
            }
        }
    }

    if ( ! allClean ) {
        QMessageBox::information(this, QString(), mapMsg.value("i_found_editing_win"));
        return false;
    }

    if ( mpMdi->subWindowList().size() > 0 ) {
        if ( confirmDialog(this,
                             mapMsg.value("i_need_close_other_wins_first"),
                             mapMsg.value("i_close_all_other_win_now"),
                             mapMsg.value("btn_ok"),
                             mapMsg.value("btn_cancel"),
                             QMessageBox::Warning) )
            closeAllSubWin();
        else
            return false;
    }

    return true;
}

void BsMain::setMenuAllowable()
{
    //以下须与lstRegisWinTableNames、lstSheetWinTableNames、lstQueryWinTableNames三变量一致

    mpMenuCargo->setEnabled(true);
    mpMenuSupplier->setEnabled(true);
    mpMenuCustomer->setEnabled(true);

    mpMenuSheetCGD->setEnabled( canDo("cgd"));
    mpMenuSheetCGJ->setEnabled( canDo("cgj"));
    mpMenuSheetCGT->setEnabled( canDo("cgt"));
    mpMenuQryVICGD->setEnabled( canDo("vicgd"));
    mpMenuQryVICGJ->setEnabled( canDo("vicgj"));
    mpMenuQryVICGT->setEnabled( canDo("vicgt"));
    mpMenuQryVIYCG->setEnabled( canDo("vicgrest"));
    mpMenuQryVIJCG->setEnabled( canDo("vicg"));
    mpMenuQryVIJCGM->setEnabled(canDo("vicgcash"));

    mpMenuSheetPFD->setEnabled( canDo("pfd"));
    mpMenuSheetPFF->setEnabled( canDo("pff"));
    mpMenuSheetPFT->setEnabled( canDo("pft"));
    mpMenuSheetLSD->setEnabled( canDo("lsd"));
    mpMenuQryVIPFD->setEnabled( canDo("vipfd"));
    mpMenuQryVIPFF->setEnabled( canDo("vipff"));
    mpMenuQryVIPFT->setEnabled( canDo("vipft"));
    mpMenuQryVILSD->setEnabled( canDo("vilsd"));
    mpMenuQryVIYPF->setEnabled( canDo("vipfrest"));
    mpMenuQryVIJPF->setEnabled( canDo("vipf"));
    mpMenuQryVIJPFM->setEnabled(canDo("vipfcash"));
    mpMenuQryVIJXS->setEnabled( canDo("vixs"));
    mpMenuQryVIJXSM->setEnabled(canDo("vixscash"));

    mpMenuSheetDBD->setEnabled(canDo("dbd"));
    mpMenuSheetSYD->setEnabled(canDo("syd"));
    mpMenuQryVIDBD->setEnabled(canDo("vidbd"));
    mpMenuQryVISYD->setEnabled(canDo("visyd"));
    mpMenuQryVIYKC->setEnabled(canDo("vistock"));
    mpMenuQryVIALL->setEnabled(canDo("viall"));
}

void BsMain::updateSheetDateForMemo()
{
    QString sql;
    QSqlQuery qry;

    sql = QString("select max(dated) from lsd;");
    qry.exec(sql);
    qry.next();
    qint64 maxLsdDate  = qry.value(0).toLongLong();

    sql = QString("select max(uptime) from lsd;");
    qry.exec(sql);
    qry.next();
    qint64 maxUpTime  = qry.value(0).toLongLong();

    if ( maxLsdDate - maxUpTime > 7 * 3600 * 24 ) {
        return;     //已经更新过
    }

    qint64 secsDiff = QDateTime(QDate::currentDate()).toMSecsSinceEpoch() / 1000 - maxLsdDate;

    QStringList sqls;
    sqls << QStringLiteral("update cgd set dated=dated+%1;").arg(secsDiff)
         << QStringLiteral("update cgj set dated=dated+%1;").arg(secsDiff)
         << QStringLiteral("update cgt set dated=dated+%1;").arg(secsDiff)
         << QStringLiteral("update pfd set dated=dated+%1;").arg(secsDiff)
         << QStringLiteral("update pff set dated=dated+%1;").arg(secsDiff)
         << QStringLiteral("update pft set dated=dated+%1;").arg(secsDiff)
         << QStringLiteral("update lsd set dated=dated+%1;").arg(secsDiff)
         << QStringLiteral("update dbd set dated=dated+%1;").arg(secsDiff)
         << QStringLiteral("update syd set dated=dated+%1;").arg(secsDiff);

    QString sqlErr = sqliteCommit(sqls);

    if ( sqlErr.isEmpty() )
        QMessageBox::information(this, QString(), mapMsg.value("i_update_demo_book_date"));
}

// BsMdiArea ==============================================================================

BsMdiArea::BsMdiArea(QWidget *parent) : QMdiArea(parent)
{
    mppMain = qobject_cast<QMainWindow*>(parent);
    Q_ASSERT(mppMain);
}

void BsMdiArea::paintEvent(QPaintEvent *e)
{
    QMdiArea::paintEvent(e);

    QPainter painter(viewport());

    QString colorText = mapOption.value("app_company_pcolor");
    QString r = colorText.left(2);
    QString g = colorText.mid(2, 2);
    QString b = colorText.right(2);
    bool ok;
    QColor comColor = QColor(r.toInt(&ok, 16), g.toInt(&ok, 16), b.toInt(&ok, 16));

    QByteArray logoData = mapOption.value("app_company_plogo").toLatin1();
    if ( logoData.length() > 100 ) {
        QImage img = QImage::fromData(QByteArray::fromBase64(logoData));
        painter.fillRect(rect(), comColor);
        bool center = mapOption.value("app_company_plway").contains(QStringLiteral("中"));
        if ( center ) {
            painter.drawImage((width() - 300) / 2, (height() - 300) / 2, img, 0, 0, 300, 300);
        } else {
            for ( int i = 0; i < (width() / 300) + 2; ++i ) {
                for ( int j = 0; j < (height() / 300) + 2; ++j ) {
                    painter.drawImage(300 * i, 300 * j, img, 0, 0, 300, 300);
                }
            }
        }
    }
}

}
