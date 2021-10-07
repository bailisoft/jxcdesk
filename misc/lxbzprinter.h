#ifndef LXBZPRINTER_H
#define LXBZPRINTER_H

#include <QtWidgets>
#include <QtSql>
#include <QPrinter>
#include <QPrinterInfo>
#include <QPrintDialog>

namespace BailiSoft {

class BsAbstractSheetWin;
class LxPreviewer;
class LxPrevPaper;

//对应数据库存储——nSecType：0页头，1页尾，2表体，3表头，4表尾
enum LxPrintSecType{lpstPageHeader, lpstPageFooter, lpstGridBody, lpstGridHeader, lpstGridFooter};

//纯数据类，无方法
class LxPrintUnit
{
public:
    explicit LxPrintUnit(const QString prValue, const QString prExp, const int prPosX, const int prPosY,
                         const int prWidth, const QString prFontName, const int prFontPoint, const int prFontAlign)
                        : mValue(prValue), mExp(prExp), mPosXmmOrColNo(prPosX), mPosYmmOrRowNo(prPosY), mWidth(prWidth),
                          mFontName(prFontName), mFontPoint(prFontPoint), mFontAlign(prFontAlign) {}
    QString mValue;             //用户设置用户看（中文）
    QString mExp;               //字段名或程序约定写法
    int     mPosXmmOrColNo;     //X向定位（表格则为列序）
    int     mPosYmmOrRowNo;     //Y向定位（表格则仅小票时有意义，代表行序）
    int     mWidth;
    QString mFontName;
    int     mFontPoint;
    int     mFontAlign;

    //根据mExp公式计算结果。每次打印前必须调用doCalculateAllObjects()计算获取。
    QString mText;      //最终打印值
};

//功能控件，非UI控件
class LxPrinter : public QObject
{
    Q_OBJECT
public:
    explicit LxPrinter(QWidget *parent);
    ~LxPrinter();
    void loadPrintSettings();  //从数据库读取
    void loadPreviewSettings(const QString &printerName,
                             const QString &paperName,
                             const int htPHeader,
                             const int htGHeader,
                             const int htGridRow,
                             const int htGFooter,
                             const int htPFooter,
                             const bool gridRowLinee,
                             const bool gridColLinee,
                             const bool gridColTitlee,
                             const QList<LxPrintUnit *> &lstPHeader,
                             const QList<LxPrintUnit *> &lstGHeader,
                             const QList<LxPrintUnit *> &lstGrid,
                             const QList<LxPrintUnit *> &lstGFooter,
                             const QList<LxPrintUnit *> &lstPFooter);     //从设置对话框调用取得
    QString doPreview(LxPreviewer *previewer);
    QString doPrint();
    bool testSetBailiSheetPaper(const QString &protocolPaperName);
    bool needQueryTraderSumOwe();
    void fetchSizeFromPaperName(const QString &protocolPaperName, int *pWidth, int *pHeight);

private:
    void painterDraw(QPrinter *printer, LxPreviewer *previewer, QPainter *painter);
    void loadPrintSectionSettings(const LxPrintSecType prType, int *mHtValue, QList<LxPrintUnit *> &prList);

    void doCalculateAllObjects();
    void calculateSideObjects(QList<LxPrintUnit *> *prList);
    void calculateGridObjects();
    int calculateTitleHeight(const int dataRowHeight, const int sizerTypeCount);
    int calculateSizerTitleFontPoint(const int dataRowFontPoint, const int sizerTypeCount);

    void doPaginateAllObjects(QPainter *painter);
    int  getGridWidth();

    void printPageSideSection(QPainter *painter, QList<LxPrintUnit *> *prList, const int iPage, const bool prFooter);
    void printGridSideSection(QPainter *painter, QList<LxPrintUnit *> *prList, const int yPos);

    void printGridColTitleToTicket(QPainter *painter, const int yPos, int *usedHt);                 //小票打印
    void printGridRowToTicket(QPainter *painter, const int yPos, const int iRowIdx, int *usedHt);   //小票打印

    void printGridColTitleToPage(QPainter *painter, const int yPos, int *usedHt);                   //分页打印
    void printGridRowToPage(QPainter *painter, const int yPos, const int iRowIdx);                  //分页打印

    inline int getUnitsByMm(const int prMm) { return int((mCurrentDpi * prMm) / 25.4); }

    QString     mPrinterName;
    QString     mPaperName;
    int         mCurrentDpi;

    bool        mTickett;
    int         mHtPHeader;
    int         mHtGHeader;
    int         mHtGridRow;
    int         mHtGFooter;
    int         mHtPFooter;
    bool        mGridPrintSpec;
    bool        mGridRowLinee;
    bool        mGridColLinee;
    bool        mGridColTitlee;

    QList<LxPrintUnit *>     mLstPHeader;
    QList<LxPrintUnit *>     mLstGHeader;
    QList<LxPrintUnit *>     mLstGrid;
    QList<LxPrintUnit *>     mLstGFooter;
    QList<LxPrintUnit *>     mLstPFooter;

    QList<int>      mLstPageFirstRow;  //存储每页第一行的mppWin->mpGrid表格行序号

    BsAbstractSheetWin*    mppSheet;
};

//预览容器
class LxPreviewer : public QWidget
{
    Q_OBJECT
public:
    explicit LxPreviewer(QWidget *parent);
    void initPreview(const int w, const int h);
    LxPrevPaper* newPaper();
    int                     mPaperWidth;
    int                     mPaperHeight;   //0表示小票不分页

private:
    void layoutPapers();
    QList<LxPrevPaper*>     lstPapers;
};

//预览页
class LxPrevPaper : public QWidget
{
    Q_OBJECT
public:
    explicit LxPrevPaper(LxPreviewer *parent);
    ~LxPrevPaper();
    QImage              *mpImage;
protected:
    void paintEvent(QPaintEvent *e);
};

}

#endif // LXBZPRINTER_H
