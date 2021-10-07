#include "lxbzprintsetting.h"
#include "lxbzprinter.h"
#include "main/bailicode.h"
#include "main/bailidata.h"
#include "main/bailiwins.h"
#include "main/bailigrid.h"
#include "dialog/bspapersizedlg.h"

#define SELECT_PRINTER_PLS  "请选择打印机……"
#define SELECT_PAPER_PLS    "请选择纸张……"
#define NOPRINTER_TOSELECT  "系统中没有可用的打印机，无法进行打印设置"
#define PAGE_HEIGHT_TICKET  "不分页滚筒纸（小票）"
#define INVALID_PRINTER     "无效的打印机："
#define NONE_DATA_TO_TEST   "当前窗口没有正打开的单据，不能预览或测试。"

#define FONT_ALIGN_LEFT     "靠左"
#define FONT_ALIGN_RIGHT    "靠右"
#define FONT_ALIGN_CENTER   "居中"

#define TYPE_PAGE_HEADER    "页头"
#define TYPE_PAGE_FOOTER    "页尾"
#define TYPE_DATA_GRID      "表体（单行代表）"
#define TYPE_GRID_HEADER    "表头"
#define TYPE_GRID_FOOTER    "表尾"

#define SECINFO_COMMON      "固定空间区域\n走纸长：%1mm\n打印内容：%2项"
#define SECINFO_GRIDSINGLE  "表格区域\n单行走纸长：%1mm\n打印内容：%2项"
#define SECINFO_GRIDMULTY   "表格区域\n走纸行数：%1\n打印内容：%2项"

#define LENGTH_UNIT_MM      "毫米长度(mm)："
#define SECSET_COMM_HT      "设置空间走纸长"
#define SECSET_GROW_HT      "设置单行走纸长"
#define GRID_ROW_LINE       "是否每行打印横线"
#define GRID_COL_LINE       "是否每列打印纵线"
#define GRID_COL_TITLE      "是否每列打印标题"
#define CREATE_OBJECT       "添加内容"
#define CREATE_GRIDCOL      "添加数据列"

#define TICKET_ROW_POS      "纵向位置"
#define TICKET_NEW_ROW      "换新行"
#define OBJOF_POSXMM        "左距(mm)"
#define OBJOF_POSX_COLNO    "行内列序"
#define OBJOF_POSYMM        "区内顶距(mm)"
#define OBJOF_POSY_BTMM     "底距(mm)"
#define OBJOF_POSY_ROWNO    "折行行次"
#define OBJOF_WIDTHMM       "限宽(mm)"
#define OBJOF_FONTNAME      "字体"
#define OBJOF_FONTPOINT     "字体大小"
#define OBJOF_FONTALIGN     "字体对齐"

#define VALUE_PICK_LABEL     "请选择单据变化值："
#define VALUE_EDIT_LABEL     "固定文字请直接填写，单据变化值请用选择项："
#define PRINT_PAGE_NUM      "页码"
#define PRINT_PAGE_TTL      "总页数"

#define SECHFTTL_LEN_OVER   "页头、页尾、表头、表尾，四项长度之和，不应超过页长的2/3，否则，主表格数据空间太小。"

