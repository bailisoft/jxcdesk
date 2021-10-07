#ifndef BAILIGRID_H
#define BAILIGRID_H

#include <QtCore>
#include <QObject>
#include <QtWidgets>
#include <QtSql>

#define OFFSET_OLD_VALUE      0
#define OFFSET_EDIT_STATE     1
#define OFFSET_CELL_CHECK     2

#define SORT_TYPE_NUM         0
#define SORT_TYPE_TEXT        1000


/************************** win类与grid类继承层次完全一致 **************************/

namespace BailiSoft {

class BsWin;
class BsGrid;
class BsPickDelegate;

//提交表格时，【数据类型】和【统计类型】是最基本、必须提供的flag，特此说明。
enum BsFldFlag {
    //【数据类型（必须指定）】
    bsffText         = 0x00000001,
    bsffInt          = 0x00000002,
    bsffSpecial      = 0x00000004,              //sex, not used.

    //注：复合位判断不能简写(flags & bsffXXX)，必须完整((flags & bsffXXX) == bsffXXX)
    bsffBool         = 0x00000010 | bsffInt,    //stopp, sex, ...
    bsffDate         = 0x00000020 | bsffInt,    //dateD, ...
    bsffDateTime     = 0x00000040 | bsffInt,    //upTime, chktime, ...
    bsffNumeric      = 0x00000080 | bsffInt,    //所有需要SUM统计的数值类，包括qty

    bsffSex          = bsffBool | bsffSpecial,

    //【窗口位置类型（必须指定Head或Grid）】      //但xxxdtl.parentid字段，既不是bsffHead也不是bsffGrid
    bsffHead         = 0x00000100,
    bsffGrid         = 0x00000200,
    bsffCargoRel     = 0x00000400,

    //【主键特征】
    bsffKey          = 0x00001000,                //主键
    bsffPKey         = 0x00002000 | bsffKey,      //独立主键
    bsffSKey         = 0x00004000 | bsffKey,      //复合主键之一，比如xxxdtl.rowtime
    bsffFKey         = 0x00008000 | bsffSKey,     //复合主键之父键，比如xxdtl.parentid

    //【合计类型（必须指定AggXXX）】
    bsffAggNone      = 0x00010000,
    bsffAggCount     = 0x00020000,
    bsffAggSum       = 0x00040000,
    bsffSizeUnit     = 0x00080000,

    //【查询特征（查询窗口必须指定）】
    bsffQryAsCon     = 0x00100000,
    bsffQryAsSel     = 0x00200000,
    bsffQryAsVal     = 0x00400000,
    bsffQrySelBold   = 0x00800000,

    //【数量及隐藏属性】
    bsffBlankZero    = 0x01000000,              //进销存一览以及库存警报都是，并非只有bsffSizerUnit
    bsffHideSys      = 0x02000000,              //固定隐藏，绝不显示
//    bsffHideRun      = 0x04000000,              //动态运行用户随时设置（不需要使用，因为动态设置）
//    bsffHideCan      = 0x08000000,              //权限隐藏（未用，使用mDenyFields设置更方便）

