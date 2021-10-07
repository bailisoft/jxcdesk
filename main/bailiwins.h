#ifndef BAILIWINS_H
#define BAILIWINS_H

#include <QtWidgets>
#include <QtSql>

/************************** win类与grid类继承层次完全一致 **************************/

namespace BailiSoft {

class BsMain;
class BsSheetIdLabel;
class BsSheetCheckLabel;
class BsConCheck;
class BsFldEditor;
class BsFldBox;
class BsField;
class BsGrid;
class BsAbstractFormGrid;
class BsSheetGrid;
class BsSheetCargoGrid;
class BsSheetFinanceGrid;
class BsQueryGrid;
class BsAbstractModel;
class BsListModel;
class BsRegModel;
class BsSqlModel;
class BsQryCheckor;
class BsSheetStockPickGrid;
class LxPrinter;

enum bsWindowType { bswtMisc, bswtReg, bswtSheet, bswtQuery };

enum bsQryType {
    bsqtSumSheet    = 0x00000001,   //指单据一般合计
    bsqtSumCash     = 0x00000002,   //指只统计主表的欠款统计vijcgm, vijpfm, vijxsm
    bsqtSumMinus    = 0x00000004,   //指有减法在内的统计，vijcg, vijpf, vijxs, viycg, viypf, viykc, vijcgm, vijpfm, vijxsm
    bsqtSumOrder    = 0x00000008,   //只订单相关，vicgd, viycg, vipfd, viypf
    bsqtSumRest     = 0x00000010,   //指viycg, viypf, viykc
    bsqtSumStock    = 0x00000020,   //指viykc
    bsqtViewAll     = bsqtSumSheet | bsqtSumStock    //指viall
};

//以下序号设计必须与bssetloginer单元中initRightFlagNames中三列表完全一致，并且由于在数据库永久存储，所以一旦规定，不能再改。
enum bsRightRegis { bsrrOpen = 1, bsrrNew = 2, bsrrUpd = 4, bsrrDel = 8, bsrrExport = 16 };
enum bsRightSheet { bsrsOpen = 1, bsrsNew = 2, bsrsUpd = 4, bsrsDel = 8, bsrsCheck  = 16, bsrsPrint = 32, bsrsExport = 64 };
enum bsRightQuery { bsrqQty  = 1, bsrqMny = 2, bsrqDis = 4, bsrqPay = 8, bsrqOwe    = 16, bsrqPrint = 32, bsrqExport = 64 };

//关于Action状态控制，virtual BsAbstractFormWin::setEditable()基本控制，其余在后代setEditable()重载函数中，
//对一个个action列举设置即可。————好设计不是代码量少，是思路清晰简单，因此不要过度设计！
enum bsActionOnOff { bsacfDirty = 1, bsacfClean = 2, bsacfPlusId = 4, bsacfNotChk = 8, bsacfChecked = 16 };

// BsWin
class BsWin : public QWidget
{
    Q_OBJECT
public:
    explicit BsWin(QWidget *parent, const QString &mainTable, const QStringList &fields, const uint bsWinType);
    ~BsWin();
    virtual bool isEditing() = 0;
    BsField *getFieldByName(const QString &name);
    QString table() const { return mMainTable; }
    void setQuickDate(const QString &periodName, BsFldBox *dateB, BsFldBox *dateE, QToolButton *button);
    void exportGrid(const BsGrid* grid, const QStringList headerPairs = QStringList());
    QString pairTextToHtml(const QStringList &pairs, const bool lastRed = false);
    bool getOptValueByOptName(const QString &optName);

    QString             mMainTable;
    uint                mBsWinType;

public slots:
    void displayGuideTip(const QString &tip);
    void forceShowMessage(const QString &msg);
    void hideFoceMessage();

protected:
    void showEvent(QShowEvent *e);
    void closeEvent(QCloseEvent *e);

    virtual void doToolExport();

    BsMain          *mppMain;               //传入主窗
    BsGrid              *mpGrid;            //后代创建具体类
    BsQueryGrid         *mpQryGrid;
    BsAbstractFormGrid  *mpFormGrid;
    BsSheetGrid         *mpSheetGrid;
    BsSheetCargoGrid    *mpSheetCargoGrid;
    BsSheetFinanceGrid  *mpSheetFinanceGrid;

