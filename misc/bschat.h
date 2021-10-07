#ifndef BSCHAT_H
#define BSCHAT_H

#include <QtWidgets>
#include <QtSql>

namespace BailiSoft {

class BsMeetMenu;
class BsMeetAction;

// BsChat =====================================================================
class BsChat : public QWidget
{
    Q_OBJECT
public:
    explicit BsChat(QWidget *parent, const QString &meetId, const QString &meetName);
    void appendMessage(const qint64 msgid, const QString &senderName, const qint64 sendEpoch,
                       const bool readd, const QString &content);
    void scrollToBottom();
    QString         mMeetId;
    QString         mMeetName;

protected:
    void resizeEvent(QResizeEvent *e);
    void showEvent(QShowEvent *e);

private:
    void resizeContentPanel();
    void loadMessages();
    void sendInputMessage();
    void onRequestOk(const QWidget*sender, const QStringList &fens);

    QScrollArea*        mpScrollArea;
    QWidget*            mpPnlContent;
    QVBoxLayout*        mpLayContent;
    QWidget*            mpPnlEditor;
    QTextEdit*          mpEdtInput;
    QToolButton*        mpBtnSend;
};


// BsMsgBubble =====================================================================
class BsMsgBubble : public QLabel
{
    Q_OBJECT
public:
    explicit BsMsgBubble(QWidget*parent, const qint64 msgid, const QString &content,
                         const bool readd, const bool selff);
    qint64  mMsgId;
    QString mContent;
    bool    mReadd;
    bool    mSelff;
protected:
    void mouseReleaseEvent(QMouseEvent *e);
};


// BsActWidget =====================================================================
class BsActWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BsActWidget(QWidget *parent, const QString &meetName, const int newsCount, const int nameWidth);
    QLabel*    mpLblName;
    QLabel*    mpLblNews;
};

// BsMeetAction =======================================================================
class BsMeetAction : public QWidgetAction
{
    Q_OBJECT
public:
    explicit BsMeetAction(QObject *parent, const QString &meetName, const QString &meetId,
                          const int newsCount, const int fixedWidth)
        : QWidgetAction(parent), mMeetName(meetName), mMeetId(meetId),
          mNewsCount(newsCount), mFixedWidth(fixedWidth) {}
    QString getMeetId() { return mMeetId; }
    QString getMeetName() { return mMeetName; }
protected:
    QWidget *createWidget(QWidget *parent) { return new BsActWidget(parent, mMeetName, mNewsCount, mFixedWidth); }
private:
    QString         mMeetName;
    QString         mMeetId;
    int             mNewsCount;
    int             mFixedWidth;
};


// BsMeetMenu =====================================================================
class BsMeetMenu : public QMenu
{
    Q_OBJECT
public:
    explicit BsMeetMenu(QMainWindow *parent) : QMenu(parent){}
signals:
    void meetClicked(const QString &meetid, const QString &meetname);
protected:
    void showEvent(QShowEvent *e);
private:
    void onClicked();

};


}

#endif // BSCHAT_H