    //【其他特征】
    bsffReadOnly     = 0x10000000,              //只读
    bsffLookup       = 0x20000000,              //指非存储仅显示用字段
    bsffRecalc       = 0x40000000,              //指非存储仅显示用字段
    bsffExternal     = 0x80000000               //外不字段，非存储字段
};

enum BsFilterType { bsftNone, bsftEqual, bsftNotEqual, bsftContain };
enum BsCellCheck { bsccNone, bsccWarning, bsccError };
enum bsEditState { bsesClean, bsesNew, bsesUpdated, bsesDeleted };

class BsField {
public:
    BsField(const QString fldName,
            const QString fldCnName,
            const uint flags,
            const int len_or_dots,
            const QString &statusTip)
        : mFldName(fldName),
          mFldCnName(fldCnName),
          mFlags(flags),
          mLenDots(len_or_dots),
          mStatusTip(statusTip),
          mAggValue(0),
          mFilterType(0)
    {}
    ~BsField(){ mCountSet.clear(); }
    QString             mFldName;
    QString             mFldCnName;
    uint                mFlags;
    int                 mLenDots;       //string.length or (integer/10000).dots
    QString             mStatusTip;
    qint64              mAggValue;
    QSet<QString>       mCountSet;
    uint                mFilterType;
    QStringList         mFilterValue;
};

//重设小数点定义
void resetFieldDotsDefin(BsField *bsFld);

// BsGridItem
class BsGridItem: public QTableWidgetItem
{
public:
    BsGridItem(const QString &text, int type) : QTableWidgetItem(text, type) {}
    //inline必须在头文件中
    inline bool operator< (const QTableWidgetItem &other) const
    {
        switch ( type() ) {
        case SORT_TYPE_TEXT:
            return (this->text() < other.text());
            break;
        default:
            return (this->text().toDouble() < other.text().toDouble());
        }
    }
};

// BsFilterSelector
class BsFilterSelector : public QWidget
{
    Q_OBJECT
public:
    BsFilterSelector(QWidget *parent);
    void setPicks(const QStringList &list, const QString picked);

signals:
    void pickFinished(const QStringList &picks);

private slots:
    void okClicked();
    void cancelClicked();

private:
    QListWidget             *mpList;
};

// BsHeader
class BsHeader : public QHeaderView
{
    Q_OBJECT
public:
    explicit BsHeader(BsGrid *parent);

    BsGrid  *mpGrid;
};

// BsFooter
class BsFooter : public QTableWidget
{
    Q_OBJECT
public:
    explicit BsFooter(QWidget *parent, BsGrid *grid);
    void initCols();
    void hideAggText();
    BsGrid  *mpGrid;
signals:
    void barcodeScanned(const QString &barcode);
public slots:
    void headerSectionResized(int logicalIndex, int oldSize, int newSize);
protected:
    void focusInEvent(QFocusEvent *e);
    void keyPressEvent(QKeyEvent *e);
private:
    QString     mProbableBarcode;
};


/**********************************华丽分割线，下面为表格**************************************/

// BsGrid
class BsGrid : public QTableWidget
{
    Q_OBJECT
public:
    explicit BsGrid(QWidget *parent, const bool forQry = false, const bool forReg = false);
    ~BsGrid();
    void loadData(const QString &sql, const QStringList &fldCnameDefines = QStringList(),
                  const QString &useSizerType = QString(), const bool joinCargoPinyin = false);
    void saveColWidths(const QString &sub = QString());
    void loadColWidths(const QString &sub = QString());
    void updateColTitleSetting();
    void cancelAllFilter();
    int  getDataSizerColumnIdx() const;
    QString addCalcMoneyColByPrice(const QString &priceField);

    QString getDisplayTextOfIntData(const qint64 intV, const uint flags, const int dots = 0);
    QString getSqlValueFromDisplay(const int row, const int col);
    BsField *getFieldByName(const QString &name, int *colIdx = 0);
    bool noMoreVisibleRowsAfter(const int currentVisibleRow);
    bool noMoreVisibleColsAfter(const int currentVisibleCol);
    QString getFooterValueByField(const QString &fieldName);
    int getColumnIndexByFieldName(const QString &fieldName);
    void hideFooterText();

    void setRowHeight(const int height);
    void setFontPoint(const int point);
    int getRowHeight() const;
    int getFontPoint() const;

    //主表（视图）名，全小写
    QString                     mTable;

    //此QList与表格列绝对完全一致，不论是隐藏列，还是动态临时横排尺码列。
    //如果是查询表格，则完全自new自delete；登记类完全外部负责；单据类则是
    //只负责自new自delete格横排尺码列部分，见构造函数、析构函数、loadData三处。
    QList<BsField*>             mCols;

    //为处理方便，一起载入，但为权限禁显的字段
    QStringList                 mDenyFields;