    QLabel          *mpGuide;
    QWidget         *mpPnlMessage;
    QLabel              *mpLblMessage;
    QPushButton         *mpBtnMessage;

    QToolBar        *mpToolBar;

    QAction         *mpAcToolSeprator;
    QMenu           *mpMenuToolCase;
    QMenu           *mpMenuOptionBox;
    QAction         *mpAcMainHelp;

    QAction         *mpAcToolExport;

    QAction         *mpAcOptGuideNotShowAnymore;

    QSplitter       *mpSpl;
    QWidget             *mpBody;
    QTabWidget          *mpTaber;

    QStatusBar      *mpStatusBar;

    QString         mGuideClassTip;
    QString         mGuideObjectTip;
    bool            mGuideTipSwitch;

    QString             mRightWinName;
    QList<BsField*>     mFields;     //只是主表字段

    //公用数据集
    BsListModel         *mpDsStype;
    BsSqlModel          *mpDsStaff;
    BsAbstractModel     *mpDsTrader;

    int                 mDiscDots;
    int                 mPriceDots;
    int                 mMoneyDots;

private slots:
    void clickHelp();
    void clickToolExport() { doToolExport(); }

    void clickOptGuideNotShowAnymore();

private:
    void loadAllOptionSettings();
    void saveAllOptionSettings();
};


// BsQryWin
class BsQryWin : public BsWin
{
    Q_OBJECT
public:
    explicit BsQryWin(QWidget *parent, const QString &name, const QStringList &fields, const uint qryFlags);
    ~BsQryWin();
    bool isEditing() {return false;}

protected:
    void showEvent(QShowEvent *e);
    void resizeEvent(QResizeEvent *e);

    void doToolExport();

    QAction *mpAcMainBackQry;
    QAction *mpAcMainPrint;

    QWidget             *mpPanel;
    QWidget                 *mpPnlCon;
    QToolButton               *mpBtnPeriod;
    BsConCheck                *mpConCheck;
    BsFldBox                  *mpConDateB;
    BsFldBox                  *mpConDateE;
    BsFldBox                  *mpConShop;
    BsFldBox                  *mpConTrader;
    BsFldBox                  *mpConCargo;          //也用为mpConSubject
    BsFldBox                  *mpConColorType;
    BsFldBox                  *mpConSizerType;
    QLabel                  *mpLblCon;

    QWidget             *mpPnlQryConfirm;
    QToolButton             *mpBtnBigOk;
    QToolButton             *mpBtnBigCancel;

private slots:
    void clickQuickPeriod();
    void clickQryExecute();
    void clickBigCancel();
    void clickQryBack();
    void clickPrint();
    void conCargoChanged(const QString &text);

private:
    void setFloatorGeometry();
    void startQueryRequest();
    void onSocketRequestOk(const QWidget *sender, const QStringList &fens);

    BsField*                mSizerField;
    uint                    mQryFlags;
    QStringList             mLabelPairs;
    QMap<QString, QString>  mapRangeCon;  //field, value
    QString                 mNetCommand;

};


// BsAbstractFormWin
class BsAbstractFormWin : public BsWin
{
    Q_OBJECT
public:
    explicit BsAbstractFormWin(QWidget *parent, const QString &name, const QStringList &fields, const uint bsWinType);
    ~BsAbstractFormWin() {}
    bool isEditing() {return mEditable;}
    QSize sizeHint() const;
    virtual void setEditable(const bool editt);

protected:
    void closeEvent(QCloseEvent *e);

    virtual void doNew() = 0;
    virtual void doEdit() = 0;
    virtual void doDel() = 0;
    virtual void doSave() = 0;
    virtual void doCancel() = 0;
    virtual void doToolImport() = 0;

    virtual bool isValidRealSheetId() = 0;
    virtual bool isSheetChecked() = 0;
    virtual bool mainNeedSaveDirty() = 0;

    QAction *mpAcMainNew;
    QAction *mpAcMainEdit;
    QAction *mpAcMainDel;
    QAction *mpAcMainSave;
    QAction *mpAcMainCancel;

