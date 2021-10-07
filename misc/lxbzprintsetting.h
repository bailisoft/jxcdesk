#ifndef LXBZPRINTSETTING_H
#define LXBZPRINTSETTING_H

#include <QtWidgets>
#include <QtSql>
#include <QPrinterInfo>

namespace BailiSoft {

class BsAbstractSheetWin;
class LxPrinter;
class LxPreviewer;
class LxPrintSection;
class LxPrintCell;

class LxPrintSettingWin : public QDialog
{
    Q_OBJECT
public:
    explicit LxPrintSettingWin(QWidget *parent);
    ~LxPrintSettingWin();
    inline int getPixelsByMMs(const int prMMs) { return int((prMMs * mScreenDpi) / 25.4); }
    inline int getMMsByPixels(const int prPixels) { return  int(25.4 * (prPixels / mScreenDpi)); }
    inline bool usingTicket() const {return mPaperHeight == 0;}

    QComboBox*      mpPrinters;
    QComboBox*      mpPapers;

    qreal           mScreenDpi;

    QString                 mTable;
    BsAbstractSheetWin*     mppSheet;
    LxPrinter*              mppPrinter;

protected:
    void keyPressEvent(QKeyEvent * e);

private slots:
    void doHelp();
    void doPrinterIndexChanged(int index);
    void doPaperIndexChanged(int index);
    void doTryAddCustomPaper();
    void showPreview();
    void showDesign();
    void doPrintTest();
    void doClearSettings();
    void doAcceptDlg();

private:
    void testSettings();
    void loadSettings();
    void saveSettings();
    bool sectionReasonable();
    void createDesignSections(const bool prNeedClear);

    int             mPaperHeight;

    QStackedLayout* mpStack;
    QWidget*            mpTabDesign;
    QPushButton*            mpBtnAddPaper;
    QPushButton*            mpBtnClear;
    QWidget*                mpDesignBar;
    QPushButton*                mpBtnPreview;
    QPushButton*                mpBtnPrint;
    QPushButton*                mpBtnSave;
    QWidget*                mpPnlDesign;
    QVBoxLayout*            mpLayDesign;
    LxPrintSection*             mpPHeader;
    LxPrintSection*             mpGHeader;
    LxPrintSection*             mpGrid;
    LxPrintSection*             mpGFooter;
    LxPrintSection*             mpPFooter;
    QWidget*            mpTabPreview;
    QScrollArea*            mpPreviewArea;
    LxPreviewer*                mpPreviewer;

    friend class LxPrintSection;
};

/*******************************************************************************/

class LxPrintSection : public QWidget
{
    Q_OBJECT
public:
    explicit LxPrintSection(const uint prType, LxPrintSettingWin *parent);

    void loadSettings();
    void saveSettings();
    void updateControls();
    void updateInfoTip();
    void clearCells();
    void doShowSideCell(LxPrintCell *cell);
    void doShowGridCell(LxPrintCell *cell, const bool onlyEditt = false);    //表格列由于用户只有删除重建设计法，onlyEditt不动。

    int                 mSecHtmm;     //纵向走纸长或表格单行行高
    uint                mType;
    LxPrintSettingWin*  mppDlg;
    QObject*            mppPickedObj;
    QVBoxLayout*        mpLaySec;

    QToolButton*        mpSecBar;
    QMenu*              mpMenuSec;
    QAction*            mpActObjNew;
    QAction*            mpActSecHt;
    QAction*            mpActNeedRowLine;       //仅用于分页表格属性
    QAction*            mpActNeedColLine;       //仅用于分页表格属性
    QAction*            mpActNeedColTitle;      //仅用于分页表格属性

    QMenu*              mpMenuObj;
    QAction*            mpActObjEdit;
    QAction*            mpActObjDrop;

protected:
    void paintEvent(QPaintEvent *e);

private slots:
    void doSetSecHeight();
    void doNewObject();
    void doEditObject();
    void doDropObject();

private:
    QHBoxLayout *appendGridNewHLayout();
    int getHLayoutIndexOfWidget(QWidget *w);
    int getVLayoutIndexOfWidget(QWidget *w);
    void doUpdateGridSecSreenSpace();
    void ajustTicketGridSecHeight();
    int  calcObjectNeedHt(const QString &prFontName, const int prFontPoint);
};

/*******************************************************************************/

class LxPrintCell : public QLabel
{
    Q_OBJECT
public:
    explicit LxPrintCell(const QString &prValue, const QString &prExp, const int prPosXmm, const int prPosYmm,
                        const int prWidthmm, const QString prFontName, const int prFontPoint,
                        const int prFontAlign, LxPrintSection *parent);
    void updateEditShow();
    void updateInfoTip();

    QString     mValue;
    QString     mExp;
    int         mPosXmmOrColNo;     //左距mm。但分页表格中指打印次序。
    int         mPosYmmOrRowNo;     //顶距或底距mm。但小票表格中指折行的行次；分页表格由于限制在一行，此值无意义。
    int         mWidthmm;           //限宽mm。
    QString     mFontName;
    int         mFontPoint;
    int         mFontAlign;

    bool        mIsValue;
    bool        mIsGrid;
    bool        mIsPFooter;

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);

private:
    LxPrintSection* mppSection;
    QPoint          mPtStart;
    bool            mMoved;
};

/*******************************************************************************/

class LxPrintCellDlg : public QDialog
{
    Q_OBJECT
public:
    explicit LxPrintCellDlg(const bool prForEdit, LxPrintSection *parent);

    QFormLayout*    mpLayForm;
    QComboBox*      mpEdtValue;
    QSpinBox*       mpEdtPosXmmOrColNo;
    QSpinBox*       mpEdtPosYmmOrRowNo;
    QSpinBox*       mpEdtWidthmm;
    QComboBox*      mpEdtFontName;
    QSpinBox*       mpEdtFontPoint;
    QComboBox*      mEdtpFontAlign;

    QCheckBox*      mpChkStartTickNewRow;

private slots:
    void valueIndexChanged(int index);
    void valueTextChanged(const QString & text);

private:
    void fillSimpleValue();
    void fillBomValue();

    LxPrintSection*         mppSection;
    BsAbstractSheetWin*     mppSheet;
    QPushButton*            mpBtnOk;
};

}

#endif // LXBZPRINTSETTING_H
