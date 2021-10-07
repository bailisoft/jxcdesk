#include "bailiwins.h"
#include "bailicode.h"
#include "bailidata.h"
#include "bailiedit.h"
#include "bailigrid.h"
#include "bailifunc.h"
#include "bsmain.h"
#include "bssocket.h"
#include "comm/bsflowlayout.h"
#include "comm/pinyincode.h"
#include "misc/lxbzprinter.h"
#include "misc/lxbzprintsetting.h"
#include "misc/bsfielddefinedlg.h"
#include "dialog/bsbatchbarcodesdlg.h"
#include "dialog/bspickdatedlg.h"
#include "dialog/bscopyimportsheetdlg.h"
#include <iostream>

namespace BailiSoft {

// BsWin
BsWin::BsWin(QWidget *parent, const QString &mainTable, const QStringList &fields, const uint bsWinType)
    : QWidget(parent), mMainTable(mainTable), mBsWinType(bsWinType), mGuideTipSwitch(false)
{
    //用于主窗口查找判断
    setProperty(BSWIN_TABLE, mainTable);

    //主窗
    mppMain = qobject_cast<BsMain*>(parent);
    Q_ASSERT(mppMain);

    //表格指针
    mpGrid              = nullptr;
    mpQryGrid           = nullptr;
    mpFormGrid          = nullptr;
    mpSheetGrid         = nullptr;
    mpSheetCargoGrid    = nullptr;
    mpSheetFinanceGrid  = nullptr;

    //常用变量
    mDiscDots = mapOption.value("dots_of_discount").toInt();
    mPriceDots = mapOption.value("dots_of_price").toInt();
    mMoneyDots = mapOption.value("dots_of_money").toInt();

    //字段表（继承类中可以用getFieldByName()获取字段指针以修改具体特别设置，比如不同的trader名称等）
    for ( int i = 0, iLen = fields.length(); i < iLen; ++i )
    {
        QString fld = fields.at(i);
        QStringList defs = mapMsg.value(QStringLiteral("fld_%1").arg(fld)).split(QChar(9));
        Q_ASSERT(defs.count() > 4);
        BsField *bsFld = new BsField(fld,
                                     defs.at(0),
                                     QString(defs.at(3)).toUInt(),
                                     QString(defs.at(4)).toInt(),
                                     defs.at(2));
        resetFieldDotsDefin(bsFld);
        mFields << bsFld;

        //用户定义名称
        QString defKey = QStringLiteral("%1_%2").arg(mainTable).arg(fld);
        if ( mapFldUserSetName.contains(defKey) )
            bsFld->mFldCnName = mapFldUserSetName.value(defKey);
    }

    //补设attr字段中文名
    QString attrObj = ( mainTable.contains(QStringLiteral("szd")) || mainTable == QStringLiteral("subject"))
            ? QStringLiteral("subject") : QStringLiteral("cargo");
    for ( int i = 1, iLen = mFields.length(); i < iLen; ++i ) {
        BsField* fld = mFields.at(i);
        if ( fld->mFldName.startsWith(QStringLiteral("attr")) ) {
            QString optkey = QStringLiteral("%1_%2_name").arg(attrObj).arg(fld->mFldName);
            QString optval = mapOption.value(optkey);
            fld->mFldCnName = optval;
        }
    }

    //后代公用数据源
    if ( bsWinType != bswtMisc && bsWinType != bswtReg ) {

        //mpDsStype
        QString stypeListSql = QStringLiteral("select vsetting from bailioption where optcode='stypes_%1';").arg(mainTable);
        mpDsStype = new BsListModel(this, stypeListSql);
        mpDsStype->reload();

        //mpDsStaff
        QString staffListSql = QStringLiteral("select kname from staff");
        /*
        if ( mMainTable.toLower().startsWith("cg") ) {
            staffListSql += QStringLiteral(" where cancg<>0;");
        }
        else if ( mMainTable.toLower().startsWith("pf") ) {
            staffListSql += QStringLiteral(" where canpf<>0;");
        }
        else if ( mMainTable.toLower().startsWith("ls") ) {
            staffListSql += QStringLiteral(" where canls<>0;");
        }
        else if ( mMainTable.toLower().startsWith("db") ) {
            staffListSql += QStringLiteral(" where candb<>0;");
        }
        else if ( mMainTable.toLower().startsWith("xs") ) {
            staffListSql += QStringLiteral(" where canpf<>0 or canls<>0;");
        }
        else if ( mMainTable.toLower().startsWith("sy") ) {
            staffListSql += QStringLiteral(" where cansy<>0;");
        }
        */

        mpDsStaff = new BsSqlModel(this, staffListSql);
        mpDsStaff->reload();

        //mpDsTrader
        if ( mMainTable.contains("cg") )
            mpDsTrader = dsSupplier;
        else if ( mMainTable.contains("pf") || mMainTable.contains("xs") || mMainTable.contains("lsd") )
            mpDsTrader = dsCustomer;
        else if ( mMainTable.contains(QStringLiteral("szd")) )
            mpDsTrader = new BsSqlModel(this, QStringLiteral("select kname from supplier union all "
                                                             "select kname from customer order by kname;"));
        else
            mpDsTrader = dsShop;

        mpDsTrader->reload();
    }

    //向导条
    mpGuide = new QLabel(this);
    mpGuide->setTextFormat(Qt::RichText);
    mpGuide->setAlignment(Qt::AlignCenter);
    mpGuide->setStyleSheet("QLabel{background-color:#ff8;}");

    //通知条
    mpPnlMessage = new QWidget(this);
    mpPnlMessage->setStyleSheet(QLatin1String(".QWidget{background-color:#bb0;}"));
    QVBoxLayout *layMessage = new QVBoxLayout(mpPnlMessage);

    int pt = qApp->font().pointSize();
    mpLblMessage = new QLabel(this);
    mpLblMessage->setAlignment(Qt::AlignCenter);
    mpLblMessage->setStyleSheet(QStringLiteral("color:red;font-size:%1pt").arg(5 * pt / 4));
    mpLblMessage->setWordWrap(true);

    mpBtnMessage = new QPushButton(mapMsg.value("word_i_have_know"), this);
    mpBtnMessage->setMinimumSize(90, 30);
    mpBtnMessage->setStyleSheet(QLatin1String("background-color:red; color:white; font-weight:900; "
                                              "border:3px solid white; border-radius:8px;"));
    connect(mpBtnMessage, SIGNAL(clicked(bool)), this, SLOT(hideFoceMessage()));

    layMessage->addWidget(mpLblMessage, 1);
    layMessage->addWidget(mpBtnMessage, 0, Qt::AlignCenter);
    mpPnlMessage->hide();

    //工具条基本结构
    mpMenuToolCase = new QMenu(this);
    mpMenuOptionBox = new QMenu(this);

    QToolButton *btnToolCase = new QToolButton(this);
    btnToolCase->setMenu(mpMenuToolCase);
    btnToolCase->setIcon(QIcon(":/icon/tool.png"));
    btnToolCase->setMinimumWidth(72);
    btnToolCase->setText(mapMsg.value("btn_toolcase").split(QChar(9)).at(0));
    btnToolCase->setStatusTip(mapMsg.value("btn_toolcase").split(QChar(9)).at(1));
    btnToolCase->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnToolCase->setPopupMode(QToolButton::InstantPopup);

    QToolButton *btnOptionBox = new QToolButton(this);
    btnOptionBox->setMenu(mpMenuOptionBox);
    btnOptionBox->setIcon(QIcon(":/icon/option.png"));
    btnOptionBox->setMinimumWidth(72);
    btnOptionBox->setText(mapMsg.value("btn_optionbox").split(QChar(9)).at(0));
    btnOptionBox->setStatusTip(mapMsg.value("btn_optionbox").split(QChar(9)).at(1));
    btnOptionBox->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnOptionBox->setPopupMode(QToolButton::InstantPopup);

    mpToolBar = new QToolBar(this);
    mpToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpToolBar->setIconSize(QSize(48, 32));

    mpAcToolSeprator = mpToolBar->addSeparator();
    QAction* acToolCase = mpToolBar->addWidget(btnToolCase);
    QAction* acOptionBox = mpToolBar->addWidget(btnOptionBox);
    mpToolBar->addSeparator();
    mpAcMainHelp = mpToolBar->addAction(QIcon(":/icon/help.png"), mapMsg.value("btn_help").split(QChar(9)).at(0),
                                    this, SLOT(clickHelp()));

    //工具箱
    mpAcToolExport = mpMenuToolCase->addAction(QIcon(":/icon/export.png"),
                                           mapMsg.value("tool_export"),
                                           this, SLOT(clickToolExport()));

    //开关盒
    mpAcOptGuideNotShowAnymore = mpMenuOptionBox->addAction(QIcon(),
                                                            mapMsg.value("opt_dont_show_guide_anymore"),
                                                            this, SLOT(clickOptGuideNotShowAnymore()));
    mpAcOptGuideNotShowAnymore->setProperty("optname", "opt_dont_show_guide_anymore");

    mpAcMainHelp->setProperty(BSACFLAGS, 0);
    mpAcToolExport->setProperty(BSACFLAGS, bsacfClean | bsacfPlusId);
    mpAcOptGuideNotShowAnymore->setProperty(BSACFLAGS, 0);

    acToolCase->setProperty(BSACRIGHT, true);
    acOptionBox->setProperty(BSACRIGHT, true);
    mpAcMainHelp->setProperty(BSACRIGHT, true);
    mpAcOptGuideNotShowAnymore->setProperty(BSACRIGHT, true);

    //主体   
    mpBody = new QWidget(this);
    mpBody->setObjectName(QStringLiteral("mpBody"));
    mpBody->setStyleSheet(QLatin1String("QWidget#mpBody{background-color:white;}"));

    mpTaber = new QTabWidget(this);
    mpTaber->setMinimumHeight(mpTaber->tabBar()->sizeHint().height());
    mpTaber->setStyleSheet(QLatin1String("QTabWidget::pane {position:absolute; border-top:1px solid #ddd;} "
                                         "QTabWidget::tab-bar {alignment:center;}"));
    mpTaber->hide();

    mpSpl = new QSplitter(this);
    mpSpl->setOrientation(Qt::Vertical);
    mpSpl->addWidget(mpBody);
    mpSpl->addWidget(mpTaber);
    mpSpl->setCollapsible(0, false);
    mpSpl->setCollapsible(1, false);
    mpSpl->setStyleSheet(QLatin1String("QSplitter::handle:vertical {background-color:#ade; height:5px;}"));

    //状态条
    mpStatusBar = new QStatusBar(this);
    mpStatusBar->setStyleSheet("QStatusBar{border-style:none; border-bottom:1px solid silver; color:#160;}");

    //总布局
    QVBoxLayout *lay = new QVBoxLayout;
    lay->addWidget(mpGuide);
    lay->addWidget(mpPnlMessage);
    lay->addWidget(mpToolBar);
    lay->addWidget(mpSpl, 99);
    lay->addWidget(mpStatusBar);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    setLayout(lay);
    setWindowTitle(mapMsg.value(QStringLiteral("win_%1").arg(mainTable)).split(QChar(9)).at(0));
}

BsWin::~BsWin()
{
    qDeleteAll(mFields);
    mFields.clear();
}

BsField *BsWin::getFieldByName(const QString &name)
{
    for ( int i = 0, iLen = mFields.length(); i < iLen; ++i ) {
        BsField *fld = mFields.at(i);
        if ( fld->mFldName == name ) {
            return fld;
        }
    }
    return nullptr;
}

void BsWin::setQuickDate(const QString &periodName, BsFldBox *dateB, BsFldBox *dateE, QToolButton *button)
{
    QDate date = QDate::currentDate();
    qint64 mindv = QDateTime(date.addDays(-36500)).toMSecsSinceEpoch() / 1000;
    qint64 setdv;

    //今天
    if ( periodName.contains(mapMsg.value("menu_today")) ) {
        setdv = QDateTime(date).toMSecsSinceEpoch() / 1000;
        if ( dateB->mpEditor->isEnabled() ) dateB->mpEditor->setDataValue((setdv > mindv) ? setdv : mindv);

        dateE->mpEditor->setDataValue(setdv);
        button->setText(mapMsg.value("menu_today"));
        return;
    }

    //昨天
    if ( periodName.contains(mapMsg.value("menu_yesterday")) ) {
        date = date.addDays(-1);
        setdv = QDateTime(date).toMSecsSinceEpoch() / 1000;
        if ( dateB->mpEditor->isEnabled() ) dateB->mpEditor->setDataValue((setdv > mindv) ? setdv : mindv);
        dateE->mpEditor->setDataValue(setdv);
        button->setText(mapMsg.value("menu_yesterday"));
        return;
    }

    //本周
    if ( periodName.contains(mapMsg.value("menu_this_week")) ) {
        while ( date.dayOfWeek() != 1 ) { date = date.addDays(-1); }
        setdv = QDateTime(date).toMSecsSinceEpoch() / 1000;
        if ( dateB->mpEditor->isEnabled() ) dateB->mpEditor->setDataValue((setdv > mindv) ? setdv : mindv);

        date = date.addDays(6);
        setdv = QDateTime(date).toMSecsSinceEpoch() / 1000;
        dateE->mpEditor->setDataValue(setdv);
        button->setText(mapMsg.value("menu_this_week"));
        return;
    }

    //上周
    if ( periodName.contains(mapMsg.value("menu_last_week")) ) {
        while ( date.dayOfWeek() != 1 ) { date = date.addDays(-1); }
        date = date.addDays(-7);
        setdv = QDateTime(date).toMSecsSinceEpoch() / 1000;
        if ( dateB->mpEditor->isEnabled() ) dateB->mpEditor->setDataValue((setdv > mindv) ? setdv : mindv);

        date = date.addDays(6);
        setdv = QDateTime(date).toMSecsSinceEpoch() / 1000;
        dateE->mpEditor->setDataValue(setdv);

        button->setText(mapMsg.value("menu_last_week"));
        return;
    }

    //本月
    if ( periodName.contains(mapMsg.value("menu_this_month")) ) {
        while ( date.day() != 1 ) { date = date.addDays(-1); }
        setdv = QDateTime(date).toMSecsSinceEpoch() / 1000;
        if ( dateB->mpEditor->isEnabled() ) dateB->mpEditor->setDataValue((setdv > mindv) ? setdv : mindv);

        date = date.addDays(32);
        while ( date.day() != 1 ) { date = date.addDays(-1); }
        date = date.addDays(-1);
        setdv = QDateTime(date).toMSecsSinceEpoch() / 1000;
        dateE->mpEditor->setDataValue(setdv);
        button->setText(mapMsg.value("menu_this_month"));
        return;
    }

    //上月
    if ( periodName.contains(mapMsg.value("menu_last_month")) ) {
        while ( date.day() != 1 ) { date = date.addDays(-1); }
        date = date.addDays(-26);
        while ( date.day() != 1 ) { date = date.addDays(-1); }
        setdv = QDateTime(date).toMSecsSinceEpoch() / 1000;
        if ( dateB->mpEditor->isEnabled() ) dateB->mpEditor->setDataValue((setdv > mindv) ? setdv : mindv);

        date = date.addDays(32);
        while ( date.day() != 1 ) { date = date.addDays(-1); }
        date = date.addDays(-1);
        setdv = QDateTime(date).toMSecsSinceEpoch() / 1000;
        dateE->mpEditor->setDataValue(setdv);

        button->setText(mapMsg.value("menu_last_month"));
        return;
    }

    //今年
    if ( periodName.contains(mapMsg.value("menu_this_year")) ) {
        date = QDate(date.year(), 1, 1);
        setdv = QDateTime(date).toMSecsSinceEpoch() / 1000;
        if ( dateB->mpEditor->isEnabled() ) dateB->mpEditor->setDataValue((setdv > mindv) ? setdv : mindv);

        date = QDate(date.year(), 12, 31);
        setdv = QDateTime(date).toMSecsSinceEpoch() / 1000;
        dateE->mpEditor->setDataValue(setdv);
        button->setText(mapMsg.value("menu_this_year"));
        return;
    }

    //去年
    if ( periodName.contains(mapMsg.value("menu_last_year")) ) {
        date = QDate(date.year() - 1, 1, 1);
        setdv = QDateTime(date).toMSecsSinceEpoch() / 1000;
        if ( dateB->mpEditor->isEnabled() ) dateB->mpEditor->setDataValue((setdv > mindv) ? setdv : mindv);

        date = QDate(date.year(), 12, 31);
        setdv = QDateTime(date).toMSecsSinceEpoch() / 1000;
        dateE->mpEditor->setDataValue(setdv);

        button->setText(mapMsg.value("menu_last_year"));
        return;
    }
}

void BsWin::exportGrid(const BsGrid *grid, const QStringList headerPairs)
{
    //用户选择文件位置及命名
    QString deskPath = QStandardPaths::locate(QStandardPaths::DesktopLocation, QString(), QStandardPaths::LocateDirectory);
    QString fileName = QFileDialog::getSaveFileName(nullptr,
                                                    mapMsg.value("tool_export"),
                                                    deskPath,
                                                    mapMsg.value("i_formatted_csv_file")
#ifdef Q_OS_MAC
                                                    ,0
                                                    ,QFileDialog::DontUseNativeDialog
#endif
                                                    );
    if (fileName.length() < 1)
        return;

    //准备数据
    QStringList rows;

    //表头数据
    rows << headerPairs;

    //列名
    QStringList cols;
    for ( int j = 0, jLen = grid->columnCount(); j < jLen; ++j ) {
        if ( ! grid->isColumnHidden(j) )
            cols << grid->model()->headerData(j, Qt::Horizontal).toString();
    }
    rows << cols.join(QChar(44));

    //数据
    for ( int i = 0, iLen = grid->rowCount(); i < iLen; ++i ) {
        if ( ! grid->isRowHidden(i) ) {
            QStringList cols;
            for ( int j = 0, jLen = grid->columnCount(); j < jLen; ++j ) {
                if ( ! grid->isColumnHidden(j) ) {
                    if ( grid->item(i, j) )
                        cols << grid->item(i, j)->text();
                    else
                        cols << QString();
                }
            }
            rows << cols.join(QChar(44));
        }
    }

    //合计
    if ( grid->mpFooter && grid->columnCount() == grid->mpFooter->columnCount() ) {
        QStringList cols;
        for ( int j = 0, jLen = grid->columnCount(); j < jLen; ++j ) {
            if ( ! grid->isColumnHidden(j) ) {
                bool fldCargoo = (grid->mCols.at(j)->mFldName == QStringLiteral("cargo"));
                QString footerText = (grid->mpFooter->item(0, j)) ? grid->mpFooter->item(0, j)->text() : QString();
                QString txt = ( j == 0 && (fldCargoo || footerText.indexOf(QChar('>')) > 0) )
                        ? mapMsg.value("word_total")
                        : footerText;
                if ( txt.indexOf(QChar('>')) > 0 )
                    txt = QString();
                cols << txt;
            }
        }
        rows << cols.join(QChar(44));
    }

    //数据
    QString fileData = rows.join(QChar(10));    //除非用户定制，否则不要考虑加解密什么的。

    //保存
    QFile f(fileName);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream strm(&f);
#ifdef Q_OS_WIN
        //EXCEL打不开无BOM头的UTF-8文件（记事本带智能检测，故可以）。
        strm.setGenerateByteOrderMark(true);
#endif
        strm << fileData;
        f.close();
    }
}

