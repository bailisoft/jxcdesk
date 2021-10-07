#include "bsloginguide.h"
#include "main/bailicode.h"
#include "main/bailidata.h"
#include "main/bailifunc.h"
#include "main/bailisql.h"
#include "main/bsmain.h"
#include "main/bssocket.h"

#include <QtSql/QSqlDatabase>

#define MN_OPEN_LINK    "创建连接"
#define MN_EDIT_LINK    "设置连接"
#define MN_DROP_LINK    "删除连接"
#define MN_MANAGE       "连接管理"

namespace BailiSoft {

BsLoginGuide::BsLoginGuide(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
    mppMain = qobject_cast<BsMain *>(parent);
    Q_ASSERT(mppMain);

    mpImageSide = new QWidget(this);
    mpImageSide->setFixedSize(160, 280);
    mpImageSide->setStyleSheet(QStringLiteral("background:url(:/image/login.png);"));

    QLabel *lblSelect = new QLabel(QStringLiteral("选择账册："));

    QMenu *mnManage = new QMenu(this);

    mnManage->addAction(QStringLiteral(MN_OPEN_LINK), this, SLOT(doOpenLink()));
    mpAcDelLink = mnManage->addAction(QStringLiteral(MN_DROP_LINK), this, SLOT(doDelLink()));

    mnManage->addSeparator();
    mnManage->addAction(QStringLiteral("帮助"), this, SLOT(doHelp()));

    QToolButton *btnManage = new QToolButton(this);
    btnManage->setText(QStringLiteral(MN_MANAGE));
    btnManage->setIcon(QIcon(QLatin1String(":/icon/config.png")));
    btnManage->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnManage->setMenu(mnManage);
    btnManage->setPopupMode(QToolButton::InstantPopup);
    btnManage->setAutoRaise(true);

    QHBoxLayout *layMenu = new QHBoxLayout;
    layMenu->addWidget(lblSelect, 0, Qt::AlignBottom);
    layMenu->addStretch();
    layMenu->addWidget(btnManage);

    mpLinks = new QListWidget(this);
    mpLinks->setStyleSheet("QAbstractItemView::item {min-height: 24px;}");

    QWidget *tabBook = new QWidget(this);
    QVBoxLayout *layBook = new QVBoxLayout;
    layBook->addLayout(layMenu);
    layBook->addWidget(mpLinks);
    layBook->setContentsMargins(0, 0, 0, 0);
    tabBook->setLayout(layBook);

    QLabel *lblUsers = new QLabel(QStringLiteral("账号："));
    mpUsers = new QComboBox(this);
    mpUsers->setItemDelegate(new QStyledItemDelegate()); //为使下面item的样式生效，必须如此
    mpUsers->setStyleSheet("QComboBox { min-height:20px; }"
                           "QComboBox QAbstractItemView::item { min-height: 20px; }");

    QLabel *lblPassword = new QLabel(QStringLiteral("密码："));
    mpPassword = new QLineEdit(this);
    disableEditInputMethod(mpPassword);  //密码不能有输入法，输入法不安全。
    mpPassword->setEchoMode(QLineEdit::Password);
    mpPassword->setMinimumHeight(24);

    QWidget *tabLogin = new QWidget(this);
    QVBoxLayout *layLogin = new QVBoxLayout;
    layLogin->addStretch(1);
    layLogin->addWidget(lblUsers);
    layLogin->addWidget(mpUsers);
    layLogin->addSpacing(20);
    layLogin->addWidget(lblPassword);
    layLogin->addWidget(mpPassword);
    layLogin->addStretch(1);
    layLogin->setSpacing(5);
    layLogin->setContentsMargins(40, 0, 40, 0);
    tabLogin->setLayout(layLogin);

    mpStack = new QStackedLayout;
    mpStack->setContentsMargins(0, 0, 0, 0);
    mpStack->addWidget(tabBook);
    mpStack->addWidget(tabLogin);

    mpPrev = new QPushButton(QStringLiteral("<"));
    mpPrev->setFixedSize(32, 32);

    mpNext = new QPushButton(QStringLiteral("下一步"));
    mpNext->setFixedSize(120, 32);
    mpNext->setDefault(true);    

    mpCancel = new QPushButton(QStringLiteral("取消"));
    mpCancel->setFixedSize(120, 32);

    mpLayDlgBtns = new QHBoxLayout;
    mpLayDlgBtns->addStretch(1);
    mpLayDlgBtns->addWidget(mpPrev);
    mpLayDlgBtns->addWidget(mpNext);
    mpLayDlgBtns->addWidget(mpCancel);
    mpLayDlgBtns->addStretch(1);
    mpLayDlgBtns->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *layWork = new QVBoxLayout;
    layWork->addLayout(mpStack, 1);
    layWork->addLayout(mpLayDlgBtns);
    layWork->setContentsMargins(0, 3, 10, 10);

    QHBoxLayout *layMain = new QHBoxLayout(this);
    layMain->addWidget(mpImageSide);
    layMain->addLayout(layWork, 1);
    layMain->setContentsMargins(0, 0, 0, 0);

    setFixedSize(sizeHint());
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(QStringLiteral("登录向导"));

#ifdef Q_OS_MAC
    QString btnStyle = "QPushButton{border:1px solid #999; border:6px; background-color:"
            "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f8f8f8, stop:1 #dddddd);} "
            "QPushButton:pressed{background-color:qlineargradient(x1:0, y1:0, x2:0, y2:1, "
            "stop:0 #dddddd, stop:1 #f8f8f8);}";
    mpPrev->setStyleSheet(btnStyle);
    mpNext->setStyleSheet(btnStyle);
    mpCancel->setStyleSheet(btnStyle);
#endif

    //版本名称
    mpVerName = new QLabel(this);
    mpVerName->setText(QStringLiteral("桌面安全终端"));
    mpVerName->setAlignment(Qt::AlignCenter);
    mpVerName->setStyleSheet("color:white; font-weight:900;");

    //加载账册
    reloadBookList();

    //信号
    connect(mpPrev, SIGNAL(clicked(bool)), this, SLOT(doPrev()));
    connect(mpNext, SIGNAL(clicked(bool)), this, SLOT(doNext()));
    connect(mpCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
    connect(mpLinks, SIGNAL(currentRowChanged(int)), this, SLOT(linkPicked(int)));
    connect(mpLinks, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(linkDoubleClicked(QListWidgetItem*)));

    connect(netSocket, &BsSocket::requestOk, this, &BsLoginGuide::onLoginOk);
}

void BsLoginGuide::setVisible(bool visible)
{
    QDialog::setVisible(visible);
    if ( visible ) {
        mpLinks->clearSelection();
        mpAcDelLink->setEnabled(false);
        mpStack->setCurrentIndex(0);
        mpPrev->hide();
        mpNext->setEnabled(false);
        mpLayDlgBtns->setContentsMargins(0, 0, 0, 0);
    }
}

void BsLoginGuide::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent(e);
    mpVerName->setGeometry(0, 3 * height() / 5, mpImageSide->width(), 80);
}

void BsLoginGuide::linkPicked(int)
{
    mpNext->setEnabled( mpLinks->currentItem() );
    mpAcDelLink->setEnabled( mpLinks->currentItem() );
}

void BsLoginGuide::linkDoubleClicked(QListWidgetItem *item)
{
    if ( item ) {
        doNext();
    }
}

void BsLoginGuide::doPrev()
{
    mpStack->setCurrentIndex(0);
    mpPrev->hide();
    mpUsers->setEnabled(true);
    mpUsers->clear();
    mpNext->setText(QStringLiteral("下一步"));
    mpLayDlgBtns->setContentsMargins(0, 0, 0, 0);

    mpNext->setFixedSize(120, 32);
    mpCancel->setFixedSize(120, 32);
}

void BsLoginGuide::doNext()
{
    //加载用户
    if ( mpStack->currentIndex() == 0 ) {
        if ( mpLinks->currentItem() ) {

            QSettings settings;
            settings.beginGroup(BSR17LinkList);
            settings.beginGroup(mpLinks->currentItem()->text());
            QStringList users = settings.value("users").toStringList();
            settings.endGroup();
            settings.endGroup();
            mpUsers->clear();
            mpUsers->addItems(users);

            mpStack->setCurrentIndex(1);
            mpPrev->show();
            mpNext->setText(QStringLiteral("登录"));
            mpLayDlgBtns->setContentsMargins(0, 0, 0, 50);
            mpPassword->clear();
            mpPassword->setFocus();
            mpNext->setFixedSize(75, 32);
            mpCancel->setFixedSize(75, 32);
        } else {
            QMessageBox::information(this, QString(), QStringLiteral("请选择后台！"));
        }
    }
    //登录
    else {
        mTryingUserName = mpUsers->currentText();
        mTryingUserPass = mpPassword->text();

        QSettings settings;
        settings.beginGroup(BSR17LinkList);
        settings.beginGroup(mpLinks->currentItem()->text());

        mTryingBackerId = settings.value("id").toString();
        mTryingBackerAes = settings.value("aes").toString();

        settings.endGroup();
        settings.endGroup();

        QString strErr = sqlInitBook(getDataPathFile(mTryingBackerId), mTryingBackerId);
        if ( strErr.isEmpty() ) {
            netSocket->connectStart(mTryingBackerId, mTryingBackerAes, mTryingUserName, mTryingUserPass);
        } else {
            QMessageBox::information(this, QString(), strErr);
        }
    }
}

void BsLoginGuide::doOpenLink()
{
    BsBackerDlg dlg(this);
    if ( dlg.exec() != QDialog::Accepted ) return;

    mTryingBackerId     = dlg.mpEdtBackerId->text();
    mTryingBackerAes    = dlg.mpEdtBackerKey->text();
    mTryingUserName     = dlg.mpEdtUserName->text();
    mTryingUserPass     = dlg.mpEdtUserPass->text();

    QString strErr = sqlInitBook(getDataPathFile(mTryingBackerId), mTryingBackerId);
    if ( strErr.isEmpty() ) {
        netSocket->connectStart(mTryingBackerId, mTryingBackerAes, mTryingUserName, mTryingUserPass);
    } else {
        QMessageBox::information(this, QString(), strErr);
    }
}

void BsLoginGuide::doDelLink()
{
    //账册信息
    QString linkName = mpLinks->currentItem()->text();
    QString tipMsg = QStringLiteral("此删除仅仅移除此连接信息，不会影响后台任何数据。");
    QString askMsg = QStringLiteral("确定要删除这个连接信息吗？");

    //提示
    if ( ! confirmDialog(this, tipMsg, askMsg,
                         mapMsg.value("btn_ok"),
                         mapMsg.value("btn_cancel"),
                         QMessageBox::Warning) )
        return;

    //记录
    QSettings settings;
    settings.beginGroup(BSR17LinkList);
    settings.remove(linkName);
    settings.endGroup();
    settings.sync();

    //刷新
    reloadBookList();
}

void BsLoginGuide::doHelp()
{
    QDesktopServices::openUrl(QUrl("https://www.bailisoft.com/passage/jyb_login_guide.html"));
}

void BsLoginGuide::onLoginOk(const QWidget *sender, const QStringList &retList)
{
    Q_UNUSED(sender);
    if ( mTryingUserName == bossAccount ) {
        netSocket->exitLogin();
        QMessageBox::information(this, QString(), QStringLiteral("电脑终端不能登录全权账号！"));
        return;
    }

    //数据保存见BsSocket::saveLoginData
    if ( retList.at(0) == QStringLiteral("LOGIN") ) {
        loginer = mTryingUserName;
        loginPassword = mTryingUserPass;
        accept();
    }
}

void BsLoginGuide::reloadBookList()
{
    mpLinks->clear();
    QSettings settings;
    settings.beginGroup(BSR17LinkList);
    QStringList links = settings.childGroups();
    settings.endGroup();

    foreach (QString link, links) {
        QListWidgetItem *item = new QListWidgetItem(link, mpLinks);
        item->setData(Qt::DecorationRole, QIcon(":/icon/netlink.png"));
    }
}

void BsLoginGuide::upgradeSqlCheck()
{
    QStringList sqls;

    sqls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                           "'dots_of_qty', '数量小数位数', '0', '0', '0~4位');");

