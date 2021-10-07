#include "bschat.h"
#include "main/bailicode.h"
#include "main/bailidata.h"
#include "main/bssocket.h"

namespace BailiSoft {

// BsChat =========================================================================
BsChat::BsChat(QWidget *parent, const QString &meetId, const QString &meetName)
    : QWidget(parent), mMeetId(meetId), mMeetName(meetName)
{
    QLabel* title = new QLabel(meetName, this);
    QFont ft = title->font();
    ft.setPointSize( 3 * ft.pointSize() / 2 );
    ft.setBold(true);
    title->setFont(ft);

    mpPnlContent = new QWidget(this);
    mpLayContent = new QVBoxLayout(mpPnlContent);
    mpLayContent->setSizeConstraint(QLayout::SetMinAndMaxSize);
    mpLayContent->setContentsMargins(0, 0, 0, 0);

    mpScrollArea = new QScrollArea;
    mpScrollArea->setWidgetResizable(true);
    mpScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mpScrollArea->setWidget(mpPnlContent);

    if ( meetId.length() < 16 ) {
        mpScrollArea->setBackgroundRole(QPalette::ToolTipBase);
    }

    mpEdtInput = new QTextEdit;
    mpEdtInput->setFixedHeight(80);

    mpBtnSend = new QToolButton;
    mpBtnSend->setToolButtonStyle(Qt::ToolButtonIconOnly);
    mpBtnSend->setIconSize(QSize(32, 32));
    mpBtnSend->setFixedSize(48, 48);
    mpBtnSend->setIcon(QIcon(":/icon/send.png"));
    mpBtnSend->setToolTip(QStringLiteral("发送"));
    connect(mpBtnSend, &QToolButton::clicked, this, &BsChat::sendInputMessage);

    mpPnlEditor = new QWidget(this);
    QHBoxLayout* layEditor = new QHBoxLayout(mpPnlEditor);
    layEditor->setContentsMargins(0, 0, 0, 0);
    layEditor->addWidget(mpEdtInput, 1);
    layEditor->addWidget(mpBtnSend, 0, Qt::AlignCenter);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->addWidget(title, 0, Qt::AlignCenter);
    lay->addWidget(mpScrollArea, 1);
    lay->addWidget(mpPnlEditor);

    loadMessages();
    setMinimumSize(350, 550);

    connect(netSocket, &BsSocket::requestOk, this, &BsChat::onRequestOk);
}

void BsChat::appendMessage(const qint64 msgid, const QString &senderName, const qint64 sendEpoch,
                           const bool readd, const QString &content)
{
    QLabel *lblSenderName = new QLabel(senderName, this);
    if ( senderName == loginer ) {
        lblSenderName->setAlignment(Qt::AlignRight);
    } else {
        lblSenderName->setAlignment(Qt::AlignLeft);
    }

    QLabel *lblSenderTime = new QLabel(QDateTime::fromSecsSinceEpoch(sendEpoch).toString("MM-dd hh:mm"), this);
    if ( senderName == loginer ) {
        lblSenderTime->setAlignment(Qt::AlignRight);
    } else {
        lblSenderTime->setAlignment(Qt::AlignLeft);
    }

    BsMsgBubble *bubble = new BsMsgBubble(this, msgid, content, readd, senderName == loginer);

    QWidget *msg = new QWidget(this);
    QVBoxLayout *lay = new QVBoxLayout(msg);
    lay->setSpacing(0);
    lay->addWidget(lblSenderName);
    lay->addWidget(lblSenderTime);
    lay->addWidget(bubble, 1);
    msg->setMinimumHeight(msg->sizeHint().height());

    if ( senderName == loginer ) {
        mpLayContent->addWidget(msg, 0, Qt::AlignRight);
    } else {
        mpLayContent->addWidget(msg, 0, Qt::AlignLeft);
    }
}

void BsChat::scrollToBottom()
{
    resizeContentPanel();
    mpScrollArea->verticalScrollBar()->setValue(mpScrollArea->verticalScrollBar()->maximum());
}

void BsChat::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    resizeContentPanel();
}