    QAction *mpAcToolImport;
    QAction *mpToolHideCurrentCol;
    QAction *mpToolShowAllCols;

    QAction *mpAcOptHideDropRow;

    QLabel      *mpSttValKey;
    QLabel      *mpSttLblUpman;
    QLabel      *mpSttValUpman;
    QLabel      *mpSttLblUptime;
    QLabel      *mpSttValUptime;

    QStringList     mDenyFields;

private slots:
    void clickNew() { doNew(); }
    void clickEdit() { doEdit(); }
    void clickDel() { doDel(); }
    void clickSave() { doSave(); }
    void clickCancel() { doCancel(); }

    void clickToolImport() { doToolImport(); }

    void clickOptHideDropRow();

private:
    bool    mEditable;
};


// BsAbstractSheetWin
class BsAbstractSheetWin : public BsAbstractFormWin
{
    Q_OBJECT
public:
    explicit BsAbstractSheetWin(QWidget *parent, const QString &name, const QStringList &fields);
    ~BsAbstractSheetWin();
    void setEditable(const bool editt);
    void openSheet(const int sheetId);
    void savedReconcile(const int sheetId, const qint64 uptime);
    void cancelRestore();

    //以下主要用于打印调用
    QString getPrintValue(const QString &valueName) const;
    QString getPrintValue(const QString &cargoTableField, const int gridRow);
    QString getGridItemValue(const int row, const int col) const;
    QString getGridItemValue(const int row, const QString &fieldName) const;
    BsField* getGridFieldByName(const QString &fieldName) const;
    int getGridColByField(const QString &fieldName) const;
    int getGridRowCount() const;
    QStringList getSizerNameListForPrint() const;
    QStringList getSizerQtysOfRowForPrint(const int row);
    bool isLastRow(const int row);

    LxPrinter   *mpPrinter;
    bool         mAllowPriceMoney;
    int          mCurrentSheetId;

protected:
    void closeEvent(QCloseEvent *e);
    void resizeEvent(QResizeEvent *e);

    void doOpenFind();
    void doCheck();
    void doPrintRequest();
    void doPrintContinue();

    void doNew();
    void doEdit();
    void doDel();
    void doSave();
    void doCancel();

    bool isValidRealSheetId() { return mCurrentSheetId > 0; }
    bool isSheetChecked() { return ! mpSttValChkTime->text().isEmpty(); }
    bool mainNeedSaveDirty();
    double getTraderDisByName(const QString &name);

    virtual bool printZeroSizeQty() { return false; }
    virtual void doOpenQuery() = 0;
    virtual void doUpdateQuery() = 0;
    virtual void updateTabber(const bool editablee) = 0;

    QAction*    mpAcMainOpen;
    QAction*    mpAcMainCheck;
    QAction*    mpAcMainPrint;

    QAction*    mpToolPrintSetting;
    QAction*    mpToolUnCheck;
    QAction*    mpToolAdjustCurrentRowPosition;

    QAction*    mpAcOptSortBeforePrint;
    QAction*    mpAcOptHideNoQtySizerColWhenOpen;
    QAction*    mpAcOptHideNoQtySizerColWhenPrint;

    QVBoxLayout         *mpLayBody;

    QWidget                 *mpPnlOpener;
    QWidget                     *mpPnlOpenCon;
    QToolButton                     *mpBtnOpenBack;
    QToolButton                     *mpBtnPeriod;
    BsConCheck                      *mpConCheck;
    BsFldBox                      *mpConDateB;
    BsFldBox                      *mpConDateE;
    BsFldBox                      *mpConShop;
    BsFldBox                      *mpConStype;
    BsFldBox                      *mpConStaff;
    BsFldBox                      *mpConTrader;
    QToolButton                     *mpBtnQuery;
    BsQueryGrid                 *mpFindGrid;

    QWidget                 *mpPnlHeader;
    QHBoxLayout                 *mpLayTitleBox;
    QLabel                          *mpSheetName;
    BsSheetIdLabel                  *mpSheetId;
    BsSheetCheckLabel               *mpCheckMark;
    QGridLayout                 *mpLayEditBox;
    BsFldBox                      *mpDated;
    BsFldBox                      *mpStype;
    BsFldBox                      *mpShop;
    BsFldBox                      *mpProof;
    BsFldBox                      *mpStaff;
    BsFldBox                      *mpTrader;
    BsFldBox                  *mpRemark;