namespace BailiSoft {

LxPrintSettingWin::LxPrintSettingWin(QWidget *parent) : QDialog(parent)
{
    mPaperHeight    = -1;
    mScreenDpi      = qApp->primaryScreen()->logicalDotsPerInch();  //经实量测试，不能用physicalDotsPerInch
    mppSheet        = qobject_cast<BsAbstractSheetWin*>(parent);
    Q_ASSERT(mppSheet);
    mTable          = mppSheet->table();
    mppPrinter      = mppSheet->mpPrinter;

    //工作页——顶部选择栏
    QWidget *pTopBar = new QWidget(this);
    QHBoxLayout *pLayWorkBar = new QHBoxLayout(pTopBar);
    pLayWorkBar->setContentsMargins(0, 0, 0, 0);

    mpPrinters = new QComboBox(this);
    mpPrinters->setEditable(false);
    mpPrinters->setFixedWidth(300);

    mpPapers = new QComboBox(this);
    mpPapers->setEditable(false);
    mpPapers->setFixedWidth(300);

    mpBtnAddPaper = new QPushButton(QStringLiteral("添加规格"), this);
    mpBtnAddPaper->setEnabled(false);

    mpBtnClear = new QPushButton(QStringLiteral("清空重设"), this);
    mpBtnClear->setEnabled(false);

    pLayWorkBar->addWidget(mpPrinters);
    pLayWorkBar->addWidget(mpPapers);
    pLayWorkBar->addWidget(mpBtnAddPaper);
    pLayWorkBar->addStretch();
    pLayWorkBar->addWidget(mpBtnClear);

    //工作页——设计容器
    mpPnlDesign = new QWidget(this);
    mpPnlDesign->setStyleSheet(".QWidget{background-color:rgb(255,255,200); border:1px solid gray;}");
    mpPnlDesign->setMinimumSize(getPixelsByMMs(240), getPixelsByMMs(140));

    createDesignSections(false);        //创建设计容器内容

    //工作页——底部工具栏
    mpDesignBar = new QWidget(this);
    QHBoxLayout *layDesignBar = new QHBoxLayout(mpDesignBar);
    layDesignBar->setContentsMargins(0, 0, 0, 0);

    mpBtnPreview = new QPushButton(QIcon(":/icon/preview.png"), QStringLiteral("预览效果"), this);
    mpBtnPreview->setIconSize(QSize(24, 24));
    mpBtnPreview->setFixedWidth(90);
    mpBtnPreview->setEnabled(false);

    mpBtnPrint = new QPushButton(QIcon(":/icon/print.png"), QStringLiteral("打印测试"), this);
    mpBtnPrint->setIconSize(QSize(24, 24));
    mpBtnPrint->setFixedWidth(90);
    mpBtnPrint->setEnabled(false);

    QPushButton *btnHelp = new QPushButton(QIcon(":/icon/help.png"),QStringLiteral("帮助"), this);
    btnHelp->setIconSize(QSize(24, 24));
    btnHelp->setFixedWidth(90);

    mpBtnSave = new QPushButton(QIcon(":/icon/ok.png"), QStringLiteral("保存使用"), this);
    mpBtnSave->setIconSize(QSize(24, 24));
    mpBtnSave->setFixedWidth(90);

    QPushButton *pBtnCancel = new QPushButton(QIcon(":/icon/cancel.png"), mapMsg.value("btn_cancel"), this);
    pBtnCancel->setIconSize(QSize(24, 24));
    pBtnCancel->setFixedWidth(90);

    layDesignBar->addWidget(mpBtnPreview);
    layDesignBar->addWidget(mpBtnPrint);
    layDesignBar->addWidget(btnHelp);
    layDesignBar->addStretch();
    layDesignBar->addWidget(mpBtnSave);
    layDesignBar->addWidget(pBtnCancel);

    //工作页——布局
    mpTabDesign = new QWidget(this);
    QVBoxLayout* layTabDesign = new QVBoxLayout(mpTabDesign);
    layTabDesign->setSpacing(0);
    layTabDesign->addWidget(pTopBar);
    layTabDesign->addSpacing(10);
    layTabDesign->addWidget(mpPnlDesign, 1);
    layTabDesign->addSpacing(10);
    layTabDesign->addWidget(mpDesignBar);

    //预览页
    mpTabPreview = new QWidget(this);
    QVBoxLayout *pLayPreview = new QVBoxLayout(mpTabPreview);

    mpPreviewArea = new QScrollArea(this);
    mpPreviewArea->setMinimumSize(mpPnlDesign->sizeHint());

    mpPreviewer = new LxPreviewer(this);
    mpPreviewArea->setWidget(mpPreviewer);

    QPushButton *btnBackDesign = new QPushButton(QIcon(":/icon/backqry.png"), QStringLiteral("返回设计"), this);
    btnBackDesign->setIconSize(QSize(24, 24));
    btnBackDesign->setFixedWidth(160);

    pLayPreview->addWidget(mpPreviewArea, 1);
    pLayPreview->addWidget(btnBackDesign, 0, Qt::AlignCenter);

    //总布局
    mpStack = new QStackedLayout(this);
    mpStack->addWidget(mpTabDesign);
    mpStack->addWidget(mpTabPreview);
    setMinimumSize(sizeHint().width(), sizeHint().height());
    setWindowFlags(windowFlags() &~ Qt::WindowContextHelpButtonHint);
    setWindowFlags(windowFlags() &~ Qt::WindowCloseButtonHint);

    //装载数据
    QList<QPrinterInfo> lstPrinterInfo = QPrinterInfo::availablePrinters();
    if ( lstPrinterInfo.size() > 0 ) {
        mpPrinters->addItem(QStringLiteral(SELECT_PRINTER_PLS));
        for ( int i = 0; i < lstPrinterInfo.size(); ++i ) {
            mpPrinters->addItem(lstPrinterInfo.at(i).printerName());
        }
    }
    else {
        mpPrinters->addItem(QStringLiteral(NOPRINTER_TOSELECT));
        mpPrinters->setEnabled(false);
    }

    //事件
    connect(mpPrinters, SIGNAL(currentIndexChanged(int)), this, SLOT(doPrinterIndexChanged(int)));
    connect(mpPapers, SIGNAL(currentIndexChanged(int)), this, SLOT(doPaperIndexChanged(int)));
    connect(mpBtnPreview, SIGNAL(clicked()), this, SLOT(showPreview()));
    connect(mpBtnPrint, SIGNAL(clicked(bool)), this, SLOT(doPrintTest()));
    connect(mpBtnAddPaper, SIGNAL(clicked()), this, SLOT(doTryAddCustomPaper()));
    connect(mpBtnClear, SIGNAL(clicked()), this, SLOT(doClearSettings()));
    connect(btnBackDesign, SIGNAL(clicked(bool)), this, SLOT(showDesign()));
    connect(mpBtnSave, SIGNAL(clicked()), this, SLOT(doAcceptDlg()));
    connect(pBtnCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(btnHelp, SIGNAL(clicked(bool)), this, SLOT(doHelp()));

    //装载数据
    loadSettings();
    mpPHeader->loadSettings();
    mpGHeader->loadSettings();
    mpGrid->loadSettings();
    mpGFooter->loadSettings();
    mpPFooter->loadSettings();
}

LxPrintSettingWin::~LxPrintSettingWin()
{
    //还原打印设置，以便单据窗口正确用于打印。
    mppPrinter->loadPrintSettings();
}

void LxPrintSettingWin::keyPressEvent(QKeyEvent *e)
{
    if ( e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return || e->key() == Qt::Key_Escape )
        return;
    QDialog::keyPressEvent(e);
}

void LxPrintSettingWin::doHelp()
{
    QDesktopServices::openUrl(QUrl("https://www.bailisoft.com/passage/jyb_print_setting.html"));
}

void LxPrintSettingWin::doPrinterIndexChanged(int index)
{
    mpPapers->clear();
    mpPapers->addItem(QStringLiteral(SELECT_PAPER_PLS));

    if ( index > 0 )
    {
        QPrinterInfo prnInfo = QPrinterInfo::printerInfo(mpPrinters->currentText());

        QList<QPageSize> pss = prnInfo.supportedPageSizes();

        mpPapers->addItem(QStringLiteral(PAGE_HEIGHT_TICKET));

        for ( int i = 0; i < pss.size(); ++i ) {
            QPageSize ps = pss.at(i);
            mpPapers->addItem(ps.name(), ps.size(QPageSize::Millimeter).height());
        }
    }

    mPaperHeight = -1;
    mpPapers->setCurrentIndex(0);
    mpBtnAddPaper->setEnabled(index > 0);
    mpPnlDesign->setEnabled( false );
    mpBtnPreview->setEnabled(false);
    mpBtnPrint->setEnabled(false);
    mpBtnClear->setEnabled(false);
    mpBtnSave->setEnabled( false );
}

void LxPrintSettingWin::doPaperIndexChanged(int index)
{
    if ( index < 0 ) return;

    //纸张模式切换，需要清空重设表格内容
    if ( (mPaperHeight == 0 && index > 1) ||
         (mPaperHeight > 0 && index == 1) ) {
        mpGrid->clearCells();
    }

    //然后改变纸张值
    if ( index == 0 )
        mPaperHeight = -1;
    else if ( index == 1 )
        mPaperHeight = 0;
    else
        mPaperHeight = mpPapers->currentData().toInt();

    //更新各Section菜单状态及提示方式等
    mpPHeader->updateControls();
    mpPFooter->updateControls();
    mpGHeader->updateControls();
    mpGFooter->updateControls();
    mpGrid->updateControls();

    //可见性
    mpPHeader->setVisible( mPaperHeight != 0 );
    mpPFooter->setVisible( mPaperHeight != 0 );

    //更新本窗口按钮状态
    mpPnlDesign->setEnabled( mpPrinters->currentIndex() > 0 && index > 0 );
    mpBtnPreview->setEnabled( mpPrinters->currentIndex() > 0 && index > 0 );
    mpBtnPrint->setEnabled( mpPrinters->currentIndex() > 0 && index > 0 );
    mpBtnClear->setEnabled( mpPrinters->currentIndex() > 0 && index > 0 );
    mpBtnSave->setEnabled( mpPrinters->currentIndex() > 0 && index > 0 );
}

void LxPrintSettingWin::doTryAddCustomPaper()
{
    BsPaperSizeDlg dlg(this);
    if ( dlg.exec() != QDialog::Accepted )
        return;

    QString w = dlg.mpWidth->text();
    QString h = dlg.mpHeight->text();
    QString paperName = mapMsg.value("word_baili_sheet_paper") + QStringLiteral("(%1x%2)").arg(w).arg(h);

    QPageSize pageSize(QSizeF(w.toDouble(), h.toDouble()), QPageSize::Millimeter, QString(), QPageSize::ExactMatch);

    //检测打印机
    QString printerName = mpPrinters->currentText();
    QPrinterInfo prnInfo = QPrinterInfo::printerInfo(printerName);
    if ( prnInfo.isNull() ) {
        QMessageBox::information(this, QString(), QStringLiteral(INVALID_PRINTER) + printerName);
        return;
    }

    //打印机实例
    QPrinter printer(prnInfo);

    //设纸规格
    if ( !printer.setPageSize(pageSize) ) {
        QMessageBox::information(this, QString(), mapMsg.value("i_printer_not_support_this_paper_size"));
        return;
    }
    mpPapers->addItem(paperName, pageSize.size(QPageSize::Millimeter).height());
    mpPapers->setCurrentText(paperName);
}

void LxPrintSettingWin::showPreview()
{
    if ( ! sectionReasonable() )
        return;

    if ( mppSheet->mCurrentSheetId <= 0 ) {
        QMessageBox::information(this, QString(), QStringLiteral(NONE_DATA_TO_TEST));
        return;
    }

    //更换设置
    testSettings();

    //预览
    QString strErr = mppPrinter->doPreview(mpPreviewer);

    //显示结果
    if ( strErr.isEmpty() ) {
        mpStack->setCurrentWidget(mpTabPreview);
    } else {
        QMessageBox::information(this, QStringLiteral("打印设计预览"), QStringLiteral("错误：") + strErr);
    }
}

void LxPrintSettingWin::showDesign()
{
    mpStack->setCurrentWidget(mpTabDesign);
}

void LxPrintSettingWin::doPrintTest()
{
    if ( ! sectionReasonable() )
        return;

    if ( mppSheet->mCurrentSheetId <= 0 ) {
        QMessageBox::information(this, QString(), QStringLiteral(NONE_DATA_TO_TEST));
        return;
    }

    //更换设置
    testSettings();

    //打印
    QString strErr = mppPrinter->doPrint();

    //错误报告
    if ( !strErr.isEmpty() ) {
        QMessageBox::information(this, QStringLiteral("打印测试"), QStringLiteral("错误：") + strErr);
    }
}

void LxPrintSettingWin::doClearSettings()
{
    createDesignSections(true);
    mpPHeader->setVisible( mPaperHeight != 0 );
    mpPFooter->setVisible( mPaperHeight != 0 );
}

void LxPrintSettingWin::doAcceptDlg()
{
    if ( ! sectionReasonable() )
        return;

    qApp->setOverrideCursor(Qt::WaitCursor);
    saveSettings();
    mpPHeader->saveSettings();
    mpGHeader->saveSettings();
    mpGrid->saveSettings();
    mpGFooter->saveSettings();
    mpPFooter->saveSettings();
    qApp->restoreOverrideCursor();

    accept();
}

void LxPrintSettingWin::testSettings()
{
    QList<LxPrintUnit *> lstPHeaderUnits;
    QList<LxPrintCell *> lstPHeaderCells = mpPHeader->findChildren<LxPrintCell *>();
    for ( int i = 0, iLen = lstPHeaderCells.count(); i < iLen; ++i ) {
        LxPrintCell *pCell = lstPHeaderCells.at(i);
        lstPHeaderUnits << new LxPrintUnit(pCell->mValue, pCell->mExp, pCell->mPosXmmOrColNo, pCell->mPosYmmOrRowNo, pCell->mWidthmm,
                                           pCell->mFontName, pCell->mFontPoint, pCell->mFontAlign);
    }

    QList<LxPrintUnit *> lstGHeaderUnits;
    QList<LxPrintCell *> lstGHeaderCells = mpGHeader->findChildren<LxPrintCell *>();
    for ( int i = 0, iLen = lstGHeaderCells.count(); i < iLen; ++i ) {
        LxPrintCell *pCell = lstGHeaderCells.at(i);
        lstGHeaderUnits << new LxPrintUnit(pCell->mValue, pCell->mExp, pCell->mPosXmmOrColNo, pCell->mPosYmmOrRowNo, pCell->mWidthmm,
                                           pCell->mFontName, pCell->mFontPoint, pCell->mFontAlign);
    }

    QList<LxPrintUnit *> lstGridUnits;
    QList<LxPrintCell *> lstGridCells = mpGrid->findChildren<LxPrintCell *>();
    for ( int i = 0, iLen = lstGridCells.count(); i < iLen; ++i ) {
        LxPrintCell *pCell = lstGridCells.at(i);
        lstGridUnits << new LxPrintUnit(pCell->mValue, pCell->mExp, pCell->mPosXmmOrColNo, pCell->mPosYmmOrRowNo, pCell->mWidthmm,
                                           pCell->mFontName, pCell->mFontPoint, pCell->mFontAlign);
    }

    QList<LxPrintUnit *> lstGFooterUnits;
    QList<LxPrintCell *> lstGFooterCells = mpGFooter->findChildren<LxPrintCell *>();
    for ( int i = 0, iLen = lstGFooterCells.count(); i < iLen; ++i ) {
        LxPrintCell *pCell = lstGFooterCells.at(i);
        lstGFooterUnits << new LxPrintUnit(pCell->mValue, pCell->mExp, pCell->mPosXmmOrColNo, pCell->mPosYmmOrRowNo, pCell->mWidthmm,
                                           pCell->mFontName, pCell->mFontPoint, pCell->mFontAlign);
    }

    QList<LxPrintUnit *> lstPFooterUnits;
    QList<LxPrintCell *> lstPFooterCells = mpPFooter->findChildren<LxPrintCell *>();
    for ( int i = 0, iLen = lstPFooterCells.count(); i < iLen; ++i ) {
        LxPrintCell *pCell = lstPFooterCells.at(i);
        lstPFooterUnits << new LxPrintUnit(pCell->mValue, pCell->mExp, pCell->mPosXmmOrColNo, pCell->mPosYmmOrRowNo, pCell->mWidthmm,
                                           pCell->mFontName, pCell->mFontPoint, pCell->mFontAlign);
    }

    mppPrinter->loadPreviewSettings(mpPrinters->currentText(),
                                    ((mpPapers->currentIndex() > 1) ? mpPapers->currentText() : ""),
                                    mpPHeader->mSecHtmm,
                                    mpGHeader->mSecHtmm,
                                    mpGrid->mSecHtmm,
                                    mpGFooter->mSecHtmm,
                                    mpPFooter->mSecHtmm,
                                    mpGrid->mpActNeedRowLine->isChecked(),
                                    mpGrid->mpActNeedColLine->isChecked(),
                                    mpGrid->mpActNeedColTitle->isChecked(),
                                    lstPHeaderUnits,
                                    lstGHeaderUnits,
                                    lstGridUnits,
                                    lstGFooterUnits,
                                    lstPFooterUnits);
}

void LxPrintSettingWin::loadSettings()
{
    QSqlQuery qry;
    qry.setForwardOnly(true);
    qry.setNumericalPrecisionPolicy(QSql::LowPrecisionInt64);
    QString sql = QString("SELECT sPrinterName, sPaperName FROM lxPrintPage WHERE sBizName='%1';").arg(mTable);

    qry.exec(sql);
    if ( qry.next() )
    {
        //打印机
        int idxPrinterCombo = mpPrinters->findText(qry.value(0).toString());
        if ( idxPrinterCombo > 0 )
            mpPrinters->setCurrentIndex(idxPrinterCombo);
        else
            mpPrinters->setCurrentIndex(0);

        //纸张
        QString paperName = qry.value(1).toString();
        int idxPaperCombo = mpPapers->findText(paperName);
        if ( idxPaperCombo > 0 )
        {
            mpPapers->setCurrentIndex(idxPaperCombo);        
        }
        else if ( paperName.startsWith(mapMsg.value("word_baili_sheet_paper")) )
        {
            int w, h;
            mppPrinter->fetchSizeFromPaperName(paperName, &w, &h);
            mpPapers->addItem(paperName, h);
            mpPapers->setCurrentText(paperName);
        }
        else
        {
            mpPapers->setCurrentIndex(1);
        }
    }

    mpPnlDesign->setEnabled( mpPrinters->currentIndex() > 0 && mpPapers->currentIndex() > 0 );
    mpBtnSave->setEnabled( mpPrinters->currentIndex() > 0 && mpPapers->currentIndex() > 0 );
}

void LxPrintSettingWin::saveSettings()
{
    QString strPaperName = ( mpPapers->currentIndex() > 1 ) ? mpPapers->currentText() : "";
    QSqlQuery qry;
    qry.exec(QString("DELETE FROM lxPrintPage WHERE sBizName='%1'; ").arg(mTable));
    qry.exec(QString("INSERT INTO lxPrintPage(sBizName, sPrinterName, sPaperName) "
                     "VALUES('%1', '%2', '%3');")
             .arg(mTable)
             .arg(mpPrinters->currentText())
             .arg(strPaperName));
    if ( qry.lastError().isValid() ) qDebug() << qry.lastError().text() << "\n" << qry.lastQuery();
}

bool LxPrintSettingWin::sectionReasonable()
{
    if ( mPaperHeight > 0 )
    {
        int outGridLenMM = mpPHeader->mSecHtmm + mpGHeader->mSecHtmm + mpGFooter->mSecHtmm + mpPFooter->mSecHtmm;
        if ( outGridLenMM > 2 * mPaperHeight / 3 )
        {
            QMessageBox::information(this, windowTitle(), QStringLiteral(SECHFTTL_LEN_OVER));
            return false;
        }
    }
    return true;
}

void LxPrintSettingWin::createDesignSections(const bool prNeedClear)
{
    if ( prNeedClear )
    {
        delete mpPHeader;
        delete mpPFooter;
        delete mpGHeader;
        delete mpGFooter;
        delete mpGrid;
        delete mpLayDesign;
    }

    mpPHeader = new LxPrintSection(lpstPageHeader, this);
    mpGHeader = new LxPrintSection(lpstGridHeader, this);
    mpGrid    = new LxPrintSection(lpstGridBody, this);
    mpGFooter = new LxPrintSection(lpstGridFooter, this);
    mpPFooter = new LxPrintSection(lpstPageFooter, this);

    mpLayDesign = new QVBoxLayout(mpPnlDesign);
    mpLayDesign->addWidget(mpPHeader, 0);
    mpLayDesign->addWidget(mpGHeader, 0);
    mpLayDesign->addWidget(mpGrid, 0);
    mpLayDesign->addWidget(mpGFooter, 0);
    mpLayDesign->addStretch();
    mpLayDesign->addWidget(mpPFooter, 0);
}

/*******************************************************************************/

LxPrintSection::LxPrintSection(const uint prType, LxPrintSettingWin *parent) : QWidget(parent)
{
    mType           = prType;
    mppDlg          = parent;
    mppPickedObj    = nullptr;
    mSecHtmm        = (prType == lpstGridBody) ? 5 : 0;

    //功能菜单
    QString strNew   = ( prType == lpstGridBody ) ? QStringLiteral(CREATE_GRIDCOL) : QStringLiteral(CREATE_OBJECT);
    QString strSecHt = ( prType == lpstGridBody ) ? QStringLiteral(SECSET_GROW_HT) : QStringLiteral(SECSET_COMM_HT);

    mpMenuSec = new QMenu(this);
    mpActObjNew = mpMenuSec->addAction(QIcon(":/png/add.png"), strNew);
    mpActSecHt  = mpMenuSec->addAction(strSecHt);
    mpActNeedRowLine  = mpMenuSec->addAction(QStringLiteral(GRID_ROW_LINE));
    mpActNeedColLine  = mpMenuSec->addAction(QStringLiteral(GRID_COL_LINE));
    mpActNeedColTitle = mpMenuSec->addAction(QStringLiteral(GRID_COL_TITLE));

    mpMenuObj = new QMenu(this);
    mpActObjEdit = mpMenuObj->addAction(QIcon(":/png/edit.png"), QStringLiteral("编辑"));
    mpActObjDrop = mpMenuObj->addAction(QIcon(":/png/del.png"), QStringLiteral("删除"));

    connect(mpActObjNew,  SIGNAL(triggered()), this, SLOT(doNewObject()));
    connect(mpActObjEdit, SIGNAL(triggered()), this, SLOT(doEditObject()));
    connect(mpActObjDrop, SIGNAL(triggered()), this, SLOT(doDropObject()));
    connect(mpActSecHt, SIGNAL(triggered()), this, SLOT(doSetSecHeight()));
    mpActNeedRowLine->setCheckable(true);
    mpActNeedColLine->setCheckable(true);
    mpActNeedColTitle->setCheckable(true);

    //区域名标题文字
    QString strTitle = "";
    switch ( prType )
    {
    case lpstPageHeader:
        strTitle = QStringLiteral(TYPE_PAGE_HEADER);
        break;
    case lpstPageFooter:
        strTitle = QStringLiteral(TYPE_PAGE_FOOTER);
        break;
    case lpstGridHeader:
        strTitle = QStringLiteral(TYPE_GRID_HEADER);
        break;
    case lpstGridFooter:
        strTitle = QStringLiteral(TYPE_GRID_FOOTER);
        break;
    default:
        strTitle = QStringLiteral(TYPE_DATA_GRID);
        break;
    }

    //区域控制头
    mpSecBar = new QToolButton(this);
    mpSecBar->setText(strTitle);
    mpSecBar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    mpSecBar->setMenu(mpMenuSec);
    mpSecBar->setPopupMode(QToolButton::InstantPopup);
    mpSecBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    //总布局
    mpLaySec = new QVBoxLayout(this);
    mpLaySec->setContentsMargins(0, 0, 0, 0);
    mpLaySec->setSpacing(0);
    mpLaySec->addWidget(mpSecBar);
    mpLaySec->addStretch();

    //初始
    setFixedHeight( mppDlg->getPixelsByMMs(mSecHtmm) + mpSecBar->sizeHint().height());
    updateControls();
}

void LxPrintSection::loadSettings()
{
    QSqlQuery qry;
    qry.setForwardOnly(true);
    qry.setNumericalPrecisionPolicy(QSql::LowPrecisionInt64);
    qry.exec(QString("SELECT sValue, sExp, nPosX, nPosY, nWidth, sFontName, nFontPoint, nFontAlign FROM lxPrintObj "
                     "WHERE (sBizName='%1') AND (nSecType=%2);").arg(mppDlg->mTable).arg(mType));
    while ( qry.next() )
    {
        LxPrintCell *pObjNew = new LxPrintCell(qry.value(0).toString(),
                                             qry.value(1).toString(),
                                             qry.value(2).toInt(),
                                             qry.value(3).toInt(),
                                             qry.value(4).toInt(),
                                             qry.value(5).toString(),
                                             qry.value(6).toInt(),
                                             qry.value(7).toInt(),
                                             this );
        if ( mType == lpstGridBody )
            doShowGridCell(pObjNew);
        else
            doShowSideCell(pObjNew);
    }

    qry.exec(QString("SELECT nSecHeight, nRowLine, nColLine, nColTitle FROM lxPrintSec "
                     "WHERE (sBizName='%1') AND (nSecType=%2);")
             .arg(mppDlg->mTable).arg(mType));
    if ( qry.next() )
    {
        mSecHtmm = qry.value(0).toInt();
        mpActNeedRowLine->setChecked(qry.value(1).toBool());
        mpActNeedColLine->setChecked(qry.value(2).toBool());
        mpActNeedColTitle->setChecked(qry.value(3).toBool());
        setFixedHeight( mppDlg->getPixelsByMMs(mSecHtmm) + mpSecBar->height() );
    }
    qry.finish();

    ajustTicketGridSecHeight();

    updateInfoTip();
}

void LxPrintSection::saveSettings()
{
    QStringList sqls;

    sqls << QStringLiteral("DELETE FROM lxPrintSec WHERE (sBizName='%1') AND (nSecType=%2);")
             .arg(mppDlg->mTable).arg(mType);

    sqls << QStringLiteral("DELETE FROM lxPrintObj WHERE (sBizName='%1') AND (nSecType=%2);")
             .arg(mppDlg->mTable).arg(mType);

    sqls << QStringLiteral("INSERT INTO lxPrintSec("
                          "sBizName, nSecType, nSecHeight, nRowLine, nColLine, nColTitle) "
                          "VALUES('%1', %2, %3, %4, %5, %6);")
            .arg(mppDlg->mTable)
            .arg(mType)
            .arg(mSecHtmm)
            .arg( (mpActNeedRowLine->isChecked())? 1 : 0 )
            .arg( (mpActNeedColLine->isChecked())? 1 : 0 )
            .arg( (mpActNeedColTitle->isChecked()) ? 1 : 0 );

    QList<LxPrintCell *> lstCells = findChildren<LxPrintCell *>();
    for ( int i = 0, iLen = lstCells.count(); i < iLen; ++i )
    {
        LxPrintCell *pCell = lstCells.at(i);

        //表格列由于用户只有删除重建设计法（xPos与yPos都从layout获取）
        if ( mType == lpstGridBody ) {
            pCell->mPosXmmOrColNo = getHLayoutIndexOfWidget(pCell) + 1;
            pCell->mPosYmmOrRowNo = getVLayoutIndexOfWidget(pCell) + 1;
        }

        sqls << QStringLiteral("INSERT INTO lxPrintObj(sBizName, nSecType, sValue, sExp, nPosX, nPosY, "
                                 "nWidth, sFontName, nFontPoint, nFontAlign) VALUES("
                                 "'%1', %2, '%3', '%4', %5, %6, %7, '%8', %9, %10);")
                .arg(mppDlg->mTable)
                .arg(mType)
                .arg(pCell->mValue)
                .arg(pCell->mExp)
                .arg(pCell->mPosXmmOrColNo)
                .arg(pCell->mPosYmmOrRowNo)
                .arg(pCell->mWidthmm)
                .arg(pCell->mFontName)
                .arg(pCell->mFontPoint)
                .arg(pCell->mFontAlign);
    }

    QString strErr = sqliteCommit(sqls);
    if ( !strErr.isEmpty() )
        QMessageBox::information(this, QString(), strErr);
}

void LxPrintSection::updateControls()
{
    mpActNeedRowLine->setVisible( mType == lpstGridBody && !mppDlg->usingTicket() );
    mpActNeedColLine->setVisible( mType == lpstGridBody && !mppDlg->usingTicket() );
    mpActNeedColTitle->setVisible( mType == lpstGridBody );
    updateInfoTip();
}

void LxPrintSection::updateInfoTip()
{
    QList<LxPrintCell *> lstCells = findChildren<LxPrintCell *>();
    if ( mType == lpstGridBody )
    {
        if ( mppDlg->usingTicket() )
            mpSecBar->setToolTip(QStringLiteral(SECINFO_GRIDMULTY).arg(mSecHtmm).arg(lstCells.count()));
        else
            mpSecBar->setToolTip(QStringLiteral(SECINFO_GRIDSINGLE).arg(mSecHtmm).arg(lstCells.count()));
    }
    else
        mpSecBar->setToolTip(QStringLiteral(SECINFO_COMMON).arg(mSecHtmm).arg(lstCells.count()));
}

void LxPrintSection::clearCells()
{
    QList<LxPrintCell *> lstCells = findChildren<LxPrintCell *>();
    for ( int i = lstCells.count() - 1; i >= 0; i-- )
    {
        LxPrintCell *pCell = lstCells.at(i);
        delete pCell;
    }
}

void LxPrintSection::doShowSideCell(LxPrintCell *cell)
{
    //检查区域宽（不保存）
    int x = mppDlg->getPixelsByMMs(cell->mPosXmmOrColNo);
    int w = mppDlg->getPixelsByMMs(cell->mWidthmm);

    //检查区域高（如有明显增加，需要保存）
    int y = mppDlg->getPixelsByMMs(cell->mPosYmmOrRowNo);
    int iNeedHt = y + calcObjectNeedHt(cell->mFontName, cell->mFontPoint);
    int iOldHt  = height() - mpSecBar->height();
    if ( iNeedHt > iOldHt )
        setFixedHeight(iNeedHt + mpSecBar->height() + 4);
    if ( iNeedHt > iOldHt + mppDlg->getPixelsByMMs(1) )
    {
        mSecHtmm = mppDlg->getMMsByPixels(iNeedHt);
        updateInfoTip();
    }

    //对象放置
    int h = cell->sizeHint().height();
    if ( mType == lpstPageFooter )
        cell->setGeometry( x, height() - y - h, w, h);
    else
        cell->setGeometry( x, y + mpSecBar->height(), w, h );
    cell->raise();
    cell->show();
}

void LxPrintSection::doShowGridCell(LxPrintCell *cell, const bool onlyEditt)
{
    //小票表格
    if ( mppDlg->usingTicket() )
    {
        //第一个或新行第1个，cell已经有，但HLayout还没有
        if ( findChildren<LxPrintCell *>().count() == 1 || cell->mPosXmmOrColNo == 1 )
        {
            if ( !onlyEditt ) {
                QHBoxLayout *pLay = appendGridNewHLayout();
                pLay->insertWidget(pLay->count() - 1, cell);
            }
        }
        else
        {
            if ( !onlyEditt ) {
                QHBoxLayout *pLay = static_cast<QHBoxLayout*>(mpLaySec->itemAt(mpLaySec->count() - 2)->layout());
                pLay->insertWidget(pLay->count() - 1, cell);
            }
        }
    }
    //分页表格
    else {
        //第一个，cell已经有，但HLayout还没有
        QHBoxLayout *pLay = ( findChildren<LxPrintCell *>().count() == 1 )
                ? appendGridNewHLayout()
                : static_cast<QHBoxLayout*>(mpLaySec->itemAt(1)->layout());  //第0位是mpSecBar
        if ( !onlyEditt )
            pLay->insertWidget(pLay->count() - 1, cell);

        cell->updateInfoTip();
        cell->setFixedWidth( mppDlg->getPixelsByMMs(cell->mWidthmm) );
        doUpdateGridSecSreenSpace();
    }

    //更新与定位
    cell->updateInfoTip();
    cell->setFixedWidth( mppDlg->getPixelsByMMs(cell->mWidthmm) );
    doUpdateGridSecSreenSpace();
}

void LxPrintSection::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    QPainter painter(this);
    painter.fillRect(rect().adjusted(2, 2, -2, -2), QBrush(Qt::white));
}

void LxPrintSection::doSetSecHeight()
{
    QInputDialog dlg(this);
    dlg.setInputMode(QInputDialog::IntInput);

    if ( mType == lpstGridBody )
    {
        dlg.setIntMinimum(3);
        dlg.setIntMaximum(30);
        dlg.setIntValue(5);
        dlg.setWindowTitle(QStringLiteral(SECSET_GROW_HT));
    }
    else
    {
        dlg.setIntMinimum(0);
        dlg.setIntMaximum(100);
        dlg.setIntValue(20);
        dlg.setWindowTitle(QStringLiteral(SECSET_COMM_HT));
    }
    dlg.setLabelText(QStringLiteral(LENGTH_UNIT_MM));
    dlg.setOkButtonText(mapMsg.value("btn_ok"));
    dlg.setCancelButtonText(mapMsg.value("btn_cancel"));
    dlg.adjustSize();
    if ( dlg.exec() != QDialog::Accepted )
        return;

    mSecHtmm = dlg.intValue();
    setFixedHeight( mppDlg->getPixelsByMMs(mSecHtmm) + mpSecBar->height() );

    ajustTicketGridSecHeight();

    updateInfoTip();
}

void LxPrintSection::doNewObject()
{
    //调用对话框
    LxPrintCellDlg dlg(false, this);
    dlg.setWindowTitle(QStringLiteral("添加打印内容"));
    if ( dlg.mpChkStartTickNewRow && findChildren<LxPrintCell *>().count() == 0 )
    {
        dlg.mpChkStartTickNewRow->setChecked(true);
        dlg.mpChkStartTickNewRow->setEnabled(false);
    }
    dlg.adjustSize();
    if ( dlg.exec() != QDialog::Accepted )
        return;

    //定位值特别处理
    int posXmmOrColNo;
    int posYmmOrRowNo;
    if ( mType == lpstGridBody ) {

        //posXmmOrColNo（（1基行序号。考虑每个HLayout末位都已有Stretch）
        if ( mppDlg->usingTicket() ) {
            if ( dlg.mpChkStartTickNewRow->isChecked() )
                posXmmOrColNo = 1;  //此时HBoxLayou待显示函数创建
            else {
                QHBoxLayout *pLay = static_cast<QHBoxLayout*>(mpLaySec->itemAt(mpLaySec->count() - 2)->layout());
                posXmmOrColNo = pLay->count();
            }
        }
        else {
            if ( findChildren<LxPrintCell *>().count() == 0 )
                posXmmOrColNo = 1;  //此时HBoxLayou待显示函数创建
            else {
                QHBoxLayout *pLay = static_cast<QHBoxLayout*>(mpLaySec->itemAt(mpLaySec->count() - 2)->layout());
                posXmmOrColNo = pLay->count();
            }
        }

        //posYmmOrRowNo（1基行序号。考虑mpLaySec本身头有mpSecTitle尾有stretch）
        if ( mppDlg->usingTicket() ) {
            if ( dlg.mpChkStartTickNewRow->isChecked() )
                posYmmOrRowNo = mpLaySec->count() - 1;  //此时HBoxLayou待建
            else
                posYmmOrRowNo = mpLaySec->count() - 2;  //此时HBoxLayou已建
        }
        else {
            posYmmOrRowNo = 1;  //其实无意义，tip中也不显示
        }
    }
    else {
        posXmmOrColNo = dlg.mpEdtPosXmmOrColNo->value();
        posYmmOrRowNo = dlg.mpEdtPosYmmOrRowNo->value();
    }

    //新增
    LxPrintCell *pObjNew = new LxPrintCell(dlg.mpEdtValue->currentText(),                 //value
                                           dlg.mpEdtValue->currentData().toString(),      //exp
                                           posXmmOrColNo,
                                           posYmmOrRowNo,
                                           dlg.mpEdtWidthmm->value(),
                                           dlg.mpEdtFontName->currentText(),
                                           dlg.mpEdtFontPoint->value(),
                                           dlg.mEdtpFontAlign->itemData(dlg.mEdtpFontAlign->currentIndex()).toInt(),
                                           this );
    //显示
    if ( mType == lpstGridBody )
        doShowGridCell(pObjNew);
    else
        doShowSideCell(pObjNew);

    //基本信息
    updateInfoTip();
}

void LxPrintSection::doEditObject()
{
    //调用对话框
    LxPrintCellDlg dlg(true, this);
    LxPrintCell *pEdtObj = qobject_cast<LxPrintCell *>(mppPickedObj);
    dlg.mpEdtValue->addItem(pEdtObj->mValue, pEdtObj->mExp);
    dlg.mpEdtValue->setCurrentIndex(0);
    if (dlg.mpEdtPosXmmOrColNo) dlg.mpEdtPosXmmOrColNo->setValue(pEdtObj->mPosXmmOrColNo);
    if (dlg.mpEdtPosYmmOrRowNo) dlg.mpEdtPosYmmOrRowNo->setValue(pEdtObj->mPosYmmOrRowNo);
    dlg.mpEdtWidthmm->setValue(pEdtObj->mWidthmm);
    dlg.mpEdtFontName->setCurrentIndex(dlg.mpEdtFontName->findText(pEdtObj->mFontName));
    dlg.mpEdtFontPoint->setValue(pEdtObj->mFontPoint);
    dlg.mEdtpFontAlign->setCurrentIndex(dlg.mEdtpFontAlign->findData(pEdtObj->mFontAlign));
    dlg.adjustSize();
    dlg.setWindowTitle(QStringLiteral("设置【%1】").arg(pEdtObj->mValue));
    if ( dlg.exec() != QDialog::Accepted )
        return;

    //执行修改
    if ( dlg.mpEdtPosXmmOrColNo )
        pEdtObj->mPosXmmOrColNo = dlg.mpEdtPosXmmOrColNo->value();

    if (dlg.mpEdtPosYmmOrRowNo)
        pEdtObj->mPosYmmOrRowNo = dlg.mpEdtPosYmmOrRowNo->value();

    pEdtObj->mWidthmm = dlg.mpEdtWidthmm->value();
    pEdtObj->mFontName = dlg.mpEdtFontName->currentText();
    pEdtObj->mFontPoint = dlg.mpEdtFontPoint->value();
    pEdtObj->mFontAlign = dlg.mEdtpFontAlign->currentData().toInt();

    //更新显示
    pEdtObj->updateEditShow();
    pEdtObj->updateInfoTip();
    updateInfoTip();
    if ( pEdtObj->mIsGrid )
        doShowGridCell(pEdtObj, true);
    else
        doShowSideCell(pEdtObj);
}

void LxPrintSection::doDropObject()
{
    LxPrintCell *pDropObj = qobject_cast<LxPrintCell *>(mppPickedObj);
    if ( !pDropObj ) return;
    int iTickRow = pDropObj->mPosYmmOrRowNo;

    delete mppPickedObj;
    mppPickedObj = nullptr;

    //如果是非首页行，则检查如果行内isEmpty，则删除一个Layout，然后重新计算区域高
    if ( mType == lpstGridBody && mppDlg->usingTicket() && iTickRow > 1 )
    {
        QList<LxPrintCell *> lstCells = findChildren<LxPrintCell *>();
        int iLeftRowObjs = 0;
        for ( int i = 0; i < lstCells.count(); ++i )
        {
            if ( lstCells.at(i)->mPosYmmOrRowNo == iTickRow )
                iLeftRowObjs++;
        }
        if ( iLeftRowObjs == 0 )
        {
            QLayoutItem *pRowLayItem =  mpLaySec->takeAt(iTickRow);
            while ( ! pRowLayItem->isEmpty() )
            {
                QLayoutItem *child = pRowLayItem->layout()->takeAt(0);
                if ( child )
                    delete child;
            }
            delete pRowLayItem;
            doUpdateGridSecSreenSpace();
        }
    }

    //背景刷新
    repaint();

    //基本信息
    updateInfoTip();
}

QHBoxLayout *LxPrintSection::appendGridNewHLayout()
{
    QHBoxLayout *lay = new QHBoxLayout;
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    lay->addStretch();
    mpLaySec->insertLayout( mpLaySec->count() - 1, lay );  //因为纵向最后一个必须保持为stretch
    return lay;
}

int LxPrintSection::getHLayoutIndexOfWidget(QWidget *w)
{
    for ( int i = 0, iLen = mpLaySec->count(); i < iLen; ++i )
    {
        QHBoxLayout *lay = static_cast<QHBoxLayout*>(mpLaySec->itemAt(i)->layout());
        if ( lay )
        {
            int foundIdx = lay->indexOf(w);
            if ( foundIdx >= 0 )
                return foundIdx;
        }
    }
    return -1;
}

int LxPrintSection::getVLayoutIndexOfWidget(QWidget *w)
{
    int idx = -1;
    for ( int i = 0, iLen = mpLaySec->count(); i < iLen; ++i )
    {
        QHBoxLayout *lay = static_cast<QHBoxLayout*>(mpLaySec->itemAt(i)->layout());
        if ( lay )
        {
            idx++;
            if ( lay->indexOf(w) >= 0 )
                return idx;
        }
    }
    return idx;
}

void LxPrintSection::doUpdateGridSecSreenSpace()
{
    int iNowRows = (mppDlg->usingTicket()) ? mpLaySec->count() - 2 : 1;
    setFixedHeight( iNowRows * mppDlg->getPixelsByMMs(mSecHtmm) + mpSecBar->height() + 3 );
}

void LxPrintSection::ajustTicketGridSecHeight()
{
    if (  mType == lpstGridBody && mppDlg->usingTicket() ) {
        int iGridRows = mpLaySec->count() - 2;  //去掉mpSecBar，再去掉一个尾部addStretch()的弹簧元素
        setFixedHeight( iGridRows * mppDlg->getPixelsByMMs(mSecHtmm) + mpSecBar->height() );
        QList<LxPrintCell *> lstCells = findChildren<LxPrintCell *>();
        for ( int i = 0; i < lstCells.count(); ++i ) {
            lstCells.at(i)->setFixedHeight(mppDlg->getPixelsByMMs(mSecHtmm));
        }
    }
}

int LxPrintSection::calcObjectNeedHt(const QString &prFontName, const int prFontPoint)
{
    QFont f(prFontName);
    f.setPointSize(prFontPoint);
    QFontMetrics fm(f);
    return fm.height();
}

/*******************************************************************************/

LxPrintCell::LxPrintCell(const QString &prValue, const QString &prExp, const int prPosXmm, const int prPosYmm,
                       const int prWidthmm, const QString prFontName, const int prFontPoint,
                       const int prFontAlign, LxPrintSection *parent)
    : QLabel(prValue, parent)
{
    mValue      = prValue;
    mExp        = prExp;
    mPosXmmOrColNo     = prPosXmm;
    mPosYmmOrRowNo     = prPosYmm;
    mWidthmm    = prWidthmm;
    mFontName   = prFontName;
    mFontPoint  = prFontPoint;
    mFontAlign  = prFontAlign;

    mppSection = parent;
    mIsValue = prValue.at(0) == QChar('=');
    mIsGrid = parent->mType == lpstGridBody;
    mIsPFooter = parent->mType == lpstPageFooter;
    mMoved = false;

    updateEditShow();
    updateInfoTip();
}

void LxPrintCell::updateEditShow()
{
    QFont f(mFontName);
    f.setPointSize(mFontPoint);
    setFont(f);
    setAlignment(Qt::Alignment(mFontAlign));
    setFixedWidth( mppSection->mppDlg->getPixelsByMMs(mWidthmm) );
    setStyleSheet("QLabel{border:1px solid #ccc;}");
}

void LxPrintCell::updateInfoTip()
{
    QString str = mValue;

    if ( mppSection->mType == lpstGridBody )
        str += QString("\n%1: %2").arg(QStringLiteral(OBJOF_POSX_COLNO)).arg(mPosXmmOrColNo);
    else
        str += QString("\n%1: %2mm").arg(QStringLiteral(OBJOF_POSXMM)).arg(mPosXmmOrColNo);

    if ( mppSection->mType != lpstGridBody || mppSection->mppDlg->usingTicket() )
    {
        if ( mppSection->mType == lpstGridBody )
            str += QString("\n%1: %2").arg(QStringLiteral(OBJOF_POSY_ROWNO)).arg(mPosYmmOrRowNo);
        else if ( mppSection->mType == lpstPageFooter )
            str += QString("\n%1: %2mm").arg(QStringLiteral(OBJOF_POSY_BTMM)).arg(mPosYmmOrRowNo);
        else
            str += QString("\n%1: %2mm").arg(QStringLiteral(OBJOF_POSYMM)).arg(mPosYmmOrRowNo);
    }

    str += QString("\n%1: %2").arg(QStringLiteral(OBJOF_WIDTHMM)).arg(mWidthmm);
    str += QString("\n%1: %2").arg(QStringLiteral(OBJOF_FONTNAME)).arg(mFontName);
    str += QString("\n%1: %2").arg(QStringLiteral(OBJOF_FONTPOINT)).arg(mFontPoint);

    QString sAlign;
    if ( mFontAlign == Qt::AlignLeft )
        sAlign = QStringLiteral(FONT_ALIGN_LEFT);
    else if ( mFontAlign == Qt::AlignRight )
        sAlign = QStringLiteral(FONT_ALIGN_RIGHT);
    else
        sAlign = QStringLiteral(FONT_ALIGN_CENTER);

    str += QString("\n%1: %2").arg(QStringLiteral(OBJOF_FONTALIGN)).arg(sAlign);

    setToolTip(str);
}

void LxPrintCell::mousePressEvent(QMouseEvent *event)
{
    mMoved = false;
    if ( event->button() == Qt::LeftButton )
    {
        mPtStart = event->pos();
    }
    QLabel::mousePressEvent(event);
}

void LxPrintCell::mouseMoveEvent(QMouseEvent *event)
{
    if ( event->buttons() & Qt::LeftButton )
    {
        if ( mppSection->mType != lpstGridBody &&
             (event->pos() - mPtStart).manhattanLength() > 3 )
        {
            QPoint ptMove = mapToParent(event->pos() - mPtStart);
            int leftTo  = rect().topLeft().x() + ptMove.x();
            int rightTo = rect().topRight().x() + ptMove.x();
            int topTo   = rect().topLeft().y() + ptMove.y();
            int botTo   = rect().bottomLeft().y() + ptMove.y();

            QRect rThis = QRect( mapToParent(this->rect().topLeft()), mapToParent(this->rect().bottomRight()) );
            QRect rOuter = mppSection->rect().united(rThis).adjusted(0, mppSection->mpSecBar->height(), 0, 0);
            if ( leftTo >= rOuter.left() && topTo >= rOuter.top() &&
                 rightTo <= rOuter.right() && botTo <= rOuter.bottom() )
            {
                if ( abs(ptMove.x()) < 8 ) ptMove.setX(0);
                if ( abs(ptMove.y()) < 8 ) ptMove.setY(0);

                move(ptMove);
            }

            mMoved = true;
            return;
        }
    }
    QLabel::mouseMoveEvent(event);
}

void LxPrintCell::mouseReleaseEvent(QMouseEvent *event)
{
    if ( event->button() == Qt::LeftButton )
    {
        mppSection->mppPickedObj = this;  //标记用于编辑或删除
        if ( mMoved )
        {
            QPoint p = mapToParent(QPoint(0, 0));
            mPosXmmOrColNo = mppSection->mppDlg->getMMsByPixels(p.x());

            if  ( mppSection->mType == lpstPageFooter )
                mPosYmmOrRowNo = mppSection->mppDlg->getMMsByPixels(mppSection->height() - p.y() - height());
            else
                mPosYmmOrRowNo = mppSection->mppDlg->getMMsByPixels(p.y() - mppSection->mpSecBar->height());

            updateInfoTip();
        }
        else
        {
            mppSection->mpMenuObj->popup( mapToGlobal(event->pos()) );
        }
    }
    mMoved = false;
    QLabel::mouseReleaseEvent(event);
}

/*******************************************************************************/

LxPrintCellDlg::LxPrintCellDlg(const bool prForEdit, LxPrintSection *parent) : QDialog(parent)
{
    mppSection = parent;
    mppSheet = mppSection->mppDlg->mppSheet;

    QFont f(font());
    f.setBold(true);
    int iEditBoxW = fontMetrics().horizontalAdvance("WWWWWWWWWWWWWWWWWWWWWWWW");

    mpChkStartTickNewRow    = nullptr;
    mpEdtPosXmmOrColNo      = nullptr;
    mpEdtPosYmmOrRowNo      = nullptr;

    //选择值
    mpEdtValue = new QComboBox(this);
    mpEdtValue->setFont(f);
    mpEdtValue->setMinimumWidth(5 * iEditBoxW / 2);

    if ( prForEdit ) {
        mpEdtValue->setEnabled(false);
    }
    else {
        if ( parent->mType == lpstGridBody ) {
            fillBomValue();
        }
        else {
            fillSimpleValue();
        }

        mpEdtValue->setCurrentIndex(-1);
        if ( parent->mType == lpstGridBody ) {
            mpEdtValue->setEditable( false );
        }
        else {
            mpEdtValue->setEditable( true );
            connect(mpEdtValue->lineEdit(), SIGNAL(textChanged(QString)), this, SLOT(valueTextChanged(QString)));
        }
        connect(mpEdtValue, SIGNAL(currentIndexChanged(int)), this, SLOT(valueIndexChanged(int)));  //仅激活OK钮

        if ( parent->mType == lpstGridBody && parent->mppDlg->usingTicket() ) {
                mpChkStartTickNewRow = new QCheckBox(QStringLiteral(TICKET_NEW_ROW), this);
                mpChkStartTickNewRow->setFixedWidth(iEditBoxW);
        }
    }

    //X向定位
    //由于让用户数值控制定位解释难，只使用删除重新创建的方法来达到任意设计的目的。
    //保存需要的nPosX值是从layout排序中获取的。
    if ( parent->mType != lpstGridBody ) {
        //左距|行内列序
        mpEdtPosXmmOrColNo = new QSpinBox(this);
        mpEdtPosXmmOrColNo->setMinimum(0);
        mpEdtPosXmmOrColNo->setMaximum(280);
        mpEdtPosXmmOrColNo->setValue(0);
        mpEdtPosXmmOrColNo->setFixedWidth(iEditBoxW);
    }

    //Y向定位
    //同X向定位原因，表格不设计用户设置，保存需要的nPosY值是从layout排序中获取。
    //用户设计新行是利用mpChkStartTickNewRow来达到设计目的的。
    if ( parent->mType != lpstGridBody ) {
        //顶距|底距|折行行序
        mpEdtPosYmmOrRowNo = new QSpinBox(this);
        mpEdtPosYmmOrRowNo->setMinimum(0);
        mpEdtPosYmmOrRowNo->setMaximum(180);
        mpEdtPosYmmOrRowNo->setValue(0);
        mpEdtPosYmmOrRowNo->setFixedWidth(iEditBoxW);
    }

    //限宽
    mpEdtWidthmm = new QSpinBox(this);
    mpEdtWidthmm->setMinimum(1);
    mpEdtWidthmm->setMaximum(280);
    mpEdtWidthmm->setValue(20);
    mpEdtWidthmm->setFixedWidth(iEditBoxW);

    //字体
    mpEdtFontName = new QComboBox(this);
    QFontDatabase fdb;
    QString strFontFamily = font().family();
    mpEdtFontName->addItems(fdb.families(QFontDatabase::Any));
    int ftIdx = mpEdtFontName->findText(strFontFamily);
    mpEdtFontName->setCurrentIndex( (ftIdx >= 0) ? ftIdx : 0 );
    mpEdtFontName->setEditable(false);
    mpEdtFontName->setFixedWidth(iEditBoxW);

    //字大
    mpEdtFontPoint = new QSpinBox(this);
    mpEdtFontPoint->setMinimum(5);
    mpEdtFontPoint->setMaximum(60);
    mpEdtFontPoint->setValue(10);
    mpEdtFontPoint->setFixedWidth(iEditBoxW);

    //对齐
    mEdtpFontAlign = new QComboBox(this);
    mEdtpFontAlign->addItem(QStringLiteral(FONT_ALIGN_LEFT), Qt::AlignLeft);
    mEdtpFontAlign->addItem(QStringLiteral(FONT_ALIGN_RIGHT), Qt::AlignRight);
    mEdtpFontAlign->addItem(QStringLiteral(FONT_ALIGN_CENTER), Qt::AlignHCenter);
    mEdtpFontAlign->setCurrentIndex(0);
    mEdtpFontAlign->setEditable(false);
    mEdtpFontAlign->setFixedWidth(iEditBoxW);

    //表单布局
    mpLayForm = new QFormLayout;

    if ( mpChkStartTickNewRow )
        mpLayForm->addRow(QStringLiteral(TICKET_ROW_POS), mpChkStartTickNewRow);

    if ( mpEdtPosXmmOrColNo ) {
        if ( parent->mType == lpstGridBody )
            mpLayForm->addRow(QStringLiteral(OBJOF_POSX_COLNO), mpEdtPosXmmOrColNo);
        else
            mpLayForm->addRow(QStringLiteral(OBJOF_POSXMM), mpEdtPosXmmOrColNo);
    }

    if ( mpEdtPosYmmOrRowNo ) {
        if ( parent->mType != lpstGridBody || parent->mppDlg->usingTicket() ) {
            if ( parent->mType == lpstGridBody )
                mpLayForm->addRow(QStringLiteral(OBJOF_POSY_ROWNO), mpEdtPosYmmOrRowNo);
            else if ( parent->mType == lpstPageFooter )
                mpLayForm->addRow(QStringLiteral(OBJOF_POSY_BTMM), mpEdtPosYmmOrRowNo);
            else
                mpLayForm->addRow(QStringLiteral(OBJOF_POSYMM), mpEdtPosYmmOrRowNo);
        }
    }

    mpLayForm->addRow(QStringLiteral(OBJOF_WIDTHMM), mpEdtWidthmm);
    mpLayForm->addRow(QStringLiteral(OBJOF_FONTNAME), mpEdtFontName);
    mpLayForm->addRow(QStringLiteral(OBJOF_FONTPOINT), mpEdtFontPoint);
    mpLayForm->addRow(QStringLiteral(OBJOF_FONTALIGN), mEdtpFontAlign);

    QHBoxLayout *pLayParam = new QHBoxLayout;
    pLayParam->addStretch(1);
    pLayParam->addLayout(mpLayForm, 0);
    pLayParam->addStretch(2);

    //主值标签
    QLabel *lblValue = new QLabel(this);
    if ( prForEdit ) {
        if (parent->mType == lpstGridBody) {
            lblValue->setText(QStringLiteral("表体数据位置改变，需使用删后重建的方法。"));
            lblValue->setAlignment(Qt::AlignCenter);
            lblValue->setStyleSheet(QLatin1String("color:#666"));
        }
    }
    else {
        if (parent->mType == lpstGridBody)
            lblValue->setText(QStringLiteral(VALUE_PICK_LABEL));
        else
            lblValue->setText(QStringLiteral(VALUE_EDIT_LABEL));
    }

    //确定取消
    QDialogButtonBox *pBox = new QDialogButtonBox(this);
    pBox->setOrientation(Qt::Horizontal);

    mpBtnOk = pBox->addButton(mapMsg.value("btn_ok"), QDialogButtonBox::AcceptRole);
    mpBtnOk->setIcon(QIcon(":/icon/ok.png"));
    mpBtnOk->setIconSize(QSize(20, 20));
    connect(mpBtnOk, SIGNAL(clicked()), this, SLOT(accept()));
    mpBtnOk->setEnabled( prForEdit );

    QPushButton *pBtnCancel = pBox->addButton(mapMsg.value("btn_cancel"), QDialogButtonBox::RejectRole);
    pBtnCancel->setIcon(QIcon(":/icon/cancel.png"));
    pBtnCancel->setIconSize(QSize(20, 20));
    connect(pBtnCancel, SIGNAL(clicked()), this, SLOT(reject()));

    //总布局
    QVBoxLayout *pLayDlg = new QVBoxLayout(this);
    if ( !prForEdit )
        pLayDlg->addWidget(lblValue);

    pLayDlg->addWidget(mpEdtValue);

    if ( prForEdit && !lblValue->text().isEmpty() )
        pLayDlg->addWidget(lblValue);

    pLayDlg->addSpacing(10);
    pLayDlg->addLayout(pLayParam, 1);
    pLayDlg->addSpacing(20);
    pLayDlg->addWidget(pBox);
}

void LxPrintCellDlg::valueIndexChanged(int index)
{
    mpBtnOk->setEnabled( index >= 0 || ! mpEdtValue->currentText().isEmpty() );
}

void LxPrintCellDlg::valueTextChanged(const QString &text)
{
    if ( text.isEmpty() ) mpEdtValue->setCurrentIndex( -1 );
    mpBtnOk->setEnabled( mpEdtValue->currentIndex() >= 0 || ! text.isEmpty() );
}


void LxPrintCellDlg::fillSimpleValue()
{
    //各item的text是value，data是exp
    if ( !mppSection->mppDlg->usingTicket() &&
         ( mppSection->mType == lpstPageHeader || mppSection->mType == lpstPageFooter) )
    {
        mpEdtValue->addItem( "=" + QStringLiteral(PRINT_PAGE_NUM), "pagenum");
        mpEdtValue->addItem( "=" + QStringLiteral(PRINT_PAGE_TTL), "pagettl");
    }

    mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("sheetid"))->mFldCnName, QStringLiteral("sheetid"));
    mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("dated"))->mFldCnName, QStringLiteral("dated"));
    mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("proof"))->mFldCnName, QStringLiteral("proof"));
    mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("stype"))->mFldCnName, QStringLiteral("stype"));
    mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("staff"))->mFldCnName, QStringLiteral("staff"));
    mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("shop"))->mFldCnName, QStringLiteral("shop"));
    mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("trader"))->mFldCnName, QStringLiteral("trader"));
    mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("remark"))->mFldCnName, QStringLiteral("remark"));

    if ( mppSheet->table() != QStringLiteral("szd") ) {
        mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("sumqty"))->mFldCnName, QStringLiteral("sumqty"));
        mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("summoney"))->mFldCnName, QStringLiteral("summoney"));
        mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("sumdis"))->mFldCnName, QStringLiteral("sumdis"));
        mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("actpay"))->mFldCnName, QStringLiteral("actpay"));
        mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("actowe"))->mFldCnName, QStringLiteral("actowe"));
        mpEdtValue->addItem( "=" + mapMsg.value("word_print_total_owe"), QStringLiteral("sumowe"));
    }

    mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("upman"))->mFldCnName, QStringLiteral("upman"));
    mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("uptime"))->mFldCnName, QStringLiteral("uptime"));

    if ( mppSheet->table() != QStringLiteral("szd") ) {
        mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("shop"))->mFldCnName
                             + QStringLiteral("收件地址"), QStringLiteral("shop.regaddr"));
        mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("shop"))->mFldCnName
                             + QStringLiteral("收件人"), QStringLiteral("shop.regman"));
        mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("shop"))->mFldCnName
                             + QStringLiteral("收件电话"), QStringLiteral("shop.regtele"));

        mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("trader"))->mFldCnName
                             + QStringLiteral("收件地址"), QStringLiteral("trader.regaddr"));
        mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("trader"))->mFldCnName
                             + QStringLiteral("收件人"), QStringLiteral("trader.regman"));
        mpEdtValue->addItem( "=" + mppSheet->getFieldByName(QStringLiteral("trader"))->mFldCnName
                             + QStringLiteral("收件电话"), QStringLiteral("trader.regtele"));
    }
}

