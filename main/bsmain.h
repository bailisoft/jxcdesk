#ifndef BSMAIN_H
#define BSMAIN_H

#include <QMainWindow>
#include <QMdiArea>
#include <QtCore>
#include <QtWidgets>

namespace BailiSoft {

class BsMdiArea;
class BsMeetMenu;

class BsMain : public QMainWindow
{
    Q_OBJECT
public:
    BsMain(QWidget *parent = nullptr);
    bool eventFilter(QObject *watched, QEvent *event);
    BsMdiArea*  mpMdi;

protected:
    void closeEvent(QCloseEvent *e);

private slots:
    void openLoginGuide();

    void openRegCargo();
    void openRegSupplier();
    void openRegCustomer();

    void openSheetCGD();
    void openSheetCGJ();
    void openSheetCGT();
    void openQryVICGD();
    void openQryVICGJ();
    void openQryVICGT();
    void openQryViCgRest();
    void openQryViCgNeat();
    void openQryViCgCash();

    void openSheetPFD();
    void openSheetPFF();
    void openSheetPFT();
    void openSheetLSD();
    void openQryVIPFD();
    void openQryVIPFF();
    void openQryVIPFT();
    void openQryVILSD();
    void openQryViPfRest();
    void openQryViPfNeat();
    void openQryViPfCash();
    void openQryViXsNeat();
    void openQryViXsCash();

    void openSheetDBD();
    void openSheetSYD();
    void openQryVIDBD();
    void openQryVISYD();
    void openQryViStock();
    void openQryViewAll();

    void openToolBarcodeMaker();

    void openHelpManual();
    void openHelpSite();
    void openHelpAbout();

    void onMeetClicked(const QString &meetid, const QString &meetname);
    void onClickMsgCheckButton();
    void onClickNetStatus();
    void onNetOffline(const QString &reason);
    void onMsgArrived(const QStringList &fens);
    void onMeetingEvent(const QStringList &fens);
    void onSocketRequestOk(const QWidget*sender, const QStringList &retList);

private:
    bool checkRaiseSubWin(const QString &winTable);
    void addNewSubWin(QWidget *win, const bool setCenterr = false);
    void closeAllSubWin();
    bool questionCloseAllSubWin();
    void setMenuAllowable();
    void updateSheetDateForMemo();

    QAction* mpMenuCargo;
    QAction* mpMenuSupplier;
    QAction* mpMenuCustomer;

    QAction* mpMenuSheetCGD;
    QAction* mpMenuSheetCGJ;
    QAction* mpMenuSheetCGT;
    QAction* mpMenuQryVICGD;
    QAction* mpMenuQryVICGJ;
    QAction* mpMenuQryVICGT;
    QAction* mpMenuQryVIYCG;
    QAction* mpMenuQryVIJCG;
    QAction* mpMenuQryVIJCGM;

    QAction* mpMenuSheetPFD;
    QAction* mpMenuSheetPFF;
    QAction* mpMenuSheetPFT;
    QAction* mpMenuSheetLSD;
    QAction* mpMenuQryVIPFD;
    QAction* mpMenuQryVIPFF;
    QAction* mpMenuQryVIPFT;
    QAction* mpMenuQryVILSD;
    QAction* mpMenuQryVIYPF;
    QAction* mpMenuQryVIJPF;
    QAction* mpMenuQryVIJPFM;
    QAction* mpMenuQryVIJXS;
    QAction* mpMenuQryVIJXSM;

    QAction* mpMenuSheetDBD;
    QAction* mpMenuSheetSYD;
    QAction* mpMenuQryVIDBD;
    QAction* mpMenuQryVISYD;
    QAction* mpMenuQryVIYKC;
    QAction* mpMenuQryVIALL;

    QAction* mpMenuToolBarcodeMaker;

    QLabel*         mpLoginedFile;
    QLabel*         mpLoginedUser;
    QLabel*         mpLoginedRole;
    QToolButton*    mpBtnNetMessage;
    QToolButton*    mpBtnNetStatus;
    BsMeetMenu*    mpMeetMenu;

    bool            mOnlinee = false;
};

// BsMdiArea ==============================================================================

class BsMdiArea : public QMdiArea
{
    Q_OBJECT
public:
    BsMdiArea(QWidget *parent);
protected:
    void paintEvent(QPaintEvent *e);
private:
    QMainWindow*     mppMain;
};

}

#endif // BSMAIN_H