    QWidget                 *mpPnlPayOwe;
    BsFldBox                  *mpActPay;
    BsFldBox                  *mpActOwe;

    QLabel      *mpSttLblChecker;
    QLabel      *mpSttValChecker;
    QLabel      *mpSttLblChkTime;
    QLabel      *mpSttValChkTime;

    QToolButton *mpBtnSwitchLimBind;

    QList<BsField*>     mFindFlds;

private slots:
    void clickOpenFind() { doOpenFind(); }
    void clickCheck() { doCheck(); }
    void clickPrint() { doPrintRequest(); }

    void clickToolPrintSetting();
    void clickToolUnCheck();
    void clickToolAdjustCurrentRowPosition();

    void clickOptSortBeforePrint();

    void clickQuickPeriod();
    void clickExecuteQuery();
    void clickCancelOpenPage();
    void doubleClickOpenSheet(QTableWidgetItem *item);
    void clickSwitchLimTrader();

private:
    void relocateSwitchLimButton();
};


// BsSheetCargoWin
class BsSheetCargoWin : public BsAbstractSheetWin
{
    Q_OBJECT
public:
    explicit BsSheetCargoWin(QWidget *parent, const QString &name, const QStringList &fields);
    ~BsSheetCargoWin();

protected:
    void showEvent(QShowEvent *e);
    void doOpenQuery() {  /* client use continueQrySheet to open data */ }
    void doUpdateQuery();
    void updateTabber(const bool editablee);
    bool printZeroSizeQty() { return mpAcOptPrintZeroSizeQty->isChecked(); }

    void doToolExport();
    void doToolImport();

private slots:
    void traderPicked(const QString &text);
    void showCargoInfo(const QStringList &values);
    void sumMoneyChanged(const QString &sumValue);
    void actPayChanged();
    void taberIndexChanged(int index);
    void doToolDefineFieldName();
    void doToolImportBatchBarcodes();
    void pickStockTraderChecked();
    void loadPickStock();
    void onSocketRequestOk(const QWidget *sender, const QStringList &fens);

private:
    void restoreTaberMiniHeight();
    void loadFldsUserNameSetting();

    void continueQrySheet();
    void continueQryPick();
    void continueSheetSave(const int useSheetId, const qint64 uptime);
    void continueSheetDelete(const int deletedSheetId, const qint64 deletedUptime);

    BsListModel*        mpDsAttr1;
    BsListModel*        mpDsAttr2;
    BsListModel*        mpDsAttr3;
    BsListModel*        mpDsAttr4;
    BsListModel*        mpDsAttr5;
    BsListModel*        mpDsAttr6;

    QList<BsField*>     mGridFlds;
    BsField*            mPickSizerFld;
    BsField*            mPickAttr1Fld;
    BsField*            mPickAttr2Fld;
    BsField*            mPickAttr3Fld;
    BsField*            mPickAttr4Fld;
    BsField*            mPickAttr5Fld;
    BsField*            mPickAttr6Fld;

    QLabel*             mpPnlScan;
    QWidget*            mpPnlPick;
    BsSheetStockPickGrid*       mpPickGrid;
    QWidget*                    mpPickCons;
    BsFldEditor*                    mpPickSizerType;
    BsFldEditor*                    mpPickAttr1;
    BsFldEditor*                    mpPickAttr2;
    BsFldEditor*                    mpPickAttr3;
    BsFldEditor*                    mpPickAttr4;
    BsFldEditor*                    mpPickAttr5;
    BsFldEditor*                    mpPickAttr6;
    QCheckBox*                      mpPickDate;
    QCheckBox*                      mpPickCheck;
    QCheckBox*                      mpPickTrader;

    QAction*    mpAcToolDefineName;
    QAction*    mpAcToolImportBatchBarcodes;

    QAction*    mpAcOptPrintZeroSizeQty;
    QAction*    mpAcOptAutoUseFirstColor;
};

}



#endif // BAILIWINS_H