    //使用窗口，主要用于获取开关盒值
    BsWin*                      mppWin;

    //打印调用，所以public
    BsHeader            *mpHeader;
    BsFooter            *mpFooter;

signals:
    void filterDone();
    void filterEmpty();
    void focusInned();
    void focusOuted();
    void shootHintMessage(const QString &tip);
    void shootForceMessage(const QString &msg);
    void sheetSumMoneyChanged(const QString &sumValue);

public slots:
    void sortByRowTime();

protected:
    void showEvent(QShowEvent *e);
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);

    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

    void updateFooterSumCount(const bool checkFilter);
    void updateAllColTitles();
    void updateSizerColTitles(const int row);
    void updateFooterColWidths();
    void setSizerHCellsFromText(const int row,  const int qtyCol, const QString &sizersText, const QString &usingSizerType = QString());

    BsFilterSelector    *mpPicker;
    QToolButton         *mpCorner;

    QMenu               *mpMenu;
    QAction             *mpFilterIn;
    QAction             *mpFilterOut;
    QAction             *mpRestoreCol;
    QAction             *mpRestoreAll;

    //属性变量
    int                  mRowHeight;
    int                  mFontPoint;
    int                  mDiscDots;
    int                  mPriceDots;
    int                  mMoneyDots;

    bool                 mForQuery;     //恒定属性
    bool                 mForRegister;  //恒定属性
    bool                 mFiltering;    //状态属性，变动属性。
    bool                 mEditable;     //状态属性，变动属性。但查询表格始终为false

    //以下变量特用于有横排尺码的情况，如单据，或带明细的查询。每次openQuery()要重设
    int                 mSizerPrevCol;
    int                 mSizerColCount;
    QString             mLoadSizerType;

private slots:
    void filterIn();
    void filterOut();
    void filterRestoreCol();
    void filterRestoreAll();
    void requestImage(QTableWidgetItem *item);
    void adjustCornerPosition();
    void updateFooterGeometry();
    void takeFilterInPicks(const QStringList &picks);

private:
    QString sizerTextSum(const QString &str);

friend class BsFooter;
};



// BsQueryGrid
class BsQueryGrid : public BsGrid
{
    Q_OBJECT
public:
    explicit BsQueryGrid(QWidget *parent);
    void doPrint(const QString &title, const QStringList &conPairs,
                 const QString &printMan, const QString &printTime);
};



// BsSheetStockPickGrid
class BsSheetStockPickGrid : public BsGrid
{
    Q_OBJECT
public:
    explicit BsSheetStockPickGrid(QWidget *parent);
    void tryLocateCargoRow(const QString &cargo, const QString &color);
    void setPickDelta(const int delta);
    void updateHint(const QString &msgHint);

signals:
    void pickedCell(const QString &cargo, const QString &colorName, const QString &sizerName);
    void cargoRowSelected(const QString &cargo, const QString &color);
    //void helpHint(const QString &text);  //XXX

protected:
    void inputMethodEvent(QInputMethodEvent *);
    void keyPressEvent(QKeyEvent *e);
    void paintEvent(QPaintEvent *e);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private slots:
    void cellQtyDoubleClicked(QTableWidgetItem *item);

private:
    void updateHelpStatus();
    int         mPickDelta = -1;
    bool        mNeedFilterPressHint;
    bool        mNeedPickPressHint;
    QString     mMsgHint;
    QString     mKeyHint;
};