void BsChat::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    QTimer::singleShot(30, [=] { scrollToBottom(); });
}

void BsChat::resizeContentPanel()
{
    int vbarw = ( mpScrollArea->verticalScrollBar()->isVisible() )
            ? mpScrollArea->verticalScrollBar()->width()
            : 0;
    int fitw = mpScrollArea->width() - vbarw - 1;
    int msgh = mpPnlContent->sizeHint().height();
    mpPnlContent->setFixedSize(fitw, msgh);
}

void BsChat::loadMessages()
{
    QString sql = QStringLiteral("select msgid, senderId, senderName, content, openRead "
                                 "from msglog where receiverId='%1' group by msgid;").arg(mMeetId);
    QSqlQuery qry;
    qry.exec(sql);
    if ( qry.lastError().isValid() ) qDebug() << qry.lastError() << "\t" << sql;
    while ( qry.next() ) {
        qint64 msgid = qry.value(0).toLongLong();
        QString senderName = qry.value(2).toString();
        qint64 sendEpoch = msgid / 1000000;
        bool readd = qry.value(4).toInt();
        QString content = qry.value(3).toString();
        appendMessage(msgid, senderName, sendEpoch, readd, content);
    }
    qry.finish();
}

void BsChat::sendInputMessage()
{
    QString msg = mpEdtInput->toPlainText().trimmed();
    if ( msg.isEmpty() ) return;

    QString senderId = BsSocket::calcShortMd5(loginer);
    QString bossId = BsSocket::calcShortMd5(bossAccount);

    QStringList params;
    params << QStringLiteral("MESSAGE")
           << QString::number(QDateTime::currentMSecsSinceEpoch() * 1000)
           << senderId
           << loginer
           << ((mMeetId == senderId) ? bossId : mMeetId)
           << msg;
    netSocket->netRequest(this, params, QStringLiteral("正在发送……"));
}

void BsChat::onRequestOk(const QWidget *sender, const QStringList &fens)
{
    if ( sender != this ) return;

    qint64 msgId = QString(fens.at(2)).toLongLong();
    QString msgContent = mpEdtInput->toPlainText().trimmed();

    //存储
    QSqlQuery qry;
    QString prepare = QStringLiteral("insert into msglog(msgid, senderid, sendername, "
                                     "receiverid, receivername, content, openread) "
                                     "values(?, ?, ?, ?, ?, ?, ?);");
    qry.prepare(prepare);
    qry.bindValue(0, msgId);
    qry.bindValue(1, BsSocket::calcShortMd5(loginer));
    qry.bindValue(2, loginer);
    qry.bindValue(3, mMeetId);
    qry.bindValue(4, mMeetName);
    qry.bindValue(5, msgContent);
    qry.bindValue(6, 999);
    qry.exec();
    if ( qry.lastError().isValid() ) qDebug() << qry.lastError() << "\t" << prepare;

    //显示
    appendMessage(msgId, loginer, msgId / 1000000, true, msgContent);
    QTimer::singleShot(300, [=] {
        mpEdtInput->clear();
        scrollToBottom();
    });
}

// BsMsgBubble =====================================================================
BsMsgBubble::BsMsgBubble(QWidget *parent, const qint64 msgid, const QString &content,
                         const bool readd, const bool selff)
    : QLabel(parent), mMsgId(msgid), mContent(content), mReadd(readd), mSelff(selff)
{
    setFixedWidth(220);
    setWordWrap(true);

    if ( readd ) {
        setText(content);
        if ( content.length() > 16 ) setToolTip(content);
    } else {
        setPixmap(QPixmap(":/image/msg.png"));
    }
    if ( selff ) {
        setStyleSheet(QLatin1String("background-color:#0d0;padding:8px;"
                                    "border:2px solid #080; border-radius:6px;"));
    } else {
        setStyleSheet(QLatin1String("background-color:#ccc;padding:8px;"
                                    "border:2px solid #aaa; border-radius:6px;"));
    }
}