QString BsWin::pairTextToHtml(const QStringList &pairs, const bool lastRed)
{
    QString html;
    for ( int i = 0, iLen = pairs.length(); i < iLen; ++i ) {
        QStringList pair = QString(pairs.at(i)).split(QChar(9));
        QString color = ( i == iLen - 1 && lastRed ) ? "red" : "#260";
        html += QStringLiteral("<b>%1：</b><font color='%2'>%3</font>&nbsp;&nbsp;&nbsp;&nbsp;")
                .arg(pair.at(0)).arg(color).arg(pair.at(1));
    }
    return html;
}

bool BsWin::getOptValueByOptName(const QString &optName)
{
    for ( int i = 0, iLen = mpMenuOptionBox->actions().length(); i < iLen; ++i ) {
        QAction *act = mpMenuOptionBox->actions().at(i);
        if ( act->property("optname").toString() == optName )
            return act->isChecked();
    }
    return false;
}

void BsWin::displayGuideTip(const QString &tip)
{
    if ( tip.isEmpty() ) {
        mpGuide->setText( (mGuideTipSwitch) ? mGuideClassTip : mGuideObjectTip);
    } else {
        mpGuide->setText(tip);
        mGuideTipSwitch = !mGuideTipSwitch;
    }
}

void BsWin::forceShowMessage(const QString &msg)
{
    mpLblMessage->setText(msg);
    mpPnlMessage->show();
    QResizeEvent *e = new QResizeEvent(size(), size());
    resizeEvent(e);
    delete e;
}

void BsWin::hideFoceMessage()
{
    mpPnlMessage->hide();
    QResizeEvent *e = new QResizeEvent(size(), size());
    resizeEvent(e);
    delete e;
}

void BsWin::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    loadAllOptionSettings();

    //此处应用option
    mpGuide->setVisible(!mpAcOptGuideNotShowAnymore->isChecked());
}

void BsWin::closeEvent(QCloseEvent *e)
{
    saveAllOptionSettings();
    QWidget::closeEvent(e);
}

void BsWin::doToolExport() {
    exportGrid(mpGrid);
}

void BsWin::clickHelp()
{
    QString winName;
    switch ( mBsWinType ) {
    case bswtMisc:
        winName = QStringLiteral("misc");
        break;
    case bswtReg:
        winName = QStringLiteral("regis");
        break;
    case bswtSheet:
        winName = QStringLiteral("sheet");
        break;
    default:
        winName = QStringLiteral("query");
    }
    QString url = QStringLiteral("https://www.bailisoft.com/passage/jyb_win_%1.html?win=%2").arg(winName).arg(mRightWinName);
    QDesktopServices::openUrl(QUrl(url));
}

void BsWin::clickOptGuideNotShowAnymore()
{
    if ( mpAcOptGuideNotShowAnymore->isChecked() ) {
        mpGuide->hide();
    }
}

void BsWin::loadAllOptionSettings()
{
    QSettings settings;
    settings.beginGroup(BSR17OptionBox);
    settings.beginGroup(mMainTable);
    for ( int i = 0, iLen = mpMenuOptionBox->actions().length(); i < iLen; ++i ) {
        QAction *act = mpMenuOptionBox->actions().at(i);
        act->setCheckable(true);
        act->setChecked( settings.value(act->text(), false).toBool() );
    }
    settings.endGroup();
    settings.endGroup();
}

void BsWin::saveAllOptionSettings()
{
    QSettings settings;
    settings.beginGroup(BSR17OptionBox);
    settings.beginGroup(mMainTable);
    for ( int i = 0, iLen = mpMenuOptionBox->actions().length(); i < iLen; ++i ) {
        QAction *act = mpMenuOptionBox->actions().at(i);
        settings.setValue(act->text(), act->isChecked());
    }
    settings.endGroup();
    settings.endGroup();
}