// BsAbstractFormGrid 还待用于杂项设置
class BsAbstractFormGrid : public BsGrid
{
    Q_OBJECT
public:
    explicit BsAbstractFormGrid(QWidget *parent, const bool forReg = false);
    void setAllowFlags(const bool inss, const bool updd, const bool dell);
    void appendNewRow();
    void updateRowState(const int row);
    uint saveCheck();
    void savedReconcile();
    void cancelRestore();
    void setEditable(const bool editable);
    bool getEditable() const;
    void setDroppedRowByOption(const bool hideDropRow);
    bool needSaveDirty();

signals:
    void shootCurrentRowSysValue(const QStringList &values);    //包含了主键值（最前）
    void barcodeScanned(const QString &barcode);

public slots:
    void hideCurrentCol();
    void showHiddenCols();

protected:
//    void inputMethodEvent(QInputMethodEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void commitData(QWidget *editor);
    void wheelEvent(QWheelEvent *e);
    void resizeEvent(QResizeEvent *e);

    void cancelRestorRow(const int row);
    void updateRowColor(int row);
    void updateRowButton(int row);

    virtual QStringList getSqliteLimitKeyFields(const bool forNew) = 0;
    virtual QStringList getSqliteLimitKeyValues(const int row, const bool forNew) = 0;

    QToolButton     *mpBtnRow;

private slots:
    void currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void rowButtonClicked();

private:
    bool    mAllowIns;
    bool    mAllowUpd;
    bool    mAllowDel;

    QString     mCurrentHeadText;
    QString     mProbableBarcode;
};



// BsSheetGrid
class BsSheetGrid : public BsAbstractFormGrid
{
    Q_OBJECT
public:
    explicit BsSheetGrid(QWidget *parent, const QString &table);
    void openBySheetId(const int sheetId);
    double getColSumByFieldName(const QString &fld);
    void adjustCurrentRowPosition();
    bool isCleanSort();

protected:
    void keyPressEvent(QKeyEvent *e);
    void commitData(QWidget *editor);
    QStringList getSqliteLimitKeyFields(const bool forNew);
    QStringList getSqliteLimitKeyValues(const int row, const bool forNew);

    int     mSheetId;           //-1空表新建态，0空表浏览态
};


// BsSheetCargoGrid
class BsSheetCargoGrid : public BsSheetGrid
{
    Q_OBJECT
public:
    explicit BsSheetCargoGrid(QWidget *parent, const QString &table, const QList<BsField*> &flds);
    ~BsSheetCargoGrid();
    void setTraderDiscount(const double dis);
    void setTraderName(const QString &traderName);
    QString inputNewCargoRow(const QString &cargo, const QString &colorCodeOrName,
                             const QString &sizerCodeOrName, const qint64 inputDataQty = 10000,
                             const bool scanNotImport = true);  //scan用code，Import用Name
    bool scanBarcode(const QString &barcode, QString *pCargo, QString *pColorCode, QString *pSizerCode);
    void uniteCargoColorPrice();
    QStringList getSizerNameListForPrint();
    QStringList getSizerQtysOfRowForPrint(const int row, const bool printZeroQty = false);
    void tryLocateCargoRow(const QString &cargo, const QString &color);
    QStringList getPostDataLines();

signals:
    void cargoRowSelected(const QString &cargo, const QString &color);

public slots:
    void addOneCargo(const QString &cargo, const QString &colorName, const QString &sizerName);
    void autoHideNoQtySizerCol();

protected:
    void commitData(QWidget *editor);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private slots:
    void scanBarocdeOneByOne(const QString &barcode);

private:
    bool commitCheckCargo(QTableWidgetItem *it);
    void commitCheckColor(QTableWidgetItem *it);
    void commitCheckSizer(QTableWidgetItem *it);

    void readyColor(const int row, const QString &cargo);
    void readySizer(const int row, const QString &cargo);
    void readyPrice(const int row, const QString &cargo);
    void readyHpRef(const int row, const QString &cargo);

    void checkShrinkSizeColCountForNewCargoCancel(const int row);
    void recalcRow(const int row, const int byColIndex);
    void updateHideSizersForSave(const int row);

    BsPickDelegate  *mpDelegateCargo;
    BsPickDelegate  *mpDelegateColor;

    int     mColorColIdx;
    double  mTraderDiscount = 1.0;
    QString mTraderName;
};


}



#endif // BAILIGRID_H
