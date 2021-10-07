#ifndef BSLOGINGUIDE_H
#define BSLOGINGUIDE_H

#include <QtWidgets>
#include <QSqlDatabase>
#include <QUdpSocket>

namespace BailiSoft {

class BsMain;

class BsLoginGuide : public QDialog
{
    Q_OBJECT
public:
    explicit BsLoginGuide(QWidget *parent, Qt::WindowFlags f = Qt::WindowFlags());
    void setVisible(bool visible);

    QString mTryingBackerId;
    QString mTryingBackerAes;
    QString mTryingUserName;
    QString mTryingUserPass;

protected:
    void resizeEvent(QResizeEvent *e);

private slots:
    void linkPicked(int);
    void linkDoubleClicked(QListWidgetItem *item);
    void doPrev();
    void doNext();
    void doOpenLink();
    void doDelLink();
    void doHelp();

private:
    void onLoginOk(const QWidget*sender, const QStringList &retList);
    void reloadBookList();
    void upgradeSqlCheck();

    BsMain          *mppMain;

    QWidget         *mpImageSide;
    QListWidget     *mpLinks;
    QComboBox       *mpUsers;
    QLineEdit       *mpPassword;
    QPushButton     *mpPrev;
    QPushButton     *mpNext;
    QPushButton     *mpCancel;
    QStackedLayout  *mpStack;
    QHBoxLayout     *mpLayDlgBtns;
    QAction         *mpAcDelLink;

    QLabel          *mpVerName;
};

//=========================================================================================

class BsBackerDlg : public QDialog
{
    Q_OBJECT
public:
    explicit BsBackerDlg(QWidget *parent);

    QLineEdit*      mpEdtBackerId;
    QLineEdit*      mpEdtBackerKey;

    QLineEdit*      mpEdtUserName;
    QLineEdit*      mpEdtUserPass;

protected:
    void showEvent(QShowEvent *e);

private:
    void onTextChanged(const QString &text);

    QPushButton*    mpBtnOk;
    QPushButton*    mpBtnCancel;

};

}

#endif // BSLOGINGUIDE_H