// BsQryWin
BsQryWin::BsQryWin(QWidget *parent, const QString &name, const QStringList &fields, const uint qryFlags)
    : BsWin(parent, name, fields, bswtQuery), mQryFlags(qryFlags)
{
    //用于权限判断
    mRightWinName = name;
    mRightWinName.replace(QChar('_'), QString());
    //Q_ASSERT(lstQueryWinTableNames.indexOf(mRightWinName) >= 0);
    //由于viszd也是用的BsQryWin但没有权限细设，所以不能用此断言。
    //此断言主要怕vi类命名太多没搞一致，这儿用不用没关系。

    //具体化mFields（包括列名、帮助提示、特性等）
    //以下代码，与单据表以及视图命名强关联，特别注意。
    //sheetname变量只是用于使用mapMsg查找字段中文名称，因此可以替换，但按mapMsg键约定必须用某单据表名
    QStringList nameSecs = name.split(QChar('_'));
    Q_ASSERT(nameSecs.length() >= 2);
    QString sheetName = nameSecs.at(1);
    if ( sheetName == QStringLiteral("cg") )
        sheetName = QStringLiteral("cgj");
    if ( sheetName == QStringLiteral("pf") || sheetName == QStringLiteral("xs") )
        sheetName = QStringLiteral("pff");

    BsField *fld;
    fld = getFieldByName("shop");
    fld->mFldCnName = mapMsg.value(QStringLiteral("fldcname_%1_shop").arg(sheetName));
    if ( (qryFlags & bsqtSumStock) == bsqtSumStock || (qryFlags & bsqtViewAll) == bsqtViewAll )
        fld->mFldCnName = mapMsg.value("qry_stock_shop_cname");

    fld = getFieldByName("trader");
    fld->mFldCnName = mapMsg.value(QStringLiteral("fldcname_%1_trader").arg(sheetName));
    if ( nameSecs.contains("xs") )
        fld->mFldCnName = mapMsg.value("qry_pff_lsd_trader_cname");

    fld = getFieldByName("actpay");
    if ( fld )
        fld->mFldCnName = mapMsg.value(QStringLiteral("fldcname_%1_actpay").arg(sheetName));

    mSizerField = new BsField(QStringLiteral("sizer"),
                              QStringLiteral("尺码"),
                              (bsffText | bsffGrid | bsffAggCount | bsffQryAsSel),
                              10,
                              QStringLiteral("选择尺码条件"));

    if ( (qryFlags & bsqtViewAll) == bsqtViewAll ) {
        mNetCommand = QStringLiteral("QRYVIEW");
    } else if ( (qryFlags & bsqtSumStock) == bsqtSumStock ) {
        mNetCommand = QStringLiteral("QRYSTOCK");
    } else if ( (qryFlags & bsqtSumRest) == bsqtSumRest ) {
        mNetCommand = QStringLiteral("QRYREST");
    } else if ( (qryFlags & bsqtSumCash) == bsqtSumCash ) {
        mNetCommand = QStringLiteral("QRYCASH");
    } else {
         mNetCommand = QStringLiteral("QRYSUMM");
    }

    //帮助提示条
    mGuideObjectTip = mapMsg.value(QStringLiteral("win_%1").arg(name)).split(QChar(9)).at(1);
    mGuideClassTip = mapMsg.value("win_<query>").split(QChar(9)).at(1);
    mpGuide->setText(mGuideClassTip);
    mpStatusBar->hide();

    //工具条
    QLabel *lblWinTitle = new QLabel(this);
    lblWinTitle->setText(mapMsg.value(QStringLiteral("win_%1").arg(name)).split(QChar(9)).at(0));
    lblWinTitle->setStyleSheet("font-size:30pt; padding-left:10px; padding-right:10px; ");
    lblWinTitle->setAlignment(Qt::AlignCenter);
    mpToolBar->insertWidget(mpAcToolSeprator, lblWinTitle);

    QToolButton *btnQryBack = new QToolButton(this);
    btnQryBack->setText(mapMsg.value("btn_back_requery").split(QChar(9)).at(0));
    btnQryBack->setIcon(QIcon(":/icon/backqry.png"));
    btnQryBack->setIconSize(QSize(32, 32));
    btnQryBack->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btnQryBack->setStyleSheet(QStringLiteral("QToolButton {border: 1px solid #999; border-radius: 6px; %1} "
                                             "QToolButton:hover{background-color: #ccc;} ")
                              .arg(mapMsg.value("css_vertical_gradient")));
    btnQryBack->setMinimumWidth(90);
    connect(btnQryBack, SIGNAL(clicked(bool)), this, SLOT(clickQryBack()));
    mpAcMainBackQry = mpToolBar->insertWidget(mpAcToolSeprator, btnQryBack);

    mpAcMainPrint = mpToolBar->addAction(QIcon(":/icon/print.png"), mapMsg.value("btn_print").split(QChar(9)).at(0),
                                     this, SLOT(clickPrint()));

    mpToolBar->insertAction(mpAcToolSeprator, mpAcMainPrint);

    mpAcMainBackQry->setProperty(BSACRIGHT, true);
    mpAcMainPrint->setProperty(BSACRIGHT, canDo(mRightWinName, bsrqPrint));
    mpAcToolExport->setProperty(BSACRIGHT, canDo(mRightWinName, bsrqExport));

    //开关盒

    //条件范围
    QMenu *mnPeriod = new QMenu(this);
    mnPeriod->addAction(mapMsg.value("menu_today"), this, SLOT(clickQuickPeriod()));
    mnPeriod->addAction(mapMsg.value("menu_yesterday"), this, SLOT(clickQuickPeriod()));
    mnPeriod->addSeparator();
    mnPeriod->addAction(mapMsg.value("menu_this_week"), this, SLOT(clickQuickPeriod()));
    mnPeriod->addAction(mapMsg.value("menu_last_week"), this, SLOT(clickQuickPeriod()));
    mnPeriod->addSeparator();
    mnPeriod->addAction(mapMsg.value("menu_this_month"), this, SLOT(clickQuickPeriod()));
    mnPeriod->addAction(mapMsg.value("menu_last_month"), this, SLOT(clickQuickPeriod()));
    mnPeriod->addSeparator();
    mnPeriod->addAction(mapMsg.value("menu_this_year"), this, SLOT(clickQuickPeriod()));
    mnPeriod->addAction(mapMsg.value("menu_last_year"), this, SLOT(clickQuickPeriod()));
    mnPeriod->setStyleSheet("QMenu::item {padding-left:16px;}");
    mpBtnPeriod  = new QToolButton(this);
    mpBtnPeriod->setIcon(QIcon(":/icon/calendar.png"));
    mpBtnPeriod->setText(mapMsg.value("i_period_quick_select"));
    mpBtnPeriod->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mpBtnPeriod->setIconSize(QSize(12, 12));
    mpBtnPeriod->setMenu(mnPeriod);
    mpBtnPeriod->setPopupMode(QToolButton::InstantPopup);

    bool useTristate = (qryFlags & bsqtSumStock) != bsqtSumStock &&
            (qryFlags & bsqtSumRest) != bsqtSumRest &&
            (qryFlags & bsqtSumCash) != bsqtSumCash &&
            (qryFlags & bsqtViewAll) != bsqtViewAll;
    mpConCheck = new BsConCheck(this, useTristate);

    mpConDateB  = new BsFldBox(this, getFieldByName("dated"), nullptr, true);
    mpConDateB->mpEditor->setMyPlaceText(mapMsg.value("i_date_begin_text"));
    mpConDateB->mpEditor->setMyPlaceColor(QColor(Qt::gray));
    mpConDateB->mpLabel->setText(mapMsg.value("i_date_begin_label"));
    mpConDateB->mpEditor->setDataValue(QDateTime(QDate::currentDate()).toMSecsSinceEpoch() / 1000);

    mpConDateE  = new BsFldBox(this, getFieldByName("dated"), nullptr, true);
    mpConDateE->mpEditor->setMyPlaceText(mapMsg.value("i_date_end_text"));
    mpConDateE->mpEditor->setMyPlaceColor(QColor(Qt::gray));
    mpConDateE->mpLabel->setText(mapMsg.value("i_date_end_label"));
    mpConDateE->mpEditor->setDataValue(QDateTime(QDate::currentDate()).toMSecsSinceEpoch() / 1000);

    BsField *fldShop = getFieldByName("shop");
    mpConShop   = new BsFldBox(this, fldShop, dsShop, true);
    mpConShop->mpEditor->setMyPlaceText(fldShop->mFldCnName);
    mpConShop->mpEditor->setMyPlaceColor(QColor(Qt::gray));

    BsField *fldTrader = getFieldByName("trader");
    mpConTrader = new BsFldBox(this, fldTrader, mpDsTrader, true);
    mpConTrader->mpEditor->setMyPlaceText(fldTrader->mFldCnName);
    mpConTrader->mpEditor->setMyPlaceColor(QColor(Qt::gray));

    if ( name.contains("szd") ) {
        BsField *fldCargo = getFieldByName("subject");
        mpConCargo   = new BsFldBox(this, fldCargo, dsSubject, true);
        mpConCargo->mpEditor->setMyPlaceText(fldCargo->mFldCnName);
        mpConCargo->mpEditor->setMyPlaceColor(QColor(Qt::gray));

        mpConColorType = nullptr;

        mpConSizerType = nullptr;
    }
    else {
        BsField *fldCargo = getFieldByName("cargo");
        mpConCargo   = new BsFldBox(this, fldCargo, dsCargo, true);
        mpConCargo->mpEditor->setMyPlaceText(fldCargo->mFldCnName);
        mpConCargo->mpEditor->setMyPlaceColor(QColor(Qt::gray));

        BsField *fldColorList = getFieldByName("color");
        mpConColorType = new BsFldBox(this, fldColorList, dsColorList, true);
        mpConColorType->mpEditor->setMyPlaceText(fldColorList->mFldCnName);
        mpConColorType->mpEditor->setMyPlaceColor(QColor(Qt::gray));

        mpConSizerType = new BsFldBox(this, mSizerField, dsSizerType, true);
        if ( dsSizerType->rowCount() == 1 ) {
            QString sizerType = dsSizerType->data(dsSizerType->index(0, 0)).toString();
            mpConSizerType->mpEditor->setDataValue(sizerType);
            mpConSizerType->setEnabled(false);
        }
        else {
            mpConSizerType->mpEditor->setMyPlaceText(mSizerField->mFldCnName);
            mpConSizerType->mpEditor->setMyPlaceColor(QColor(Qt::gray));
        }

        connect(mpConCargo, &BsFldBox::editingChanged, this, &BsQryWin::conCargoChanged);
    }

    mpPnlCon = new QWidget(this);
    QVBoxLayout *layCon = new QVBoxLayout(mpPnlCon);

    layCon->addWidget(mpBtnPeriod, 0, Qt::AlignCenter);
    layCon->addWidget(mpConDateB);
    layCon->addWidget(mpConDateE);
    layCon->addWidget(mpConCheck, 0, Qt::AlignCenter);
    layCon->addWidget(mpConShop);

    //条件范围特别设置
    if ( (qryFlags & bsqtSumStock) == bsqtSumStock || (qryFlags & bsqtViewAll) == bsqtViewAll ) {
        layCon->addWidget(mpConCargo);
        layCon->addWidget(mpConColorType);
        layCon->addWidget(mpConSizerType);
        mpConTrader->hide();
    }
    else if ( (qryFlags & bsqtSumCash) == bsqtSumCash ) {
        layCon->addWidget(mpConTrader);
        mpConColorType->hide();
        mpConSizerType->hide();
        mpConCargo->hide();
    }
    else if ( (qryFlags & bsqtSumRest) == bsqtSumRest ) {
        layCon->addWidget(mpConCargo);
        layCon->addWidget(mpConColorType);
        layCon->addWidget(mpConSizerType);
        layCon->replaceWidget(mpConShop, mpConTrader);
        mpConShop->hide();
    }
    else {
        layCon->addWidget(mpConTrader);
        if ( mpConColorType && mpConSizerType ) {
            layCon->addWidget(mpConCargo);
            layCon->addWidget(mpConColorType);
            layCon->addWidget(mpConSizerType);
        }
        else {
            layCon->addWidget(mpConCargo);
        }
        if ( name.contains(QStringLiteral("syd")) )
            mpConTrader->hide();
    }
    layCon->addStretch();

    //选择项与统计值中文名特别设置
    if ( (qryFlags & bsqtSumStock) == bsqtSumStock || (qryFlags & bsqtViewAll) == bsqtViewAll ) {

        fld = getFieldByName(QStringLiteral("yeard"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("monthd"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("dated"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("trader"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("actpay"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsVal;

        fld = getFieldByName(QStringLiteral("actowe"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsVal;

        fld = getFieldByName(QStringLiteral("sumqty"));
        if ( fld ) fld->mFldCnName = mapMsg.value("qry_stock_qty");

        fld = getFieldByName(QStringLiteral("summoney"));
        if ( fld ) fld->mFldCnName = mapMsg.value("qry_money_occupied");

        fld = getFieldByName(QStringLiteral("sumdis"));
        if ( fld ) fld->mFldCnName = mapMsg.value("qry_dismoney_gain");
    }
    else {

        fld = getFieldByName(QStringLiteral("sumqty"));
        if ( fld ) fld->mFldCnName = mapMsg.value("qry_qty_cname");

        fld = getFieldByName(QStringLiteral("summoney"));
        if ( fld ) fld->mFldCnName = mapMsg.value("qry_money_cname");

        fld = getFieldByName(QStringLiteral("sumdis"));
        if ( fld ) fld->mFldCnName = mapMsg.value("qry_dismoney_cname");

        fld = getFieldByName(QStringLiteral("actpay"));
        if ( fld ) fld->mFldCnName = name.contains(QStringLiteral("cg"))
                ? mapMsg.value("qry_actpayout_cname")
                : mapMsg.value("qry_actpayin_cname");

        fld = getFieldByName(QStringLiteral("actowe"));
        if ( fld ) fld->mFldCnName = mapMsg.value("qry_actowe_cname");
    }

    //补正查询flag
    if ( (qryFlags & bsqtSumOrder) == bsqtSumOrder ) {

        fld = getFieldByName(QStringLiteral("sumdis"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsVal;

        fld = getFieldByName(QStringLiteral("actpay"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsVal;

        fld = getFieldByName(QStringLiteral("actowe"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsVal;
    }

    if ( (qryFlags & bsqtSumRest) == bsqtSumRest ) {

        mpConDateB->mpEditor->setDataValue(QDateTime(QDate::fromString("1900-01-01", "yyyy-MM-dd")).toMSecsSinceEpoch() / 1000);
        mpConDateB->disableShow();
        mpConDateB->hide();
        mpBtnPeriod->hide();

        fld = getFieldByName(QStringLiteral("yeard"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("monthd"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("dated"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("stype"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("staff"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        if ( (qryFlags & bsqtSumStock) != bsqtSumStock ) {

            fld = getFieldByName(QStringLiteral("shop"));
            if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

            fld = getFieldByName(QStringLiteral("sumdis"));
            if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsVal;
        }
    }

    if ( (qryFlags & bsqtSumCash) == bsqtSumCash ) {

        mpConDateB->mpEditor->setDataValue(QDateTime(QDate::fromString("1900-01-01", "yyyy-MM-dd")).toMSecsSinceEpoch() / 1000);
        mpConDateB->disableShow();
        mpConDateB->hide();
        mpBtnPeriod->hide();

        fld = getFieldByName(QStringLiteral("yeard"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("monthd"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("dated"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("shop"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("cargo"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("hpname"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("setprice"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("unit"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("color"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("sizers"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("attr1"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("attr2"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("attr3"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("attr4"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("attr5"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("attr6"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;
    }
    else {

        fld = getFieldByName(QStringLiteral("actpay"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsVal;

        fld = getFieldByName(QStringLiteral("actowe"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsVal;
    }

    if ( name.contains("syd") ) {

        fld = getFieldByName(QStringLiteral("trader"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsSel;

        fld = getFieldByName(QStringLiteral("actpay"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsVal;

        fld = getFieldByName(QStringLiteral("actowe"));
        if ( fld ) fld->mFlags = fld->mFlags &~ bsffQryAsVal;
    }

    //选择范围标签（用于查询结果出来后显示）
    mpLblCon = new QLabel(this);
    mpLblCon->setWordWrap(true);

    //面板总排版
    mpPanel = new QWidget(this);
    QVBoxLayout *layPanel = new QVBoxLayout(mpPanel);
    layPanel->setContentsMargins(0, 3, 0, 0);
    layPanel->addWidget(mpPnlCon);
    layPanel->addWidget(mpLblCon);
    layPanel->addStretch();
    mpPanel->setFixedWidth(lblWinTitle->sizeHint().width());

    //表格
    mpQryGrid = new BsQueryGrid(this);
    mpGrid = mpQryGrid;

    //总布局
    QHBoxLayout *layBody = new QHBoxLayout(mpBody);
    layBody->setContentsMargins(3, 0, 3, 3);
    layBody->setSpacing(0);
    layBody->addWidget(mpPanel);
    layBody->addWidget(mpQryGrid, 1);
    mpBody->setObjectName("qrywinbody");
    mpBody->setStyleSheet("QWidget#qrywinbody{background-color:#e9e9e9;}");

    //大按钮面板
    mpPnlQryConfirm = new QWidget(this);
    QVBoxLayout *layConfirm = new QVBoxLayout(mpPnlQryConfirm);
    layConfirm->setContentsMargins(0, 0, 0, 0);
    mpPnlQryConfirm->setStyleSheet(QStringLiteral("QWidget {background: #fff;} "
                                     "QToolButton{border: 1px solid #999; border-radius: 16px; %1} "
                                     "QToolButton:hover{background: #ccc;} ")
                             .arg(mapMsg.value("css_vertical_gradient")));

    mpBtnBigOk = new QToolButton(this);
    mpBtnBigOk->setIcon(QIcon(":/icon/bigok.png"));
    mpBtnBigOk->setIconSize(QSize(128, 128));
    connect(mpBtnBigOk, SIGNAL(clicked(bool)), this, SLOT(clickQryExecute()));

    mpBtnBigCancel = new QToolButton(this);
    mpBtnBigCancel->setIcon(QIcon(":/icon/bigcancel.png"));
    mpBtnBigCancel->setIconSize(QSize(128, 128));
    connect(mpBtnBigCancel, SIGNAL(clicked(bool)), this, SLOT(clickBigCancel()));

    QHBoxLayout *layButtonRow = new QHBoxLayout;
    layButtonRow->addStretch();
    layButtonRow->addWidget(mpBtnBigOk);
    layButtonRow->addWidget(mpBtnBigCancel);
    layButtonRow->addStretch();

    layConfirm->addStretch();
    layConfirm->addLayout(layButtonRow);
    layConfirm->addStretch();

    //具体化StatusTip
    mpAcMainBackQry->setStatusTip(mapMsg.value("btn_back_requery").split(QChar(9)).at(1));

    //外观细节
    mpToolBar->setIconSize(QSize(64, 32));
    mpBtnBigCancel->hide();
    mpLblCon->hide();
    mpAcMainBackQry->setVisible(false);
    mpAcMainPrint->setVisible(false);
    setMinimumSize(640, 440);

    //加载选择下拉数据集
    if ( (qryFlags & bsqtSumCash) != bsqtSumCash ) {
        dsCargo->reload();
        dsSubject->reload();
        dsColorType->reload();
        dsSizerType->reload();
    }

    connect(netSocket, &BsSocket::requestOk, this, &BsQryWin::onSocketRequestOk);
}

BsQryWin::~BsQryWin()
{
    delete mSizerField;
}

void BsQryWin::showEvent(QShowEvent *e)
{
    BsWin::showEvent(e);

    //显示浮动大按钮
    setFloatorGeometry();
}

void BsQryWin::resizeEvent(QResizeEvent *e)
{
    BsWin::resizeEvent(e);
    setFloatorGeometry();
}

void BsQryWin::doToolExport()
{
    QStringList headPairs;
    foreach (QString pair, mLabelPairs ) {
        headPairs << pair.replace(QChar(9), QChar(44));
    }
    exportGrid(mpGrid, headPairs);
}

void BsQryWin::clickQuickPeriod()
{
    QAction *act = qobject_cast<QAction*>(QObject::sender());
    Q_ASSERT(act);
    setQuickDate(act->text(), mpConDateB, mpConDateE, mpBtnPeriod);
}

void BsQryWin::clickQryExecute()
{
    //保存列宽
    if ( mpQryGrid->rowCount() > 0 )
        mpQryGrid->saveColWidths();

    //绑店检查
    if ( !loginTrader.isEmpty() ) {
        QString conShopText = mpConShop->mpEditor->getDataValue();
        QString conCargoText = mpConCargo->mpEditor->getDataValue();

        if ( mMainTable.contains(QStringLiteral("stock")) ) {
            //库存分布查询
            if ( conShopText.isEmpty() ) {
                if ( conCargoText.isEmpty() || conCargoText.contains(QChar('%')) || conCargoText.contains(QChar('_')) ) {
                    QMessageBox::information(this, QString(), QStringLiteral("门店查询库存分布必须指定具体货号！"));
                    return;
                }
            }
            //查具体门店
            else {
                if ( conCargoText.isEmpty() && conShopText != loginTrader ) {
                    QMessageBox::information(this, QString(), QStringLiteral("不限货号只能查询自己店库存！"));
                    return;
                }
            }
        }
    }

    //执行查询
    startQueryRequest();
}

void BsQryWin::clickBigCancel()
{
    mpLblCon->show();
    mpAcMainBackQry->setVisible(true);
    mpAcMainPrint->setVisible(true);
    mpPnlCon->hide();
    mpPnlQryConfirm->hide();
}

void BsQryWin::clickQryBack()
{
    mpLblCon->hide();
    mpAcMainBackQry->setVisible(false);
    mpAcMainPrint->setVisible(false);
    mpPnlCon->show();
    mpPnlQryConfirm->show();
    mpBtnBigCancel->show();
    setFloatorGeometry();
}

void BsQryWin::clickPrint()
{
    QStringList conPairs;
    if ( mpConDateB->mpEditor->isEnabled() )
        conPairs << (mpConDateB->getFieldCnName() + QChar(9) + mpConDateB->mpEditor->text());

    conPairs << (mpConDateE->getFieldCnName() + QChar(9) + mpConDateE->mpEditor->text());

    if ( !mpConShop->mpEditor->getDataValue().isEmpty() )
        conPairs << (mpConShop->getFieldCnName() + QChar(9) + mpConShop->mpEditor->getDataValue());

    if ( !mpConTrader->mpEditor->getDataValue().isEmpty() )
        conPairs << (mpConTrader->getFieldCnName() + QChar(9) + mpConTrader->mpEditor->getDataValue());

    if ( !mpConCargo->mpEditor->getDataValue().isEmpty() )
        conPairs << (mpConCargo->getFieldCnName() + QChar(9) + mpConCargo->mpEditor->getDataValue());

    if ( mpConColorType && mpConSizerType ) {
        if ( !mpConColorType->mpEditor->getDataValue().isEmpty() )
            conPairs << (mpConColorType->getFieldCnName() + QChar(9) + mpConColorType->mpEditor->getDataValue());

        if ( !mpConSizerType->mpEditor->getDataValue().isEmpty() )
            conPairs << (mpConSizerType->getFieldCnName() + QChar(9) + mpConSizerType->mpEditor->getDataValue());
    }

    mpQryGrid->doPrint(windowTitle(), conPairs, loginer, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));
}

void BsQryWin::conCargoChanged(const QString &text)
{
    mpConColorType->mpEditor->setDataValue(QString());
    mpConSizerType->mpEditor->setDataValue(QString());

    if ( dsCargo->foundKey(text) ) {
        QString colorType = dsCargo->getValue(text, "colortype");
        dsColorList->setFilterByCargoType(colorType);

        QString sizerType = dsCargo->getValue(text, "sizertype");
        dsSizerList->setFilterByCargoType(sizerType);

        mpConColorType->setEnabled(true);
        mpConSizerType->setEnabled(true);
    }
    else {
        mpConColorType->setEnabled(false);
        mpConSizerType->setEnabled(false);
    }
}

void BsQryWin::setFloatorGeometry()
{
    QPoint pointGridLeftTop = mpQryGrid->mapTo(this, QPoint(0, 0));
    mpPnlQryConfirm->setGeometry(pointGridLeftTop.x(), pointGridLeftTop.y(), mpQryGrid->width(), mpQryGrid->height());
}

void BsQryWin::startQueryRequest()
{
    if ( mNetCommand == QStringLiteral("QRYVIEW") ) {
        if ( mpConCargo->mpEditor->getDataValue().isEmpty() ) {
            QMessageBox::information(this, QString(), QStringLiteral("货号必须指定！"));
            return;
        }
    }

    //条件范围
    mLabelPairs.clear();
    mapRangeCon.clear();

    if ( mpConDateB->mpEditor->isEnabled() ) {
        mapRangeCon.insert("dateb", mpConDateB->mpEditor->getDataValue());
        mLabelPairs << QStringLiteral("%1\t%2").arg(mpConDateB->getFieldCnName()).arg(mpConDateB->mpEditor->text());
    }

    mapRangeCon.insert("datee", mpConDateE->mpEditor->getDataValue());
    mLabelPairs << QStringLiteral("%1\t%2").arg(mpConDateE->getFieldCnName()).arg(mpConDateE->mpEditor->text());

    if ( mpConShop->isVisible() && !mpConShop->mpEditor->getDataValue().isEmpty() ) {
        mapRangeCon.insert(mpConShop->getFieldName(), mpConShop->mpEditor->getDataValue());
        mLabelPairs << QStringLiteral("%1\t%2").arg(mpConShop->getFieldCnName()).arg(mpConShop->mpEditor->text());
    }

    if ( mpConTrader->isVisible() && !mpConTrader->mpEditor->getDataValue().isEmpty() ) {
        mapRangeCon.insert(mpConTrader->getFieldName(), mpConTrader->mpEditor->getDataValue());
        mLabelPairs << QStringLiteral("%1\t%2").arg(mpConTrader->getFieldCnName()).arg(mpConTrader->mpEditor->text());
    }

    if ( mpConCargo->isVisible() && !mpConCargo->mpEditor->getDataValue().isEmpty() ) {
        mapRangeCon.insert(mpConCargo->getFieldName(), mpConCargo->mpEditor->getDataValue());
        mLabelPairs << QStringLiteral("%1\t%2").arg(mpConCargo->getFieldCnName()).arg(mpConCargo->mpEditor->text());
    }

    if ( mpConColorType && mpConSizerType ) {
        if ( mpConColorType->isVisible() && !mpConColorType->mpEditor->getDataValue().isEmpty() ) {
            mapRangeCon.insert(mpConColorType->getFieldName(), mpConColorType->mpEditor->getDataValue());
            mLabelPairs << QStringLiteral("%1\t%2").arg(mpConColorType->getFieldCnName()).arg(mpConColorType->mpEditor->text());
        }

        if ( mpConSizerType->isVisible() && !mpConSizerType->mpEditor->getDataValue().isEmpty() ) {
            mapRangeCon.insert(mpConSizerType->getFieldName(), mpConSizerType->mpEditor->getDataValue());
            mLabelPairs << QStringLiteral("%1\t%2").arg(mpConSizerType->getFieldCnName()).arg(mpConSizerType->mpEditor->text());
        }
    }

    QString currentChkConVal;

    if ( mpConCheck->isTristate() ) {
        if ( mpConCheck->checkState() == Qt::Checked ) {
            currentChkConVal = "<>0";
            mLabelPairs << QStringLiteral("%1\t%2").arg(mapMsg.value("word_check_range")).arg(mapMsg.value("word_only_checked"));
        }
        else if ( mpConCheck->checkState() == Qt::Unchecked ) {
            currentChkConVal = "=0";
            mLabelPairs << QStringLiteral("%1\t%2").arg(mapMsg.value("word_check_range")).arg(mapMsg.value("word_not_checked"));
        }
        else
            mLabelPairs << QStringLiteral("%1\t%2").arg(mapMsg.value("word_check_range")).arg(mapMsg.value("word_any_checked"));
    }
    else {
        if ( mpConCheck->checkState() == Qt::Checked ) {
            currentChkConVal = "<>0";
            mLabelPairs << QStringLiteral("%1\t%2").arg(mapMsg.value("word_check_range")).arg(mapMsg.value("word_only_checked"));
        }
        else
            mLabelPairs << QStringLiteral("%1\t%2").arg(mapMsg.value("word_check_range")).arg(mapMsg.value("word_any_checked"));
    }

    if ( !currentChkConVal.isEmpty() ) {
        mapRangeCon.insert("chktime", currentChkConVal);
    }

    QString waitMsg = QStringLiteral("正在查询，请稍侯……");

    //qDebug() << mMainTable;   //vi_stock，vi_all，vi_pfd，vi_pf_rest，vi_pf_cash
    QString reqtname;
    QStringList nameSecs = mMainTable.split(QChar('_'));
    Q_ASSERT(nameSecs.length() >= 2);
    reqtname = nameSecs.at(1);

    QString reqshop = (mpConShop && mpConShop->isEnabled() && mpConShop->isVisible() )
            ? mpConShop->mpEditor->getDataValue()
            : QString();

    QString reqtrader = (mpConTrader && mpConTrader->isEnabled() && mpConTrader->isVisible() )
            ? mpConTrader->mpEditor->getDataValue()
            : QString();

    QString reqcargo = (mpConCargo && mpConCargo->isEnabled() && mpConCargo->isVisible() )
            ? mpConCargo->mpEditor->getDataValue()
            : QString();

    QString reqcolor = (mpConColorType && mpConColorType->isEnabled() && mpConColorType->isVisible() )
            ? mpConColorType->mpEditor->getDataValue()
            : QString();

    QString reqsizer = (mpConSizerType && mpConSizerType->isEnabled() && mpConSizerType->isVisible() )
            ? mpConSizerType->mpEditor->getDataValue()
            : QString();

    int reqcheckk = 0;
    if ( mpConCheck->isTristate() ) {
        if ( mpConCheck->checkState() == Qt::Checked ) {
            reqcheckk = 1;
        }
        if ( mpConCheck->checkState() == Qt::Unchecked ) {
            reqcheckk = 2;
        }
    }
    else {
        if ( mpConCheck->checkState() == Qt::Checked ) {
            reqcheckk = 1;
        }
    }

    QStringList params;
    params << mNetCommand
           << QString::number(QDateTime::currentMSecsSinceEpoch() * 1000)
           << reqtname
           << reqshop
           << reqtrader
           << reqcargo
           << reqcolor
           << reqsizer
           << mpConDateB->mpEditor->text()
           << mpConDateE->mpEditor->text()
           << QString::number(reqcheckk);
    netSocket->netRequest(this, params, waitMsg);   // => onSocketRequestOk
}

void BsQryWin::onSocketRequestOk(const QWidget* sender, const QStringList &fens)
{
    if ( sender != this )
        return;

    Q_ASSERT(fens.length() > 2);
    QStringList rows = QString(fens.at(2)).split(QChar('\n'));
    QStringList flds = QString(rows.at(0)).split(QChar('\t'));
    QString conw;
    QStringList ords = flds;

    if ( mMainTable.contains(QStringLiteral("stock")) ) {
        for ( int i = 0, iLen = flds.length(); i < iLen; ++i ) {
            QString fld = flds.at(i);
            if ( fld.contains(QStringLiteral("qty")) ) {
                conw = QStringLiteral("where %1<>0").arg(fld);
            }
        }
    }

    if ( flds.length() > 3 ) {
        ords = ords.mid(0, 2);
    } else if ( flds.length() > 2 ) {
        ords = ords.mid(0, 1);
    }
    QString sql = QStringLiteral("select %1 from tmp_query_dataset %2 order by %3;")
            .arg(flds.join(QChar(','))).arg(conw).arg(ords.join(QChar(',')));

    mpQryGrid->loadData(sql);
    mpQryGrid->loadColWidths();

    mpLblCon->setText(pairTextToHtml(mLabelPairs, false));
    mpLblCon->show();
    mpAcMainBackQry->setVisible(true);
    mpAcMainPrint->setVisible(true);
    mpPnlCon->hide();
    mpPnlQryConfirm->hide();
}


// BsAbstractFormWin
BsAbstractFormWin::BsAbstractFormWin(QWidget *parent, const QString &name, const QStringList &fields, const uint bsWinType)
    : BsWin(parent, name, fields, bsWinType)
{
    //主按钮
    mpAcMainNew = mpToolBar->addAction(QIcon(":/icon/new.png"), QString(), this, SLOT(clickNew()));
    mpAcMainEdit = mpToolBar->addAction(QIcon(":/icon/edit.png"), QString(), this, SLOT(clickEdit()));
    mpAcMainDel = mpToolBar->addAction(QIcon(":/icon/del.png"), QString(), this, SLOT(clickDel()));
    mpAcMainSave = mpToolBar->addAction(QIcon(":/icon/save.png"), QString(), this, SLOT(clickSave()));
    mpAcMainCancel = mpToolBar->addAction(QIcon(":/icon/cancel.png"), QString(), this, SLOT(clickCancel()));

    mpAcMainNew->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    mpAcMainEdit->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
    mpAcMainSave->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));

    mpToolBar->insertAction(mpAcToolSeprator, mpAcMainNew);
    mpToolBar->insertAction(mpAcToolSeprator, mpAcMainEdit);
    mpToolBar->insertAction(mpAcToolSeprator, mpAcMainDel);
    mpToolBar->insertAction(mpAcToolSeprator, mpAcMainSave);
    mpToolBar->insertAction(mpAcToolSeprator, mpAcMainCancel);

    //工具箱
    mpAcToolImport = mpMenuToolCase->addAction(QIcon(), mapMsg.value("tool_import_data"),
                                               this, SLOT(clickToolImport()));

    mpMenuToolCase->addSeparator();

    mpToolHideCurrentCol = mpMenuToolCase->addAction(mapMsg.value("tool_hide_current_col"));    //事件槽表格尚未创建，后代connect
    mpToolHideCurrentCol->setProperty(BSACFLAGS,  0);
    mpToolHideCurrentCol->setProperty(BSACRIGHT, true);

    mpToolShowAllCols = mpMenuToolCase->addAction(mapMsg.value("tool_show_all_cols"));    //事件槽表格尚未创建，后代connect
    mpToolShowAllCols->setProperty(BSACFLAGS,  0);
    mpToolShowAllCols->setProperty(BSACRIGHT, true);

    //开关盒
    mpAcOptHideDropRow = mpMenuOptionBox->addAction(QIcon(), mapMsg.value("opt_hide_drop_red_row"),
                                                    this, SLOT(clickOptHideDropRow()));
    mpAcOptHideDropRow->setProperty("optname", "opt_hide_drop_red_row");

    mpAcMainNew->setProperty(BSACFLAGS, bsacfClean);
    mpAcMainEdit->setProperty(BSACFLAGS, bsacfClean  | bsacfPlusId | bsacfNotChk);
    mpAcMainDel->setProperty(BSACFLAGS, bsacfClean | bsacfPlusId | bsacfNotChk);
    mpAcMainSave->setProperty(BSACFLAGS, bsacfDirty);
    mpAcMainCancel->setProperty(BSACFLAGS, bsacfDirty);
    mpAcToolImport->setProperty(BSACFLAGS, bsacfDirty);
    mpAcOptHideDropRow->setProperty(BSACFLAGS, bsacfDirty);

    mpAcMainSave->setProperty(BSACRIGHT, true);
    mpAcMainCancel->setProperty(BSACRIGHT, true);

    //状态栏
    mpSttValKey = new QLabel(this);
    mpSttValKey->setMinimumWidth(100);

    mpSttLblUpman = new QLabel(mapMsg.value("fld_upman").split(QChar(9)).at(0), this);
    mpSttLblUpman->setStyleSheet("color:#666; font-weight:900;");

    mpSttValUpman = new QLabel(this);
    mpSttValUpman->setMinimumWidth(50);

    mpSttLblUptime = new QLabel(mapMsg.value("fld_uptime").split(QChar(9)).at(0), this);
    mpSttLblUptime->setStyleSheet("color:#666; font-weight:900;");

    mpSttValUptime = new QLabel(this);
    mpSttValUptime->setMinimumWidth(150);

    mpStatusBar->addWidget(mpSttValKey);
    mpStatusBar->addWidget(mpSttLblUpman);
    mpStatusBar->addWidget(mpSttValUpman);
    mpStatusBar->addWidget(mpSttLblUptime);
    mpStatusBar->addWidget(mpSttValUptime);
}

QSize BsAbstractFormWin::sizeHint() const
{
    QSettings settings;
    settings.beginGroup(BSR17WinSize);
    QStringList winSize = settings.value(mMainTable).toString().split(QChar(','));
    int winW = ( winSize.length() == 2 ) ? QString(winSize.at(0)).toInt() : 800;
    int winH = ( winSize.length() == 2 ) ? QString(winSize.at(1)).toInt() : 550;
    if ( winW <= 0 ) winW = 800;
    if ( winH <= 0 ) winH = 550;
    settings.endGroup();
    return QSize(winW, winH);
}

void BsAbstractFormWin::setEditable(const bool editt)
{
    mEditable = editt;

    //状态
    uint stateFlags;

    if ( editt )
        stateFlags = bsacfDirty;
    else
        stateFlags = bsacfClean;

    if ( isSheetChecked() )
        stateFlags |= bsacfChecked;
    else
        stateFlags |= bsacfNotChk;

    if ( isValidRealSheetId() )
        stateFlags |= bsacfPlusId;

    //主按钮
    for ( int i = 0, iLen = mpToolBar->actions().length(); i < iLen; ++i ) {
        QAction *act = mpToolBar->actions().at(i);
        uint acFlags = act->property(BSACFLAGS).toUInt();
        bool rightAllow = act->property(BSACRIGHT).toBool();
        act->setEnabled(rightAllow && (stateFlags & acFlags) == acFlags);
    }

    //工具箱
    for ( int i = 0, iLen = mpMenuToolCase->actions().length(); i < iLen; ++i ) {
        QAction *act = mpMenuToolCase->actions().at(i);
        uint acFlags = act->property(BSACFLAGS).toUInt();
        bool rightAllow = act->property(BSACRIGHT).toBool();
        act->setEnabled(rightAllow && (stateFlags & acFlags) == acFlags);
    }

    //开关盒
    for ( int i = 0, iLen = mpMenuOptionBox->actions().length(); i < iLen; ++i ) {
        QAction *act = mpMenuOptionBox->actions().at(i);
        uint acFlags = act->property(BSACFLAGS).toUInt();
        bool rightAllow = act->property(BSACRIGHT).toBool();
        act->setEnabled(rightAllow && (stateFlags & acFlags) == acFlags);
    }

    //附加切换不同状态下的提示
    mpAcMainHelp->setStatusTip((editt) ? mapMsg.value("i_edit_mode_tip") : mapMsg.value("i_read_mode_tip"));
    mpFormGrid->setStatusTip((editt) ? mapMsg.value("i_edit_mode_tip") : mapMsg.value("i_read_mode_tip"));
}

void BsAbstractFormWin::closeEvent(QCloseEvent *e)
{
    //编辑态禁止关闭，但无明细空表格除外
    if ( mainNeedSaveDirty() || mpFormGrid->needSaveDirty() ) {

        if ( ! confirmDialog(this,
                           QStringLiteral("窗口处于编辑状态，直接关闭窗口会放弃最近未保存的修改和编辑。"),
                           QStringLiteral("确定要放弃保存吗？"),
                           mapMsg.value("btn_giveup_save"),
                           mapMsg.value("btn_cancel"),
                           QMessageBox::Question) ) {
            e->ignore();
            return;
        }
    }

    //保存窗口大小
    QSettings settings;
    settings.beginGroup(BSR17WinSize);
    settings.setValue(mMainTable, QStringLiteral("%1,%2").arg(width()).arg(height()));
    settings.endGroup();

    //保存表格列宽
    mpFormGrid->saveColWidths();

    //完成
    e->accept();

    //必须，里面有保存列宽
    BsWin::closeEvent(e);
}

void BsAbstractFormWin::clickOptHideDropRow()
{
    mpFormGrid->setDroppedRowByOption(mpAcOptHideDropRow->isChecked());
}


// BsAbstractSheetWin
BsAbstractSheetWin::BsAbstractSheetWin(QWidget *parent, const QString &name, const QStringList &fields)
    : BsAbstractFormWin(parent, name, fields, bswtSheet), mAllowPriceMoney(true), mCurrentSheetId(0)
{
    //用于权限判断
    mRightWinName = name;
    //Q_ASSERT(lstSheetWinTableNames.indexOf(mRightWinName) >= 0);
    //由于szd也是用的BsAbstractSheetWin但没有权限细设，所以不能用此断言。
    //此断言主要怕vi类命名太多没搞一致，这儿用不用没关系。

    //具体化mFields（包括列名、帮助提示、特性、……）
    BsField *fldShop = getFieldByName("shop");
    fldShop->mFldCnName = mapMsg.value(QStringLiteral("fldcname_%1_shop").arg(name));

    BsField *fldTrader = getFieldByName("trader");
    fldTrader->mFldCnName = mapMsg.value(QStringLiteral("fldcname_%1_trader").arg(name));

    BsField *fldActPay = getFieldByName("actpay");
    if ( fldActPay )
        fldActPay->mFldCnName = mapMsg.value(QStringLiteral("fldcname_%1_actpay").arg(name));

    //窗口
    mGuideObjectTip = mapMsg.value(QStringLiteral("win_%1").arg(name)).split(QChar(9)).at(1);
    mGuideClassTip = mapMsg.value("win_<sheet>").split(QChar(9)).at(1);
    mpGuide->setText(mGuideClassTip);

    //主按钮
    mpAcMainOpen = mpToolBar->addAction(QIcon(":/icon/find.png"), mapMsg.value("btn_sheet_open").split(QChar(9)).at(0),
                                    this, SLOT(clickOpenFind()));

    mpAcMainCheck = mpToolBar->addAction(QIcon(":/icon/check.png"), mapMsg.value("btn_sheet_check").split(QChar(9)).at(0),
                                     this, SLOT(clickCheck()));

    mpAcMainPrint = mpToolBar->addAction(QIcon(":/icon/print.png"), mapMsg.value("btn_print").split(QChar(9)).at(0),
                                     this, SLOT(clickPrint()));

    mpAcMainPrint->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));

    mpToolBar->insertAction(mpAcMainNew, mpAcMainOpen);
    mpToolBar->insertAction(mpAcToolSeprator, mpAcMainCheck);
    mpToolBar->insertAction(mpAcToolSeprator, mpAcMainPrint);

    mpAcMainOpen->setProperty(BSACFLAGS, bsacfClean);
    mpAcMainCheck->setProperty(BSACFLAGS, bsacfClean | bsacfPlusId | bsacfNotChk);
    mpAcMainPrint->setProperty(BSACFLAGS, bsacfClean | bsacfPlusId);

    mpAcMainOpen->setProperty(BSACRIGHT, true);
    mpAcMainNew->setProperty(BSACRIGHT, canDo(mRightWinName, bsrsNew));
    mpAcMainEdit->setProperty(BSACRIGHT, canDo(mRightWinName, bsrsUpd));
    mpAcMainDel->setProperty(BSACRIGHT, canDo(mRightWinName, bsrsDel));
    mpAcMainCheck->setProperty(BSACRIGHT, canDo(mRightWinName, bsrsCheck));
    mpAcMainPrint->setProperty(BSACRIGHT, canDo(mRightWinName, bsrsPrint));
    mpAcOptHideDropRow->setProperty(BSACRIGHT, canDo(mRightWinName, bsrsUpd));
    mpAcToolImport->setProperty(BSACRIGHT, canDo(mRightWinName, bsrsNew) || canDo(mRightWinName, bsrsUpd));
    mpAcToolExport->setProperty(BSACRIGHT, canDo(mRightWinName, bsrsExport));

    //工具箱
    mpToolPrintSetting = mpMenuToolCase->addAction(mapMsg.value("tool_print_setting"),
                                                   this, SLOT(clickToolPrintSetting()));
    mpToolPrintSetting->setProperty(BSACFLAGS,  0);
    mpToolPrintSetting->setProperty(BSACRIGHT, canDo(mRightWinName, bsrsPrint));
    mpMenuToolCase->insertAction(mpAcToolExport, mpToolPrintSetting);

    mpToolUnCheck = mpMenuToolCase->addAction(mapMsg.value("tool_sheet_uncheck"),
                                                   this, SLOT(clickToolUnCheck()));
    mpToolUnCheck->setProperty(BSACFLAGS, bsacfClean | bsacfPlusId | bsacfChecked);
    mpToolUnCheck->setProperty(BSACRIGHT, false);
    mpMenuToolCase->insertAction(mpAcToolExport, mpToolUnCheck);

    mpToolAdjustCurrentRowPosition = mpMenuToolCase->addAction(mapMsg.value("tool_adjust_current_row_position"),
                                                   this, SLOT(clickToolAdjustCurrentRowPosition()));
    mpToolAdjustCurrentRowPosition->setProperty(BSACFLAGS,  bsacfDirty);
    mpToolAdjustCurrentRowPosition->setProperty(BSACRIGHT, canDo(mRightWinName, bsrsUpd) || canDo(mRightWinName, bsrsNew));

    //开关盒
    mpAcOptSortBeforePrint = mpMenuOptionBox->addAction(QIcon(), mapMsg.value("opt_sort_befor_print"),
                                                      this, SLOT(clickOptSortBeforePrint()));
    mpAcOptSortBeforePrint->setProperty("optname", "opt_sort_befor_print");
    mpAcOptSortBeforePrint->setProperty(BSACFLAGS, 0);
    mpAcOptSortBeforePrint->setProperty(BSACRIGHT, canDo(mRightWinName, bsrsPrint));

    mpAcOptHideNoQtySizerColWhenOpen = mpMenuOptionBox->addAction(QIcon(), mapMsg.value("opt_hide_noqty_sizercol_when_open"),
                                                                this, SLOT(clickOptSortBeforePrint()));
    mpAcOptHideNoQtySizerColWhenOpen->setProperty("optname", "opt_hide_noqty_sizercol_when_open");
    mpAcOptHideNoQtySizerColWhenOpen->setProperty(BSACFLAGS, 0);
    mpAcOptHideNoQtySizerColWhenOpen->setProperty(BSACRIGHT, true);
    mpAcOptHideNoQtySizerColWhenOpen->setVisible(false);  //后代BsSheetCargoWin构造函数中才显示

    mpAcOptHideNoQtySizerColWhenPrint = mpMenuOptionBox->addAction(QIcon(), mapMsg.value("opt_hide_noqty_sizercol_when_print"),
                                                                 this, SLOT(clickOptSortBeforePrint()));
    mpAcOptHideNoQtySizerColWhenPrint->setProperty("optname", "opt_hide_noqty_sizercol_when_print");
    mpAcOptHideNoQtySizerColWhenPrint->setProperty(BSACFLAGS, 0);
    mpAcOptHideNoQtySizerColWhenPrint->setProperty(BSACRIGHT, canDo(mRightWinName, bsrsPrint));
    mpAcOptHideNoQtySizerColWhenPrint->setVisible(false); //后代BsSheetCargoWin构造函数中才显示

    //具体化help_tip
    mpAcMainOpen->setStatusTip(mapMsg.value("btn_sheet_open").split(QChar(9)).at(1));
    mpAcMainCheck->setStatusTip(mapMsg.value("btn_sheet_check").split(QChar(9)).at(1));
    mpAcMainPrint->setStatusTip(mapMsg.value("btn_print").split(QChar(9)).at(1));

    mpAcMainNew->setText(mapMsg.value("btn_sheet_new").split(QChar(9)).at(0));
    mpAcMainEdit->setText(mapMsg.value("btn_sheet_edit").split(QChar(9)).at(0));
    mpAcMainDel->setText(mapMsg.value("btn_sheet_del").split(QChar(9)).at(0));
    mpAcMainSave->setText(mapMsg.value("btn_sheet_save").split(QChar(9)).at(0));
    mpAcMainCancel->setText(mapMsg.value("btn_sheet_cancel").split(QChar(9)).at(0));

    mpAcMainNew->setStatusTip(mapMsg.value("btn_sheet_new").split(QChar(9)).at(1));
    mpAcMainEdit->setStatusTip(mapMsg.value("btn_sheet_edit").split(QChar(9)).at(1));
    mpAcMainDel->setStatusTip(mapMsg.value("btn_sheet_del").split(QChar(9)).at(1));
    mpAcMainSave->setStatusTip(mapMsg.value("btn_sheet_save").split(QChar(9)).at(1));
    mpAcMainCancel->setStatusTip(mapMsg.value("btn_sheet_cancel").split(QChar(9)).at(1));

    //打开查询内容===========================BEGIN
    mpBtnOpenBack  = new QToolButton(this);
    mpBtnOpenBack->setIcon(QIcon(":/icon/cancel.png"));
    mpBtnOpenBack->setIconSize(QSize(32, 32));

    QMenu *mnPeriod = new QMenu(this);
    mnPeriod->addAction(mapMsg.value("menu_today"), this, SLOT(clickQuickPeriod()));
    mnPeriod->addAction(mapMsg.value("menu_yesterday"), this, SLOT(clickQuickPeriod()));
    mnPeriod->addSeparator();
    mnPeriod->addAction(mapMsg.value("menu_this_week"), this, SLOT(clickQuickPeriod()));
    mnPeriod->addAction(mapMsg.value("menu_last_week"), this, SLOT(clickQuickPeriod()));
    mnPeriod->addSeparator();
    mnPeriod->addAction(mapMsg.value("menu_this_month"), this, SLOT(clickQuickPeriod()));
    mnPeriod->addAction(mapMsg.value("menu_last_month"), this, SLOT(clickQuickPeriod()));
    mnPeriod->addSeparator();
    mnPeriod->addAction(mapMsg.value("menu_this_year"), this, SLOT(clickQuickPeriod()));
    mnPeriod->addAction(mapMsg.value("menu_last_year"), this, SLOT(clickQuickPeriod()));
    mnPeriod->setStyleSheet("QMenu::item {padding-left:16px;}");
    mpBtnPeriod  = new QToolButton(this);
    mpBtnPeriod->setIcon(QIcon(":/icon/calendar.png"));
    mpBtnPeriod->setText(mapMsg.value("i_period_quick_select"));
    mpBtnPeriod->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mpBtnPeriod->setIconSize(QSize(12, 12));
    mpBtnPeriod->setMenu(mnPeriod);
    mpBtnPeriod->setPopupMode(QToolButton::InstantPopup);

    mpConCheck = new BsConCheck(this, true);
    mpConDateB  = new BsFldBox(this, getFieldByName("dated"), nullptr, true);
    mpConDateE  = new BsFldBox(this, getFieldByName("dated"), nullptr, true);
    mpConShop   = new BsFldBox(this, fldShop, dsShop, true);
    mpConStype  = new BsFldBox(this, getFieldByName("stype"), mpDsStype, true);
    mpConStaff  = new BsFldBox(this, getFieldByName("staff"), mpDsStaff, true);
    mpConTrader = new BsFldBox(this, fldTrader, mpDsTrader, true);

    mpConDateB->mpEditor->setMyPlaceText(mapMsg.value("i_date_begin_text"));
    mpConDateE->mpEditor->setMyPlaceText(mapMsg.value("i_date_end_text"));
    mpConShop->mpEditor->setMyPlaceText(fldShop->mFldCnName);
    mpConStype->mpEditor->setMyPlaceText(getFieldByName("stype")->mFldCnName);
    mpConStaff->mpEditor->setMyPlaceText(getFieldByName("staff")->mFldCnName);
    mpConTrader->mpEditor->setMyPlaceText(fldTrader->mFldCnName);

    mpConDateB->mpEditor->setMyPlaceColor(QColor(Qt::gray));
    mpConDateE->mpEditor->setMyPlaceColor(QColor(Qt::gray));
    mpConShop->mpEditor->setMyPlaceColor(QColor(Qt::gray));
    mpConStype->mpEditor->setMyPlaceColor(QColor(Qt::gray));
    mpConStaff->mpEditor->setMyPlaceColor(QColor(Qt::gray));
    mpConTrader->mpEditor->setMyPlaceColor(QColor(Qt::gray));

    mpConDateB->mpLabel->setText(mapMsg.value("i_date_begin_label"));
    mpConDateE->mpLabel->setText(mapMsg.value("i_date_end_label"));

    mpBtnQuery  = new QToolButton(this);
    mpBtnQuery->setIcon(QIcon(":/icon/zoomlens.png"));
    mpBtnQuery->setIconSize(QSize(32, 32));

    mpPnlOpenCon = new QWidget(this);
    QGridLayout *layCon = new QGridLayout(mpPnlOpenCon);
    layCon->setContentsMargins(15, 0, 15, 0);
    layCon->setHorizontalSpacing(20);
    layCon->addWidget(mpBtnOpenBack,    0, 0, 2, 1);
    layCon->addWidget(mpBtnPeriod,      0, 1);
    layCon->addWidget(mpConDateB,       0, 2);
    layCon->addWidget(mpConDateE,       0, 3);
    layCon->addWidget(mpConShop,        0, 4);
    layCon->addWidget(mpBtnQuery,       0, 5, 2, 1);
    layCon->addWidget(mpConCheck,      1, 1, Qt::AlignCenter);
    layCon->addWidget(mpConStype,       1, 2);
    layCon->addWidget(mpConStaff,       1, 3);
    layCon->addWidget(mpConTrader,      1, 4);
    layCon->setColumnStretch(0, 0);
    layCon->setColumnStretch(1, 0);
    layCon->setColumnStretch(2, 2);
    layCon->setColumnStretch(3, 2);
    layCon->setColumnStretch(4, 3);
    layCon->setColumnStretch(5, 0);
    int size = 2 * mpConShop->sizeHint().height() + layCon->verticalSpacing();
    mpBtnOpenBack->setFixedSize(size, size);
    mpBtnQuery->setFixedSize(size, size);

    mpFindGrid = new BsQueryGrid(this);
    mpFindGrid->setStatusTip(mapMsg.value("i_sheet_query_open_tip"));
    connect(mpFindGrid, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(doubleClickOpenSheet(QTableWidgetItem*)));

    mpPnlOpener = new QWidget(this);
    mpPnlOpener->setStyleSheet(QLatin1String(".QWidget{background-color:#eeeeeb;}"));
    QVBoxLayout *layOpener = new QVBoxLayout(mpPnlOpener);
    layOpener->setContentsMargins(3, 10, 3, 0);
    layOpener->addWidget(mpPnlOpenCon);
    layOpener->addWidget(mpFindGrid, 1);

    mpLayBody = new QVBoxLayout(mpBody);
    mpLayBody->setContentsMargins(1, 0, 1, 8);
    mpLayBody->setSpacing(0);
    mpLayBody->addWidget(mpPnlOpener, 1);

    mpConDateB->mpEditor->setDataValue(QDateTime(QDate::currentDate()).toMSecsSinceEpoch() / 1000);
    mpConDateE->mpEditor->setDataValue(QDateTime(QDate::currentDate()).toMSecsSinceEpoch() / 1000);
    mpPnlOpener->hide();

    mpBtnSwitchLimBind = new QToolButton(this);
    mpBtnSwitchLimBind->setIconSize(QSize(24, 24));
    mpBtnSwitchLimBind->setToolButtonStyle(Qt::ToolButtonIconOnly);
    mpBtnSwitchLimBind->setIcon(QIcon(":/icon/switch.png"));
    mpBtnSwitchLimBind->setStyleSheet(QLatin1String("QToolButton{border-radius:5px;}"));
    mpBtnSwitchLimBind->hide();
    connect(mpBtnSwitchLimBind, &QToolButton::clicked, this, &BsAbstractSheetWin::clickSwitchLimTrader);

    //绑店条件
    if ( ! loginTrader.isEmpty() ) {
        mpConShop->mpEditor->setDataValue(loginTrader);
        mpConShop->mpEditor->setEnabled(false);
    }

    connect(mpBtnOpenBack, SIGNAL(clicked(bool)), this, SLOT(clickCancelOpenPage()));
    connect(mpBtnQuery, SIGNAL(clicked(bool)), this, SLOT(clickExecuteQuery()));
    //打开查询内容===========================END

    //表头
    QString sheetName = mapMsg.value(QStringLiteral("win_%1").arg(mMainTable)).split(QChar(9)).at(0);
    mpSheetName = new QLabel(sheetName, this);
    mpSheetName->setStyleSheet("font-size:12pt;");
    mpSheetId = new BsSheetIdLabel(this, name);
    mpCheckMark  = new BsSheetCheckLabel(this);

    mpLayTitleBox = new QHBoxLayout();
    mpLayTitleBox->setContentsMargins(0, 0, 0, 0);
    mpLayTitleBox->setSpacing(100);
    mpLayTitleBox->addWidget(mpSheetName);
    mpLayTitleBox->addWidget(mpSheetId);
    mpLayTitleBox->addWidget(mpCheckMark);
    mpLayTitleBox->addStretch();

    mpDated  = new BsFldBox(this, getFieldByName("dated"), nullptr);
    mpStype  = new BsFldBox(this, getFieldByName("stype"), mpDsStype);
    mpShop  = new BsFldBox(this, fldShop, dsShop);
    mpProof  = new BsFldBox(this, getFieldByName("proof"), nullptr);
    mpStaff  = new BsFldBox(this, getFieldByName("staff"), mpDsStaff);
    mpTrader  = new BsFldBox(this, fldTrader, mpDsTrader);
    mpRemark  = new BsFldBox(this, getFieldByName("remark"), nullptr);

    mpLayEditBox = new QGridLayout();
    mpLayEditBox->setContentsMargins(0, 0, 0, 0);
    mpLayEditBox->setHorizontalSpacing(30);
    mpLayEditBox->addWidget(mpDated,  0, 0);
    mpLayEditBox->addWidget(mpStype,  0, 1);
    mpLayEditBox->addWidget(mpShop,   0, 2);
    mpLayEditBox->addWidget(mpProof,  1, 0);
    mpLayEditBox->addWidget(mpStaff,  1, 1);
    mpLayEditBox->addWidget(mpTrader, 1, 2);
    mpLayEditBox->addWidget(mpRemark, 2, 0, 1, 3);
    mpLayEditBox->setColumnStretch(0, 2);
    mpLayEditBox->setColumnStretch(1, 2);
    mpLayEditBox->setColumnStretch(2, 3);

    mpPnlHeader = new QWidget(this);
    QVBoxLayout *layHeader = new QVBoxLayout(mpPnlHeader);
    layHeader->setContentsMargins(10, 5, 10, 5);
    layHeader->addLayout(mpLayTitleBox);
    layHeader->addLayout(mpLayEditBox);

    //表尾
    if ( fldActPay && getFieldByName("actowe") ) {
        mpActPay = new BsFldBox(this, fldActPay, nullptr);
        mpActOwe = new BsFldBox(this, getFieldByName("actowe"), nullptr);

        mpPnlPayOwe = new QWidget(this);
        QHBoxLayout *layFooter = new QHBoxLayout(mpPnlPayOwe);
        layFooter->setContentsMargins(10, 3, 10, 0);
        layFooter->setSpacing(30);
        layFooter->addStretch(1);
        layFooter->addWidget(mpActPay, 1);
        layFooter->addWidget(mpActOwe, 1);
    }
    else {
        mpPnlPayOwe = nullptr;
        mpActPay = nullptr;
        mpPnlPayOwe = nullptr;
    }

    //状态栏增加
    mpSttLblChecker = new QLabel(mapMsg.value("fld_checker").split(QChar(9)).at(0), this);
    mpSttLblChecker->setStyleSheet("color:#666; font-weight:900;");

    mpSttValChecker = new QLabel(this);
    mpSttValChecker->setMinimumWidth(50);

    mpSttLblChkTime = new QLabel(mapMsg.value("i_status_check_time"), this);
    mpSttLblChkTime->setStyleSheet("color:#666; font-weight:900;");

    mpSttValChkTime = new QLabel(this);

    mpStatusBar->addWidget(mpSttLblChecker);
    mpStatusBar->addWidget(mpSttValChecker);
    mpStatusBar->addWidget(mpSttLblChkTime);
    mpStatusBar->addWidget(mpSttValChkTime);

    //布局
    mpLayBody->addWidget(mpPnlHeader);
    if ( mpPnlPayOwe ) {
        mpLayBody->addWidget(mpPnlPayOwe);
    }

    //打开数据源
    dsShop->reload();

    //绑店限定
    if ( ! loginTrader.isEmpty() ) {
        mpShop->mpEditor->setDataValue(loginTrader);
        mpShop->mpEditor->setEnabled(false);
    }

    //打印控制件
    mpPrinter = new LxPrinter(this);
}

BsAbstractSheetWin::~BsAbstractSheetWin()
{
    delete mpPrinter;
}

void BsAbstractSheetWin::setEditable(const bool editt)
{
    BsAbstractFormWin::setEditable(editt);

    for ( int i = 0, iLen = mpPnlHeader->children().count(); i < iLen; ++i ) {
        BsFldBox *edt = qobject_cast<BsFldBox*>(mpPnlHeader->children().at(i));
        if ( edt ) {
            edt->mpEditor->setReadOnly(!editt);
        }
    }
    if ( mpPnlPayOwe ) {
        for ( int i = 0, iLen = mpPnlPayOwe->children().count(); i < iLen; ++i ) {
            BsFldBox *edt = qobject_cast<BsFldBox*>(mpPnlPayOwe->children().at(i));
            if ( edt ) {
                edt->mpEditor->setReadOnly(!editt);
            }
        }
    }
    mpSheetGrid->setEditable(editt);

    //绑店检查
    if ( ! loginTrader.isEmpty() ) {
        mpShop->mpEditor->setDataValue(loginTrader);
        mpShop->mpEditor->setEnabled(false);
    }

    //助手
    updateTabber(editt);
}

void BsAbstractSheetWin::openSheet(const int sheetId)   //注意与doSave()的一致性
{
    //表头
    if ( sheetId <= 0 ) {
        mpSheetId->setDataValue(sheetId);
        mpCheckMark->setDataValue(false, sheetId);
        mpDated->mpEditor->setDataValue( (sheetId < 0) ? QDateTime::currentMSecsSinceEpoch() / 1000 : 0 );
        mpStype->mpEditor->setDataValue(QString());
        mpStaff->mpEditor->setDataValue(QString());
        mpProof->mpEditor->setDataValue(QString());
        mpShop->mpEditor->setDataValue(QString());
        mpTrader->mpEditor->setDataValue(QString());
        mpRemark->mpEditor->setDataValue(QString());
        if ( mpActPay && mpActOwe ) {
            mpActPay->mpEditor->setDataValue(0);
            mpActOwe->mpEditor->setDataValue(0);
        }
        mpSttValKey->clear();
        mpSttValUpman->clear();
        mpSttValUptime->clear();
        mpSttValChecker->clear();
        mpSttValChkTime->clear();

        mpSheetCargoGrid->setTraderDiscount(1.0);
        mpSheetCargoGrid->setTraderName(QString());
    }
    else {
        QStringList sels;
        for ( int i = 0, iLen = mFields.length(); i < iLen; ++i ) {
            BsField *fld = mFields.at(i);
            uint flags = fld->mFlags;
            if ( ((flags & bsffHead) == bsffHead || (flags & bsffHideSys) == bsffHideSys) && (flags & bsffGrid) != bsffGrid ) {
                sels << fld->mFldName;
            }
        }

        QString sql = QStringLiteral("SELECT %1 FROM %2 WHERE sheetid=%3;")
                .arg(sels.join(QChar(44))).arg(mMainTable).arg(sheetId);
        QSqlQuery qry;
        qry.setForwardOnly(true);
        qry.setNumericalPrecisionPolicy(QSql::LowPrecisionInt64);
        qry.exec(sql);
        if ( qry.lastError().isValid() ) qDebug() << qry.lastError().text() << "\n" << sql;
        QSqlRecord rec = qry.record();

        if ( qry.next() ) {
            int idxSheetId = rec.indexOf(QStringLiteral("sheetid"));
            Q_ASSERT(idxSheetId >= 0);
            mpSheetId->setDataValue(qry.value(idxSheetId));

            int idxChkTime = rec.indexOf(QStringLiteral("chktime"));
            Q_ASSERT(idxChkTime >= 0);
            mpCheckMark->setDataValue(qry.value(idxChkTime), sheetId);

            for ( int i = 0, iLen = mpPnlHeader->children().count(); i < iLen; ++i ) {
                BsFldBox *edt = qobject_cast<BsFldBox*>(mpPnlHeader->children().at(i));
                if ( edt ) {
                    int idx = rec.indexOf(edt->getFieldName());
                    Q_ASSERT(idx >= 0);
                    edt->mpEditor->setDataValue(qry.value(idx));
                    edt->setProperty(BSVALUE_OLD, edt->mpEditor->text());
                }
            }

            if ( mpPnlPayOwe ) {
                for ( int i = 0, iLen = mpPnlPayOwe->children().count(); i < iLen; ++i ) {
                    BsFldBox *edt = qobject_cast<BsFldBox*>(mpPnlPayOwe->children().at(i));
                    if ( edt ) {
                        int idx = rec.indexOf(edt->getFieldName());
                        Q_ASSERT(idx >= 0);
                        edt->mpEditor->setDataValue(qry.value(idx));
                        edt->setProperty(BSVALUE_OLD, edt->mpEditor->text());
                    }
                }
            }

            mpSttValKey->setText(QStringLiteral("%1%2").arg(mMainTable.toUpper())
                                 .arg(qry.value(idxSheetId).toInt(), 8, 10, QLatin1Char('0')));

            int idxUpman = rec.indexOf(QStringLiteral("upman"));
            Q_ASSERT(idxUpman >= 0);
            mpSttValUpman->setText(qry.value(idxUpman).toString());

            int idxUptime = rec.indexOf(QStringLiteral("uptime"));
            Q_ASSERT(idxUptime >= 0);
            qint64 uptime = qry.value(idxUptime).toLongLong();
            QDateTime dtUptime = QDateTime::fromMSecsSinceEpoch(uptime * 1000);
            QString uptimeStr = (uptime > 0) ? dtUptime.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")) : QString();
            mpSttValUptime->setText(uptimeStr);

            int idxChecker = rec.indexOf(QStringLiteral("checker"));
            Q_ASSERT(idxChecker >= 0);
            mpSttValChecker->setText(qry.value(idxChecker).toString());

            qint64 chktime = qry.value(idxChkTime).toLongLong();
            QDateTime dtChkTime = QDateTime::fromMSecsSinceEpoch(chktime * 1000);
            QString chktimeStr = (chktime > 0) ? dtChkTime.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")) : QString();
            mpSttValChkTime->setText(chktimeStr);
        }
        qry.finish();
    }

    //表格
    mpSheetGrid->openBySheetId(sheetId);

    //记录当前单据号
    mCurrentSheetId = sheetId;

    //状态
    setEditable(sheetId < 0);

    //显示
    mpLayBody->setContentsMargins(1, 0, 1, 8);
    mpPnlOpener->hide();
    mpBtnSwitchLimBind->hide();
    mpToolBar->show();
    mpPnlHeader->show();
    mpSheetGrid->show();
    mpStatusBar->show();
    if ( mpPnlPayOwe ) {
        mpPnlPayOwe->setVisible(mAllowPriceMoney && mMainTable != QStringLiteral("syd"));
    }

    //选项
    if ( mpAcOptHideNoQtySizerColWhenOpen->isVisible() && mpAcOptHideNoQtySizerColWhenOpen->isChecked())
        mpSheetCargoGrid->autoHideNoQtySizerCol();
}

void BsAbstractSheetWin::savedReconcile(const int sheetId, const qint64 uptime)
{
    mCurrentSheetId = sheetId;
    mpSheetId->setDataValue(sheetId);
    mpCheckMark->setDataValue(0, sheetId);
    mpSttValKey->setText(QStringLiteral("%1%2").arg(mMainTable.toUpper())
                         .arg(sheetId, 8, 10, QLatin1Char('0')));
    mpSttValUpman->setText(loginer);
    QDateTime dtUptime = QDateTime::fromMSecsSinceEpoch(uptime * 1000);
    mpSttValUptime->setText(dtUptime.toString(QStringLiteral("yyyy-MM-dd hh:mm:ss")));

    for ( int i = 0, iLen = mpPnlHeader->children().count(); i < iLen; ++i ) {
        BsFldBox *edt = qobject_cast<BsFldBox*>(mpPnlHeader->children().at(i));
        if ( edt ) {
            edt->setProperty(BSVALUE_OLD, edt->mpEditor->text());
        }
    }

    if ( mpPnlPayOwe ) {
        for ( int i = 0, iLen = mpPnlPayOwe->children().count(); i < iLen; ++i ) {
            BsFldBox *edt = qobject_cast<BsFldBox*>(mpPnlPayOwe->children().at(i));
            if ( edt ) {
                edt->setProperty(BSVALUE_OLD, edt->mpEditor->text());
            }
        }
    }
}

void BsAbstractSheetWin::cancelRestore()
{
    for ( int i = 0, iLen = mpPnlHeader->children().count(); i < iLen; ++i ) {
        BsFldBox *edt = qobject_cast<BsFldBox*>(mpPnlHeader->children().at(i));
        if ( edt ) {
            edt->mpEditor->setText( edt->property(BSVALUE_OLD).toString() );
        }
    }

    if ( mpPnlPayOwe ) {
        for ( int i = 0, iLen = mpPnlPayOwe->children().count(); i < iLen; ++i ) {
            BsFldBox *edt = qobject_cast<BsFldBox*>(mpPnlPayOwe->children().at(i));
            if ( edt ) {
                edt->mpEditor->setText( edt->property(BSVALUE_OLD).toString() );
            }
        }
    }
}

QString BsAbstractSheetWin::getPrintValue(const QString &valueName) const
{
    if ( valueName == QStringLiteral("sheetid") )
        return mpSheetId->getDisplayText();

    else if ( valueName == QStringLiteral("dated") )
        return mpDated->mpEditor->text();

    else if ( valueName == QStringLiteral("proof") )
        return mpProof->mpEditor->text();

    else if ( valueName == QStringLiteral("stype") )
        return mpStype->mpEditor->text();

    else if ( valueName == QStringLiteral("staff") )
        return mpStaff->mpEditor->text();

    else if ( valueName == QStringLiteral("shop") )
        return mpShop->mpEditor->text();

    else if ( valueName == QStringLiteral("trader") )
        return mpTrader->mpEditor->text();

    else if ( valueName == QStringLiteral("remark") )
        return mpRemark->mpEditor->text();

    else if ( valueName == QStringLiteral("actpay") )
        return mpActPay->mpEditor->text();

    else if ( valueName == QStringLiteral("actowe") )
        return mpActOwe->mpEditor->text();

    else if ( valueName == QStringLiteral("upman") )
        return mpSttValUpman->text();

    else if ( valueName == QStringLiteral("uptime") )
        return mpSttValUptime->text();

    else if ( valueName == QStringLiteral("sumqty") )
        return mpGrid->getFooterValueByField(QStringLiteral("qty"));

    else if ( valueName == QStringLiteral("summoney") )
        return mpGrid->getFooterValueByField(QStringLiteral("actmoney"));

    else if ( valueName == QStringLiteral("sumdis") )
        return mpGrid->getFooterValueByField(QStringLiteral("dismoney"));

    else if ( valueName == QStringLiteral("sumowe") )
    {
        QString sql = QStringLiteral("select sumowe from tmp_query_dataset;");
        QSqlQuery qry;
        qry.exec(sql);
        if ( qry.next() ) {
            QString sumOweValue = bsNumForRead(qry.value(0).toLongLong(), mapOption.value("dots_of_money").toInt());
            if ( !mpCheckMark->getDataCheckedValue() ) {
                sumOweValue += QStringLiteral("未审");
            }
            return sumOweValue;
        }
        return QString();
    }

    else if ( valueName.indexOf(QChar('.')) > 0 )
    {
        QStringList vpair = valueName.split(QChar('.'));
        Q_ASSERT(vpair.length() == 2);
        QString vtbl = vpair.at(0);

        QString dbtbl;
        if ( vtbl == QStringLiteral("trader") ) {
            if ( mMainTable.startsWith(QStringLiteral("cg")) )
                dbtbl = QStringLiteral("supplier");
            else if ( mMainTable.startsWith(QStringLiteral("pf"))
                      || mMainTable.startsWith(QStringLiteral("ls")) )
                dbtbl = QStringLiteral("customer");
            else
                dbtbl = QStringLiteral("shop");
        }

        QString who = (vtbl == QStringLiteral("trader"))
                ? mpTrader->mpEditor->getDataValue() : mpShop->mpEditor->getDataValue();

        QString sql = QStringLiteral("select %1 from %2 where kname='%3';")
                .arg(vpair.at(1)).arg(dbtbl).arg(who);
        QSqlQuery qry;
        qry.exec(sql);
        if ( qry.next() )
            return qry.value(0).toString();
        return QString();
    }

    return QString();
}

QString BsAbstractSheetWin::getPrintValue(const QString &cargoTableField, const int gridRow)
{
    if ( cargoTableField == QStringLiteral("rowno") )
        return QString::number(gridRow + 1);

    if ( gridRow >= mpGrid->rowCount() )
        return mapMsg.value("word_total");

    QString svalue = dsCargo->getValue(mpGrid->item(gridRow, 0)->text(), cargoTableField);

    QStringList fDefs = mapMsg.value(QStringLiteral("fld_%1").arg(cargoTableField)).split(QChar(9));
    if ( fDefs.length() >= 5 ) {
        uint ff = QString(fDefs.at(3)).toUInt();
        uint dots = QString(fDefs.at(4)).toInt();
        if ( (ff & bsffNumeric) == bsffNumeric && dots < 5 ) {
            double fvalue = svalue.toLongLong() / 10000.0;
            return QString::number(fvalue, 'f', dots);
        }
    }

    return svalue;
}

QString BsAbstractSheetWin::getGridItemValue(const int row, const int col) const
{
    if ( row < mpGrid->rowCount() )
    {
        if (row >= 0 && col >= 0 && col < mpGrid->columnCount() )
        {
            if ( mpGrid->item(row, col) )
                return mpGrid->item(row, col)->text();
        }
    }
    else
    {
        if (col > 0 && col < mpGrid->mpFooter->columnCount() )
        {
            if ( mpGrid->mpFooter->item(0, col) ) {
                QString txt = mpGrid->mpFooter->item(0, col)->text();
                if ( txt.contains(QChar('<')) )
                    return QString();
                else
                    return txt;
            }
        }
        else
            return mapMsg.value("word_total");
    }
    return QString();
}

QString BsAbstractSheetWin::getGridItemValue(const int row, const QString &fieldName) const
{
    int col = mpGrid->getColumnIndexByFieldName(fieldName);
    if ( row < mpGrid->rowCount() && col >= 0 )
    {
        if ( mpGrid->item(row, col) )
            return mpGrid->item(row, col)->text();
    }
    return QString();
}

BsField *BsAbstractSheetWin::getGridFieldByName(const QString &fieldName) const
{
    return mpGrid->getFieldByName(fieldName);
}

int BsAbstractSheetWin::getGridColByField(const QString &fieldName) const
{
    return mpGrid->getColumnIndexByFieldName(fieldName);
}

int BsAbstractSheetWin::getGridRowCount() const
{
    return mpGrid->rowCount();
}

QStringList BsAbstractSheetWin::getSizerNameListForPrint() const
{
    return mpSheetCargoGrid->getSizerNameListForPrint();
}

QStringList BsAbstractSheetWin::getSizerQtysOfRowForPrint(const int row)
{
    return mpSheetCargoGrid->getSizerQtysOfRowForPrint(row, printZeroSizeQty());
}

bool BsAbstractSheetWin::isLastRow(const int row)
{
    return row == mpGrid->rowCount();
}

void BsAbstractSheetWin::closeEvent(QCloseEvent *e)
{
    mpFindGrid->saveColWidths();
    BsAbstractFormWin::closeEvent(e);
}

void BsAbstractSheetWin::resizeEvent(QResizeEvent *e)
{
    BsAbstractFormWin::resizeEvent(e);

    QTimer::singleShot(0, [=]{
        relocateSwitchLimButton();
    });
}

void BsAbstractSheetWin::doOpenFind()
{
    mpLayBody->setContentsMargins(0, 0, 0, 0);
    mpPnlOpener->show();
    mpToolBar->hide();
    mpPnlHeader->hide();
    mpGrid->hide();
    mpStatusBar->hide();
    if ( mpPnlPayOwe )
        mpPnlPayOwe->hide();

    QTimer::singleShot(0, [=]{
        relocateSwitchLimButton();
    });
}

void BsAbstractSheetWin::doCheck()
{
    if ( ! confirmDialog(this,
                         mapMsg.value("i_check_sheet_notice"),
                         mapMsg.value("i_check_sheet_confirm"),
                         mapMsg.value("btn_ok"),
                         mapMsg.value("btn_cancel"),
                         QMessageBox::Warning) )
        return;

    qint64 chktime = QDateTime::currentMSecsSinceEpoch() / 1000;
    QString sql = QStringLiteral("update %1 set checker='%2', chktime=%3 where sheetid=%4;")
            .arg(mMainTable).arg(loginer).arg(chktime).arg(mCurrentSheetId);
    QSqlQuery qry;
    qry.exec(sql);
    if ( qry.lastError().isValid() )
        QMessageBox::information(this, QString(), mapMsg.value("i_check_sheet_failed"));
    else {
        mpAcMainCheck->setEnabled(false);
        mpCheckMark->setDataValue(chktime, mCurrentSheetId);
        mpSttValChecker->setText(loginer);
        mpSttValChkTime->setText(QDateTime::fromMSecsSinceEpoch(1000 * chktime).toString("yyyy-MM-dd hh:mm:ss"));
        QMessageBox::information(this, QString(), mapMsg.value("i_check_sheet_success"));
    }
}

void BsAbstractSheetWin::doPrintRequest()
{
    if ( mpPrinter->needQueryTraderSumOwe() ) {
        QStringList params;
        params << QStringLiteral("QRYPRINTOWE");
        params << QString::number(QDateTime::currentMSecsSinceEpoch() * 1000);
        params << (mMainTable.startsWith(QStringLiteral("pf")) ? QStringLiteral("pf") : QStringLiteral("cg"));
        params << mpTrader->mpEditor->getDataValue();
        params << (mpCheckMark->getDataCheckedValue() ? QStringLiteral("999") : QStringLiteral("0"));
        netSocket->netRequest(this, params, QStringLiteral("正在查询欠款值，请稍候……"));       // => onSocketRequestOk
    }
    else {
        doPrintContinue();
    }
}

void BsAbstractSheetWin::doPrintContinue()
{
    if ( mpAcOptSortBeforePrint->isChecked() ) {
        mpGrid->sortByRowTime();
    }

    if ( mpAcOptHideNoQtySizerColWhenPrint->isVisible() && mpAcOptHideNoQtySizerColWhenPrint->isChecked())
        mpSheetCargoGrid->autoHideNoQtySizerCol();

    QString printErr = mpPrinter->doPrint();
    if ( !printErr.isEmpty() )
        QMessageBox::information(this, QString(), printErr);
}

void BsAbstractSheetWin::doNew()
{
    openSheet(-1);
}

void BsAbstractSheetWin::doEdit()
{
    setEditable(true);

    QString trader = mpTrader->mpEditor->text();
    mpSheetCargoGrid->setTraderDiscount(getTraderDisByName(trader));
    mpSheetCargoGrid->setTraderName(trader);
}

void BsAbstractSheetWin::doDel()
{
    if ( ! confirmDialog(this,
                         mapMsg.value("i_delete_sheet_notice"),
                         mapMsg.value("i_delete_sheet_confirm"),
                         mapMsg.value("btn_ok"),
                         mapMsg.value("btn_cancel"),
                         QMessageBox::Warning) )
    {
        return;
    }

    QString waitMsg = QStringLiteral("正在删除，请稍侯……");

    QStringList params;
    params << QStringLiteral("BIZDELETE");
    params << QString::number(QDateTime::currentMSecsSinceEpoch() * 1000);
    params << mMainTable;
    params << QString::number(mCurrentSheetId);
    netSocket->netRequest(this, params, waitMsg);       // => onSocketRequestOk
}

void BsAbstractSheetWin::doSave() //注意与openSheet()的一致性
{
    //存前检查
    uint ret = mpSheetGrid->saveCheck();
    if ( ret == bsccError )
    {
        QMessageBox::critical(nullptr, QString(), mapMsg.value("i_save_found_error"));
        return;
    }
    else if ( ret == bsccWarning )
    {
        if ( ! confirmDialog(this,
                             mapMsg.value("i_save_found_warning"),
                             mapMsg.value("i_save_ask_warning"),
                             mapMsg.value("btn_ok"),
                             mapMsg.value("btn_cancel"),
                             QMessageBox::Warning) )
            return;
    }

    //主表值
    QStringList mainValues;
    mainValues << mpShop->mpEditor->getDataValue();
    mainValues << mpTrader->mpEditor->getDataValue();
    mainValues << mpStype->mpEditor->getDataValue();
    mainValues << mpStaff->mpEditor->getDataValue();
    mainValues << mpRemark->mpEditor->getDataValue();
    mainValues << mpActPay->mpEditor->getDataValue();

    //从表值行
    QStringList dtlLines = mpSheetCargoGrid->getPostDataLines();

    //提示文字
    QString waitMsg = QStringLiteral("正在提交，请稍侯……");

    //提交
    if ( mCurrentSheetId > 0 ) {
        QStringList params;
        params << QStringLiteral("BIZEDIT");
        params << QString::number(QDateTime::currentMSecsSinceEpoch() * 1000);
        params << mMainTable;
        params << QString::number(mCurrentSheetId);
        params << mainValues.join(QChar('\t'));
        params << dtlLines.join(QChar('\n'));
        netSocket->netRequest(this, params, waitMsg);       // => onSocketRequestOk
    }
    else {
        QStringList params;
        params << QStringLiteral("BIZINSERT");
        params << QString::number(QDateTime::currentMSecsSinceEpoch() * 1000);
        params << mMainTable;
        params << mainValues.join(QChar('\t'));
        params << dtlLines.join(QChar('\n'));
        netSocket->netRequest(this, params, waitMsg);       // => onSocketRequestOk
    }
}

void BsAbstractSheetWin::doCancel()
{
    if ( mCurrentSheetId > 0 ) {
        mpSheetGrid->cancelRestore();
        mpSheetGrid->setEditable(false);
        cancelRestore();
        setEditable(false);
    }
    else {
        openSheet(0);
    }
}

bool BsAbstractSheetWin::mainNeedSaveDirty()
{
    if ( ! isEditing() )
        return false;

    bool actPayDirty = ( mpActPay ) ? mpActPay->mpEditor->isDirty() : false;

    return mpStype->mpEditor->isDirty() ||
            mpStaff->mpEditor->isDirty() ||
            mpShop->mpEditor->isDirty() ||
            mpTrader->mpEditor->isDirty() ||
            mpProof->mpEditor->isDirty() ||
            ( mpDated->mpEditor->isDirty() && mCurrentSheetId > 0 ) ||
            mpRemark->mpEditor->isDirty() ||
            actPayDirty;
}

double BsAbstractSheetWin::getTraderDisByName(const QString &name)
{
    if ( mMainTable.contains("cg") ) {
        if ( dsSupplier->foundKey(name) ) {
            return dsSupplier->getValue(name, QStringLiteral("regdis")).toLongLong() / 10000.0;
        }
    }
    else if ( mMainTable.contains("pf") || mMainTable.contains("xs") || mMainTable.contains("lsd") ) {
        if ( dsCustomer->foundKey(name) ) {
            return dsCustomer->getValue(name, QStringLiteral("regdis")).toLongLong() / 10000.0;
        }
    }
    else {
        if ( dsShop->foundKey(name) ) {
            return dsShop->getValue(name, QStringLiteral("regdis")).toLongLong() / 10000.0;
        }
    }
    return 1.0;
}

void BsAbstractSheetWin::clickToolPrintSetting()
{
    LxPrintSettingWin dlg(this);
    dlg.setWindowTitle(windowTitle() + mapMsg.value("tool_print_setting"));
    if ( QDialog::Accepted == dlg.exec() )
        mpPrinter->loadPrintSettings();
}

void BsAbstractSheetWin::clickToolUnCheck()
{
}

void BsAbstractSheetWin::clickToolAdjustCurrentRowPosition()
{
    //先检查原序
    if ( !mpSheetGrid->isCleanSort() ) {
        QMessageBox::information(this, QString(), mapMsg.value("i_adjust_rowtime_need_clean_sort"));
        return;
    }
    //调整
    mpSheetGrid->adjustCurrentRowPosition();
}

void BsAbstractSheetWin::clickOptSortBeforePrint()
{
    //Nothing need to do
}

void BsAbstractSheetWin::clickQuickPeriod()
{
    QAction *act = qobject_cast<QAction*>(QObject::sender());
    Q_ASSERT(act);
    setQuickDate(act->text(), mpConDateB, mpConDateE, mpBtnPeriod);
}

void BsAbstractSheetWin::clickExecuteQuery()
{
    QStringList params;
    params << QStringLiteral("QRYSHEET");
    params << QString::number(QDateTime::currentMSecsSinceEpoch() * 1000);
    params << mMainTable;
    params << mpConDateB->mpEditor->text();
    params << mpConDateE->mpEditor->text();
    params << mpConShop->mpEditor->getDataValue();
    params << mpConTrader->mpEditor->getDataValue();
    params << mpConStype->mpEditor->getDataValue();
    params << mpConStaff->mpEditor->getDataValue();

    QString chkExp = mpConCheck->getConExp();
    if ( chkExp.endsWith(QStringLiteral("=0")) ) {
        params << QString::number(1);
    }
    else if ( chkExp.endsWith(QStringLiteral("=0")) ) {
        params << QString::number(2);
    }
    else {
        params << QString();
    }

    QString waitMsg = QStringLiteral("正在查询，请稍侯……");
    netSocket->netRequest(this, params, waitMsg);       // => onSocketRequestOk
}

void BsAbstractSheetWin::clickCancelOpenPage()
{
    mpLayBody->setContentsMargins(1, 0, 1, 8);
    mpPnlOpener->hide();
    mpToolBar->show();
    mpPnlHeader->show();
    mpGrid->show();
    mpStatusBar->show();
    if ( mpPnlPayOwe && mMainTable != QStringLiteral("syd") )
        mpPnlPayOwe->show();
    mpBtnSwitchLimBind->hide();
}

void BsAbstractSheetWin::doubleClickOpenSheet(QTableWidgetItem *item)
{
    if ( ! item ) return;

    QStringList params;
    params << QStringLiteral("BIZOPEN");
    params << QString::number(QDateTime::currentMSecsSinceEpoch() * 1000);
    params << mMainTable;
    params << mpFindGrid->item(item->row(), 0)->text();

    QString waitMsg = QStringLiteral("正在获取，请稍侯……");
    netSocket->netRequest(this, params, waitMsg);       // => onSocketRequestOk
}

void BsAbstractSheetWin::clickSwitchLimTrader()
{
    if ( ! loginTrader.isEmpty() ) {

        if ( mpConShop->mpEditor->isEnabled() ) {
            mpConShop->mpEditor->setDataValue(loginTrader);
            mpConShop->mpEditor->setEnabled(false);
            mpConTrader->mpEditor->setDataValue(QString());
            mpConTrader->mpEditor->setEnabled(true);
        } else {
            mpConTrader->mpEditor->setDataValue(loginTrader);
            mpConTrader->mpEditor->setEnabled(false);
            mpConShop->mpEditor->setDataValue(QString());
            mpConShop->mpEditor->setEnabled(true);
        }
    }
}

void BsAbstractSheetWin::relocateSwitchLimButton()
{
    if (!mpPnlOpener->isHidden() &&
            mMainTable == QStringLiteral("dbd") &&
            !loginTrader.isEmpty()) {
        QPoint pt1 = mpConShop->mapTo(this, QPoint(mpConShop->width(), mpConShop->height() / 2 ));
        QPoint pt2 = mpConTrader->mapTo(this, QPoint(mpConTrader->width(), mpConTrader->height() / 2));
        int block = pt2.y() - pt1.y();
        int x = pt1.x() - block - 6;
        int y = pt1.y();
        mpBtnSwitchLimBind->setGeometry(x, y, block, block);
        mpBtnSwitchLimBind->show();
    } else {
        mpBtnSwitchLimBind->hide();
    }
}


// BsSheetCargoWin
BsSheetCargoWin::BsSheetCargoWin(QWidget *parent, const QString &name, const QStringList &fields)
    : BsAbstractSheetWin(parent, name, fields)
{
    //禁权字段
    if ( (!canRett && name == QStringLiteral("lsd")) ||
         (!canLott && name.startsWith(QStringLiteral("pf"))) ||
         (!canBuyy && name.startsWith(QStringLiteral("cg"))) ||
         (!canRett && !canLott && !canBuyy) )
    {
        mAllowPriceMoney = false;
        mDenyFields << "summoney" << "sumdis" << "actpay" << "actowe" << "price" << "discount" << "actmoney" << "dismoney";
    }
    else
        mAllowPriceMoney = true;

    //表格（隐藏列必须放最后，否则表格列数有变时，首列前会出现不可控宽度的无效列的BUG）
    int hpMarkNum = mapOption.value("sheet_hpmark_define").toInt();
    QStringList cols;
    cols << QStringLiteral("cargo");
    if ( hpMarkNum > 0 && hpMarkNum < 7 ) {
        cols << QStringLiteral("hpmark");
    }

    if ( mapOption.value("show_hpname_in_sheet_grid") == QStringLiteral("是") ) {
        cols << QStringLiteral("hpname");
    }

    if ( mapOption.value("show_hpunit_in_sheet_grid") == QStringLiteral("是") ) {
        cols << QStringLiteral("unit");
    }

    if ( mapOption.value("show_hpprice_in_sheet_grid") == QStringLiteral("是") ) {
        cols << QStringLiteral("setprice");
    }

    cols << QStringLiteral("color")
         << QStringLiteral("qty") << QStringLiteral("price") << QStringLiteral("actmoney")
         << QStringLiteral("discount") << QStringLiteral("dismoney")
         << QStringLiteral("rowmark") << QStringLiteral("rowtime") << QStringLiteral("sizers");

    for ( int i = 0, iLen = cols.length(); i < iLen; ++i ) {
        QString col = cols.at(i);
        QStringList defs = mapMsg.value(QStringLiteral("fld_%1").arg(col)).split(QChar(9));
        Q_ASSERT(defs.count() > 4);
        BsField* bsCol = new BsField(col,
                                     defs.at(0),
                                     QString(defs.at(3)).toUInt(),
                                     QString(defs.at(4)).toInt(),
                                     defs.at(2));
        resetFieldDotsDefin(bsCol);
        mGridFlds << bsCol;
    }

    mpSheetCargoGrid = new BsSheetCargoGrid(this, name, mGridFlds);
    mpGrid = mpSheetCargoGrid;
    mpFormGrid = mpSheetCargoGrid;
    mpSheetGrid = mpSheetCargoGrid;

    mpSheetCargoGrid->mDenyFields << mDenyFields;
    mpLayBody->insertWidget(2, mpSheetCargoGrid, 1);

    connect(mpSheetCargoGrid, SIGNAL(shootHintMessage(QString)),  this, SLOT(displayGuideTip(QString)));
    connect(mpSheetCargoGrid, SIGNAL(shootForceMessage(QString)), this, SLOT(forceShowMessage(QString)));
    connect(mpSheetCargoGrid, SIGNAL(filterDone()), mpToolBar, SLOT(hide()));
    connect(mpSheetCargoGrid, SIGNAL(filterEmpty()), mpToolBar, SLOT(show()));
    connect(mpSheetCargoGrid, SIGNAL(focusOuted()), mpStatusBar, SLOT(clearMessage()));
    connect(mpSheetCargoGrid, SIGNAL(shootCurrentRowSysValue(QStringList)), this, SLOT(showCargoInfo(QStringList)));
    connect(mpSheetCargoGrid, SIGNAL(sheetSumMoneyChanged(QString)), this, SLOT(sumMoneyChanged(QString)));
    connect(mpToolHideCurrentCol, SIGNAL(triggered(bool)), mpSheetCargoGrid, SLOT(hideCurrentCol()));
    connect(mpToolShowAllCols, SIGNAL(triggered(bool)), mpSheetCargoGrid, SLOT(showHiddenCols()));
    connect(mpTrader, &BsFldBox::editingChanged, this, &BsSheetCargoWin::traderPicked);

    //表格底部收付款
    if ( mpPnlPayOwe ) {
        if ( name == QStringLiteral("cgd") || name == QStringLiteral("pfd") || name == QStringLiteral("syd") )
            mpPnlPayOwe->hide();
        else
            connect(mpActPay, SIGNAL(editingFinished()), this, SLOT(actPayChanged()));
    }

    //工具箱
    mpAcToolDefineName = mpMenuToolCase->addAction(QIcon(),
                                                   mapMsg.value("tool_define_name"),
                                                   this, SLOT(doToolDefineFieldName()));
    mpAcToolDefineName->setProperty(BSACFLAGS, 0);
    mpAcToolDefineName->setProperty(BSACRIGHT, false);
    mpMenuToolCase->insertAction(mpToolPrintSetting, mpAcToolDefineName);

    mpAcToolImportBatchBarcodes = mpMenuToolCase->addAction(QIcon(), mapMsg.value("tool_import_batch_barcodes"),
                                                            this, SLOT(doToolImportBatchBarcodes()));
    mpAcToolImportBatchBarcodes->setProperty(BSACFLAGS, bsacfDirty);
    mpAcToolImportBatchBarcodes->setProperty(BSACRIGHT, canDo(mRightWinName, bsrsNew) || canDo(mRightWinName, bsrsUpd));

    mpAcToolImport->setText(mapMsg.value("tool_copy_import_sheet"));

    //开关盒
    mpAcOptHideNoQtySizerColWhenOpen->setVisible(true);
    mpAcOptHideNoQtySizerColWhenPrint->setVisible(true);

    mpAcOptPrintZeroSizeQty = mpMenuOptionBox->addAction(QIcon(), mapMsg.value("opt_print_zero_size_qty"),
                                                         this, SLOT(clickOptHideDropRow()));
    mpAcOptPrintZeroSizeQty->setProperty("optname", "opt_print_zero_size_qty");
    mpAcOptPrintZeroSizeQty->setProperty(BSACFLAGS, 0);
    mpAcOptPrintZeroSizeQty->setProperty(BSACRIGHT, canDo(mRightWinName, bsrsPrint));

    mpAcOptAutoUseFirstColor = mpMenuOptionBox->addAction(QIcon(), mapMsg.value("opt_auto_use_first_color"));
    mpAcOptAutoUseFirstColor->setProperty("optname", "opt_auto_use_first_color");
    mpAcOptAutoUseFirstColor->setProperty(BSACFLAGS, bsacfDirty);
    mpAcOptAutoUseFirstColor->setProperty(BSACRIGHT, canDo(mRightWinName, bsrsNew) || canDo(mRightWinName, bsrsUpd));

    //窗口
    loadFldsUserNameSetting();

    if ( name == QStringLiteral("syd") ) {
        mpConTrader->hide();
        mpTrader->hide();
        mpPnlPayOwe->hide();
    }


    //扫描助手
    mpPnlScan = new QLabel;
    mpPnlScan->setText(mapMsg.value("i_scan_assistant"));
    mpPnlScan->setAlignment(Qt::AlignCenter);
    mpPnlScan->setStyleSheet("color:#999;");

    //拣货助手
    mpDsAttr1 = new BsListModel(this, QStringLiteral("select distinct attr1 from cargo order by attr1;"), false);
    mpDsAttr2 = new BsListModel(this, QStringLiteral("select distinct attr2 from cargo order by attr2;"), false);
    mpDsAttr3 = new BsListModel(this, QStringLiteral("select distinct attr3 from cargo order by attr3;"), false);
    mpDsAttr4 = new BsListModel(this, QStringLiteral("select distinct attr4 from cargo order by attr4;"), false);
    mpDsAttr5 = new BsListModel(this, QStringLiteral("select distinct attr5 from cargo order by attr5;"), false);
    mpDsAttr6 = new BsListModel(this, QStringLiteral("select distinct attr6 from cargo order by attr6;"), false);

    mpDsAttr1->reload();
    mpDsAttr2->reload();
    mpDsAttr3->reload();
    mpDsAttr4->reload();
    mpDsAttr5->reload();
    mpDsAttr6->reload();

    QStringList sizerTypeDefs = mapMsg.value(QStringLiteral("fld_sizertype")).split(QChar(9));
    Q_ASSERT(sizerTypeDefs.count() > 4);
    mPickSizerFld = new BsField("sizertype", sizerTypeDefs.at(0), QString(sizerTypeDefs.at(3)).toUInt(),
                                QString(sizerTypeDefs.at(4)).toInt(), sizerTypeDefs.at(2));

    QStringList attr1Defs = mapMsg.value(QStringLiteral("fld_attr1")).split(QChar(9));
    Q_ASSERT(attr1Defs.count() > 4);
    mPickAttr1Fld = new BsField("attr1", attr1Defs.at(0), QString(attr1Defs.at(3)).toUInt(),
                                QString(attr1Defs.at(4)).toInt(), attr1Defs.at(2));
    mPickAttr1Fld->mFldCnName = mapOption.value(QStringLiteral("cargo_attr1_name"));

    QStringList attr2Defs = mapMsg.value(QStringLiteral("fld_attr2")).split(QChar(9));
    Q_ASSERT(attr2Defs.count() > 4);
    mPickAttr2Fld = new BsField("attr2", attr2Defs.at(0), QString(attr2Defs.at(3)).toUInt(),
                                QString(attr2Defs.at(4)).toInt(), attr2Defs.at(2));
    mPickAttr2Fld->mFldCnName = mapOption.value(QStringLiteral("cargo_attr2_name"));

    QStringList attr3Defs = mapMsg.value(QStringLiteral("fld_attr3")).split(QChar(9));
    Q_ASSERT(attr3Defs.count() > 4);
    mPickAttr3Fld = new BsField("attr3", attr3Defs.at(0), QString(attr3Defs.at(3)).toUInt(),
                                QString(attr3Defs.at(4)).toInt(), attr3Defs.at(2));
    mPickAttr3Fld->mFldCnName = mapOption.value(QStringLiteral("cargo_attr3_name"));

    QStringList attr4Defs = mapMsg.value(QStringLiteral("fld_attr4")).split(QChar(9));
    Q_ASSERT(attr4Defs.count() > 4);
    mPickAttr4Fld = new BsField("attr4", attr4Defs.at(0), QString(attr4Defs.at(3)).toUInt(),
                                QString(attr4Defs.at(4)).toInt(), attr4Defs.at(2));
    mPickAttr4Fld->mFldCnName = mapOption.value(QStringLiteral("cargo_attr4_name"));

    QStringList attr5Defs = mapMsg.value(QStringLiteral("fld_attr5")).split(QChar(9));
    Q_ASSERT(attr5Defs.count() > 4);
    mPickAttr5Fld = new BsField("attr5", attr5Defs.at(0), QString(attr5Defs.at(3)).toUInt(),
                                QString(attr5Defs.at(4)).toInt(), attr5Defs.at(2));
    mPickAttr5Fld->mFldCnName = mapOption.value(QStringLiteral("cargo_attr5_name"));

    QStringList attr6Defs = mapMsg.value(QStringLiteral("fld_attr6")).split(QChar(9));
    Q_ASSERT(attr6Defs.count() > 4);
    mPickAttr6Fld = new BsField("attr6", attr6Defs.at(0), QString(attr6Defs.at(3)).toUInt(),
                                QString(attr6Defs.at(4)).toInt(), attr6Defs.at(2));
    mPickAttr6Fld->mFldCnName = mapOption.value(QStringLiteral("cargo_attr6_name"));

    mpPnlPick = new QWidget;
    QHBoxLayout *layPick = new QHBoxLayout(mpPnlPick);
    layPick->setContentsMargins(1, 0, 1, 0);
    layPick->setSpacing(0);

    mpPickGrid = new BsSheetStockPickGrid(this);

    QStringList sdefs = mapMsg.value(QStringLiteral("fld_sizertype")).split(QChar(9));
    Q_ASSERT(sdefs.count() > 4);
    mPickSizerFld = new BsField("sizertype", sdefs.at(0), QString(sdefs.at(3)).toUInt(),
                                QString(sdefs.at(4)).toInt(), sdefs.at(2));

    mpPickSizerType = new BsFldEditor(this, mPickSizerFld, dsSizerType);
    mpPickSizerType->setMyPlaceText(mPickSizerFld->mFldCnName);
    mpPickSizerType->setMyPlaceColor(QColor(255, 0, 0));

    mpPickAttr1 = new BsFldEditor(this, mPickAttr1Fld, mpDsAttr1);
    mpPickAttr1->setMyPlaceText(mPickAttr1Fld->mFldCnName);
    mpPickAttr1->setMyPlaceColor(QColor(0, 150, 0));

    mpPickAttr2 = new BsFldEditor(this, mPickAttr2Fld, mpDsAttr2);
    mpPickAttr2->setMyPlaceText(mPickAttr2Fld->mFldCnName);
    mpPickAttr2->setMyPlaceColor(QColor(0, 150, 0));

    mpPickAttr3 = new BsFldEditor(this, mPickAttr3Fld, mpDsAttr3);
    mpPickAttr3->setMyPlaceText(mPickAttr3Fld->mFldCnName);
    mpPickAttr3->setMyPlaceColor(QColor(0, 150, 0));

    mpPickAttr4 = new BsFldEditor(this, mPickAttr4Fld, mpDsAttr4);
    mpPickAttr4->setMyPlaceText(mPickAttr4Fld->mFldCnName);
    mpPickAttr4->setMyPlaceColor(QColor(0, 150, 0));

    mpPickAttr5 = new BsFldEditor(this, mPickAttr5Fld, mpDsAttr5);
    mpPickAttr5->setMyPlaceText(mPickAttr5Fld->mFldCnName);
    mpPickAttr5->setMyPlaceColor(QColor(0, 150, 0));

    mpPickAttr6 = new BsFldEditor(this, mPickAttr6Fld, mpDsAttr6);
    mpPickAttr6->setMyPlaceText(mPickAttr6Fld->mFldCnName);
    mpPickAttr6->setMyPlaceColor(QColor(0, 150, 0));

    mpPickDate = new QCheckBox(QStringLiteral("不限日期"), this);

    mpPickCheck = new QCheckBox(QStringLiteral("不限审核"), this);

    mpPickTrader = new QCheckBox(mapMsg.value("word_pick_trader_stock"), this);
    mpPickTrader->setVisible(mMainTable == QStringLiteral("dbd"));

    QPushButton *btnPickQuery = new QPushButton(QIcon(":/icon/query.png"), mapMsg.value("word_query"), this);
    btnPickQuery->setFixedWidth(80);

    mpPickCons = new QWidget(this);
    QVBoxLayout *layFilterPick = new QVBoxLayout(mpPickCons);
    layFilterPick->setContentsMargins(9, 0, 9, 0);
    layFilterPick->setSpacing(2);
    layFilterPick->addWidget(mpPickSizerType);
    layFilterPick->addWidget(mpPickAttr1);
    layFilterPick->addWidget(mpPickAttr2);
    layFilterPick->addWidget(mpPickAttr3);
    layFilterPick->addWidget(mpPickAttr4);
    layFilterPick->addWidget(mpPickAttr5);
    layFilterPick->addWidget(mpPickAttr6);
    layFilterPick->addWidget(mpPickDate);
    layFilterPick->addWidget(mpPickCheck);
    layFilterPick->addWidget(mpPickTrader);
    layFilterPick->addWidget(btnPickQuery);
    layFilterPick->addStretch();

    layPick->addWidget(mpPickGrid, 1);
    layPick->addWidget(mpPickCons);

    //助手布局
    mpTaber->addTab(mpPnlScan, QIcon(":/icon/scan.png"), mapMsg.value("word_scan_assitant"));
    mpTaber->addTab(mpPnlPick, QIcon(":/icon/shop.png"), mapMsg.value("word_pick_assitant"));

    connect(mpTaber, SIGNAL(currentChanged(int)), this, SLOT(taberIndexChanged(int)));
    connect(mpPickTrader, SIGNAL(clicked(bool)), this, SLOT(pickStockTraderChecked()));
    connect(btnPickQuery, SIGNAL(clicked(bool)), this, SLOT(loadPickStock()));
    connect(mpPickGrid, SIGNAL(pickedCell(QString,QString,QString)),
            mpSheetCargoGrid, SLOT(addOneCargo(QString,QString,QString)));
    connect(mpPickGrid, &BsSheetStockPickGrid::cargoRowSelected, mpSheetCargoGrid, &BsSheetCargoGrid::tryLocateCargoRow);
    connect(mpSheetCargoGrid, &BsSheetCargoGrid::cargoRowSelected, mpPickGrid, &BsSheetStockPickGrid::tryLocateCargoRow);

    connect(netSocket, &BsSocket::requestOk, this, &BsSheetCargoWin::onSocketRequestOk);

    //初始
    openSheet(0);
}

BsSheetCargoWin::~BsSheetCargoWin()
{
    delete mPickSizerFld;
    delete mPickAttr1Fld;
    delete mPickAttr2Fld;
    delete mPickAttr3Fld;
    delete mPickAttr4Fld;
    delete mPickAttr5Fld;
    delete mPickAttr6Fld;

    delete mpDsAttr1;
    delete mpDsAttr2;
    delete mpDsAttr3;
    delete mpDsAttr4;
    delete mpDsAttr5;
    delete mpDsAttr6;

    qDeleteAll(mGridFlds);
    mGridFlds.clear();
}

void BsSheetCargoWin::showEvent(QShowEvent *e)
{
    BsAbstractSheetWin::showEvent(e);

    mpSheetCargoGrid->setFontPoint(9);
    mpSheetCargoGrid->setRowHeight(22);
}

void BsSheetCargoWin::doUpdateQuery()
{
    int sheetIdCol = mpFindGrid->getColumnIndexByFieldName("sheetid");
    if ( sheetIdCol < 0 )
        return;

    int row = -1;
    for ( int i = 0, iLen = mpFindGrid->rowCount(); i < iLen; ++i ) {
        if ( mpFindGrid->item(i, sheetIdCol)->text().toInt() == mCurrentSheetId ) {
            row = i;
            break;
        }
    }
    if ( row < 0 )
        return;

    int datedCol = mpFindGrid->getColumnIndexByFieldName("dated");
    if ( datedCol > 0 )
        mpFindGrid->item(row, datedCol)->setText(getPrintValue("dated"));

    int stypeCol = mpFindGrid->getColumnIndexByFieldName("stype");
    if ( stypeCol > 0 )
        mpFindGrid->item(row, stypeCol)->setText(getPrintValue("stype"));

    int staffCol = mpFindGrid->getColumnIndexByFieldName("staff");
    if ( staffCol > 0 )
        mpFindGrid->item(row, staffCol)->setText(getPrintValue("staff"));

    int shopCol = mpFindGrid->getColumnIndexByFieldName("shop");
    if ( shopCol > 0 )
        mpFindGrid->item(row, shopCol)->setText(getPrintValue("shop"));

    int traderCol = mpFindGrid->getColumnIndexByFieldName("trader");
    if ( traderCol > 0 )
        mpFindGrid->item(row, traderCol)->setText(getPrintValue("trader"));

    int qtyCol = mpFindGrid->getColumnIndexByFieldName("sumqty");
    if ( qtyCol > 0 )
        mpFindGrid->item(row, qtyCol)->setText(getPrintValue("sumqty"));

    int mnyCol = mpFindGrid->getColumnIndexByFieldName("summoney");
    if ( mnyCol > 0 )
        mpFindGrid->item(row, mnyCol)->setText(getPrintValue("summoney"));

    int payCol = mpFindGrid->getColumnIndexByFieldName("actpay");
    if ( payCol > 0 )
        mpFindGrid->item(row, payCol)->setText(getPrintValue("actpay"));

    int oweCol = mpFindGrid->getColumnIndexByFieldName("actowe");
    if ( oweCol > 0 )
        mpFindGrid->item(row, oweCol)->setText(getPrintValue("actowe"));
}

void BsSheetCargoWin::updateTabber(const bool editablee)
{
    mpPickGrid->hide();
    mpPickCons->hide();
    mpTaber->setCurrentIndex(0);
    mpTaber->setVisible(editablee);
    if ( editablee )
        restoreTaberMiniHeight();
}

void BsSheetCargoWin::doToolExport()
{
    QStringList headPairs;
    headPairs << QStringLiteral("%1:,%2").arg(getFieldByName("sheetid")->mFldCnName).arg(mpSheetId->getDisplayText())
              << QStringLiteral("%1:,%2").arg(getFieldByName("dated")->mFldCnName).arg(mpDated->mpEditor->text())
              << QStringLiteral("%1:,%2").arg(getFieldByName("stype")->mFldCnName).arg(mpStype->mpEditor->text())
              << QStringLiteral("%1:,%2").arg(getFieldByName("staff")->mFldCnName).arg(mpStaff->mpEditor->text())
              << QStringLiteral("%1:,%2").arg(getFieldByName("shop")->mFldCnName).arg(mpShop->mpEditor->text())
              << QStringLiteral("%1:,%2").arg(getFieldByName("trader")->mFldCnName).arg(mpTrader->mpEditor->text())
              << QStringLiteral("%1:,%2").arg(getFieldByName("remark")->mFldCnName).arg(mpRemark->mpEditor->text())
              << QStringLiteral("%1:,%2").arg(getFieldByName("actpay")->mFldCnName).arg(mpActPay->mpEditor->text())
              << QStringLiteral("%1:,%2").arg(getFieldByName("actowe")->mFldCnName).arg(mpActOwe->mpEditor->text());
    exportGrid(mpGrid, headPairs);
}

void BsSheetCargoWin::doToolImport()
{
    BsCopyImportSheetDlg dlg(this);
    if ( dlg.exec() != QDialog::Accepted )
        return;

    QString tbl = dlg.mpCmbSheetName->currentData().toString();
    int sheetId = dlg.mpEdtSheetId->text().toInt();

    QString sql = QStringLiteral("select cargo, color, sizers from %1dtl where parentid=%2;").arg(tbl).arg(sheetId);
    QSqlQuery qry;
    qry.setForwardOnly(true);
    qry.exec(sql);
    while ( qry.next() ) {
        QString cargo = qry.value(0).toString();
        QString color = qry.value(1).toString();
        QStringList sizers = qry.value(2).toString().split(QChar(10));
        for ( int i = 0, iLen = sizers.length(); i < iLen; ++i ) {
            QStringList sizerPair = QString(sizers.at(i)).split(QChar(9));
            if ( sizerPair.length() == 2 ) {
                QString sizerName = sizerPair.at(0);
                qint64 sizerQty = QString(sizerPair.at(1)).toLongLong();
                mpSheetCargoGrid->inputNewCargoRow(cargo, color, sizerName, sizerQty, false);
            }
        }
    }
    qry.finish();
}

void BsSheetCargoWin::traderPicked(const QString &text)
{
    mpSheetCargoGrid->setTraderDiscount(getTraderDisByName(text));
    mpSheetCargoGrid->setTraderName(text);
}

void BsSheetCargoWin::showCargoInfo(const QStringList &values)
{
    Q_ASSERT(values.length()==2);
    QString cargoInfo = dsCargo->getCargoBasicInfo(values.at(0));
    if ( !cargoInfo.isEmpty() )
        mpStatusBar->showMessage(cargoInfo);
}

void BsSheetCargoWin::sumMoneyChanged(const QString &sumValue)
{
    double actPay = mpActPay->mpEditor->text().toDouble();
    double actSum = sumValue.toDouble();
    mpActOwe->mpEditor->setText(QString::number(actSum - actPay, 'f', mMoneyDots));
}

void BsSheetCargoWin::actPayChanged()
{
    double actPay = mpActPay->mpEditor->text().toDouble();
    double actSum = mpGrid->getFooterValueByField(QStringLiteral("actmoney")).toDouble();
    mpActOwe->mpEditor->setText(QString::number(actSum - actPay, 'f', mMoneyDots));
}

void BsSheetCargoWin::taberIndexChanged(int index)
{
    if ( mpShop->mpEditor->getDataValue().isEmpty() && index == 1 ) {
        QMessageBox::information(this, QString(), mapMsg.value("word_pick_assitant") + mapMsg.value("i_please_pick_shop_first"));
        mpTaber->setCurrentIndex(0);
        return;
    }
    mpPickGrid->setVisible(index > 0);
    mpPickCons->setVisible(index > 0);
    if ( index == 0 ) {
        restoreTaberMiniHeight();
    }
    if ( index == 1 ) {
        mpSpl->setSizes(QList<int>() << height() / 2 << height() / 3);
    }
}

void BsSheetCargoWin::doToolDefineFieldName()
{
    int hpMarkNum = mapOption.value("sheet_hpmark_define").toInt();

    QList<BsField*> flds;
    flds << getFieldByName("stype")
         << getFieldByName("staff")
         << getFieldByName("shop");

    if ( mMainTable != QStringLiteral("syd") )
        flds << getFieldByName("trader")
             << getFieldByName("actpay")
             << getFieldByName("actowe");

    flds << mpSheetCargoGrid->getFieldByName("cargo")
         << mpSheetCargoGrid->getFieldByName("color");

    if ( mMainTable != QStringLiteral("syd") )
        flds << mpSheetCargoGrid->getFieldByName("price")
             << mpSheetCargoGrid->getFieldByName("actmoney")
             << mpSheetCargoGrid->getFieldByName("dismoney");

    if ( hpMarkNum > 0 && hpMarkNum < 7 )
        flds << mpSheetCargoGrid->getFieldByName("hpmark");

    flds << mpSheetCargoGrid->getFieldByName("rowmark");

    BsFieldDefineDlg dlg(this, mMainTable, flds);
    if ( dlg.exec() == QDialog::Accepted ) {

        QStringList sqls;
        sqls << QStringLiteral("delete from bailifldname where tblname='%1';").arg(mMainTable);

        QList<QLineEdit *> edts = dlg.findChildren<QLineEdit *>();
        for ( int i = 0, iLen = edts.length(); i < iLen; ++i ) {
            QLineEdit *edt = edts.at(i);
            QString fld = edt->property("field_name").toString();
            QString oldName = edt->property("field_value").toString();
            QString newName = edt->text().trimmed();
            if ( oldName != newName && !newName.isEmpty() ) {

                BsField *bsFld = getFieldByName(fld);
                if ( bsFld )
                    bsFld->mFldCnName = newName;

                bsFld = mpSheetCargoGrid->getFieldByName(fld);
                if ( bsFld )
                    bsFld->mFldCnName = newName;

                sqls << QStringLiteral("insert or ignore into bailifldname(tblname, fldname, cname) values('%1', '%2', '%3');")
                        .arg(mMainTable).arg(fld).arg(newName);
            }
        }

        QString sqlErr = sqliteCommit(sqls);

        if ( sqlErr.isEmpty() )
            loadFldsUserNameSetting();
        else
            QMessageBox::information(this, QString(), sqlErr);
    }
}

void BsSheetCargoWin::doToolImportBatchBarcodes()
{
    QString dir = QStandardPaths::locate(QStandardPaths::DesktopLocation, QString(), QStandardPaths::LocateDirectory);
    QString openData = openLoadTextFile(mapMsg.value("tool_import_batch_barcodes"), dir,
                                        mapMsg.value("i_common_text_file"), this);
    if ( openData.isEmpty() )
        return;

    //原数据分行
    QStringList lines = openData.split(QChar(10));

    //待求条码列序，和数量列序
    int idxBar = 0;
    int idxQty = -1;

    //推测分隔符
    QChar colSplittor = (openData.indexOf(QChar(9)) > 0) ? QChar(9) : QChar(44);

    //推测是否单列，不是单列，对话框让用户给出
    int testRow = int(floor(lines.length() / 2));
    QStringList testCols = QString(lines.at(testRow)).split(colSplittor);
    if ( testCols.length() > 1 )
    {
        BsBatchBarcodesDlg dlg(this, openData);
        if ( QDialog::Accepted != dlg.exec() )
            return;

        idxBar = dlg.mpBarCol->text().toInt() - 1;  //人数1基
        idxQty = dlg.mpQtyCol->text().toInt() - 1;  //人数1基
    }

    //就绪
    for ( int i = 0, iLen = lines.length(); i < iLen; ++i )
    {
        QStringList cols = QString(lines.at(i)).split(colSplittor);
        if ( cols.length() > idxBar && cols.length() > idxQty )
        {
            //取出
            QString barcode = cols.at(idxBar);
            int qty = ( idxQty >= 0 ) ? QString(cols.at(idxQty)).toInt() : 1;

            //扫描
            QString cargo, colorCode, sizerCode;
            if ( mpSheetCargoGrid->scanBarcode(barcode, &cargo, &colorCode, &sizerCode) )
            {
                QString err = mpSheetCargoGrid->inputNewCargoRow(cargo, colorCode, sizerCode, qty * 10000, true);
                if ( !err.isEmpty() ) {
                    QMessageBox::information(this, QString(), err);
                    return;
                }
            }
        }
    }
}

void BsSheetCargoWin::pickStockTraderChecked()
{
    QString trader = mpTrader->mpEditor->getDataValue();
    if ( QObject::sender() == mpPickTrader && trader.isEmpty() ) {
        QMessageBox::information(this, QString(), mapMsg.value("i_please_pick_trader_first"));
        mpPickTrader->setChecked(false);
    }
}

void BsSheetCargoWin::loadPickStock()
{
    QString shop = mpShop->mpEditor->getDataValue();
    QString trader = mpTrader->mpEditor->getDataValue();
    QString stockShop = (mpPickTrader->isVisible() && mpPickTrader->isChecked()) ? trader : shop;
    QString sizerType = mpPickSizerType->getDataValue();
    QString attr1 = mpPickAttr1->getDataValue();
    QString attr2 = mpPickAttr2->getDataValue();
    QString attr3 = mpPickAttr3->getDataValue();
    QString attr4 = mpPickAttr4->getDataValue();
    QString attr5 = mpPickAttr5->getDataValue();
    QString attr6 = mpPickAttr6->getDataValue();
    QString datee = (mpPickDate->isChecked()) ? QString() : mpDated->mpEditor->text();
    int checkk = (mpPickCheck->isChecked()) ? 999 : 0;
    if ( sizerType.isEmpty() ) {
        QMessageBox::information(this, QString(), mapMsg.value("i_please_pick_sizertype_first"));
        return;
    }

    QStringList attrCons;
    if ( !attr1.isEmpty() ) attrCons << QStringLiteral("1\t%1").arg(attr1);
    if ( !attr2.isEmpty() ) attrCons << QStringLiteral("2\t%1").arg(attr2);
    if ( !attr3.isEmpty() ) attrCons << QStringLiteral("3\t%1").arg(attr3);
    if ( !attr4.isEmpty() ) attrCons << QStringLiteral("4\t%1").arg(attr4);
    if ( !attr5.isEmpty() ) attrCons << QStringLiteral("5\t%1").arg(attr5);
    if ( !attr6.isEmpty() ) attrCons << QStringLiteral("6\t%1").arg(attr6);

    if ( mpPickGrid->rowCount() > 0 ) mpPickGrid->saveColWidths("pick");

    int pickDelta = -1;
    if ( mMainTable == QStringLiteral("cgj") || mMainTable == QStringLiteral("pft") ||
         ( mMainTable == QStringLiteral("dbd") && mpPickTrader->isChecked() ) ) {
        pickDelta = 1;
    }
    mpPickGrid->setPickDelta(pickDelta);

    mpPickGrid->cancelAllFilter();

    QString waitMsg = QStringLiteral("正在查询库存，请稍侯……");

    QStringList params;
    params << QStringLiteral("QRYPICK")
           << QString::number(QDateTime::currentMSecsSinceEpoch() * 1000)
           << stockShop
           << attrCons.join(QChar('\n'))
           << sizerType
           << datee
           << QString::number(checkk);
    netSocket->netRequest(this, params, waitMsg);       // => onSocketRequestOk
}

void BsSheetCargoWin::onSocketRequestOk(const QWidget* sender, const QStringList &fens)
{
    if ( sender != this )
        return;

    QString cmd = fens.at(0);

    if ( cmd == QStringLiteral("QRYSHEET") ) {
        continueQrySheet();
    }

    if ( cmd == QStringLiteral("QRYPICK") ) {
        continueQryPick();
    }

    if ( cmd == QStringLiteral("BIZOPEN") ) {
        int retSheetId = QString(fens.at(3)).toInt();
        openSheet(retSheetId);
    }

    if ( cmd == QStringLiteral("BIZINSERT") || cmd == QStringLiteral("BIZEDIT") ) {
        int retSheetId = QString(fens.at(2)).toInt();
        qint64 retUptime = QString(fens.at(3)).toLongLong();
        continueSheetSave(retSheetId, retUptime);
    }

    if ( cmd == QStringLiteral("BIZDELETE") ) {
        int retSheetId = QString(fens.at(2)).toInt();
        qint64 retUptime = QString(fens.at(3)).toLongLong();
        continueSheetDelete(retSheetId, retUptime);
    }

    if ( cmd == QStringLiteral("QRYPRINTOWE") ) {
        doPrintContinue();
    }
}

void BsSheetCargoWin::restoreTaberMiniHeight()
{
    int bodyHt = mpBody->height() - mpTaber->tabBar()->height() - mpPnlScan->sizeHint().height();
    QList<int> szs;
    szs << bodyHt << mpTaber->tabBar()->height();
    mpSpl->setSizes(szs);
}

void BsSheetCargoWin::loadFldsUserNameSetting()
{
    for ( int i = 0, iLen = mpPnlHeader->children().count(); i < iLen; ++i ) {
        BsFldBox *edt = qobject_cast<BsFldBox*>(mpPnlHeader->children().at(i));
        if ( edt )
            edt->mpLabel->setText(edt->mpEditor->mpField->mFldCnName);
    }
    if ( mpPnlPayOwe ) {
        for ( int i = 0, iLen = mpPnlPayOwe->children().count(); i < iLen; ++i ) {
            BsFldBox *edt = qobject_cast<BsFldBox*>(mpPnlPayOwe->children().at(i));
            if ( edt )
                edt->mpLabel->setText(edt->mpEditor->mpField->mFldCnName);
        }
    }
    mpSheetCargoGrid->updateColTitleSetting();
}

void BsSheetCargoWin::continueQrySheet()
{
    QStringList colTitles;
    colTitles << QStringLiteral("shop\t%1").arg(getFieldByName(QStringLiteral("shop"))->mFldCnName)
         << QStringLiteral("trader\t%1").arg(getFieldByName(QStringLiteral("trader"))->mFldCnName)
         << QStringLiteral("actpay\t%1").arg(getFieldByName(QStringLiteral("actpay"))->mFldCnName);

    //summoney, actpay, actowe值权限不用管，因为电脑端使用硬件狗控制发放。
    QString sql = QStringLiteral("SELECT sheetid, dated, stype, staff, shop, trader, "
                                 "sumqty, summoney, actpay, actowe, chktime "
                                 "FROM tmp_query_dataset ORDER BY sheetid;");

    //加载数据
    mpFindGrid->loadData(sql, colTitles);

    //加载列宽
    mpFindGrid->loadColWidths();
}

void BsSheetCargoWin::continueQryPick()
{
    QStringList sqls;
    sqls << QStringLiteral("alter table tmp_query_dataset add column hpname text default '';");
    sqls << QStringLiteral("alter table tmp_query_dataset add column unit text default '';");
    sqls << QStringLiteral("alter table tmp_query_dataset add column setprice int default 0;");
    sqls << QStringLiteral("update tmp_query_dataset set "
                           "hpname=(select hpname from cargo where hpcode=tmp_query_dataset.cargo), "
                           "unit=(select unit from cargo where hpcode=tmp_query_dataset.cargo),"
                           "setprice=(select setprice from cargo where hpcode=tmp_query_dataset.cargo) "
                           "where cargo=(select hpcode from cargo where hpcode=tmp_query_dataset.cargo);");

    QString strErr = sqliteCommit(sqls);
    if ( !strErr.isEmpty() ) {
        QMessageBox::information(this, QString(), strErr);
        return;
    }

    //第一列必须cargo，第二列必须hpname，最后列必须sizers，约定见BsGrid::loadData()
    QString sql = QStringLiteral("SELECT cargo, hpname, unit, setprice, color, "
                                 "qty, sizers "
                                 "FROM tmp_query_dataset "
                                 "WHERE qty<>0 "
                                 "ORDER BY cargo, color;");

    mpPickGrid->loadData(sql, QStringList(), mpPickSizerType->getDataValue(), true);
    mpPickGrid->loadColWidths("pick");

}

void BsSheetCargoWin::continueSheetSave(const int useSheetId, const qint64 uptime)
{
    mpSheetGrid->savedReconcile();
    mpSheetGrid->setEditable(false);
    savedReconcile(useSheetId, uptime);
    setEditable(false);
    doUpdateQuery();
}

void BsSheetCargoWin::continueSheetDelete(const int deletedSheetId, const qint64 deletedUptime)
{
    openSheet(0);
    mCurrentSheetId = deletedSheetId;
    mpSheetId->setDataValue(deletedSheetId);
    mpSttValUpman->setText(loginer);
    mpSttValUptime->setText(QDateTime::fromMSecsSinceEpoch(1000 * deletedUptime).toString("yyyy-MM-dd hh:mm:ss"));
    doUpdateQuery();
}

}
