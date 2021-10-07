#include "lxbzprinter.h"
#include "main/bailiwins.h"
#include "main/bailigrid.h"
#include "main/bailicode.h"

#define PRINTER_LOST        "没找到指定打印机："
#define PAPER_LOST          "没找到指定类型纸张："
#define FOUNDNOT_SETTINGS   "本单尚未进行打印设置！"

namespace BailiSoft {

LxPrinter::LxPrinter(QWidget *parent) : QObject(parent)
{
    mppSheet = qobject_cast<BsAbstractSheetWin*>(parent);
    Q_ASSERT(mppSheet);

    mHtPHeader = 0;
    mHtGHeader = 0;
    mHtGridRow = 5;
    mHtGFooter = 0;
    mHtPFooter = 0;

    loadPrintSettings();
}

LxPrinter::~LxPrinter()
{
    qDeleteAll(mLstPHeader);
    qDeleteAll(mLstPFooter);
    qDeleteAll(mLstGHeader);
    qDeleteAll(mLstGFooter);
    qDeleteAll(mLstGrid);

    mLstPHeader.clear();
    mLstPFooter.clear();
    mLstGHeader.clear();
    mLstGFooter.clear();
    mLstGrid.clear();
}

void LxPrinter::loadPrintSettings()
{
    QString sql = QStringLiteral("SELECT sPrinterName, sPaperName FROM lxPrintPage WHERE sBizName='%1';")
            .arg(mppSheet->table());

    QSqlQuery qry;
    qry.setForwardOnly(true);
    qry.setNumericalPrecisionPolicy(QSql::LowPrecisionInt64);
    qry.exec(sql);
    if ( qry.next() )
    {
        mPrinterName = qry.value(0).toString();
        mPaperName = qry.value(1).toString();
        mTickett = mPaperName.isEmpty();
    }
    else
    {
        mPrinterName = QString();
        mPaperName = QString();
        mTickett = false;
    }

    loadPrintSectionSettings(lpstPageHeader, &mHtPHeader, mLstPHeader);
    loadPrintSectionSettings(lpstGridHeader, &mHtGHeader, mLstGHeader);
    loadPrintSectionSettings(lpstGridBody,   &mHtGridRow, mLstGrid);
    loadPrintSectionSettings(lpstGridFooter, &mHtGFooter, mLstGFooter);
    loadPrintSectionSettings(lpstPageFooter, &mHtPFooter, mLstPFooter);
}

void LxPrinter::loadPreviewSettings(const QString &printerName,
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
                                      const QList<LxPrintUnit *> &lstPFooter)
{
    mPrinterName    = printerName;
    mPaperName      = paperName;
    mHtPHeader      = htPHeader;
    mHtGHeader      = htGHeader;
    mHtGridRow      = htGridRow;
    mHtGFooter      = htGFooter;
    mHtPFooter      = htPFooter;
    mGridRowLinee   = gridRowLinee;
    mGridColLinee   = gridColLinee;
    mGridColTitlee  = gridColTitlee;
    mLstPHeader     = lstPHeader;
    mLstGHeader     = lstGHeader;
    mLstGrid        = lstGrid;
    mLstGFooter     = lstGFooter;
    mLstPFooter     = lstPFooter;

    mCurrentDpi = int(qApp->primaryScreen()->logicalDotsPerInch());  //经实量测试，不能用physicalDotsPerInch
    mTickett    = mPaperName.isEmpty();
}

QString LxPrinter::doPreview(LxPreviewer *previewer)
{
    //检测打印机
    if ( mPrinterName.isEmpty() )
        return QStringLiteral(FOUNDNOT_SETTINGS);

    QPrinterInfo prnInfo = QPrinterInfo::printerInfo(mPrinterName);
    if ( prnInfo.isNull() )
        return QStringLiteral(PRINTER_LOST) + mPrinterName;

    //分页
    if ( testSetBailiSheetPaper(mPaperName) ) {
        int w, h;
        fetchSizeFromPaperName(mPaperName, &w, &h);
        QPageSize pageSize(QSizeF(w, h), QPageSize::Millimeter, QString(), QPageSize::ExactMatch);
        previewer->initPreview(pageSize.sizePixels(mCurrentDpi).width(),
                               pageSize.sizePixels(mCurrentDpi).height());
    }
    else if ( ! mPaperName.isEmpty() )
    {
        QList<QPageSize> pss = prnInfo.supportedPageSizes();
        bool foundPaper = false;
        for ( int i = 0, iLen = pss.count(); i < iLen; ++i ) {
            QPageSize ps = pss.at(i);
            if ( ps.name() == mPaperName ) {
                previewer->initPreview(ps.sizePixels(mCurrentDpi).width(),
                                       ps.sizePixels(mCurrentDpi).height());
                foundPaper = true;
                break;
            }
        }
        if ( ! foundPaper )
            return QStringLiteral(PAPER_LOST) + mPaperName;
    }
    //小票
    else {
        previewer->initPreview(500, 0);   //height必须填0
    }

    //预览
    QPainter painter(previewer->newPaper()->mpImage);
    painterDraw(nullptr, previewer, &painter);

    return QString();
}

QString LxPrinter::doPrint()
{
    //检测打印机
    if ( mPrinterName.isEmpty() )
        return QStringLiteral(FOUNDNOT_SETTINGS);

    QPrinterInfo prnInfo = QPrinterInfo::printerInfo(mPrinterName);
    if ( prnInfo.isNull() )
        return QStringLiteral(PRINTER_LOST) + mPrinterName;

    //打印机实例
    QPrinter printer(prnInfo);
    mCurrentDpi = printer.resolution();     //必须， 内联函数getUnitsByMm()需要使用

    //检测纸张
    if ( testSetBailiSheetPaper(mPaperName) ) {
        int w, h;
        fetchSizeFromPaperName(mPaperName, &w, &h);
        QPageSize pageSize(QSizeF(w, h), QPageSize::Millimeter, QString(), QPageSize::ExactMatch);
        if ( ! printer.setPageSize(pageSize) )
            return mapMsg.value("i_printer_not_support_this_paper_size") + mPaperName;
    }
    else if ( ! mPaperName.isEmpty() )
    {
        QList<QPageSize> pss = prnInfo.supportedPageSizes();
        bool foundPaper = false;
        for ( int i = 0, iLen = pss.count(); i < iLen; ++i ) {
            if ( pss.at(i).name() == mPaperName ) {
                printer.setPageSize(pss.at(i));
                foundPaper = true;
                break;
            }
        }
        if ( ! foundPaper )
            return QStringLiteral(PAPER_LOST) + mPaperName;
    }

#ifdef QT_DEBUG
    QString deskPath = QStandardPaths::locate(QStandardPaths::DesktopLocation,"", QStandardPaths::LocateDirectory);
    QString filepath = QFileDialog::getSaveFileName(mppSheet,
                                                    QStringLiteral("保存为..."),
                                                    QDir(deskPath).absoluteFilePath("PrintOut.pdf"),
                                                    QStringLiteral("PDF格式(*.pdf)"));
    if ( filepath.isEmpty() )
        return QString();
    printer.setOutputFileName(filepath);
    printer.setOutputFormat(QPrinter::PdfFormat);
#endif

    //打印
    QPainter painter(&printer);
    painterDraw(&printer, nullptr, &painter);

    return QString();
}

bool LxPrinter::testSetBailiSheetPaper(const QString &protocolPaperName)
{
    return ( protocolPaperName.startsWith(mapMsg.value("word_baili_sheet_paper"))
         && protocolPaperName.endsWith(")")
         && protocolPaperName.indexOf(QChar('(')) > 1
         && protocolPaperName.indexOf(QChar('x')) > mPaperName.indexOf(QChar('(')) + 1
             && protocolPaperName.indexOf(QChar(')')) > mPaperName.indexOf(QChar('x')) + 1);
}

bool LxPrinter::needQueryTraderSumOwe()
{
    //mExp中此时已经没有=开头
    for ( int i = 0, iLen = mLstPHeader.length(); i < iLen; ++i ) {
        LxPrintUnit *unit = mLstPHeader.at(i);
        if ( unit->mExp.endsWith(QStringLiteral("sumowe")) ) {
            return true;
        }
    }

    for ( int i = 0, iLen = mLstGHeader.length(); i < iLen; ++i ) {
        LxPrintUnit *unit = mLstGHeader.at(i);
        if ( unit->mExp.endsWith(QStringLiteral("sumowe")) ) {
            return true;
        }
    }

    for ( int i = 0, iLen = mLstGFooter.length(); i < iLen; ++i ) {
        LxPrintUnit *unit = mLstGFooter.at(i);
        if ( unit->mExp.endsWith(QStringLiteral("sumowe")) ) {
            return true;
        }
    }

    for ( int i = 0, iLen = mLstPFooter.length(); i < iLen; ++i ) {
        LxPrintUnit *unit = mLstPFooter.at(i);
        if ( unit->mExp.endsWith(QStringLiteral("sumowe")) ) {
            return true;
        }
    }

    return false;
}

void LxPrinter::fetchSizeFromPaperName(const QString &protocolPaperName, int *pWidth, int *pHeight)
{
    int posFrom = protocolPaperName.indexOf(QChar('('));
    int posTo = protocolPaperName.indexOf(QChar(')'));
    QString sizeStr = protocolPaperName.mid( posFrom + 1, posTo - posFrom - 1);
    QStringList spair = sizeStr.split(QChar('x'));
    if ( spair.length() == 2 )
    {
        *pWidth  = QString(spair.at(0)).toInt();
        *pHeight = QString(spair.at(1)).toInt();
    }
    else {
        *pWidth  = 0;
        *pHeight = 0;
    }
}

void LxPrinter::painterDraw(QPrinter *printer, LxPreviewer *previewer, QPainter *painter)
{
    //计算替换动态值（页码数据及表格行数据，只是取得公式，仍然不是最终打印值）
    doCalculateAllObjects();

    //设置画笔
    QPainter *pp = painter;

    //打印
    if ( mTickett )
    {
        //表头
        printGridSideSection(pp, &mLstGHeader, 0);

        //表体
        int yPos = getUnitsByMm(mHtGHeader);

        if ( mGridColTitlee )
        {
            int usedHt;
            printGridColTitleToTicket(pp, yPos, &usedHt);
            yPos += usedHt;
        }

        for ( int i = 0, iLen = mppSheet->getGridRowCount(); i < iLen; ++i )
        {
            int usedHt;
            printGridRowToTicket(pp, yPos, i, &usedHt);
            yPos += usedHt;
        }

        //表尾使用上面yPos中间结果
        printGridSideSection(pp, &mLstGFooter, yPos);
    }
    else
    {
        //先估算页，得到正确的mLstPageFirstRow结果
        doPaginateAllObjects(pp);

        //保存换页产生的预览画笔，用于用完清除。
        QList<QPainter *>   lstPreviewPainter;

        //按mLstPageFirstRow分页打印
        for ( int i = 0; i < mLstPageFirstRow.size(); ++i )
        {
            //换页
            if ( i > 0 ) {
                if ( printer ) {
                    printer->newPage();
                }
                else {
                    LxPrevPaper *newPaper = previewer->newPaper();
                    QPainter *newPainter = new QPainter(newPaper->mpImage);
                    lstPreviewPainter << newPainter;
                    pp = newPainter;
                }
            }

            //每页打印
            printPageSideSection(pp, &mLstPHeader, i + 1, false);
            printPageSideSection(pp, &mLstPFooter, i + 1, true);

            //首页打印
            int yPos = getUnitsByMm(mHtPHeader) + 1;
            if ( i == 0 )
            {
                 printGridSideSection(pp, &mLstGHeader, yPos);
                 yPos += getUnitsByMm(mHtGHeader) + 1;
            }

            //表体
            if ( mGridColTitlee )
            {
                int usedHt;
                printGridColTitleToPage(pp, yPos, &usedHt);
                yPos += usedHt;
            }

            int iStartGridRow = mLstPageFirstRow.at(i);
            int iEndGridRow = ( i < mLstPageFirstRow.size() - 1 )
                    ? mLstPageFirstRow.at(i + 1) - 1
                    : mppSheet->getGridRowCount();              //==RowCount行为合计行

            if ( i < mLstPageFirstRow.size() - 1 && iEndGridRow < 0 )
                iEndGridRow = mppSheet->getGridRowCount();      //==RowCount行为合计行

            if ( i == mLstPageFirstRow.size() - 1 && iStartGridRow < 0 )
                iEndGridRow = -9;  //让下面的for语句不执行

            for ( int row = iStartGridRow;  row <= iEndGridRow; ++row )
            {
                printGridRowToPage(pp, yPos, row);
                yPos += getUnitsByMm(mHtGridRow);
            }

            //尾页打印
            if ( i == mLstPageFirstRow.size() - 1 )
                printGridSideSection(pp, &mLstGFooter, yPos);
        }

        //预览清除画笔
        qDeleteAll(lstPreviewPainter);
        lstPreviewPainter.clear();
    }
}

void LxPrinter::loadPrintSectionSettings(const LxPrintSecType prType, int *mHtValue,
                                        QList<LxPrintUnit *> &prList)
{
    QSqlQuery qry;
    qry.setForwardOnly(true);
    qry.setNumericalPrecisionPolicy(QSql::LowPrecisionInt64);
    QString sql = QStringLiteral("SELECT nSecHeight, nRowLine, nColLine, nColTitle FROM lxPrintSec "
                                 "WHERE (sBizName='%1') AND (nSecType=%2); ").arg(mppSheet->table()).arg(prType);
    qry.exec(sql);
    if ( qry.next() )
    {
        *mHtValue = qry.value(0).toInt();
        if ( prType == lpstGridBody )
        {
            mGridRowLinee  = qry.value(1).toBool();
            mGridColLinee  = qry.value(2).toBool();
            mGridColTitlee = qry.value(3).toBool();
        }
    }

    qDeleteAll(prList);
    prList.clear();

    qry.exec(QStringLiteral("SELECT sValue, sExp, nPosX, nPosY, nWidth, sFontName, nFontPoint, "
                            "nFontAlign FROM lxPrintObj "
                            "WHERE (sBizName='%1') AND (nSecType=%2) ORDER BY nPosX; ")     //ORDER BY重要！！！因为nPosX还有ColNo意义
             .arg(mppSheet->table()).arg(prType));
    while ( qry.next() )
    {
        prList.append(new LxPrintUnit(qry.value(0).toString(),
                                      qry.value(1).toString(),
                                      qry.value(2).toInt(),
                                      qry.value(3).toInt(),
                                      qry.value(4).toInt(),
                                      qry.value(5).toString(),
                                      qry.value(6).toInt(),
                                      qry.value(7).toInt() ) );
    }
}

void LxPrinter::doCalculateAllObjects()
{
    if ( ! mTickett )
    {
        calculateSideObjects(&mLstPHeader);
        calculateSideObjects(&mLstPFooter);
    }
    calculateSideObjects(&mLstGHeader);
    calculateSideObjects(&mLstGFooter);

    calculateGridObjects();
}

void LxPrinter::calculateSideObjects(QList<LxPrintUnit *> *prList)
{
    for ( int i = 0, iLen = prList->size(); i < iLen; ++i )
    {
        LxPrintUnit *pObj = prList->at(i);

        if ( pObj->mValue.at(0) == QChar('=') )
        {
            if ( pObj->mExp.startsWith(QString("page")) ) {
                pObj->mText = "=" + pObj->mExp;     //记为=mExp，值延后计算
            }
            else {
                pObj->mText = mppSheet->getPrintValue(pObj->mExp);
            }
        }
        else
        {
            pObj->mText = pObj->mValue;
        }
    }
}

void LxPrinter::calculateGridObjects()
{
    for ( int i = 0, iLen = mLstGrid.size(); i < iLen; ++i )
    {
        LxPrintUnit *pObj = mLstGrid.at(i);

        if ( pObj->mValue.at(0) == QChar('=') )
        {
            int colIdx = mppSheet->getGridColByField(pObj->mExp);
            if ( colIdx >= 0 )
                pObj->mText = QString::number(colIdx);   //记为列索引，值延后计算
            else
                pObj->mText = pObj->mExp;   //货号登记列或行号，记为cargo表字段名，值延后计算
        }
        else
        {
            pObj->mText.clear();
            pObj->mText.append(pObj->mValue);
        }
    }
}

int LxPrinter::calculateTitleHeight(const int dataRowHeight, const int sizerTypeCount)
{
    switch ( sizerTypeCount ) {
    case 1:
        return dataRowHeight;
    case 2:
        return (dataRowHeight * sizerTypeCount * 2) / 3;
    default:
        return (dataRowHeight * sizerTypeCount) / 2;
    }
}

int LxPrinter::calculateSizerTitleFontPoint(const int dataRowFontPoint, const int sizerTypeCount)
{
    switch ( sizerTypeCount ) {
    case 1:
        return dataRowFontPoint;
    case 2:
        return (dataRowFontPoint * 3) / 4;
    default:
        return (dataRowFontPoint * 2) / 3;
    }
}

void LxPrinter::doPaginateAllObjects(QPainter *painter)
{
    mLstPageFirstRow.clear();
    mLstPageFirstRow.append(0);

    //基本行高值（小票格式中mHtGridRow仅表示行数，但小票格式不用此函数）
    int rowHt = getUnitsByMm(mHtGridRow);

    //列标题使用高（约定列头使用半行高。注：这不会干扰套打，套打不会打列头的）
    int titleHt = 0;
    if ( mGridColTitlee) {
        titleHt = (mppSheet->table() == QStringLiteral("szd"))
                ? rowHt : calculateTitleHeight(rowHt, mppSheet->getSizerNameListForPrint().count());
    }

    //每页扣除页头页尾余单可打印高
    int restHeightExcludePageHeaderFooter =
            painter->window().height() - getUnitsByMm(mHtPHeader) - getUnitsByMm(mHtPFooter);

    //每页开始打印时的Y坐标（页头结束处为零点参照系）
    int currentY = getUnitsByMm(mHtGHeader) + titleHt;

    //表体
    for ( int i = 0, iLen = mppSheet->getGridRowCount(); i <= iLen; ++i )   //因为加上合计行，所以用<=iLen
    {
        if ( currentY + titleHt + rowHt > restHeightExcludePageHeaderFooter )
        {
            currentY = titleHt;
            mLstPageFirstRow.append(i);
        }
        currentY += rowHt;
    }

    //表尾块
    if ( currentY + getUnitsByMm(mHtGFooter) > restHeightExcludePageHeaderFooter )
        mLstPageFirstRow.append(-1);            //特殊标记，表示最后表格尾内容需要换新页。-1用不到，但需要增加占数。
}

int LxPrinter::getGridWidth()
{
    int w = 0;
    for ( int i = 0, iLen = mLstGrid.size(); i < iLen; ++i )
    {
        LxPrintUnit *pObj = mLstGrid.at(i);
        w += getUnitsByMm(pObj->mWidth);
    }
    return w;
}

void LxPrinter::printPageSideSection(QPainter *painter, QList<LxPrintUnit *> *prList, const int iPage,
                                       const bool prFooter)
{
    for ( int i = 0, iLen = prList->size(); i < iLen; ++i )
    {
        LxPrintUnit *pObj = prList->at(i);

        //求值。因为pObj->mText值要在多行循环使用，故不能改变
        QString strPrint = pObj->mText;
        if ( pObj->mText == "=pagenum" )
            strPrint = QString::number(iPage);
        if ( pObj->mText == "=pagettl" )
            strPrint = QString::number(mLstPageFirstRow.size());

        //字体
        QFont font(pObj->mFontName);
        font.setPointSize(pObj->mFontPoint);
        painter->setFont(font);

        //绘制
        int preMaxH = getUnitsByMm((prFooter) ? mHtPFooter : mHtPHeader );
        int w = getUnitsByMm(pObj->mWidth);
        int h = painter->boundingRect(0, 0, w, preMaxH, Qt::TextWordWrap, pObj->mText).height();
        int x = getUnitsByMm(pObj->mPosXmmOrColNo);
        int y = (prFooter)
                ? painter->window().height() - getUnitsByMm(pObj->mPosYmmOrRowNo) - h
                : getUnitsByMm(pObj->mPosYmmOrRowNo);
        painter->drawText(x, y, w, h, pObj->mFontAlign | Qt::AlignTop, strPrint);
    }
}

void LxPrinter::printGridSideSection(QPainter *painter, QList<LxPrintUnit *> *prList, const int yPos)
{
    for ( int i = 0, iLen = prList->size(); i < iLen; ++i )
    {
        LxPrintUnit *pObj = prList->at(i);

        //字体
        QFont font(pObj->mFontName);
        font.setPointSize(pObj->mFontPoint);
        painter->setFont(font);
        QFontMetrics fm = painter->fontMetrics();

        //绘制
        int w = getUnitsByMm(pObj->mWidth);
        int h = fm.height();
        int x = getUnitsByMm(pObj->mPosXmmOrColNo);
        int y = yPos + getUnitsByMm(pObj->mPosYmmOrRowNo);
        painter->drawText(x, y, w, h, pObj->mFontAlign | Qt::AlignTop, pObj->mText);
    }
}

void LxPrinter::printGridColTitleToTicket(QPainter *painter, const int yPos, int *usedHt)
{
    int xOff = 0, yOff = 0;
    int rowHt = getUnitsByMm(mHtGridRow);
    LxPrintUnit *pObj = nullptr;

    for ( int i = 0, iLen = mLstGrid.size(); i < iLen; ++i )
    {
        pObj = mLstGrid.at(i);

        //列名
        QString strPrint = pObj->mValue.mid(1);

        //字体
        QFont font(pObj->mFontName);
        font.setPointSize(pObj->mFontPoint);
        painter->setFont(font);

        //绘制主表格各列数据
        int w = getUnitsByMm(pObj->mWidth);
        if ( pObj->mPosXmmOrColNo == 1 )
        {
            xOff = 0;
            yOff += rowHt;
        }
        painter->drawText(xOff, yPos + yOff, w, rowHt, pObj->mFontAlign | Qt::AlignVCenter, strPrint);
        xOff += w;
    }

    //增加半行高的空白
    *usedHt = yOff + rowHt / 2;
}

void LxPrinter::printGridRowToTicket(QPainter *painter, const int yPos, const int iRowIdx, int *usedHt)
{
    int xOff = 0, yOff = 0;
    int rowHt = getUnitsByMm(mHtGridRow);
    LxPrintUnit *pObj = nullptr;

    //各数据
    for ( int i = 0, iLen = mLstGrid.size(); i < iLen; ++i )
    {
        pObj = mLstGrid.at(i);

        //求值。因为pObj->mText值要在多行循环使用，故不能改变
        bool validNum;        
        int iColIdx = pObj->mText.toInt(&validNum);

        QString strPrint;
        if ( validNum && iColIdx >= 0 )
        {
            QString dataStr = mppSheet->getGridItemValue(iRowIdx, iColIdx);
            if ( pObj->mExp.contains(QStringLiteral("sizers")) ) {
                QStringList txts;
                QStringList pairs = dataStr.split(QChar(10));
                for ( int j = 0, jLen = pairs.length(); j < jLen; ++j )
                {
                    QStringList pair = QString(pairs.at(j)).split(QChar(9));
                    QString sname = pair.at(0);
                    QString sqty = bsNumForRead(QString(pair.at(1)).toLongLong(), 0);
                    txts << QStringLiteral("%1/%2").arg(sname).arg(sqty);
                }
                strPrint = txts.join(QStringLiteral(", "));
            }
            else
                strPrint = dataStr;
        }
        else
            strPrint = mppSheet->getPrintValue(pObj->mText, iRowIdx);

        //字体
        QFont font(pObj->mFontName);
        font.setPointSize(pObj->mFontPoint);
        painter->setFont(font);

        //绘制主表格各列数据
        int w = getUnitsByMm(pObj->mWidth);
        if ( pObj->mPosXmmOrColNo == 1 )
        {
            xOff = 0;
            yOff += rowHt;
        }
        painter->drawText(xOff, yPos + yOff, w, rowHt, pObj->mFontAlign | Qt::AlignVCenter, strPrint);
        xOff += w;
    }

    //增加半行高的空白
    *usedHt = yOff + rowHt / 2;
}

void LxPrinter::printGridColTitleToPage(QPainter *painter, const int yPos, int *usedHt)
{
    //单行高、表格宽
    int rowHt = getUnitsByMm(mHtGridRow);
    int gridW = getGridWidth();

    //标题总高(预设)
    int titleHt = rowHt;

    //尺码数据
    QStringList sizerNameList;
    int sizesTypeCount = 1;
    int sizerCols = 1;

    //补设
    if ( mppSheet->table() != QStringLiteral("szd") ) {
        sizerNameList = mppSheet->getSizerNameListForPrint();
        sizesTypeCount = sizerNameList.count();
        sizerCols = QString(sizerNameList.at(0)).split(QChar(9)).count();
        titleHt = calculateTitleHeight(rowHt, sizesTypeCount);
    }

    //标题顶线、边线
    if ( mGridRowLinee || mGridColLinee ) {
        QPen pen;
        pen.setWidth(2);
        painter->setPen(pen);
    }

    if ( mGridRowLinee )
        painter->drawLine(QPoint(1, yPos), QPoint(gridW - 1, yPos));

    if ( mGridColLinee )
        painter->drawLine(QPoint(0, yPos), QPoint(0, yPos + titleHt));

    //恢复细线
    QPen pen;
    pen.setWidth(1);
    painter->setPen(pen);

    //位置
    int x = 0;  //每列位置由上列决定，首列为零。由mLstGrid元素添加时SQLORDER严格顺序保证

    //逐列
    for ( int i = 0, iLen = mLstGrid.size(); i < iLen; ++i )
    {
        //设置数据
        LxPrintUnit *pObj = mLstGrid.at(i);

        //字体
        QFont font(pObj->mFontName);
        font.setPointSize(pObj->mFontPoint);
        font.setBold(true);
        painter->setFont(font);

        //列宽度
        int colw = getUnitsByMm(pObj->mWidth);

        //绘制
        if ( pObj->mExp.contains(QStringLiteral("sizers")) )
        {
            //字体
            font.setPointSize(calculateSizerTitleFontPoint(font.pointSize(), sizesTypeCount));
            painter->setFont(font);

            //单尺码平分高度
            int sh = titleHt / sizesTypeCount;

            //单尺码平分总宽
            int sw = colw / sizerCols;

            //纵向位置
            int innerY = 0;

            //标题中的多行（一张单据多个不同尺码品类）
            for ( int j = 0, jLen = sizesTypeCount; j < jLen; ++j )
            {
                //横向位置
                int innerX = 0;

                //细列
                QStringList sizers = QString(sizerNameList.at(j)).split(QChar(9));
                for ( int k = 0; k < sizerCols; ++k )
                {
                    //文字
                    QString txt = ( k < sizers.length() ) ? sizers.at(k) : QString();
                    if ( pObj->mFontAlign == Qt::AlignLeft )
                        txt.prepend(QChar(32));
                    else if ( pObj->mFontAlign == Qt::AlignRight )
                        txt.append(QChar(32));

                    //内容
                    painter->drawText(x + innerX, yPos + innerY, sw, sh, pObj->mFontAlign | Qt::AlignVCenter, txt);

                    //纵线
                    if ( mGridColLinee )
                        painter->drawLine(QPoint(x + innerX + sw, yPos + innerY), QPoint(x + innerX + sw, yPos + innerY + sh));

                    //坐标
                    innerX += sw;
                }

                //纵向位置
                innerY += sh;
            }
        }
        else
        {
            //文字
            QString txt = pObj->mValue.mid(1);
            if ( pObj->mFontAlign == Qt::AlignLeft )
                txt.prepend(QChar(32));
            else if ( pObj->mFontAlign == Qt::AlignRight )
                txt.append(QChar(32));

            //内容
            painter->drawText(x, yPos, colw, titleHt, pObj->mFontAlign | Qt::AlignVCenter, txt);

            //纵线
            if ( mGridColLinee )
            {
                if ( i == mLstGrid.length() - 1 ) {
                    QPen pen;
                    pen.setWidth(2);
                    painter->setPen(pen);
                    painter->drawLine(QPoint(gridW - 1, yPos), QPoint(gridW - 1, yPos + titleHt));
                }
                else {
                    QPen pen;
                    pen.setWidth(1);
                    painter->setPen(pen);
                    painter->drawLine(QPoint(x + colw, yPos), QPoint(x + colw, yPos + titleHt));
                }
            }
        }

        //移位
        x += colw;
    }

    //标题底线
    if ( mGridRowLinee )
        painter->drawLine(QPoint(1, yPos + titleHt), QPoint(gridW - 1, yPos + titleHt));

    //右边框线
    if ( mGridColLinee ) {
        QPen pen;
        pen.setWidth(2);
        painter->setPen(pen);
        painter->drawLine(QPoint(gridW - 1, yPos), QPoint(gridW - 1, yPos + titleHt));
    }

    //传递高度
    *usedHt = titleHt;
}

void LxPrinter::printGridRowToPage(QPainter *painter, const int yPos, const int iRowIdx)
{
    //单行高、表格宽
    int rowHt = getUnitsByMm(mHtGridRow);
    int gridW = getGridWidth();

    //位置
    int x = 0;  //每列位置由上列决定，首列为零。由mLstGrid元素添加时SQLORDER严格顺序保证

    //边线
    if ( mGridColLinee ) {
        QPen pen;
        pen.setWidth(2);
        painter->setPen(pen);
        painter->drawLine(QPoint(0, yPos), QPoint(0, yPos + rowHt));
    }

    //恢复细线
    QPen pen;
    pen.setWidth(1);
    painter->setPen(pen);

    //逐列
    for ( int i = 0, iLen = mLstGrid.size(); i < iLen; ++i )
    {
        //设置源
        LxPrintUnit *pObj = mLstGrid.at(i);

        //求值。因为pObj->mText值要在多行循环使用，故不能改变
        bool validNum;
        int iColIdx = pObj->mText.toInt(&validNum);

        QString strPrint;
        if ( validNum && iColIdx >= 0 )
            strPrint = mppSheet->getGridItemValue(iRowIdx, iColIdx);
        else
            strPrint = mppSheet->getPrintValue(pObj->mText, iRowIdx);

        //字体
        QFont font(pObj->mFontName);
        font.setPointSize(pObj->mFontPoint);
        painter->setFont(font);

        //列宽
        int colw = getUnitsByMm(pObj->mWidth);

        //尺码多绘
        if ( pObj->mExp.contains(QStringLiteral("sizers")) )
        {
            //数据
            QStringList qtyList = mppSheet->getSizerQtysOfRowForPrint(iRowIdx);

            //细列宽
            int sw = colw / qtyList.count();

            //内位置
            int innerX = 0;

            //细历
            for ( int j = 0, jLen = qtyList.length(); j < jLen; ++j )
            {
                //文字
                QString txt = qtyList.at(j);
                if ( pObj->mFontAlign == Qt::AlignLeft )
                    txt.prepend(QChar(32));
                else if ( pObj->mFontAlign == Qt::AlignRight )
                    txt.append(QChar(32));

                //内容
                painter->drawText(x + innerX, yPos, sw, rowHt, pObj->mFontAlign | Qt::AlignVCenter, txt);

                //纵线
                if ( mGridColLinee )
                    painter->drawLine(QPoint(x + innerX + sw, yPos), QPoint(x + innerX + sw, yPos + rowHt));

                //坐标
                innerX += sw;
            }
        }
        //普通单绘
        else {
            //文字
            QString txt = strPrint;
            if ( pObj->mFontAlign == Qt::AlignLeft )
                txt.prepend(QChar(32));
            else if ( pObj->mFontAlign == Qt::AlignRight )
                txt.append(QChar(32));

            //内容
            painter->drawText(x, yPos, colw, rowHt, pObj->mFontAlign | Qt::AlignVCenter, txt);

            //纵线
            if ( mGridColLinee )
            {
                if ( i == mLstGrid.length() - 1 ) {
                    QPen pen;
                    pen.setWidth(2);
                    painter->setPen(pen);
                    painter->drawLine(QPoint(gridW - 1, yPos), QPoint(gridW - 1, yPos + rowHt));
                }
                else {
                    QPen pen;
                    pen.setWidth(1);
                    painter->setPen(pen);
                    painter->drawLine(QPoint(x + colw, yPos), QPoint(x + colw, yPos + rowHt));
                }
            }
        }

        //位置
        x += colw;
    }

    //行细线
    if ( mGridRowLinee )
    {
        QPen pen;
        pen.setWidth( (mppSheet->isLastRow(iRowIdx)) ? 2 : 1 );
        painter->setPen(pen);
        painter->drawLine(QPoint(1, yPos + rowHt), QPoint(gridW - 1, yPos + rowHt));
    }
}



// LxPreviewer ==========================================================================================

LxPreviewer::LxPreviewer(QWidget *parent) : QWidget(parent)
{
    mPaperWidth = -1;
    mPaperHeight = -1;
}

void LxPreviewer::initPreview(const int w, const int h)
{
    mPaperWidth = w;
    mPaperHeight = h;
    qDeleteAll(lstPapers);
    lstPapers.clear();
}

LxPrevPaper *LxPreviewer::newPaper()
{
    if ( mPaperWidth <= 0 || mPaperHeight < 0 ) {
        QMessageBox::warning(this, QStringLiteral("设计预览"), QStringLiteral("请先设定纸张！"));
        return nullptr;
    }

    LxPrevPaper *newPaper = new LxPrevPaper(this);
    lstPapers << newPaper;

    layoutPapers();
    return newPaper;
}

void LxPreviewer::layoutPapers()
{
    const int spacing = 10;
    int yOff = spacing;
    int useHt = ( mPaperHeight > 0 ) ? mPaperHeight : 10000;
    for ( int i = 0, iLen = lstPapers.length(); i < iLen; ++i ) {
        LxPrevPaper *paper = lstPapers.at(i);
        paper->setGeometry(spacing, yOff, mPaperWidth, useHt);
        yOff += (useHt + spacing);
    }
    setFixedSize(mPaperWidth, yOff + spacing);
}

// LxPrevPaper ==========================================================================================

LxPrevPaper::LxPrevPaper(LxPreviewer *parent) : QWidget(parent)
{
    QPalette pal(palette());
    pal.setColor(QPalette::Window, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);

    int useHt = ( parent->mPaperHeight > 0 ) ? parent->mPaperHeight : 10000;

    setFixedSize(parent->mPaperWidth + 2, useHt);
    mpImage = new QImage(parent->mPaperWidth, useHt, QImage::Format_Grayscale8);
    mpImage->fill(QColor(Qt::white));
}

LxPrevPaper::~LxPrevPaper()
{
    delete mpImage;
}

void LxPrevPaper::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    QPainter painter(this);
    painter.drawImage(1, 0, *mpImage);
}


}