    sqls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                           "'show_hpname_in_sheet_grid', '单据表格显示品名', '否', '否', '请填“是”或“否”');");

    sqls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                           "'show_hpunit_in_sheet_grid', '单据表格显示单位', '否', '否', '请填“是”或“否”');");

    sqls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                           "'show_hpprice_in_sheet_grid', '单据表格显示标牌价', '否', '否', '请填“是”或“否”');");

    sqls << QStringLiteral("update bailiOption set vsetting='%1', vdefault='%1' "
                           "where optcode='app_image_path' and vdefault='';").arg(imageDir);

    //数据库连接，先取用于getExistsFieldsOfTable中PROGMA取得已有字段信息
    QSqlDatabase defaultdb = QSqlDatabase::database();
    QStringList checkFields;

    //检查添加列（以下一直保留，这样不用担心初始SQL字段不齐全）
    checkFields = getExistsFieldsOfTable(QStringLiteral("baililoginer"), defaultdb);
    if ( checkFields.indexOf(QStringLiteral("limcargoexp")) < 0 )
        sqls << QStringLiteral("alter table baililoginer add column limcargoexp text default '';");

    checkFields = getExistsFieldsOfTable(QStringLiteral("subject"), defaultdb);
    if ( checkFields.indexOf(QStringLiteral("adminboss")) < 0 )
        sqls << QStringLiteral("alter table subject add column adminboss integer default 0;");

    //最终批处理执行
    defaultdb.transaction();
    foreach (QString sql, sqls) {
        defaultdb.exec(sql);
        if ( defaultdb.lastError().isValid() ) {
            qDebug() << defaultdb.lastError() << sql;
            defaultdb.rollback();
            return;
        }
    }
    defaultdb.commit();
}