void BsMsgBubble::mouseReleaseEvent(QMouseEvent *e)
{
    QLabel::mouseReleaseEvent(e);
    if ( !mReadd ) {
        setText(mContent);
        if ( mContent.length() > 16 ) setToolTip(mContent);
    }

    QString sql = QStringLiteral("update msglog set openread=-1 where msgid=%1;").arg(mMsgId);
    QSqlDatabase db = QSqlDatabase::database();
    db.exec(sql);
}


// BsActWidget =====================================================================
BsActWidget::BsActWidget(QWidget *parent, const QString &meetName, const int newsCount, const int nameWidth)
    : QWidget(parent)
{
    mpLblName = new QLabel(meetName, this);
    mpLblName->setFixedWidth(nameWidth + 20);
    mpLblName->setStyleSheet(QStringLiteral("QLabel{padding:3px 5px;} QLabel:hover{background-color:#fcc;}"));

    int lblw = fontMetrics().horizontalAdvance(QStringLiteral("88"));
    mpLblNews = new QLabel(this);
    mpLblNews->setFixedWidth(lblw);
    mpLblNews->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    if ( newsCount > 0 ) {
        QString lbls = (newsCount > 999) ? QStringLiteral("…") : QString::number(newsCount);
        mpLblNews->setText(lbls);
    } else {
        mpLblNews->setText(QStringLiteral(" "));
    }
    mpLblNews->setStyleSheet(QStringLiteral("QLabel{color:red;font-weight:900;}"));

    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setContentsMargins(3, 3, 3, 3);
    lay->addWidget(mpLblName);
    lay->addStretch();
    lay->addWidget(mpLblNews);
}

// BsMeetMenu ===========================================================================
void BsMeetMenu::showEvent(QShowEvent *e)
{
    //清空
    clear();

    //列名
    QList<QPair<QString, QString> > lstChats;   //id, name
    QString sql = QStringLiteral("select meetid, meetname from meeting order by meetname;");
    QSqlQuery qry;
    qry.setForwardOnly(true);
    qry.exec(sql);
    if ( qry.lastError().isValid() ) qDebug() << qry.lastError() << "\t" << sql;
    int fixedWidth = 0;
    while ( qry.next() ) {
        QString meetid = qry.value(0).toString();
        QString meetname = qry.value(1).toString();
        lstChats << qMakePair(meetid, meetname);
        int w = fontMetrics().horizontalAdvance(meetname);
        if ( w > fixedWidth ) fixedWidth = w;
    }
    qry.finish();
    lstChats << qMakePair(BsSocket::calcShortMd5(loginer), bossAccount);

    //查数
    QMap<QString, int> mapNews;    //id, newCount
    sql = QStringLiteral("select receiverId, count(msgid) as newCount "
                         "from msglog where openRead=0 group by receiverId;");
    qry.exec(sql);
    if ( qry.lastError().isValid() ) qDebug() << qry.lastError() << "\t" << sql;
    while ( qry.next() ) {
        mapNews.insert(qry.value(0).toString(), qry.value(1).toInt());
    }
    qry.finish();

    //添项
    for ( int i = 0, iLen = lstChats.length(); i < iLen; ++i ) {
        QString meetid = lstChats.at(i).first;
        QString meetname = lstChats.at(i).second;
        int news = mapNews.contains(meetid) ? mapNews.value(meetid) : 0;

        BsMeetAction* act = new BsMeetAction(this, meetname, meetid, news, fixedWidth);
        connect(act, &BsMeetAction::triggered, this, &BsMeetMenu::onClicked);
        addAction(act);
    }

    //显示
    QMenu::showEvent(e);
}

void BsMeetMenu::onClicked()
{
    BsMeetAction *bsact = qobject_cast<BsMeetAction*>(sender());
    emit meetClicked(bsact->getMeetId(), bsact->getMeetName());
    hide();
}

}