void LxPrintCellDlg::fillBomValue()
{
    int hpMarkNum = mapOption.value("sheet_hpmark_define").toInt();

    //各item的text是value，data是exp
    mpEdtValue->addItem( "=" + QStringLiteral("序号"), QStringLiteral("rowno"));
    if ( mppSheet->table() == QStringLiteral("szd") ) {
        mpEdtValue->addItem( "=" + mppSheet->getGridFieldByName(QStringLiteral("subject"))->mFldCnName, QStringLiteral("subject"));
        mpEdtValue->addItem( "=" + mppSheet->getGridFieldByName(QStringLiteral("income"))->mFldCnName, QStringLiteral("income"));
        mpEdtValue->addItem( "=" + mppSheet->getGridFieldByName(QStringLiteral("expense"))->mFldCnName, QStringLiteral("expense"));
        mpEdtValue->addItem( "=" + mppSheet->getGridFieldByName(QStringLiteral("rowmark"))->mFldCnName, QStringLiteral("rowmark"));
    }
    else {
        mpEdtValue->addItem( "=" + mppSheet->getGridFieldByName(QStringLiteral("cargo"))->mFldCnName, QStringLiteral("cargo"));
        mpEdtValue->addItem( "=" + mppSheet->getGridFieldByName(QStringLiteral("color"))->mFldCnName, QStringLiteral("color"));
        mpEdtValue->addItem( "=" + QStringLiteral("尺码明细(多列)"), QStringLiteral("sizers"));
        mpEdtValue->addItem( "=" + mppSheet->getGridFieldByName(QStringLiteral("qty"))->mFldCnName, QStringLiteral("qty"));
        mpEdtValue->addItem( "=" + mppSheet->getGridFieldByName(QStringLiteral("price"))->mFldCnName, QStringLiteral("price"));
        mpEdtValue->addItem( "=" + mppSheet->getGridFieldByName(QStringLiteral("actmoney"))->mFldCnName, QStringLiteral("actmoney"));
        mpEdtValue->addItem( "=" + mppSheet->getGridFieldByName(QStringLiteral("discount"))->mFldCnName, QStringLiteral("discount"));
        mpEdtValue->addItem( "=" + mppSheet->getGridFieldByName(QStringLiteral("dismoney"))->mFldCnName, QStringLiteral("dismoney"));
        if ( hpMarkNum > 0 && hpMarkNum < 7 )
            mpEdtValue->addItem( "=" + mppSheet->getGridFieldByName(QStringLiteral("hpmark"))->mFldCnName, QStringLiteral("hpmark"));
        mpEdtValue->addItem( "=" + mppSheet->getGridFieldByName(QStringLiteral("rowmark"))->mFldCnName, QStringLiteral("rowmark"));
        mpEdtValue->addItem( "=" + QStringLiteral("品名"), QStringLiteral("hpname"));
        mpEdtValue->addItem( "=" + QStringLiteral("单位"), QStringLiteral("unit"));
        mpEdtValue->addItem( "=" + QStringLiteral("标牌价"), QStringLiteral("setprice"));
    }
}


}