//=========================================================================================

BsBackerDlg::BsBackerDlg(QWidget *parent) : QDialog(parent)
{
    mpEdtBackerId = new QLineEdit(this);
    mpEdtBackerId->setMinimumWidth(200);
    mpEdtBackerId->setMaxLength(16);
    mpEdtBackerId->setPlaceholderText(QStringLiteral("必填，后台统一提供"));
    disableEditInputMethod(mpEdtBackerId);

    mpEdtBackerKey = new QLineEdit(this);
    mpEdtBackerKey->setEchoMode(QLineEdit::Password);
    mpEdtBackerKey->setPlaceholderText(QStringLiteral("如无则不填，如有则为后台统一提供"));
    mpEdtBackerKey->setMaxLength(32);

    mpEdtUserName = new QLineEdit(this);
    mpEdtUserName->setPlaceholderText(QStringLiteral("必填，后台分配提供"));
    mpEdtUserName->setMaxLength(16);

    mpEdtUserPass = new QLineEdit(this);
    mpEdtUserPass->setPlaceholderText(QStringLiteral("必填，后台分配提供"));
    mpEdtUserPass->setEchoMode(QLineEdit::Password);
    mpEdtUserPass->setMaxLength(32);

    QWidget *pnlLeft = new QWidget(this);
    QVBoxLayout *layLeft = new QVBoxLayout(pnlLeft);
    layLeft->setSpacing(1);
    layLeft->addWidget(new QLabel(QStringLiteral("后台名称："), this));
    layLeft->addWidget(mpEdtBackerId);
    layLeft->addSpacing(5);
    layLeft->addWidget(new QLabel(QStringLiteral("保密码："), this));
    layLeft->addWidget(mpEdtBackerKey);
    layLeft->addSpacing(15);
    layLeft->addWidget(new QLabel(QStringLiteral("登录用户："), this));
    layLeft->addWidget(mpEdtUserName);
    layLeft->addSpacing(5);
    layLeft->addWidget(new QLabel(QStringLiteral("登录密码："), this));
    layLeft->addWidget(mpEdtUserPass);

    mpBtnOk = new QPushButton(mapMsg.value("btn_ok"), this);
    mpBtnOk->setFixedSize(80, 30);
    mpBtnOk->setEnabled(false);

    mpBtnCancel = new QPushButton(mapMsg.value("btn_cancel"), this);
    mpBtnCancel->setFixedSize(80, 30);

    QWidget *pnlRight = new QWidget(this);
    QVBoxLayout *layRight = new QVBoxLayout(pnlRight);
    layRight->addStretch(1);
    layRight->addWidget(mpBtnOk);
    layRight->addWidget(mpBtnCancel);
    layRight->addStretch(7);

    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->addWidget(pnlLeft, 1);
    lay->addWidget(pnlRight);
    setMinimumSize(sizeHint());

    connect(mpEdtBackerId, &QLineEdit::textChanged, this, &BsBackerDlg::onTextChanged);
    connect(mpEdtUserName, &QLineEdit::textChanged, this, &BsBackerDlg::onTextChanged);
    connect(mpEdtUserPass, &QLineEdit::textChanged, this, &BsBackerDlg::onTextChanged);
    connect(mpBtnOk, &QPushButton::clicked, this, &BsBackerDlg::accept);
    connect(mpBtnCancel, &QPushButton::clicked, this, &BsBackerDlg::reject);
}

void BsBackerDlg::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
    mpBtnCancel->setFocus();
}

void BsBackerDlg::onTextChanged(const QString &text)
{
    mpBtnOk->setEnabled(!mpEdtBackerId->text().isEmpty() &&
                        !mpEdtUserName->text().isEmpty() &&
                        !mpEdtUserPass->text().isEmpty() );
    if ( sender() == mpEdtUserName ) {
        if ( text == mapMsg.value("word_admin") ) {
            QMessageBox::information(this, QString(), QStringLiteral("管理员用户禁止前端登录！"));
            mpEdtUserName->clear();
            return;
        }
    }
}


}

