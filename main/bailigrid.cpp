#include "bailigrid.h"
#include "bailicode.h"
#include "bailidata.h"
#include "bailiedit.h"
#include "bailiwins.h"
#include "bssocket.h"
#include "comm/expresscalc.h"
#include "comm/pinyincode.h"

#include <QPrinter>
#include <QPrinterInfo>
#include <QPrintDialog>

namespace BailiSoft {

void resetFieldDotsDefin(BsField *bsFld)
{
    QString fldName = bsFld->mFldName;
    if ( (bsFld->mFlags & bsffNumeric) == bsffNumeric )
    {
        if ( fldName.contains("qty") )
            bsFld->mLenDots = mapOption.value("dots_of_qty").toInt();

        else if ( fldName.contains("price") )
            bsFld->mLenDots = mapOption.value("dots_of_price").toInt();

        else if ( fldName == QStringLiteral("discount") )
            bsFld->mLenDots = mapOption.value("dots_of_discount").toInt();

        else if ( fldName.contains("money") || fldName.contains("act") || fldName.contains("sum") )
            bsFld->mLenDots = mapOption.value("dots_of_money").toInt();
    }
}

// BsFilterSelector
BsFilterSelector::BsFilterSelector(QWidget *parent) : QWidget(parent)
{
    mpList = new QListWidget(this);
    mpList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mpList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mpList->setStyleSheet(QStringLiteral("QListWidget{border:none;}"));
    mpList->setSelectionBehavior(QAbstractItemView::SelectRows);
    mpList->setSelectionMode(QAbstractItemView::SingleSelection);

    QPushButton *btnOk = new QPushButton(QIcon(":/icon/ok.png"),mapMsg.value("btn_ok"), this);
    QPushButton *btnCancel = new QPushButton(QIcon(":/icon/cancel.png"),mapMsg.value("btn_cancel"), this);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 8);
    lay->addWidget(mpList, 1);
    lay->addWidget(btnOk, 0, Qt::AlignCenter);
    lay->addWidget(btnCancel, 0, Qt::AlignCenter);

    setObjectName(QLatin1String("gridFilterPicker"));
    setStyleSheet(QLatin1String("QWidget#gridFilterPicker{background-color:#fff;}"));

    connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(okClicked()));
    connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(cancelClicked()));
}

void BsFilterSelector::setPicks(const QStringList &list, const QString picked)
{
    mpList->clear();
    mpList->addItems(list);

    for ( int i = 0, iLen = list.length(); i < iLen; ++i )
    {
        mpList->item(i)->setCheckState((list.at(i) == picked) ? Qt::Checked : Qt::Unchecked);
    }
}

void BsFilterSelector::okClicked()
{
    QStringList ls;
    for ( int i = 0, iLen = mpList->count(); i < iLen; ++i )
    {
        if ( mpList->item(i)->checkState() == Qt::Checked )
            ls << mpList->item(i)->text();
    }

    mpList->clear();
    this->hide();

    emit pickFinished(ls);
}

void BsFilterSelector::cancelClicked()
{
    mpList->clear();
    hide();
}


// BsHeader
BsHeader::BsHeader(BsGrid *parent) : QHeaderView(Qt::Horizontal, parent)
{
    mpGrid = parent;
    setSortIndicatorShown(true);
    setSectionsClickable(true);
    setStyleSheet("QHeaderView{border-style:none; border-bottom:1px solid silver;} ");
}


// BsFooter
BsFooter::BsFooter(QWidget *parent, BsGrid *grid) : QTableWidget(parent)
{
    mpGrid = grid;
    setRowCount(1);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    horizontalHeader()->hide();
    verticalHeader()->hide();
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalHeader()->setDefaultSectionSize(mpGrid->getRowHeight());
    setItemDelegate(new BsNoBorderDelegate(this));
    setStyleSheet(QStringLiteral("QTableWidget{border:none; font-weight:600;} QTableWidget::item{border:none; %1;}")
                  .arg(mapMsg.value("css_vertical_gradient")));
}

void BsFooter::initCols()
{
    clear();
    setColumnCount(mpGrid->mCols.count());
    for ( int i = 0; i < mpGrid->mCols.count(); ++i )
    {
        uint flags = mpGrid->mCols.at(i)->mFlags;
        QTableWidgetItem *it = new QTableWidgetItem();

        if ( (flags & bsffAggSum) == bsffAggSum )
            it->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        else
            it->setTextAlignment(Qt::AlignCenter);

        setItem(0, i, it);
        setColumnHidden(i, (flags & bsffHideSys));
    }

    if ( mpGrid->mCols.count() > 1 )
    {
        item(0, 0)->setText(mapMsg.value("i_footer_sum"));
    }
}

void BsFooter::hideAggText()
{
    for ( int i = 0; i < mpGrid->mCols.count(); ++i )
    {
        item(0, i)->setText(QString());
    }
}

void BsFooter::headerSectionResized(int logicalIndex, int oldSize, int newSize)
{
    if ( newSize != oldSize )
    {
        int setSize = newSize;
        if ( logicalIndex == 0 )
            setSize += mpGrid->verticalHeader()->width();
        setColumnWidth(logicalIndex, setSize);
    }
}

void BsFooter::focusInEvent(QFocusEvent *e)
{
    mProbableBarcode.clear();
    QTableWidget::focusInEvent(e);
}

void BsFooter::keyPressEvent(QKeyEvent *e)
{
    //下面大量使用
    const int k = e->key();

    if ( k >= 0x20 && k <= 0xff )     //可见Latin1字符
    {
        mProbableBarcode += QChar(k);
        return; //否则默认行为会自动查找包含该字符的行，造成scroll
    }
    else                            //控制字符
    {
        //判断为扫描（谁也不会赣犟到在不可录入的栏中连续多次输入）
        if ( mProbableBarcode.length() >= 5 && (k == Qt::Key_Enter || k == Qt::Key_Return) )
        {
            emit barcodeScanned(mProbableBarcode);
            mProbableBarcode.clear();
            return;
        }
    }

    mProbableBarcode.clear();

    QTableWidget::keyPressEvent(e);
}


/**********************************华丽分割线，下面为表格**************************************/

// BsGrid

BsGrid::BsGrid(QWidget *parent, const bool forQry, const bool forReg)
    : QTableWidget(parent), mForQuery(forQry), mForRegister(forReg)
{
    mppWin = qobject_cast<BsWin*>(parent);
//    Q_ASSERT(mppWin);     //因为核对小窗口中也用到BsGrid，所以不能如此断言。也因此mppWin只能用于获取开关盒选项，不能滥用。

    mRowHeight = 20;
    mFontPoint = 9;
    mFiltering = false;
    mSizerPrevCol = -1;
    mSizerColCount = 0;

    mDiscDots = mapOption.value("dots_of_discount").toInt();
    mPriceDots = mapOption.value("dots_of_price").toInt();
    mMoneyDots = mapOption.value("dots_of_money").toInt();

    mpHeader = new BsHeader(this);
    setHorizontalHeader(mpHeader);

    mpFooter = new BsFooter(horizontalScrollBar(), this);
    connect(mpHeader, SIGNAL(sectionResized(int,int,int)), mpFooter, SLOT(headerSectionResized(int,int,int)));
    connect(mpHeader, SIGNAL(sectionResized(int,int,int)), this, SLOT(updateFooterGeometry()));

    mpPicker = new BsFilterSelector(this);
    mpPicker->hide();

    mpCorner = new QToolButton(this);
    mpCorner->setToolButtonStyle(Qt::ToolButtonTextOnly);
    mpCorner->setStyleSheet("color:#666; border-style:none; ");

    mpMenu = new QMenu(this);
    mpFilterIn      = mpMenu->addAction(mapMsg.value("menu_filter_in"), this, SLOT(filterIn()));
    mpFilterOut     = mpMenu->addAction("O", this, SLOT(filterOut()));
    mpMenu->addSeparator();
    mpRestoreCol    = mpMenu->addAction(mapMsg.value("menu_filter_restore_col"), this, SLOT(filterRestoreCol()));
    mpRestoreAll    = mpMenu->addAction(mapMsg.value("menu_filter_restore_all"), this, SLOT(filterRestoreAll()));

    verticalHeader()->setDefaultSectionSize(mRowHeight);
    verticalHeader()->setStyleSheet("color:#999;");
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    horizontalScrollBar()->setStyleSheet(QLatin1String(".QScrollBar:horizontal {background:red; border-style:none;}"));

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    setSortingEnabled(true);

    connect(mpPicker, SIGNAL(pickFinished(QStringList)), this, SLOT(takeFilterInPicks(QStringList)));
    connect(mpCorner, SIGNAL(clicked(bool)), this, SLOT(sortByRowTime()));
    connect(this, &QTableWidget::itemDoubleClicked, this, &BsGrid::requestImage);
}

BsGrid::~BsGrid()
{
    delete mpFooter;

    if ( mForQuery )
    {
        qDeleteAll(mCols);
        mCols.clear();
    }
    else
    {
        for ( int i = 0, iLen = mCols.length(); i < iLen; ++i )
        {
            if ( mCols.at(i)->mFlags & bsffSizeUnit )
                delete mCols.at(i);
        }
    }
}

void BsGrid::loadData(const QString &sql, const QStringList &fldCnameDefines, const QString &useSizerType,
                      const bool joinCargoPinyin)
{
    //耗时等待光标
    qApp->setOverrideCursor(Qt::WaitCursor);

    //重置状态和原始值
    sortByColumn(-1, Qt::AscendingOrder);  //必须啊
    setCurrentCell(-1, -1);
    clear();
    setRowCount(0);
    mFiltering = false;
    mLoadSizerType = useSizerType;

    //数据库执行
    QSqlQuery qry;
    qry.setNumericalPrecisionPolicy(QSql::LowPrecisionInt64);
    qry.exec(sql);  //因为下面可能要遍历两边，所以不加setForwardOnly(true)
    if ( qry.lastError().isValid() ) qDebug() << qry.lastError().text() << "\n" << sql;

    //根据数据库字段设置列
    QSqlRecord sqlRec = qry.record();
    int sizerDataCol = sqlRec.indexOf(QStringLiteral("sizers"));
    int chkTimeCol = sqlRec.indexOf(QStringLiteral("chktime"));
    mSizerPrevCol = ( mForQuery ) ? sizerDataCol : sqlRec.indexOf(QStringLiteral("color"));
    mSizerColCount = 0;

    //查询重建mCols
    if ( mForQuery )
    {
        qDeleteAll(mCols);
        mCols.clear();
        for ( int i = 0, iLen = sqlRec.count(); i < iLen; ++i )
        {
            //new BsField()
            QString fld = sqlRec.fieldName(i);
            QStringList defs = mapMsg.value(QStringLiteral("fld_%1").arg(fld)).split(QChar(9), QString::SkipEmptyParts);
            Q_ASSERT(defs.count() > 4);
            BsField *bsCol = new BsField(fld,
                                         defs.at(0),
                                         QString(defs.at(3)).toUInt(),
                                         QString(defs.at(4)).toInt(),
                                         defs.at(2));

            //补充cname
            for ( int j = 0, jLen = fldCnameDefines.length(); j < jLen; ++j )
            {
                QStringList cndefs = QString(fldCnameDefines.at(j)).split(QChar(9), QString::SkipEmptyParts);
                if ( cndefs.at(0) == bsCol->mFldName )
                {
                    bsCol->mFldCnName = cndefs.at(1);
                    break;
                }
            }

            //加入mCols
            resetFieldDotsDefin(bsCol);
            mCols << bsCol;
        }
    }

    //加载用户字段名与小数位额外定义
    if ( mForQuery || !mForRegister ) {
        //单据直接用
        QString tblKey = mTable;

        //查询需要分析
        if ( mForQuery ) {
            QString str = sql.toLower();
            int iPos = str.indexOf(QStringLiteral(" from "));
            str = str.mid(iPos + 6);
            iPos = str.indexOf(QChar(32));
            str = str.left(iPos);
            if ( str.indexOf(QChar('_')) > 0 ) {
                QStringList nameSecs = str.split(QChar('_'));
                tblKey = nameSecs.at(1);
                if ( tblKey.contains(QStringLiteral("cg")) )
                    tblKey = QStringLiteral("cgj");
                if ( tblKey.contains(QStringLiteral("pf")) ||
                     tblKey.contains(QStringLiteral("xs")) ||
                     tblKey == QStringLiteral("stock")  )
                    tblKey = QStringLiteral("pff");
            }
            else
                tblKey = str.trimmed();  //单据窗口查找打开表格的sql
        }

        //至此，tblKey应该是单据主表3字符名，可以使用取得用户定义字段了
        for ( int i = 0, iLen = mCols.length(); i < iLen; ++i )
        {
            QString fldKey = mCols.at(i)->mFldName;
            QString defKey = QStringLiteral("%1_%2").arg(tblKey).arg(fldKey);
            if ( mapFldUserSetName.contains(defKey) )
                mCols.at(i)->mFldCnName = mapFldUserSetName.value(defKey);
        }
    }

    //尺码横排预备处理
    QStringList sheetFirstRowSizeType;  //代表显示初始尺码列头
    if ( sizerDataCol > 0 )
    {
        QStringList regList = ( useSizerType.isEmpty() ) ? QStringList() : dsSizerType->getSizerList(useSizerType);
        mSizerColCount = regList.length();
        Q_ASSERT(sqlRec.indexOf(QStringLiteral("qty")) < sqlRec.indexOf(QStringLiteral("sizers")));

        if ( !mForQuery )
        {
            //单据中可能存在不同品类尺码，maxRegCols与maxBadCols都要重新比较取得最大。
            while ( qry.next() )
            {
                QString cargo = qry.value(0).toString();
                QString sizerType = dsCargo->getValue(cargo, QStringLiteral("sizertype"));
                regList = dsSizerType->getSizerList(sizerType);
                if ( regList.length() > mSizerColCount )
                    mSizerColCount = regList.length();
                if ( sheetFirstRowSizeType.isEmpty() )
                    sheetFirstRowSizeType << regList;
            }
            qry.first();
            qry.previous();

            //单据重建列定义
            QList<BsField*> keepFlds;
            for ( int i = 0, iLen = mCols.length(); i < iLen; ++i )
            {
                if ( mCols.at(i)->mFlags & bsffSizeUnit )
                    delete mCols.at(i);
                else
                    keepFlds << mCols.at(i);
            }
            mCols.clear();
            mCols << keepFlds;

            //由于单据的sql用mCols从0到length()-1取得的，故而以下是一定的。但仍然断言防止日后乱动代码后，找原因难。
            Q_ASSERT(mSizerPrevCol < mCols.length());
            Q_ASSERT(mCols.at(mSizerPrevCol)->mFldName == QStringLiteral("color"));
        }

        //插入横排尺码列
        for ( int i = 1; i <= mSizerColCount; ++i )
        {
            //sz01开始的命名方式约定，不要变
            BsField *fld = new BsField(QStringLiteral("sz%1").arg(i), QStringLiteral("*"),
                                       bsffNumeric | bsffAggSum | bsffSizeUnit, 0, QString());
            mCols.insert(mSizerPrevCol + i, fld);
        }
    }

    //表格列数
    setColumnCount(mCols.count());

    //列隐藏
    for ( int i = mCols.length() - 1; i >= 0; --i )
        setColumnHidden(i, (mCols.at(i)->mFlags & bsffHideSys) || i == chkTimeCol );

    //填数据行
    int rows = 0;
    while ( qry.next() )
    {
        //增行
        setRowCount(++rows);

        //是否已审核行（仅用于单据窗口打开查找表格）
        bool rowChecked = ( chkTimeCol > 0 && qry.value(chkTimeCol).toBool() );

        //可能有的sizers字符串
        QString sizers;

        //逐列填值
        for ( int i = 0, iLen = sqlRec.count(); i < iLen; ++i )
        {
            //对应表格列
            int idxCol = ( i <= mSizerPrevCol ) ? i : (i + mSizerColCount);

            //准备单元对象，及字段特征flags
            BsGridItem *it = nullptr;
            uint flags = mCols.at(idxCol)->mFlags;

            //文本字段
            if ( (flags & bsffText) == bsffText )
            {
                QString strV = qry.value(i).toString();
                if ( i == sizerDataCol )
                {
                    if ( mForQuery )
                        strV = sizerTextSum(strV);  //查询的尺码明细经过GROUP_CONCAT后需要处理字符串重新整理统计
                    sizers = strV;
                }
                it = new BsGridItem(strV, SORT_TYPE_TEXT);

                //约定joinCargoPinyin的表格第一列货号，第二列品名，不可违反！见BsSheetCargoWin::loadPickStock的sql语句
                if ( joinCargoPinyin && i == 0 ) {
                    QString pinyin = strV + LxSoft::ChineseConvertor::GetFirstLetter(qry.value(1).toString());
                    it->setData(Qt::UserRole, pinyin);
                }
            }
            //数值字段
            else if ( (flags & bsffInt) == bsffInt )
            {
                qint64 intv = qry.value(i).toLongLong();
                QString txt = getDisplayTextOfIntData(intv, flags, mCols.at(idxCol)->mLenDots);
                it = new BsGridItem(txt, SORT_TYPE_NUM);

                if ( (flags & bsffBool) == bsffBool )
                    it->setTextAlignment(Qt::AlignCenter);
                else if ( (flags & bsffDate) != bsffDate && (flags & bsffDateTime) != bsffDateTime )
                    it->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            }
            //不应该存在
            else {
                qDebug() << QStringLiteral("Fatal Error Warning: Don't design real type field.");
                Q_ASSERT(1==2);
            }

            //可编辑表格附加UserRole的data值
            if ( !mForQuery )
            {
                //保存原值，以备取消还原
                it->setData(Qt::UserRole + OFFSET_OLD_VALUE, it->text());

                //主键列
                if ( idxCol == 0 )
                {
                    it->setData(Qt::UserRole + OFFSET_EDIT_STATE, bsesClean);
                    it->setFlags(it->flags() &~ Qt::ItemIsEditable);
                }
            }

            //只读字段
            if ( !mForQuery && (flags & bsffReadOnly) == bsffReadOnly )
            {
                it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                it->setBackground(QColor(244, 244, 244));
            }

            //置入单元格对象
            setItem(rows - 1, idxCol, it);

            //审核背景色
            if ( rowChecked && i == 0 )
                it->setData(Qt::DecorationRole, QIcon(":/icon/check.png"));
        }

        //横排尺码另外处理
        if ( sizerDataCol > 0 )
        {
            int recQtyColIdx = getColumnIndexByFieldName(QStringLiteral("qty"));   //不能用sqlRec.indexOf()，因为有bsffSizeUnit插入
            setSizerHCellsFromText(rows - 1, recQtyColIdx, sizers, useSizerType);
        }

    }
    qry.finish();

    //整理
    updateAllColTitles();
    mpFooter->initCols();

    //更新合计及单据头
    if ( rowCount() > 0 )
    {
        //合计
        updateFooterSumCount(false);

        //单据尺码列头（跟随第一行）
        if ( sizerDataCol > 0 && !mForQuery )
        {
            for ( int i = 0, iLen = sheetFirstRowSizeType.length(); i < iLen; ++i )
                model()->setHeaderData(mSizerPrevCol + i + 1, Qt::Horizontal, sheetFirstRowSizeType.at(i), Qt::DisplayRole);
        }
    }

    //恢复光标
    qApp->restoreOverrideCursor();
}

void BsGrid::saveColWidths(const QString &sub)
{
    QSettings settings;
    settings.beginGroup(BSR17ColumnWidth);
    settings.beginGroup(mTable + sub);

    int sizerTotalWidth = 0;
    int sizerCounts = 0;
    for ( int i = 0, iLen = mCols.length(); i < iLen; ++i ) {
        QString colFld = mCols.at(i)->mFldName;
        uint colFlags = mCols.at(i)->mFlags;
        if ( (colFlags & bsffSizeUnit) == bsffSizeUnit ) {
            sizerTotalWidth += columnWidth(i);
            sizerCounts++;
        }
        else {
            int w = ( (colFlags & bsffHideSys) == bsffHideSys ) ? -1 : columnWidth(i);
            settings.setValue(colFld, w);
        }
    }
    if ( sizerCounts > 0 )
        settings.setValue("sz", sizerTotalWidth / sizerCounts);

    settings.endGroup();
    settings.endGroup();
}

void BsGrid::loadColWidths(const QString &sub)
{
    QSettings settings;
    settings.beginGroup(BSR17ColumnWidth);
    settings.beginGroup(mTable + sub);

    for ( int i = 0, iLen = mCols.length(); i < iLen; ++i ) {
        QString colFld = mCols.at(i)->mFldName;
        uint colFlags = mCols.at(i)->mFlags;
        if ( (colFlags & bsffSizeUnit) == bsffSizeUnit ) {
            int w = settings.value("sz").toInt();
            if ( w > 0 )
                setColumnWidth(i, w);
            else
                setColumnWidth(i, 40);
        }
        else {
            int w = settings.value(colFld).toInt();
            if ( w < 0 || mDenyFields.indexOf(colFld) >= 0 )
                setColumnHidden(i, true);
            else if ( w > 0 )
                setColumnWidth(i, w);
            else
                setColumnWidth(i, 80);
        }
    }

    settings.endGroup();
    settings.endGroup();
}

void BsGrid::updateColTitleSetting()
{
    for ( int i = 0, iLen = mCols.length(); i < iLen; ++i ) {
        uint flags = mCols.at(i)->mFlags;
        if ( (flags & bsffSizeUnit) != bsffSizeUnit )
            model()->setHeaderData(i, Qt::Horizontal, mCols.at(i)->mFldCnName);
    }
}

void BsGrid::cancelAllFilter()
{
    filterRestoreAll();
}

int BsGrid::getDataSizerColumnIdx() const
{
    for ( int i = 0, iLen = mCols.length(); i < iLen; ++i ) {
        if ( mCols.at(i)->mFldName == QStringLiteral("sizers") )
            return i;
    }
    return -1;
}

QString BsGrid::addCalcMoneyColByPrice(const QString &priceField)
{
    if ( rowCount() < 1 )
        return QString();

    //确保有货号列
    int idxCargo = -1;
    for ( int i = 0, iLen = mCols.length(); i < iLen; ++i )
    {
        BsField *fld = mCols.at(i);
        if ( fld->mFldName.contains(QStringLiteral("cargo")) )
        {
            idxCargo = i;
            break;
        }
    }
    if ( idxCargo < 0 ) {
        return QStringLiteral("必须有货号列和数量列，才能演算。");
    }

    //确保有数量列
    int idxQty = -1;
    for ( int i = 0, iLen = mCols.length(); i < iLen; ++i )
    {
        BsField *fld = mCols.at(i);
        if ( fld->mFldName.contains(QStringLiteral("qty")) )
        {
            idxQty = i;
            break;
        }
    }
    if ( idxQty < 0 ) {
        return QStringLiteral("必须有货号列和数量列，才能演算。");
    }

    //列名称
    QString fld = priceField;
    fld.replace(QStringLiteral("price"), QStringLiteral("money"));
    int moneyDots = mapOption.value("dots_of_money").toInt();

    //添加列定义
    QStringList defs = mapMsg.value(QStringLiteral("fld_%1").arg(fld)).split(QChar(9));
    Q_ASSERT(defs.count() > 4);
    BsField *bsCol = new BsField(fld,
                                 defs.at(0),
                                 QString(defs.at(3)).toUInt(),
                                 QString(defs.at(4)).toInt(),
                                 defs.at(2));
    mCols.insert(idxQty + 1, bsCol);

    //添加数据
    insertColumn(idxQty + 1);
    for ( int i = 0, iLen = rowCount(); i < iLen; ++i ) {
        QString cargo = item(i, idxCargo)->text();
        double qty = item(i, idxQty)->text().toDouble();
        double price = dsCargo->getValue(cargo, priceField).toDouble() / 10000.0;
        double money = ( abs(qty) > 0.00001 && abs(price) > 0.00001 ) ? qty * price : 0.0;
        BsGridItem *mitem = new BsGridItem(QString::number(money, 'f', moneyDots), SORT_TYPE_NUM);
        mitem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        mitem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if ( i % 2 ) mitem->setBackground(QColor(240, 240, 240));
        setItem(i, idxQty + 1, mitem);
    }

    //整理
    updateAllColTitles();
    mpFooter->initCols();
    updateFooterSumCount(false);

    return QString();
}

BsField *BsGrid::getFieldByName(const QString &name, int *colIdx)
{
    BsField *found = nullptr;

    for ( int i = 0, iLen = mCols.length(); i < iLen; ++i )
    {
        BsField *fld = mCols.at(i);
        if ( fld->mFldName == name )
        {
            found = fld;
            if ( colIdx )
            {
                *colIdx = i;
            }
            break;
        }
    }

    return found;
}

bool BsGrid::noMoreVisibleRowsAfter(const int currentVisibleRow)
{
    int allRows = rowCount();

    if ( currentVisibleRow >= allRows - 1 )
        return true;

    for ( int i = currentVisibleRow + 1; i < allRows; ++i )
    {
        if (  !isRowHidden(i) )
            return false;
    }
    return true;
}

bool BsGrid::noMoreVisibleColsAfter(const int currentVisibleCol)
{
    int allCols = columnCount();

    if ( currentVisibleCol >= allCols - 1 )
        return true;

    for ( int i = currentVisibleCol + 1; i < allCols; ++i )
    {
        if (  !isColumnHidden(i) )
            return false;
    }
    return true;
}

QString BsGrid::getFooterValueByField(const QString &fieldName)
{
    int idx = getColumnIndexByFieldName(fieldName);

    if ( idx >= 0 ){
        return mpFooter->item(0, idx)->text();
    }

    return QString();
}

int BsGrid::getColumnIndexByFieldName(const QString &fieldName)
{
    for ( int i = 0, iLen = mCols.length(); i < iLen; ++i )
    {
        if ( mCols.at(i)->mFldName == fieldName )
        {
            return i;
        }
    }
    return -1;
}

void BsGrid::hideFooterText()
{
    mpFooter->hideAggText();
}

QString BsGrid::getDisplayTextOfIntData(const qint64 intV, const uint flags, const int dots)
{
    //日期
    if ( (flags & bsffDate) == bsffDate )
    {
        QDate dt = QDateTime::fromMSecsSinceEpoch(1000 * intV).date();
        return dt.toString("yyyy-MM-dd");
    }
    //布尔
    if ( (flags & bsffBool) == bsffBool )
    {
        return (intV == 0) ? QString() : mapMsg.value("word_yes");
    }
    //小数
    else if ( (flags & bsffNumeric) == bsffNumeric )
    {
        if ( (flags & bsffSizeUnit) == bsffSizeUnit || (flags & bsffBlankZero) == bsffBlankZero )       //尺码列数量0显示为空
            return ( intV == 0 ) ? QString() : bsNumForRead(intV, dots);
        else
            return bsNumForRead(intV, dots);
    }

    //整数
    return QString::number(intV);
}

QString BsGrid::getSqlValueFromDisplay(const int row, const int col)
{
    //表格值
    QString txt = item(row, col)->text();

    //字段特性
    uint flags = mCols.at(col)->mFlags;

    //文本
    if ( (flags & bsffText) == bsffText )
    {
        return QStringLiteral("'%1'").arg(txt);
    }

    //日期
    if ( (flags & bsffDate) == bsffDate )
    {
        QDate day = QDate::fromString(txt, "yyyy-MM-dd");
        QTime ztime = QTime(0, 0, 0, 0);
        QDateTime dt = QDateTime(day, ztime);
        return QString::number(dt.toMSecsSinceEpoch() / 1000);
    }

    //布尔
    if ( (flags & bsffBool) == bsffBool )
    {
        return QString();   //没法还原。反正这个函数只是用于“核对”窗口的追加条件，没有用到bool型。
    }
    //小数
    else if ( (flags & bsffNumeric) == bsffNumeric )
    {
        return bsNumForSave(txt.toDouble());
    }

    return txt;
}

void BsGrid::setRowHeight(const int height)
{
    if ( height != mRowHeight )
    {
        mRowHeight = height;
        verticalHeader()->setDefaultSectionSize(height);
    }
}

void BsGrid::setFontPoint(const int point)
{
    if ( point != mFontPoint )
    {
        mFontPoint = point;
        QFont font = qApp->font();
        font.setPointSize(point);
        setFont(font);
    }
}

int BsGrid::getRowHeight() const
{
    return mRowHeight;
}

int BsGrid::getFontPoint() const
{
    return mFontPoint;
}

void BsGrid::sortByRowTime()
{
    if ( !mForQuery && !mForRegister )
    {
        int idx = getColumnIndexByFieldName(QStringLiteral("rowtime"));
        if ( idx > 0 )
            sortByColumn(idx, Qt::AscendingOrder);
    }
}

void BsGrid::showEvent(QShowEvent *e)
{
    QTableWidget::showEvent(e);
    updateFooterGeometry();
}

void BsGrid::resizeEvent(QResizeEvent *e)
{
    QTableWidget::resizeEvent(e);
    updateFooterGeometry();
}

void BsGrid::paintEvent(QPaintEvent *e)
{
    QTableWidget::paintEvent(e);
    QPainter p(viewport());
    p.setPen(QColor(216, 216, 216));
    int x = 0;
    for ( int i = 0, iLen = columnCount(); i < iLen; ++i )
    {
        x += columnWidth(i);
        p.drawLine(x - 1, 0, x - 1, viewport()->height());
    }
}

void BsGrid::mousePressEvent(QMouseEvent *e)
{
    QTableWidget::mousePressEvent(e);

    if ( e->button() != Qt::RightButton || !currentItem() )
        return;

    //登记和单据，只有货号的attrX列可以筛选，因此，如此频繁提示不好。所以仅允许查询表格，才给提示。
    if ( mForQuery ) {
        BsField *col = mCols.at(currentColumn());
        if ( (col->mFlags & bsffAggCount) != bsffAggCount )
        {
            emit shootHintMessage(mapMsg.value("i_this_col_cannot_filter"));
            return;
        }
    }

    mpFilterOut->setText(QStringLiteral("%1“%2”")
                         .arg(mapMsg.value("menu_filter_out"))
                         .arg(currentItem()->text()));
    mpMenu->popup(e->globalPos());
}

void BsGrid::focusInEvent(QFocusEvent *e)
{
    QTableWidget::focusInEvent(e);
    emit focusInned();
}

void BsGrid::focusOutEvent(QFocusEvent *e)
{
    QTableWidget::focusOutEvent(e);
    emit focusOuted();
}

void BsGrid::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTableWidget::currentChanged(current, previous);

    if ( getDataSizerColumnIdx() >= 0 )
    {
        bool rowChanged = current.isValid();
        if ( current.isValid() && previous.isValid() )
            rowChanged = previous.row() != current.row();

        //更新尺码列特性
        if ( rowChanged )
            updateSizerColTitles(current.row());
    }
}

void BsGrid::updateFooterSumCount(const bool checkFilter)
{
    //耗时过程
    qApp->setOverrideCursor(Qt::WaitCursor);

    //清空统计值
    int visibleRows = 0;
    int filterColCounts = 0;
    for ( int j = 0, jLen = mCols.length(); j < jLen; ++j )
    {
        BsField *bsCol = mCols.at(j);
        bsCol->mCountSet.clear();
        bsCol->mAggValue = 0;
        if ( bsCol->mFilterType == bsftEqual || bsCol->mFilterType == bsftNotEqual )
            filterColCounts++;
    }

    //逐行判断并统计
    for ( int i = 0, iLen = rowCount(); i < iLen; ++i )
    {
        //可见性值最宽松开始
        bool visible = mForQuery || checkFilter || ( !checkFilter && !isRowHidden(i) );  //非mForQuery情况下不改变删除行的可见性

        //筛选可见性
        if ( checkFilter )
        {
            for ( int j = 0, jLen = mCols.length(); j < jLen; ++j )
            {
                uint ft = mCols.at(j)->mFilterType;
                QStringList fv = mCols.at(j)->mFilterValue;
                QString cellValue = item(i, j)->text();

                //留下筛选
                if ( ft == bsftEqual )
                {
                    if ( fv.indexOf(cellValue) < 0 )
                    {
                        visible = false;
                        break;
                    }
                }

                //剔除筛选
                if ( ft == bsftNotEqual )
                {
                    if ( fv.indexOf(cellValue) >= 0 )
                    {
                        visible = false;
                        break;
                    }
                }

                //bsftContain筛选仅用于拣货辅助
                if ( j == 0 && ft == bsftContain )
                {
                    QString keyChars = fv.join(QString());
                    if ( ! item(i, 0)->data(Qt::UserRole).toString().contains(keyChars) ) {
                        visible = false;
                        break;
                    }
                }
            }
        }

        //是否合计
        bool inCount = visible;
        if ( ! mForQuery )
            inCount = inCount && item(i, 0)->data(Qt::UserRole + OFFSET_EDIT_STATE).toUInt() != bsesDeleted;

        //统计各列
        if ( inCount )
        {
            for ( int j = 0, jLen = mCols.length(); j < jLen; ++j )
            {
                uint flags = mCols.at(j)->mFlags;

                if ( (flags & bsffAggCount) == bsffAggCount )
                    mCols.at(j)->mCountSet.insert(item(i, j)->text());

                if ( (flags & bsffAggSum) == bsffAggSum )
                {

                    qint64 intV = ( (flags & bsffNumeric) == bsffNumeric )
                            ? bsNumForSave(item(i, j)->text().toDouble()).toLongLong()
                            : item(i, j)->text().toLongLong();
                    mCols.at(j)->mAggValue += intV;
                }
            }
        }

        //过滤
        if ( visible )
        {
            visibleRows++;
            setRowHidden(i, false);
        }
        else {
            setRowHidden(i, true);
        }
    }

    //显示统计值
    for ( int j = 0, jLen = mCols.length(); j < jLen; ++j )
    {
        //页脚显示
        uint flags = mCols.at(j)->mFlags;
        QString aggShow;
        Qt::AlignmentFlag footAlign = Qt::AlignLeft;

        if ( (flags & bsffAggCount) == bsffAggCount || (flags & bsffAggSum) == bsffAggSum )
        {
            if ( (flags & bsffAggCount) == bsffAggCount )
            {
                aggShow = QStringLiteral("<%1>").arg(mCols.at(j)->mCountSet.count());
                footAlign = Qt::AlignCenter;
            }

            if ( (flags & bsffAggSum) == bsffAggSum )
            {
                aggShow = getDisplayTextOfIntData(mCols.at(j)->mAggValue, flags, mCols.at(j)->mLenDots);
                footAlign = Qt::AlignRight;
            }

            mpFooter->item(0, j)->setText(aggShow);
            mpFooter->item(0, j)->setTextAlignment(int(footAlign) | int(Qt::AlignVCenter));
        }

        //列头颜色（如果有筛选显示红色）
        uint ft = mCols.at(j)->mFilterType;
        QColor titleColor = (ft == bsftNone || ft == bsftContain) ? Qt::black : Qt::red;
        model()->setHeaderData(j, Qt::Horizontal, QBrush(titleColor), Qt::ForegroundRole);

        //单据需要知道合计金额
        if ( !mForQuery && !checkFilter && mCols.at(j)->mFldName == QStringLiteral("actmoney") )
            emit sheetSumMoneyChanged(aggShow);
    }
    updateFooterColWidths();

    //筛选信号
    if ( checkFilter )
    {
        if ( filterColCounts > 0 )
        {
            mFiltering = true;
            setStyleSheet(mapMsg.value("css_grid_filtering"));
            emit filterDone();
        }

        if ( filterColCounts == 0 )
        {
            mFiltering = false;
            setStyleSheet(mapMsg.value("css_grid_readonly"));
            emit filterEmpty();
        }
    }

    //角标总行数及精确位置更新
    mpCorner->setText(QString::number(visibleRows));
    QTimer::singleShot(100, this, SLOT(adjustCornerPosition()));

    //恢复光标
    qApp->restoreOverrideCursor();
}

void BsGrid::updateAllColTitles()
{
    QStringList colTitles;
    for ( int i = 0, iLen = mCols.length(); i < iLen; ++i )
        colTitles << mCols.at(i)->mFldCnName;
    setHorizontalHeaderLabels(colTitles);
}

void BsGrid::updateSizerColTitles(const int row)
{
    for ( int i = 0, iLen = mCols.length(); i < iLen; ++i )
    {
        if ( (mCols.at(i)->mFlags & bsffSizeUnit) == bsffSizeUnit )
        {
            model()->setHeaderData(i, Qt::Horizontal, item(row, i)->data(Qt::ToolTipRole).toString(), Qt::DisplayRole);
        }
    }
}

void BsGrid::updateFooterColWidths()
{
    //主要用于各横排尺码数量列变动，因此第一列不用包括在循环中。而且，第一列宽要根据情况减去horizontalScrollBar.height()，麻烦。
    for ( int i = 1, iLen = mCols.length(); i < iLen; ++i ) {
        if ( isColumnHidden(i) )
            mpFooter->setColumnHidden(i, true);
        else
            mpFooter->setColumnWidth(i, columnWidth(i));
    }
}

void BsGrid::setSizerHCellsFromText(const int row, const int qtyCol, const QString &sizersText, const QString &usingSizerType)
{
    //本行登记尺码表
    QStringList regList;
    if ( usingSizerType.isEmpty() )
    {
        QString cargo = item(row, 0)->text();
        QString sizerType = dsCargo->getValue(cargo, QStringLiteral("sizertype"));
        regList = dsSizerType->getSizerList(sizerType);
    }
    else
    {
        regList = dsSizerType->getSizerList(usingSizerType);
    }

    //尺码数量对原数据
    QStringList pairList = sizersText.split(QChar(10), QString::SkipEmptyParts);

    //登记列
    for ( int i = mSizerPrevCol + 1; i <= mSizerPrevCol + mSizerColCount; ++i )
    {
        //清空特性
        QTableWidgetItem *it = item(row, i);
        if ( ! it )
        {
            it = new BsGridItem(QString(), SORT_TYPE_NUM);
            setItem(row, i, it);
        }
        it->setText(QString());
        it->setData(Qt::DecorationRole, QVariant());
        it->setData(Qt::ToolTipRole, QString());
        it->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable);

        //登记范围内
        if ( i - mSizerPrevCol <= regList.length() )
        {
            it->setData(Qt::ToolTipRole, regList.at(i - mSizerPrevCol - 1));
            it->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
        else
        {
            it->setFlags(Qt::ItemIsSelectable);
            it->setBackground(QColor(244, 244, 244));
        }

        //设数量
        for ( int j = 0, jLen = pairList.length(); j < jLen; ++j )
        {
            QStringList pair = QString(pairList.at(j)).split(QChar(9));  //难免有空名的尺码，不能SkipEmptyParts
            Q_ASSERT(pair.length() == 2);
            if ( regList.indexOf(pair.at(0)) == i - mSizerPrevCol - 1 )
            {
                QString sqty = ( QString(pair.at(1)).toLongLong() == 0 )
                        ? QString()
                        : bsNumForRead(QString(pair.at(1)).toLongLong(), 0);
                it->setText(sqty);
            }
        }
    }

    //坏列数量填qty列ToolTip并标Warning
    QStringList badPairs;
    for ( int i = 0, iLen = pairList.length(); i < iLen; ++i )
    {
        QStringList pair = QString(pairList.at(i)).split(QChar(9));     //难免有空名的尺码，不能SkipEmptyParts
        QString sname = pair.at(0);
        Q_ASSERT(pair.length() == 2);
        if ( regList.indexOf(sname) < 0 )
        {
            if ( QString(pair.at(1)).toLongLong() != 0 )
            {
                QString sqty = bsNumForRead(QString(pair.at(1)).toLongLong(), 0);
                badPairs << QStringLiteral("%1\t%2").arg(sname).arg(sqty);
            }
        }
    }
    QTableWidgetItem *itQty = item(row, qtyCol);
    if ( badPairs.length() > 0 )
    {
        itQty->setData(Qt::ToolTipRole, badPairs.join(QChar(10)));
        itQty->setData(Qt::DecorationRole, QIcon(QStringLiteral(":/icon/error.png")));
        itQty->setData(Qt::UserRole + OFFSET_CELL_CHECK, bsccError);
    }
    else
    {
        itQty->setData(Qt::ToolTipRole, QString());
        itQty->setData(Qt::DecorationRole, QVariant());
        itQty->setData(Qt::UserRole + OFFSET_CELL_CHECK, 0);
    }
}

void BsGrid::filterIn()
{
    BsField *bsCol = mCols.at(currentIndex().column());

    //预检查
    if ( bsCol->mCountSet.count() < 2 )
    {
        QMessageBox msg;
        msg.setText(mapMsg.value("i_cannot_filter_in_because_few"));
        msg.exec();
        return;
    }

    //准备list
    QStringList ls;
    QSetIterator<QString> i(bsCol->mCountSet);
    while (i.hasNext())
    {
        ls << i.next();
    }

    //排序
    ls.sort(Qt::CaseInsensitive);

    //获取当前选中值
    QString picked = currentItem()->text();

    mpPicker->setPicks(ls, picked);

    //显示选择
    int y = horizontalHeader()->height();
    QRect rect = visualItemRect(currentItem());
    QPoint pt = mapToGlobal(QPoint(rect.x() + verticalHeader()->width() + 1, y));
    mpPicker->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    mpPicker->setGeometry(pt.x(), pt.y() + 1, columnWidth(currentColumn()) - 1, height() - y - horizontalScrollBar()->height() - 2);
    mpPicker->show();
}

void BsGrid::filterOut()
{
    int colIdx = currentIndex().column();
    BsField *bsCol = mCols.at(colIdx);

    //预检查
    if ( bsCol->mCountSet.count() < 2 )
    {
        QMessageBox msg;
        msg.setText(mapMsg.value("i_cannot_filter_out_because_few"));
        msg.exec();
        return;
    }

    //如果该列本来不是剔除筛选的，则需要重新改变本列筛选类型
    if ( bsftNotEqual != mCols.at(colIdx)->mFilterType )
    {
        mCols.at(colIdx)->mFilterType = bsftNotEqual;
        mCols.at(colIdx)->mFilterValue.clear();
    }

    //加入剔除值
    mCols.at(colIdx)->mFilterValue << currentItem()->text();

    //执行筛选
    updateFooterSumCount(true);
}

void BsGrid::filterRestoreCol()
{
    //取消本列筛选设置
    int colIdx = currentIndex().column();
    mCols.at(colIdx)->mFilterType = bsftNone;
    mCols.at(colIdx)->mFilterValue.clear();

    //执行筛选
    updateFooterSumCount(true);
}

void BsGrid::filterRestoreAll()
{
    //取消全部筛选设置
    for ( int i = 0; i < mCols.count(); ++i )
    {
        mCols.at(i)->mFilterType = bsftNone;
        mCols.at(i)->mFilterValue.clear();
    }

    //执行筛选
    updateFooterSumCount(true);
}

void BsGrid::requestImage(QTableWidgetItem *item)
{
    BsField *fld = mCols.at(item->column());
    if ( fld->mFldName == QStringLiteral("cargo") || fld->mFldName == QStringLiteral("hpcode") ) {
        QString cargo = item->text();

        QString waitMsg = QStringLiteral("图片获取中，请稍侯……");

        QStringList params;
        params << QStringLiteral("GETIMAGE")
               << QString::number(QDateTime::currentMSecsSinceEpoch() * 1000)
               << cargo;
        netSocket->netRequest(this, params, waitMsg);
    }
}

void BsGrid::adjustCornerPosition()
{
    mpCorner->setGeometry(1, 1, verticalHeader()->width() - 2, horizontalHeader()->height() - 2);
}

void BsGrid::updateFooterGeometry()
{
    QScrollBar *bar = horizontalScrollBar();
    int slideButtonSize = ( bar->maximum() > 0 ) ? bar->height() : 0;
    int deltaW = (verticalScrollBar()->isVisible()) ? verticalScrollBar()->width() : 0;
    int wt = width() - deltaW - 1;
    if ( bar->maximum() > 0 )
        wt -= 2 * slideButtonSize;
    mpFooter->setGeometry(slideButtonSize, 0, wt, bar->height());
    mpFooter->headerSectionResized(0, 0, columnWidth(0) - slideButtonSize);
}

void BsGrid::takeFilterInPicks(const QStringList &picks)
{
    if ( !picks.isEmpty() )
    {
        BsField *bsCol = mCols.at(currentIndex().column());

        //设置选中值
        bsCol->mFilterType = bsftEqual;
        bsCol->mFilterValue.clear();
        bsCol->mFilterValue << picks;

        //执行筛选
        updateFooterSumCount(true);
    }
}

QString BsGrid::sizerTextSum(const QString &str)
{
    QStringList     names;
    QList<qint64>   values;
    QStringList lines = str.split(QChar(13), QString::SkipEmptyParts);
    foreach (QString line, lines)
    {
        bool minus = line.at(0) == QChar(12);
        if ( line.length() < 1 )
        {
            QMessageBox::critical(this, QString(), QStringLiteral("The application met an error at 0x66456145."));
            qApp->quit();
        }

        QStringList mxes = line.mid(1).split(QChar(10), QString::SkipEmptyParts);
        for ( int i = 0, iLen = mxes.length(); i < iLen; ++i )
        {
            QStringList pair = QString(mxes.at(i)).split(QChar(9));      //难免有空名的尺码，不能SkipEmptyParts
            if ( pair.length() != 2 ) continue;                          //尺码列全部没填的空行忽略

            QString size = pair.at(0);
            qint64 qty = QString(pair.at(1)).toLongLong();

            if ( minus )
                qty = 0 - qty;

            int idx = names.indexOf(size);
            if ( idx < 0 )
            {
                names << size;
                values << qty;
            }
            else
            {
                values[idx] += qty;
            }
        }
    }

    QStringList pairs;
    for ( int i = 0, iLen = names.length(); i < iLen; ++i )
        pairs << QStringLiteral("%1\t%2").arg(names.at(i)).arg(values.at(i));
    return pairs.join(QChar(10));
}


// BsQueryGrid
BsQueryGrid::BsQueryGrid(QWidget *parent) : BsGrid(parent, true, false)
{
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setAlternatingRowColors(true);
    setStyleSheet(mapMsg.value("css_grid_readonly"));
}

void BsQueryGrid::doPrint(const QString &title, const QStringList &conPairs,
                          const QString &printMan, const QString &printTime)
{
    //准备列宽数据
    QList<QString> colWidths;

    int totalw = 0;
    int lastCol = -1;
    for ( int j = 0, jLen = mCols.length(); j < jLen; ++j )
    {
        BsField *col = mCols.at(j);
        if ( (col->mFlags & bsffHideSys) != bsffHideSys )
        {
            totalw += columnWidth(j);
            lastCol = j;
        }
    }

    int sumRate = 0;
    for ( int j = 0, jLen = mCols.length(); j < jLen; ++j )
    {
        BsField *col = mCols.at(j);
        if ( (col->mFlags & bsffHideSys) != bsffHideSys )
        {
            int wRate = int(floor(100 * columnWidth(j) / totalw));
            if ( j < lastCol )
                colWidths << (QString::number(wRate) + QChar('%'));
            else
                colWidths << (QString::number(100 - sumRate) + QChar('%'));
            sumRate += wRate;
        }
        else
            colWidths << QString(); //保持colWidths与mCols等长
    }

    //html
    QString html = QStringLiteral("<body>");

    //公司标识
    html += QStringLiteral("<h4 align='center'>%1</h4>").arg(mapOption.value("app_company_name"));

    //标题
    html += QStringLiteral("<h1 align='center'>%1</h1>").arg(title);

    //选择范围
    html += QStringLiteral("<p>");
    QStringList conTags;
    for ( int i = 0, iLen = conPairs.length(); i < iLen; ++i )
    {
        QStringList pair = QString(conPairs.at(i)).split(QChar(9));
        conTags << QStringLiteral("<b>%1: </b>%2").arg(pair.at(0)).arg(pair.at(1));
    }
    html += conTags.join(QStringLiteral("&nbsp;&nbsp;&nbsp;&nbsp;"));
    html += QStringLiteral("</p>");

    //表开始
    html += QStringLiteral("<table cellspacing='0'>");

    //表列头
    html += QStringLiteral("<tr bgcolor='#ccc'>");
    for ( int j = 0, jLen = mCols.length(); j < jLen; ++j )
    {
        BsField *fld = mCols.at(j);
        uint flags = fld->mFlags;
        if ( (flags & bsffHideSys) != bsffHideSys )
        {
            QString prp = ((flags & bsffNumeric) == bsffNumeric)
                    ? QStringLiteral(" width='%1' align='right'").arg(colWidths.at(j))  //注意前加空格
                    : QStringLiteral(" width='%1' align='left'").arg(colWidths.at(j));
            html += QStringLiteral("<th%1>%2</th>").arg(prp).arg(fld->mFldCnName);
        }
    }
    html += QStringLiteral("</tr>");

    //表数据行
    for ( int i = 0, iLen = rowCount(); i < iLen; ++i )
    {
        if ( (i % 2) == 0 )
            html += QStringLiteral("<tr>");
        else
            html += QStringLiteral("<tr bgcolor='#e8e8e8'>");

        for ( int j = 0, jLen = mCols.length(); j < jLen; ++j )
        {
            uint flags = mCols.at(j)->mFlags;

            QString prp = ((flags & bsffNumeric) == bsffNumeric)
                    ? QStringLiteral(" align='right'")                              //注意前加空格
                    : QString();

            if ( (flags & bsffHideSys) != bsffHideSys )
                html += QStringLiteral("<td%1>%2</td>").arg(prp).arg(item(i, j)->text());
        }

        html += QStringLiteral("</tr>");
    }

    //合计行
    html += QStringLiteral("<tr bgcolor='#ccc'>");
    for ( int j = 0, jLen = mCols.length(); j < jLen; ++j )
    {
        uint flags = mCols.at(j)->mFlags;
        if ( (flags & bsffHideSys) != bsffHideSys )
        {
            if ( (flags & bsffNumeric) == bsffNumeric && (flags & bsffAggSum) == bsffAggSum )
                html += QStringLiteral("<td align='right'>%1</td>").arg(mpFooter->item(0, j)->text());
            else if ( j == 0 )
                html += QStringLiteral("<td>合计：</td>");
            else
                html += QStringLiteral("<td></td>");
        }
    }
    html += QStringLiteral("</tr>");

    //表结束
    html += QStringLiteral("</table>");

    //打印信息
    html += QStringLiteral("<p align='right' bgcolor='white' color='#999'>打印人：%1 &nbsp; &nbsp; 打印时间：%2</p>")
            .arg(printMan).arg(printTime);

    //html结束
    html += QStringLiteral("</body>");

    //文档处理
    QTextDocument doc;
    doc.setDefaultStyleSheet(QStringLiteral("h1, h4 {margin:0; padding:0;} table {border:1px solid #ccc;} "));
    doc.setHtml(html);

    //打印
    QPrinter printer;
    QPrintDialog printDlg(&printer);
    if (printDlg.exec() == QDialog::Accepted) {
        doc.print(&printer);
    }
}



// BsSheetStockPickGrid
BsSheetStockPickGrid::BsSheetStockPickGrid(QWidget *parent)
    : BsGrid(parent, true, false)
{
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectItems);
    setAlternatingRowColors(true);
    setStyleSheet(mapMsg.value("css_grid_readonly"));

    mNeedFilterPressHint = true;
    mNeedPickPressHint = true;

    connect(this, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(cellQtyDoubleClicked(QTableWidgetItem*)));
}

void BsSheetStockPickGrid::tryLocateCargoRow(const QString &cargo, const QString &color)
{
    int found = -1;
    for ( int i = 0, iLen = rowCount(); i < iLen; ++i ) {
        QTableWidgetItem *itCargo = this->item(i, 0);
        QTableWidgetItem *itColor = this->item(i, 4);
        QString cargoText = (itCargo) ? itCargo->text() : QString();
        QString colorText = (itColor) ? itColor->text() : QString();
        if ( cargo == cargoText && color == colorText ) {
            found = i;
            break;
        }
    }

    if ( found < 0 ) {
        for ( int i = 0, iLen = rowCount(); i < iLen; ++i ) {
            QTableWidgetItem *itCargo = this->item(i, 0);
            QString cargoText = (itCargo) ? itCargo->text() : QString();
            if ( cargo == cargoText ) {
                found = i;
                break;
            }
        }
    }

    if ( found >= 0 ) {
        setCurrentCell(found, 5);
        scrollToItem(item(found, 5), QAbstractItemView::PositionAtCenter);
    }
}

void BsSheetStockPickGrid::setPickDelta(const int delta)
{
    mPickDelta = delta;
}

void BsSheetStockPickGrid::updateHint(const QString &msgHint)
{
    mKeyHint.clear();
    mMsgHint = msgHint;
    repaint();
}

void BsSheetStockPickGrid::inputMethodEvent(QInputMethodEvent *)
{
    mMsgHint = mapMsg.value("i_need_close_input_method");
    repaint();
}

void BsSheetStockPickGrid::keyPressEvent(QKeyEvent *e)
{
    QTableWidget::keyPressEvent(e);

    if ( mCols.isEmpty() ) {
        updateHelpStatus();
        return;
    }

    QString cording = mCols.at(0)->mFilterValue.join(QString());;

    //空格
    if ( e->key() == 0x20 )
    {
        mCols.at(0)->mFilterType = bsftNone;
        mCols.at(0)->mFilterValue.clear();
        updateFooterSumCount(true);
        updateHelpStatus();
        return;
    }

    //退格
    if ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete )
    {
        QStringList filterChars = mCols.at(0)->mFilterValue;
        if ( filterChars.length() < 2 )
        {
            mCols.at(0)->mFilterType = bsftNone;
            mCols.at(0)->mFilterValue.clear();
            updateFooterSumCount(true);
            updateHelpStatus();
            return;
        }
        mCols.at(0)->mFilterValue = filterChars.mid(0, filterChars.length() - 1);
        updateFooterSumCount(true);
    }

    //加号
    if ( e->key() == Qt::Key_Plus )
    {
        int row = currentRow();
        int col = currentColumn();
        QTableWidgetItem *itCurrent = currentItem();
        if ( row >= 0 && col > mSizerPrevCol && itCurrent ) {
            int colorIdx = getColumnIndexByFieldName(QStringLiteral("color"));
            int sizerIdx = mCols.at(col)->mFldName.mid(2).toInt() - 1;
            QString sizerName = dsSizerType->getSizerNameByIndex(mLoadSizerType, sizerIdx);
            mNeedPickPressHint = false;

            int oldCellQty = itCurrent->text().toInt();
            itCurrent->setText(QString::number(oldCellQty + mPickDelta));

            int qtyIdx = getColumnIndexByFieldName(QStringLiteral("qty"));
            QTableWidgetItem* itRowQty = item(row, qtyIdx);
            int oldRowQty = itRowQty->text().toInt();
            itRowQty->setText(QString::number(oldRowQty + mPickDelta));

            updateFooterSumCount(true);

            emit pickedCell(item(row, 0)->text(), item(row, colorIdx)->text(), sizerName);
        }
        updateHelpStatus();
        return;
    }

    //筛选
    if ( e->key() > 0x20 && e->key() < 0x7f && QChar(e->key()).isPrint() )
    {
        mCols.at(0)->mFilterType = bsftContain;
        mCols.at(0)->mFilterValue.append(QChar(e->key()));
        updateFooterSumCount(true);
        mNeedFilterPressHint = false;
    }
    updateHelpStatus();
}

void BsSheetStockPickGrid::paintEvent(QPaintEvent *e)
{
    BsGrid::paintEvent(e);

    QPainter p(viewport());
    p.setPen(QColor(255, 0, 0, 50));

    if ( mKeyHint.isEmpty() ) {
        QFont ft(font());
        ft.setPointSize(3 * font().pointSize());
        p.setFont(ft);
        p.drawText(0, 0, width(), viewport()->height(), Qt::AlignCenter, mMsgHint);
    }
    else {
        QFont ft(font());

        ft.setPointSize(6 * font().pointSize());
        p.setFont(ft);
        p.drawText(0, 0, viewport()->width(), viewport()->height(), Qt::AlignCenter, mKeyHint);

        int y = (viewport()->height() + p.fontMetrics().height()) / 2;

        ft.setPointSize(2 * font().pointSize());
        p.setFont(ft);
        p.drawText(0, y - 9, viewport()->width(), viewport()->height(), Qt::AlignHCenter | Qt::AlignTop, mMsgHint);
    }
}

void BsSheetStockPickGrid::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    BsGrid::currentChanged(current, previous);
    if ( isVisible() && hasFocus() && current.isValid() ) {
        QTableWidgetItem *itCargo = item(current.row(), 0);
        QTableWidgetItem *itColor = item(current.row(), 4);
        QString cargo = (itCargo) ? itCargo->text() : QString();
        QString color = (itColor) ? itColor->text() : QString();
        if ( !cargo.isEmpty() ) {
            emit cargoRowSelected(cargo, color);
        }
    }
}

void BsSheetStockPickGrid::cellQtyDoubleClicked(QTableWidgetItem *item)
{
    int row = item->row();
    int col = item->column();
    QTableWidgetItem *itCurrent = currentItem();
    if ( row >= 0 && col > mSizerPrevCol && itCurrent )
    {
        int colorIdx = getColumnIndexByFieldName(QStringLiteral("color"));
        int sizerIdx = mCols.at(col)->mFldName.mid(2).toInt() - 1;
        QString sizerName = dsSizerType->getSizerNameByIndex(mLoadSizerType, sizerIdx);
        mNeedPickPressHint = false;

        int oldCellQty = itCurrent->text().toInt();
        itCurrent->setText(QString::number(oldCellQty + mPickDelta));

        int qtyIdx = getColumnIndexByFieldName(QStringLiteral("qty"));
        QTableWidgetItem* itRowQty = this->item(row, qtyIdx);
        int oldRowQty = itRowQty->text().toInt();
        itRowQty->setText(QString::number(oldRowQty + mPickDelta));

        updateFooterSumCount(true);

        emit pickedCell(this->item(row, 0)->text(), this->item(row, colorIdx)->text(), sizerName);
    }
}

void BsSheetStockPickGrid::updateHelpStatus()
{
    if ( mCols.isEmpty() ) {
        mKeyHint.clear();
        mMsgHint = mapMsg.value("i_pick_query_first");
    }
    else {
        mKeyHint = mCols.at(0)->mFilterValue.join(QString());
        if (mNeedFilterPressHint || mNeedPickPressHint) {
            mMsgHint = (mKeyHint.isEmpty()) ? mapMsg.value("i_pick_keypress_hint") : mapMsg.value("i_pick_back_space_hint");
        } else {
            mMsgHint.clear();
        }
    }
    repaint();
}



// BsAbstractFormGrid
BsAbstractFormGrid::BsAbstractFormGrid(QWidget *parent, const bool forReg) : BsGrid(parent, false, forReg)
{
    mAllowIns = true;
    mAllowUpd = true;
    mAllowDel = true;

    //保证行操作按钮两个所需最小宽带
    verticalHeader()->setMinimumWidth( 2 * getRowHeight());

    //改为单项选
    setSelectionBehavior(QAbstractItemView::SelectItems);

    //行操作按钮
    mpBtnRow = new QToolButton(this);
    mpBtnRow->setIcon(QIcon("/icon/del.png"));
    mpBtnRow->setIconSize(QSize(2 * mRowHeight / 3, 2 * mRowHeight / 3));
    mpBtnRow->setFocusPolicy(Qt::NoFocus);
    mpBtnRow->setStyleSheet("border:none; background-color:#eee;");
    mpBtnRow->hide();

    connect(this, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(currentCellChanged(int,int,int,int)));
    connect(mpBtnRow, SIGNAL(clicked(bool)), this, SLOT(rowButtonClicked()));
}

void BsAbstractFormGrid::setAllowFlags(const bool inss, const bool updd, const bool dell)
{
    mAllowIns = inss;
    mAllowUpd = updd;
    mAllowDel = dell;
}

void BsAbstractFormGrid::appendNewRow()
{
    bool needAppendRow;
    if ( rowCount() > 0 )
    {
        QTableWidgetItem *lastRowHeadItem = item(rowCount() - 1, 0);
        needAppendRow = lastRowHeadItem && !lastRowHeadItem->text().trimmed().isEmpty();
    }
    else
        needAppendRow = true;

    if ( needAppendRow )
    {
        int oldRowCount = rowCount();
        setRowCount(oldRowCount + 1);

        for ( int i = 0, iLen = columnCount(); i < iLen; ++i )
        {
            //创建
            uint flags = mCols.at(i)->mFlags;
            BsGridItem *itNew = new BsGridItem(QString(), (flags & bsffText) ? SORT_TYPE_TEXT : SORT_TYPE_NUM);

            //格式
            if ( (flags & bsffBool) == bsffBool )
                itNew->setTextAlignment(Qt::AlignCenter);
            else if ( (flags & bsffInt) == bsffInt && (flags & bsffDate) != bsffDate && (flags & bsffDateTime) != bsffDateTime )
                itNew->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

            if ( (flags & bsffReadOnly) == bsffReadOnly )
            {
                itNew->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                itNew->setBackground(QColor(244, 244, 244));
            }

            itNew->setData(Qt::ForegroundRole, QColor(Qt::darkGreen));

            //置入表格
            setItem(oldRowCount, i, itNew);
        }

        //新行标志
        item(oldRowCount, 0)->setData(Qt::UserRole + OFFSET_EDIT_STATE, bsesNew);

        //自动数据
        int idxUpman = getColumnIndexByFieldName(QStringLiteral("upman"));
        if ( idxUpman >= 0 )
            item(oldRowCount, idxUpman)->setText(loginer);

        int idxUptime = getColumnIndexByFieldName(QStringLiteral("uptime"));
        if ( idxUptime >= 0 )
            item(oldRowCount, idxUptime)->setText(QString::number(QDateTime::currentMSecsSinceEpoch() / 1000));

        int idxRowtime = getColumnIndexByFieldName(QStringLiteral("rowtime"));
        if ( idxRowtime >= 0 )
            item(oldRowCount, idxRowtime)->setText(QString::number(QDateTime::currentMSecsSinceEpoch()));
    }
}

void BsAbstractFormGrid::updateRowState(const int row)
{
    //更新标志
    item(row, 0)->setData(Qt::UserRole + OFFSET_EDIT_STATE, bsesUpdated);

    //自动数据
    int idxUpman = getColumnIndexByFieldName(QStringLiteral("upman"));
    if ( idxUpman >= 0 )
        item(row, idxUpman)->setText(loginer);

    int idxUptime = getColumnIndexByFieldName(QStringLiteral("uptime"));
    if ( idxUptime >= 0 )
        item(row, idxUptime)->setText(QString::number(QDateTime::currentMSecsSinceEpoch() / 1000));

    int idxRowtime = getColumnIndexByFieldName(QStringLiteral("rowtime"));
    if ( idxRowtime >= 0 )
        item(row, idxRowtime)->setText(QString::number(QDateTime::currentMSecsSinceEpoch()));

    //行颜色
    updateRowColor(row);
}

uint BsAbstractFormGrid::saveCheck()
{
    //警告与错误
    uint cellCheck = bsccNone;
    for ( int i = 0, iLen = rowCount(); i < iLen; ++i )
    {
        QTableWidgetItem *itKey = item(i, 0);
        if ( itKey->data(Qt::UserRole + OFFSET_EDIT_STATE).toInt() != bsesDeleted )
        {
            for ( int j = 0, jLen = columnCount(); j < jLen; ++j )
            {
                QTableWidgetItem *it = item(i, j);
                Q_ASSERT(it);

                if ( it->data(Qt::UserRole + OFFSET_CELL_CHECK).toInt() > 0 )
                {
                    if ( cellCheck < bsccWarning && it->data(Qt::UserRole + OFFSET_CELL_CHECK) == bsccWarning )
                    {
                        cellCheck = bsccWarning;
                    }

                    if ( it->data(Qt::UserRole + OFFSET_CELL_CHECK) == bsccError )
                    {
                        return bsccError;
                    }
                }
            }
        }
    }

    return cellCheck;
}

void BsAbstractFormGrid::savedReconcile()
{
    mpBtnRow->hide();
    setCurrentCell(-1, -1);     //消除currentCellChanged()事件影响

    int iLen = rowCount();
    for ( int i = iLen - 1; i >= 0; i-- )   //因为有removeRow，所以必须倒序。
    {
        QTableWidgetItem *itKey = item(i, 0);
        int bsesState = itKey->data(Qt::UserRole + OFFSET_EDIT_STATE).toInt();

        if ( bsesState == bsesDeleted )
        {
            removeRow(i);
            continue;
        }

        if ( bsesState != bsesClean )
        {
            for ( int j = 0, jLen = columnCount(); j < jLen; ++j )
            {
                QTableWidgetItem *it = item(i, j);
                it->setData(Qt::UserRole + OFFSET_OLD_VALUE, it->text());
                it->setData(Qt::DecorationRole, QVariant());
                it->setData(Qt::UserRole + OFFSET_CELL_CHECK, 0);
            }
            itKey->setData(Qt::UserRole + OFFSET_EDIT_STATE, bsesClean);
            updateRowColor(i);
        }
    }

    setEditable(false);
}

void BsAbstractFormGrid::cancelRestore()
{
    mpBtnRow->hide();
    setCurrentCell(-1, -1);     //消除currentCellChanged()事件影响

    int iLen = rowCount();
    for ( int i = iLen - 1; i >= 0; i-- )   //因为有removeRow，所以必须倒序。
    {
        QTableWidgetItem *itKey = item(i, 0);
        int bsesState = itKey->data(Qt::UserRole + OFFSET_EDIT_STATE).toInt();

        if ( bsesState == bsesNew )
        {
            removeRow(i);
            continue;
        }

        if ( bsesState != bsesClean )
        {
            cancelRestorRow(i);
            itKey->setData(Qt::UserRole + OFFSET_EDIT_STATE, bsesClean);
            updateRowColor(i);
        }

        setRowHidden(i, false);
    }

    updateFooterSumCount(false);

    setEditable(false);
}

void BsAbstractFormGrid::setEditable(const bool editable)
{
    if ( editable )
    {
        setSelectionBehavior(QAbstractItemView::SelectItems);
        setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);
        horizontalHeader()->setSectionsClickable(false);
        setSortingEnabled(false);
        setStyleSheet(mapMsg.value("css_grid_editable"));

        if ( rowCount() == 0 && mAllowIns )
            appendNewRow();
    }
    else
    {
        mpBtnRow->hide();
        setEditTriggers(QAbstractItemView::NoEditTriggers);
        setSelectionBehavior(QAbstractItemView::SelectRows);
        setSortingEnabled(true);
        horizontalHeader()->setSectionsClickable(true);
        setStyleSheet(mapMsg.value("css_grid_readonly"));
    }
    mEditable = editable;
}

bool BsAbstractFormGrid::getEditable() const
{
    return mEditable;
}

void BsAbstractFormGrid::setDroppedRowByOption(const bool hideDropRow)
{
    if ( mFiltering )
        return;

    for ( int i = 0, iLen = rowCount(); i < iLen; ++i )
        setRowHidden(i,  hideDropRow && item(i, 0)->data(Qt::UserRole + OFFSET_EDIT_STATE).toInt() == bsesDeleted );

    mpBtnRow->hide();
}

bool BsAbstractFormGrid::needSaveDirty()
{
    for ( int i = 0, iLen = rowCount(); i < iLen; ++i )
    {
        QTableWidgetItem *itKey = item(i, 0);
        if ( itKey  )
        {
            int edtState = itKey->data(Qt::UserRole + OFFSET_EDIT_STATE).toInt();
            if ( edtState != bsesClean && !itKey->text().isEmpty() )
                return true;
        }
    }

    return false;
}

void BsAbstractFormGrid::hideCurrentCol()
{
    int col = currentColumn();
    int row = currentRow();

    if ( col <= 0 )
        return;

    setColumnHidden(col, true);

    //定位下一列
    if ( row >= 0 && col < columnCount() - 1 )
    {
        int toCol = col + 1;
        while ( toCol < columnCount() - 1 && isColumnHidden(toCol) )
            toCol++;
        setCurrentCell(row, toCol);
    }
}

void BsAbstractFormGrid::showHiddenCols()
{
    for ( int i = 0, iLen = mCols.length(); i < iLen; ++i )
    {
        uint flags = mCols.at(i)->mFlags;
        bool hidden = (flags & bsffHideSys) == bsffHideSys || mDenyFields.indexOf(mCols.at(i)->mFldName) >= 0;
        setColumnHidden(i, hidden);
        if ( !hidden && columnWidth(i) <= 0 )
            setColumnWidth(i, 50);
    }
}

void BsAbstractFormGrid::keyPressEvent(QKeyEvent *e)
{
    if ( ! getEditable() )
    {
        QTableWidget::keyPressEvent(e);
        return;
    }

    //下面大量使用
    const int k = e->key();
    const int row = currentRow();
    const int col = currentColumn();

    //监视条码枪扫描（只读列没有delegate编辑框出现，所以QTableWiget始终能接受到key）
    if ( col > 1 && (mCols.at(col)->mFlags & bsffReadOnly) == bsffReadOnly )
    {
        if ( k >= 0x20 && k <= 0xff )     //可见Latin1字符
        {
            mProbableBarcode += QChar(k);
            return; //否则默认行为会自动查找包含该字符的行，造成scroll
        }
        else                            //控制字符
        {
            //判断为扫描（谁也不会赣犟到在不可录入的栏中连续多次输入）
            if ( mProbableBarcode.length() >= 5 && (k == Qt::Key_Enter || k == Qt::Key_Return) )
            {
                emit barcodeScanned(mProbableBarcode);
                mProbableBarcode.clear();
                return;
            }
            mProbableBarcode.clear();
        }
    }
    else
    {
        mProbableBarcode.clear();

        //新行强制先填首列
        if ( (k < 0x01000000 || k > 0x01000060) && col > 0  && item(row, 0)->text().trimmed().isEmpty() )
        {
            setCurrentCell(row, 0);
            emit shootForceMessage(mapMsg.value("i_head_col_must_edit_first"));
            return;
        }
    }

    //价格折扣禁止填负值
    QString fldName = mCols.at(col)->mFldName.toLower();
    if ( fldName.contains(QStringLiteral("price")) || fldName.contains(QStringLiteral("discount")) ) {
        if ( e->key() == Qt::Key_Minus || e->text() == QStringLiteral("-") ) {
            return;
        }
    }

    //Enter改Tab
    if ( k == Qt::Key_Enter || k == Qt::Key_Return )
    {
        if ( col == columnCount() - 1 && noMoreVisibleRowsAfter(row) && mAllowIns )
        {
            commitData(cellWidget(row, col));
            appendNewRow();
        }

        //不能用setCurrentCell(row, col + 1)来设置单元格焦点，因为可能有删除列
        qApp->postEvent(this, new QKeyEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier), 0);
        return;
    }

    //尾行时
    if ( noMoreVisibleRowsAfter(row) )
    {
        //头列未填
        if ( item(row, 0)->text().trimmed().isEmpty() )
        {
            //ESC键取消新空行
            if ( k == Qt::Key_Escape && rowCount() > 1 )
            {
                //不能用setCurrentCell(row - 1, 0)来设置单元格焦点，因为可能有隐藏删除行
                qApp->postEvent(this, new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier), 0);
                return;
            }
        }
        //头列有填
        else {
            //自动产生新空行
            if ( mAllowIns && k == Qt::Key_Down )
            {
                commitData(cellWidget(row, col));
                appendNewRow();
                setCurrentCell(rowCount() - 1, 0);      //新行总是在最后(不能用row + 1，因为可能有隐藏删除行)
                return;
            }
        }
    }

    //更改主列提示（非控制键以及用户筛选习惯了的空格键排除）
    if ( currentColumn() == 0  && ( e->key() < 0x01000000 || e->key() > 0x01000060) && e->key() != Qt::Key_Space )
    {
        QTableWidgetItem *it = currentItem();
        if ( it && it->data(Qt::UserRole + OFFSET_EDIT_STATE).toInt() != bsesNew )
        {
            emit shootForceMessage(mapMsg.value("i_cannot_edit_exists_key_col"));
            return;
        }
    }

    QTableWidget::keyPressEvent(e);
}

//commitData只会在用户输入时触发，程序setText()、setData()都不会触发，所以只针对用户录入。
//不使用cellChanged信号，是因为它连所有role的data改变都触发！结果死循环崩溃。commitData中跳模式对话框也崩溃。
//后代继承执行顺序为栈式出入，本类及后代顺序如下：
//      BsSheetCargoGrid.Before
//      BsSheetGrid.Before
//      BsAbstractFormGrid.Before
//      BsAbstractFormGrid.After
//      BsSheetGrid.After
//      BsSheetCargoGrid.After
void BsAbstractFormGrid::commitData(QWidget *editor)
{
    if ( ! mAllowUpd && item(currentRow(), 0)->data(Qt::UserRole + OFFSET_EDIT_STATE).toUInt() != bsesNew )
        return;

    //预存比较提交后是否有真改变
    QString txtBefore = currentItem()->text();

    //数值列检查公式输入，并进行计算转换，确保为数值
    BsField *bsCol = mCols.at(currentColumn());
    if ( (bsCol->mFlags & bsffNumeric) == bsffNumeric )
    {
        //公式计算
        QLineEdit *edt = qobject_cast<QLineEdit*>(editor);
        if ( edt )
        {
            QString txt = edt->text().trimmed();
            if ( txt.startsWith(QChar('=')) )
            {
                bool expValid = true;
                double val = lxMathEval(txt, &expValid);
                if ( expValid )
                    edt->setText(QString::number(val, 'f', bsCol->mLenDots));
            }

            bool ok;
            double numValue = edt->text().toDouble(&ok);
            if ( ok )
                edt->setText(QString::number(numValue, 'f', bsCol->mLenDots));
            else
                edt->setText((bsCol->mFlags & bsffSizeUnit)
                             ? QString()
                             : QString::number(0.0, 'f', bsCol->mLenDots));
        }
    }
    //逗号列表检查
    else if ( mForRegister )
    {
        QLineEdit *edt = qobject_cast<QLineEdit*>(editor);
        if ( edt )
        {
            QString txt = edt->text();
            txt.replace(QChar(0xff0c), QStringLiteral(",")).replace(QRegularExpression("\\s+"), QString());
            while ( txt.length() > 0 && txt.at(0) == QChar(',') )
                txt = txt.mid(1);
            while ( txt.length() > 0 && txt.at(txt.length() - 1) == QChar(',') )
                txt = txt.left(txt.length() - 1);
            edt->setText(txt);
        }
    }
    //文本trim
    else if ( (bsCol->mFlags & bsffText) == bsffText )
    {
        QLineEdit *edt = qobject_cast<QLineEdit*>(editor);
        if ( edt )
        {
            QString txt = edt->text().trimmed();
            edt->setText(txt);
        }
    }

    //主键禁止单引号
    if ( currentColumn() == 0 ) {
        QLineEdit *edt = qobject_cast<QLineEdit*>(editor);
        if ( edt )
        {
            QString txt = edt->text();
            txt.replace(QChar(39), QString()).replace(QRegularExpression("\\s+"), QString());
            edt->setText(txt);
        }
    }

    //提交
    QTableWidget::commitData(editor);

    //没改变则先退出
    if ( currentItem()->text() == txtBefore )
        return;

    //录入错误与警告
    QTableWidgetItem *it = currentItem();
    if ( (bsCol->mFlags & bsffBool) == bsffBool )
    {
        //Need not check because it uses delegate editor.
        //But cannot remove this sentence because otherwise it will be dealed as bsffInt
    }
    else if ( (bsCol->mFlags & bsffNumeric) == bsffNumeric )
    {
        //因为包含bsffSizeUnit，不能清空ToolTip，再加上前面有确保非法转为零，故不检查
    }
    else if ( (bsCol->mFlags & bsffInt) == bsffInt )
    {
        bool ok;
        it->text().toLongLong(&ok);
        bool validd = ok || it->text().isEmpty();
        it->setData(Qt::ToolTipRole, (validd) ? QString() : mapMsg.value("i_error_invalid_number"));
        it->setData(Qt::DecorationRole, (validd) ? QVariant() : QIcon(":/icon/error.png"));
        it->setData(Qt::UserRole + OFFSET_CELL_CHECK, (validd) ? 0 : bsccError);
    }
    else if ( (bsCol->mFlags & bsffText) == bsffText ) {
        bool validd = it->text().length() <= bsCol->mLenDots;
        it->setData(Qt::ToolTipRole, (validd) ? QString() : mapMsg.value("i_warning_too_long_text"));
        it->setData(Qt::DecorationRole, (validd) ? QVariant() : QIcon(":/icon/warning.png"));
        it->setData(Qt::UserRole + OFFSET_CELL_CHECK, (validd) ? 0 : bsccWarning);
    }

    //更新行状态（新行、已更新脏行、删除行状态不变）
    QTableWidgetItem *itMaster = item(currentRow(), 0);
    if ( itMaster->data(Qt::UserRole + OFFSET_EDIT_STATE).toInt() == bsesClean )
    {
        itMaster->setData(Qt::UserRole + OFFSET_EDIT_STATE, bsesUpdated);
        updateRowColor(currentRow());
        updateRowButton(currentRow());
    }
}

void BsAbstractFormGrid::wheelEvent(QWheelEvent *e)
{
    BsGrid::wheelEvent(e);
    mpBtnRow->hide();
}

void BsAbstractFormGrid::resizeEvent(QResizeEvent *e)
{
    BsGrid::resizeEvent(e);
    mpBtnRow->hide();       //位置不对了，让用户重新点击，触发currentCellChanged()再显示。
}

void BsAbstractFormGrid::cancelRestorRow(const int row)
{
    int idxQty = getColumnIndexByFieldName(QStringLiteral("qty"));
    for ( int j = 0, jLen = columnCount(); j < jLen; ++j )
    {
        if ( ! (mCols.at(j)->mFlags & bsffSizeUnit) )
        {
            QTableWidgetItem *it = item(row, j);
            it->setText(it->data(Qt::UserRole + OFFSET_OLD_VALUE).toString());
            if ( j != idxQty )
            {
                it->setData(Qt::DecorationRole, QVariant());
                it->setData(Qt::UserRole + OFFSET_CELL_CHECK, 0);
            }
        }
    }

    int dataSizerCol = getDataSizerColumnIdx();
    if ( idxQty > 0 && dataSizerCol >= 0 )
    {
        QString sizers = item(row, dataSizerCol)->data(Qt::UserRole + OFFSET_OLD_VALUE).toString();
        setSizerHCellsFromText( row, idxQty, sizers);
    }
}

void BsAbstractFormGrid::updateRowColor(int row)
{
    QTableWidgetItem *itMaster = item(row, 0);
    if ( itMaster )
    {
        int editState = itMaster->data(Qt::UserRole + OFFSET_EDIT_STATE).toInt();
        QColor clr;
        switch ( editState )
        {
        case bsesNew:
            clr = Qt::darkGreen;
            break;
        case bsesUpdated:
            clr = Qt::blue;
            break;
        case bsesDeleted:
            clr = Qt::red;
            break;
        default:
            clr = Qt::black;
        }

        for ( int i = 0, iLen = columnCount(); i < iLen; ++i )
        {
            item(row, i)->setData(Qt::ForegroundRole, clr);
        }
    }
}

void BsAbstractFormGrid::updateRowButton(int row)
{
    if ( getEditable() )
    {
        QTableWidgetItem *itMaster = item(row, 0);
        int y;
        if ( itMaster )
        {
            if ( itMaster->data(Qt::UserRole + OFFSET_EDIT_STATE).toInt() == bsesClean )
            {
                mpBtnRow->setIcon(QIcon(":/icon/del.png"));
            }
            else  {
                mpBtnRow->setIcon(QIcon(":/icon/cancel.png"));
            }
            y = visualItemRect(itMaster).y();
        }
        else {
            mpBtnRow->setIcon(QIcon(":/icon/cancel.png"));
            if ( row > 0 )
            {
                y = visualItemRect(item(row - 1, 0)).y() + mRowHeight;
            }
            else {
                y = 0;
            }
        }
        mpBtnRow->setGeometry(1, y + horizontalHeader()->height(),
                              verticalHeader()->width() - 1, verticalHeader()->defaultSectionSize() + 1);
        mpBtnRow->show();
    }
    else {
        mpBtnRow->hide();
    }
}

void BsAbstractFormGrid::currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    mProbableBarcode.clear();

    //代码批量处理表格时，都会setCurrentCell(-1, -1); 所以如此。
    if ( currentRow < 0 || currentColumn < 0 )
        return;

    //切换当前列（显示列说明）
    if ( !mForQuery && currentColumn != previousColumn )
        emit shootHintMessage(mCols.at(currentColumn)->mStatusTip);

    //切换当前行
    if ( currentRow != previousRow )
    {
        if ( getEditable() )
        {
            //离开空行，则删除
            if ( previousRow >= 0 )
            {
                if ( item(previousRow, 0)->text().trimmed().isEmpty() && previousRow == rowCount() - 1 )
                {
                    setRowCount(rowCount() - 1);
                }
            }

            //设置行头按钮
            updateRowButton(currentRow);
        }

        //传出upman、uptime信息
        if ( mForRegister )
        {
            QTableWidgetItem *itKey  = item(currentRow, 0);
            QTableWidgetItem *itUpman  = item(currentRow, columnCount() - 2);
            QTableWidgetItem *itUptime = item(currentRow, columnCount() - 1);
            QStringList values;
            values << itKey->text() << itUpman->text() << itUptime->text();
            emit shootCurrentRowSysValue(values);
        }
    }
}

void BsAbstractFormGrid::rowButtonClicked()
{
    QTableWidgetItem *itMaster = item(currentRow(), 0);

    if ( mppWin && itMaster && !itMaster->text().trimmed().isEmpty() )
    {
        bool hideDropRoww = mppWin->getOptValueByOptName("opt_hide_drop_red_row");
        int editState = itMaster->data(Qt::UserRole + OFFSET_EDIT_STATE).toInt();
        switch ( editState )
        {

        //干净数据行————标记为删除
        case bsesClean:
            if ( mAllowDel ) {
                itMaster->setData(Qt::UserRole + OFFSET_EDIT_STATE, bsesDeleted);
                setRowHidden(currentRow(), hideDropRoww);
                updateRowButton(currentRow());
                updateRowColor(currentRow());
                updateFooterSumCount(false);
                if ( hideDropRoww )
                    mpBtnRow->hide();
            }
            break;

        //新行————删除
        case bsesNew:
            if ( rowCount() > 1 )
                removeRow(currentRow());
            else
                for ( int i = 0, iLen = columnCount(); i < iLen; ++i )
                    item(0, i)->setText(QString());
            mpBtnRow->hide();
            updateFooterSumCount(false);
            break;

        //脏数据行————恢复原值
        default:
            cancelRestorRow(currentRow());
            itMaster->setData(Qt::UserRole + OFFSET_EDIT_STATE, bsesClean);
            updateRowButton(currentRow());
            updateRowColor(currentRow());
            updateFooterSumCount(false);
        }
    }
    else if (currentRow() > 0)
    {
        //这个会触发currentCelChanged()来删除空行，如果两处删除，会崩溃。
        setCurrentCell(currentRow() - 1, 0);
    }
}



// BsSheetGrid
BsSheetGrid::BsSheetGrid(QWidget *parent, const QString &table)
    : BsAbstractFormGrid(parent, false)
{
    mTable = table;
    mSheetId = 0;
    mpCorner->setStyleSheet("QToolButton{color:#666; border-style:none;} "
                            "QToolButton:hover{background:#666; color:white;}");
}

void BsSheetGrid::openBySheetId(const int sheetId)
{
    //重要前提先设参数
    mSheetId = sheetId;

    //加载数据
    QStringList sels;
    for (int i = 0, iLen = mCols.length(); i < iLen; ++i )
    {
        if ( (mCols.at(i)->mFlags & bsffSizeUnit) != bsffSizeUnit ) {
            if ( (mCols.at(i)->mFlags & bsffLookup) == bsffLookup ) {
                sels << QStringLiteral("'' as %1").arg(mCols.at(i)->mFldName);
            } else {
                sels << mCols.at(i)->mFldName;
            }
        }
    }
    QString sql;
    sql = QStringLiteral("SELECT %1 FROM %2dtl WHERE parentid=%3 ORDER BY rowtime;")
            .arg(sels.join(QChar(44))).arg(mTable).arg(mSheetId);   //sheetId为负数，自然会返回空数据集
    loadData(sql);

    //加载列宽
    loadColWidths();

    //设置状态
    setEditable(sheetId < 0);
}

double BsSheetGrid::getColSumByFieldName(const QString &fld)
{
    int col = getColumnIndexByFieldName(fld);
    if ( col >= 0 ) {
        double ret = 0;
        for ( int i = 0, iLen = rowCount(); i < iLen; ++i )
            ret += item(i, col)->text().toDouble();
        return ret;
    }
    return 0.0;
}

void BsSheetGrid::adjustCurrentRowPosition()
{
    if ( currentRow() < 0 )
    {
        QMessageBox::information(this, QString(), mapMsg.value("i_pick_one_row_first"));
        return;
    }

    bool ok;
    int toRow = QInputDialog::getInt(this, QString(), mapMsg.value("i_tobe_put_row_num"), 1, 1, rowCount(), 1, &ok);
    if ( !ok || toRow < 1 || toRow > rowCount() )
        return;

    int rowTimeCol = getColumnIndexByFieldName(QStringLiteral("rowtime"));
    qint64 rowtime = item(toRow - 1, rowTimeCol)->text().toLongLong();
    qint64 newRowTime = ( toRow == rowCount() ) ? rowtime + 1 : rowtime - 1;
    item(currentRow(), rowTimeCol)->setText(QString::number(newRowTime));

    QTableWidgetItem* itKey = item(currentRow(), 0);
    if ( itKey && itKey->data(Qt::UserRole + OFFSET_EDIT_STATE).toUInt() == bsesClean )
    {
        itKey->setData(Qt::UserRole + OFFSET_EDIT_STATE, bsesUpdated);
        updateRowColor(currentRow());
    }

    sortByColumn(rowTimeCol, Qt::AscendingOrder);
}

bool BsSheetGrid::isCleanSort()
{
    int tcol = getColumnIndexByFieldName(QStringLiteral("rowtime"));
    if ( tcol > 0 )
    {
        qint64 preRowTime = 0;
        for ( int i = 0, iLen = rowCount(); i < iLen; ++i )
        {
            qint64 rowt = item(i, tcol)->text().toLongLong();
            if ( rowt < preRowTime )
                return false;
            preRowTime = rowt;
        }
    }
    return true;
}

void BsSheetGrid::keyPressEvent(QKeyEvent *e)
{
    BsAbstractFormGrid::keyPressEvent(e);
}

void BsSheetGrid::commitData(QWidget *editor)
{
    //记录提交前
    QString txtBefore = currentItem()->text();

    //提交
    BsAbstractFormGrid::commitData(editor);

    //没改变则先退出
    if ( currentItem()->text() == txtBefore )
        return;

    //隐藏列默认值rowtime
    int rowtimeIdx = getColumnIndexByFieldName(QStringLiteral("rowtime"));
    if ( rowtimeIdx >= 0 && item(currentRow(), 0)->data(Qt::UserRole + OFFSET_EDIT_STATE) == bsesNew )
        item(currentRow(), rowtimeIdx)->setText(QString::number(QDateTime::currentMSecsSinceEpoch()));
}

QStringList BsSheetGrid::getSqliteLimitKeyFields(const bool forNew)
{
    QStringList ls;
    if ( forNew )
        ls << QStringLiteral("parentid");
    else
        ls << QStringLiteral("parentid")  << QStringLiteral("rowtime");
    return ls;
}

QStringList BsSheetGrid::getSqliteLimitKeyValues(const int row, const bool forNew)
{
    QString sheetId = ( mSheetId > 0 ) ? QString::number(mSheetId) : mapMsg.value("app_sheetid_placeholer");
    int rowTimeCol = getColumnIndexByFieldName(QStringLiteral("rowtime"));

    QStringList ls;
    if ( forNew )
        ls << sheetId;
    else
        ls << sheetId  << item(row, rowTimeCol)->data(Qt::UserRole + OFFSET_OLD_VALUE).toString();
    return ls;
}



// BsSheetCargoGrid
BsSheetCargoGrid::BsSheetCargoGrid(QWidget *parent, const QString &table, const QList<BsField*> &flds)
    : BsSheetGrid(parent, table), mpDelegateCargo(nullptr), mpDelegateColor(nullptr)
{
    //mCols是getFieldByName()函数依据
    mTable = table;
    mCols << flds;

    //特别设置
    BsField *cargoFld = getFieldByName(QStringLiteral("cargo"));
    BsField *colorFld = getFieldByName(QStringLiteral("color"));
    mColorColIdx = getColumnIndexByFieldName(QStringLiteral("color"));
    Q_ASSERT( cargoFld && colorFld && mColorColIdx > 0 );

    mpDelegateCargo = new BsPickDelegate(this, cargoFld, dsCargo);
    mpDelegateColor = new BsPickDelegate(this, colorFld, dsColorList);
    setItemDelegateForColumn(0, mpDelegateCargo);
    setItemDelegateForColumn(mColorColIdx, mpDelegateColor);

    //货备注
    BsField *hpmarkFld = getFieldByName(QStringLiteral("hpmark"));
    if ( hpmarkFld )
    {
        QString editSet = mapOption.value("sheet_hpmark_editable");
        bool editable = ( editSet == mapMsg.value("word_yes") || editSet == QStringLiteral("yes") );
        if ( !editable )
            hpmarkFld->mFlags |= bsffReadOnly;
    }

    //引用列
    BsField *hpnameFld = getFieldByName(QStringLiteral("hpname"));
    if ( hpnameFld ) hpnameFld->mFlags |= (bsffReadOnly | bsffLookup);

    BsField *hpunitFld = getFieldByName(QStringLiteral("unit"));
    if ( hpunitFld ) hpunitFld->mFlags |= (bsffReadOnly | bsffLookup);

    BsField *hppriceFld = getFieldByName(QStringLiteral("setprice"));
    if ( hppriceFld ) hppriceFld->mFlags |= (bsffReadOnly | bsffLookup);

    //下拉数据加载
    dsCargo->reload();
    dsColorList->reload();
    dsSizerType->reload();

    //信号
    connect(this, SIGNAL(barcodeScanned(QString)), this, SLOT(scanBarocdeOneByOne(QString)));
    connect(mpFooter, SIGNAL(barcodeScanned(QString)), this, SLOT(scanBarocdeOneByOne(QString)));
}

BsSheetCargoGrid::~BsSheetCargoGrid()
{
    delete mpDelegateCargo;
    delete mpDelegateColor;
}

void BsSheetCargoGrid::setTraderDiscount(const double dis)
{
    mTraderDiscount = dis;
}

void BsSheetCargoGrid::setTraderName(const QString &traderName)
{
    mTraderName = traderName;
}

QString BsSheetCargoGrid::inputNewCargoRow(const QString &cargo, const QString &colorCodeOrName,
                                           const QString &sizerCodeOrName, const qint64 inputDataQty,
                                           const bool scanNotImport)
{
    //判断货号、颜色、尺码登记完整性
    QString colorType = dsCargo->getValue(cargo, QStringLiteral("colortype"));
    QString sizerType = dsCargo->getValue(cargo, QStringLiteral("sizertype"));

    QString colorName;
    QString sizerName;
    int sizerIndex;

    if ( scanNotImport )
    {
        if ( colorType.isEmpty() )
        {
            colorName = colorCodeOrName;
        }
        else
        {
            colorName = dsColorList->getColorByCodeInType(colorCodeOrName, colorType);
            if ( colorName.isEmpty() )
                return  mapMsg.value("i_cargo_has_no_colortype");
        }

        if ( sizerType.isEmpty() )
        {
            sizerIndex = 0;
            sizerName = mapMsg.value("mix_size_name");
        }
        else
        {
            sizerIndex = dsSizerType->getColIndexBySizerCode(sizerType, sizerCodeOrName);     //sizer is code, so byCode
            if ( sizerIndex < 0 )
                return  mapMsg.value("i_cargo_has_no_sizertype");

            sizerName = dsSizerType->getSizerNameByIndex(sizerType, sizerIndex);
            if ( sizerName.isEmpty() )
                return  mapMsg.value("i_cargo_has_no_sizertype");
        }
    }
    else
    {
        colorName = colorCodeOrName;

        sizerName = sizerCodeOrName;

        if ( sizerType.isEmpty() )
        {
            sizerIndex = 0;
        }
        else {
            sizerIndex = dsSizerType->getColIndexBySizerName(sizerType, sizerCodeOrName);     //sizer is name, so byName
            if ( sizerIndex < 0 )
                return  mapMsg.value("i_cargo_has_no_sizertype");
        }
    }

    //查找该货行
    int useRowIdx = -1;
    for ( int i = 0, iLen = rowCount(); i < iLen; ++i )
    {
        if ( !isRowHidden(i) && item(i, 0)->text() == cargo && item(i, mColorColIdx)->text() == colorName )
        {
            useRowIdx = i;
            break;
        }
    }

    //如果没找到，添加新行，并使用新行
    bool newRoww = false;
    if ( useRowIdx < 0 )
    {
        appendNewRow();
        useRowIdx = rowCount() - 1;
        newRoww = true;
    }

    //提交货号格
    QTableWidgetItem *itCargo = item(useRowIdx, 0);
    itCargo->setText(cargo);
    if ( itCargo->data(Qt::UserRole + OFFSET_EDIT_STATE).toInt() == bsesClean )
        itCargo->setData(Qt::UserRole + OFFSET_EDIT_STATE, bsesUpdated);    //上面appendNewRow()中有状态设置，这里决定不能加else

    //新行预备
    if ( newRoww ) {

        //色号预备
        readyColor(useRowIdx, cargo);

        //尺码准备
        readySizer(useRowIdx, cargo);

        //定价预备
        readyPrice(useRowIdx, cargo);

        //货备注
        readyHpRef(useRowIdx, cargo);
    }

    //提交色号格
    item(useRowIdx, mColorColIdx)->setText(colorName);

    //提交尺码数量格
    QTableWidgetItem *it = item(useRowIdx, mSizerPrevCol + 1 + sizerIndex);
    qint64 oldDataQty = bsNumForSave(it->text().toDouble()).toLongLong();
    qint64 newDataQty = oldDataQty + inputDataQty;
    it->setText(bsNumForRead(newDataQty, 0));
    it->setData(Qt::ToolTipRole, sizerName);
    updateHideSizersForSave(useRowIdx);

    //行重算
    recalcRow(useRowIdx, 0);    //第二参数只要不是金额列就行，随便。按折扣计算有精度损失，因此也不要折扣列。

    //表合计
    updateFooterSumCount(false);

    //外观
    updateRowColor(useRowIdx);

    //定位
    setCurrentCell(useRowIdx, mSizerPrevCol + mSizerColCount + 1);

    //OK
    return QString();
}

bool BsSheetCargoGrid::scanBarcode(const QString &barcode, QString *pCargo, QString *pColorCode, QString *pSizerCode)
{
    //识别
    bool validd = false;
    bool sizerMiddlee = false;
    QStringList captures;
    for ( int i = 0, iLen = vecBarcodeRule.length(); i < iLen; ++i )
    {
        QRegularExpression reg(QStringLiteral("^%1$").arg(vecBarcodeRule.at(i).first));
        QRegularExpressionMatch match = reg.match(barcode);
        if ( match.hasMatch() )
        {
            captures = match.capturedTexts();
            validd = true;
            sizerMiddlee = vecBarcodeRule.at(i).second;
            break;
        }
    }

    if ( !validd || captures.length() < 2 )
        return false;
    if ( captures.length() >= 4 ) {
        *pCargo = captures.at(1);
        *pColorCode = (sizerMiddlee) ? captures.at(3) : captures.at(2);
        *pSizerCode = (sizerMiddlee) ? captures.at(2) : captures.at(3);
    }
    else if ( captures.length() == 3 ) {
        if ( sizerMiddlee ) {
            *pCargo = captures.at(1);
            *pColorCode = QString();
            *pSizerCode = captures.at(2);
        } else {
            *pCargo = captures.at(1);
            *pColorCode = captures.at(2);
            *pSizerCode = QString();
        }
    }
    else {
        *pCargo = captures.at(1);
        *pColorCode = QString();
        *pSizerCode = QString();
    }

    //识别返回
    return true;
}

void BsSheetCargoGrid::uniteCargoColorPrice()  //合并同货同色同价行（工具箱）
{
    if ( rowCount() < 2 )
        return;

    int priceCol = getColumnIndexByFieldName(QStringLiteral("price"));
    QMap<QString, int>  mapRows;

    for ( int i = 0, iLen = rowCount(); i < iLen; ++i )     //升序（必须）
    {
        QTableWidgetItem *itCargo = item(i, 0);
        if ( itCargo->data(Qt::UserRole + OFFSET_EDIT_STATE) != bsesDeleted &&
             !itCargo->text().trimmed().isEmpty() )
        {
            QString compareStr = itCargo->text() + item(i, mColorColIdx)->text() + item(i, priceCol)->text();
            if ( ! mapRows.contains(compareStr) )
                mapRows.insert(compareStr, i);
        }
    }
    for ( int i = rowCount() - 1; i >= 0; --i )             //降序（必须）
    {
        QTableWidgetItem *itCargo = item(i, 0);
        if ( itCargo->data(Qt::UserRole + OFFSET_EDIT_STATE) != bsesDeleted )
        {
            QString compareStr = itCargo->text() + item(i, mColorColIdx)->text() + item(i, priceCol)->text();
            if ( mapRows.contains(compareStr) )
            {
                int toRow = mapRows.value(compareStr);
                if ( toRow != i )
                {
                    for ( int j = mSizerPrevCol + 1; j < mSizerPrevCol + mSizerColCount + 1; ++j )
                    {
                        double toQty = item(toRow, j)->text().toDouble();
                        double fromQty = item(i, j)->text().toDouble();
                        double resultQty = toQty + fromQty;
                        QString qtyTxt = (resultQty > 0.0001 || resultQty < -0.0001)
                                ? QString::number(resultQty, 'f', mCols.at(j)->mLenDots)
                                : QString();
                        item(toRow, j)->setText(qtyTxt);
                    }
                    recalcRow(toRow, 3);
                    removeRow(i);
                }
            }
        }
    }
}

QStringList BsSheetCargoGrid::getSizerNameListForPrint()
{
    QSet<QString> ssets;
    for ( int i = 0, iLen = rowCount(); i < iLen; ++i )
    {
        QStringList ls;
        for ( int j = mSizerPrevCol + 1; j <= mSizerPrevCol + mSizerColCount; ++j )
        {
            if ( ! isColumnHidden(j) )
                ls << item(i, j)->toolTip();
        }
        ssets.insert(ls.join(QChar(9)));
    }

    QStringList lst;
    QSetIterator<QString> i(ssets);
    while ( i.hasNext() )
        lst << i.next();

    return lst;
}

QStringList BsSheetCargoGrid::getSizerQtysOfRowForPrint(const int row, const bool printZeroQty)
{
    QStringList ls;
    for ( int i = mSizerPrevCol + 1; i <= mSizerPrevCol + mSizerColCount; ++i )
    {
        if ( ! isColumnHidden(i) )
        {
            QString sqty = ( row < rowCount() ) ? item(row, i)->text() : mpFooter->item(0, i)->text();

            if ( printZeroQty && sqty.isEmpty() )
                sqty = QStringLiteral("0");

            ls << sqty;
        }
    }
    return ls;
}

void BsSheetCargoGrid::tryLocateCargoRow(const QString &cargo, const QString &color)
{
    int found = -1;
    for ( int i = 0, iLen = rowCount(); i < iLen; ++i ) {
        QTableWidgetItem *itCargo = item(i, 0);
        QTableWidgetItem *itColor = item(i, 1);
        QString cargoText = (itCargo) ? itCargo->text() : QString();
        QString colorText = (itColor) ? itColor->text() : QString();
        if ( cargo == cargoText && color == colorText ) {
            found = i;
            break;
        }
    }

    if ( found < 0 ) {
        for ( int i = 0, iLen = rowCount(); i < iLen; ++i ) {
            QTableWidgetItem *itCargo = item(i, 0);
            QString cargoText = (itCargo) ? itCargo->text() : QString();
            if ( cargo == cargoText ) {
                found = i;
                break;
            }
        }
    }

    if ( found >= 0 ) {
        setCurrentCell(found, 2);
        scrollToItem(item(found, 2), QAbstractItemView::PositionAtCenter);
    }
}

QStringList BsSheetCargoGrid::getPostDataLines()
{
    mpBtnRow->hide();
    setCurrentCell(-1, -1);     //消除currentCellChanged()事件影响

    //尾部主列为空判断为空行，删除
    if ( item(rowCount() - 1, 0)->text().trimmed().isEmpty() )
        removeRow(rowCount() - 1);

    int idxCargo = getColumnIndexByFieldName(QStringLiteral("cargo"));
    int idxColor = getColumnIndexByFieldName(QStringLiteral("color"));
    int idxPrice = getColumnIndexByFieldName(QStringLiteral("price"));
    int idxRowmark = getColumnIndexByFieldName(QStringLiteral("rowmark"));

    //遍历行
    QStringList lines;
    for ( int i = 0, iLen = rowCount(); i < iLen; ++i )
    {
        QTableWidgetItem *itKey = item(i, 0);
        if ( itKey->data(Qt::UserRole + OFFSET_EDIT_STATE).toUInt() != bsesDeleted )
        {
            QString rowCargo = item(i, idxCargo)->text();
            QString rowColor = item(i, idxColor)->text();
            QString rowPrice = bsNumForSave(item(i, idxPrice)->text().toDouble());
            QString rowMark = item(i, idxRowmark)->text();

            //【reqBizInsert协议】与手机端一致，须每码一行，且字段顺序约定为cargo,color,sizer,qty,price,rowmark
            for ( int j = mSizerPrevCol + 1; j <= mSizerPrevCol + mSizerColCount; ++j )
            {
                QStringList cols;
                QTableWidgetItem *it = item(i, j);
                QString sname = it->toolTip();
                QString szqty = bsNumForSave(it->text().toDouble());
                if ( !sname.isEmpty() && szqty.toLongLong() != 0 )
                {
                    cols << rowCargo;
                    cols << rowColor;
                    cols << sname;
                    cols << szqty;
                    cols << rowPrice;
                    cols << ((j == mSizerPrevCol + 1) ? rowMark : QString());
                }
                if ( !cols.isEmpty() )
                {
                    lines << cols.join(QChar('\t'));
                }
            }
        }
    }

    //返回
    return lines;
}

void BsSheetCargoGrid::addOneCargo(const QString &cargo, const QString &colorName, const QString &sizerName)
{
    QString strErr = inputNewCargoRow(cargo, colorName, sizerName, 10000, false);
    if ( !strErr.isEmpty() ) {
        qApp->beep();
        QMessageBox::information(this, QString(), strErr);
    }
}

void BsSheetCargoGrid::autoHideNoQtySizerCol()
{
    for ( int i = mSizerPrevCol + 1; i <= mSizerPrevCol + mSizerColCount; ++i )
    {
        bool noQty = true;
        for ( int j = 0, jLen = rowCount(); j < jLen; ++j )
        {
            if ( item(j, i)->text().toInt() )
            {
                noQty = false;
                break;
            }
        }
        setColumnHidden(i, noQty);
    }
}

void BsSheetCargoGrid::commitData(QWidget *editor)
{
    //记录提交前
    QString txtBefore = currentItem()->text();

    //提交
    BsSheetGrid::commitData(editor);

    //没改变则先退出
    if ( currentItem()->text() == txtBefore )
        return;

    //货号检查
    if ( currentColumn() == 0 )
    {
        //有登记货号
        if ( commitCheckCargo(currentItem() ) )
        {
            //色号预备
            readyColor(currentRow(), currentItem()->text());

            //尺码准备
            readySizer(currentRow(), currentItem()->text());

            //定价预备
            readyPrice(currentRow(), currentItem()->text());

            //货备注
            readyHpRef(currentRow(), currentItem()->text());
        }
        //未登记货号
        else
        {
            //提示条码扫描提示
            if ( currentItem()->text().length() > 7 )
                emit shootForceMessage(mapMsg.value("i_barcode_scan_tip"));

            //不需继续
            return;
        }
    }

    //色号检查
    if ( currentColumn() == mColorColIdx )
        commitCheckColor(currentItem());

    //尺码预备
    if ( mCols.at(currentColumn())->mFlags & bsffSizeUnit )
        commitCheckSizer(currentItem());

    //行重算
    recalcRow(currentRow(), currentColumn());

    //表合计
    updateFooterSumCount(false);
}

void BsSheetCargoGrid::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    BsGrid::currentChanged(current, previous);
    if ( isVisible() && hasFocus() && current.isValid() ) {
        QTableWidgetItem *itCargo = item(current.row(), 0);
        QTableWidgetItem *itColor = item(current.row(), 1);
        QString cargo = (itCargo) ? itCargo->text() : QString();
        QString color = (itColor) ? itColor->text() : QString();

        if ( previous.row() != current.row() ) {
            QString colorType = dsCargo->getValue(cargo, QStringLiteral("colortype"));
            dsColorList->setFilterByCargoType(colorType);
        }

        if ( !cargo.isEmpty() ) {
            emit cargoRowSelected(cargo, color);
        }
    }
}

void BsSheetCargoGrid::scanBarocdeOneByOne(const QString &barcode)
{
    QString cargo;
    QString colorCode;
    QString sizerCode;
    if ( scanBarcode(barcode, &cargo, &colorCode, &sizerCode) )
    {
        QString strErr = inputNewCargoRow(cargo, colorCode, sizerCode, 10000, true);
        if ( !strErr.isEmpty() ) {
            qApp->beep();
            QMessageBox::information(this, QString(), strErr);
        }
    }
    else {
        qApp->beep();
        QMessageBox::information(this, QString(), mapMsg.value("i_invalid_barcode"));
    }
}

bool BsSheetCargoGrid::commitCheckCargo(QTableWidgetItem *it)
{
    bool foundd = dsCargo->foundKey(it->text());
    if ( foundd )
    {
        it->setData(Qt::ToolTipRole, QString());
        it->setData(Qt::DecorationRole, QVariant());
        it->setData(Qt::UserRole + OFFSET_CELL_CHECK, 0);
    }
    else
    {
        it->setData(Qt::ToolTipRole, mapMsg.value("i_cargo_not_registered"));
        it->setData(Qt::DecorationRole, QIcon(":/icon/warning.png"));
        it->setData(Qt::UserRole + OFFSET_CELL_CHECK, bsccWarning);
    }

    return foundd;
}

void BsSheetCargoGrid::commitCheckColor(QTableWidgetItem *it)
{
    QString cargo = item(it->row(), 0)->text();
    if ( dsCargo->foundKey(cargo) )
    {
        QString colorType = dsCargo->getValue(cargo, QStringLiteral("colortype"));
        if ( dsColorList->foundColorInType(it->text(), colorType) )
        {
            it->setData(Qt::ToolTipRole, QString());
            it->setData(Qt::DecorationRole, QVariant());
            it->setData(Qt::UserRole + OFFSET_CELL_CHECK, 0);
        }
        else
        {
            it->setData(Qt::ToolTipRole, mapMsg.value("i_color_not_found_by_cargo"));
            it->setData(Qt::DecorationRole, QIcon(":/icon/warning.png"));
            it->setData(Qt::UserRole + OFFSET_CELL_CHECK, bsccWarning);
        }
    }
    else
    {
        it->setData(Qt::ToolTipRole, mapMsg.value("i_unknown_color_of_unknow_cargo"));
        it->setData(Qt::DecorationRole, QIcon(":/icon/warning.png"));
        it->setData(Qt::UserRole + OFFSET_CELL_CHECK, bsccWarning);
    }
}

void BsSheetCargoGrid::commitCheckSizer(QTableWidgetItem *it)
{
    //更新待存尺码数据
    updateHideSizersForSave(it->row());

    //清除qty列可能有的坏尺码警告（但不清除ToolTip，因为用户可能要对照移录）
    int qtyIdx = getColumnIndexByFieldName(QStringLiteral("qty"));
    QTableWidgetItem *itQty = item(it->row(), qtyIdx);
    itQty->setData(Qt::DecorationRole, QVariant());
    itQty->setData(Qt::UserRole + OFFSET_CELL_CHECK, 0);
}

void BsSheetCargoGrid::readyColor(const int row, const QString &cargo)
{
    QTableWidgetItem *itColor = item(row, mColorColIdx);
    QString colorType = dsCargo->getValue(cargo, QStringLiteral("colortype"));
    dsColorList->setFilterByCargoType(colorType);
    if ( mppWin && mppWin->getOptValueByOptName(QStringLiteral("opt_auto_use_first_color"))
         && !colorType.isEmpty() && dsColorList->rowCount() > 0 ) {
        itColor->setText(dsColorList->getFirstColorByType(colorType));
    }
}

void BsSheetCargoGrid::readySizer(const int row, const QString &cargo)
{
    QString regType = dsCargo->getValue(cargo, QStringLiteral("sizertype"));
    QStringList regList = dsSizerType->getSizerList(regType);

    //增加列定义
    while ( mSizerColCount < regList.count() )
    {
        //sz01开始的命名方式约定，不要变
        mSizerColCount++;
        BsField *fld = new BsField(QStringLiteral("sz%1").arg(mSizerColCount), QStringLiteral("*"),
                                   bsffNumeric | bsffAggSum | bsffSizeUnit, 0, QString());
        mCols.insert(mSizerPrevCol + mSizerColCount, fld);
        insertColumn(mSizerPrevCol + mSizerColCount);
        for ( int i = 0, iLen = rowCount(); i < iLen - 1; ++i )     //-1最后新行不要包括
        {
            QTableWidgetItem *it = new QTableWidgetItem();
            it->setFlags(Qt::ItemIsSelectable);
            it->setBackground(QColor(244, 244, 244));
            setItem(i, mSizerPrevCol + mSizerColCount, it);
        }
        setColumnWidth(mSizerPrevCol + mSizerColCount, 50);
    }
    mpFooter->initCols();

    //初始化尺码格
    for ( int i = mSizerPrevCol + 1; i <= mSizerPrevCol + mSizerColCount; ++i )
    {
        QTableWidgetItem *it = item(row, i);

        //先清空，后补（因为有可能是扩张后的列又重新换少尺码的货号了。这在新行是允许的。
        if ( it )
        {
            it->setText(QString());
            it->setData(Qt::ToolTipRole, QString());
        }
        else
        {
            setItem(row, i, new BsGridItem(QString(), SORT_TYPE_NUM));
            it = item(row, i);
        }

        //补格式
        if ( i - mSizerPrevCol <= regList.length() )
        {
            it->setData(Qt::ToolTipRole, regList.at(i - mSizerPrevCol - 1));
            it->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
        else
        {
            it->setFlags(Qt::ItemIsSelectable);
            it->setBackground(QColor(244, 244, 244));
        }
    }

    //更新列头
    updateAllColTitles();

    //更新尺码头
    updateSizerColTitles(row);
}

void BsSheetCargoGrid::readyPrice(const int row, const QString &cargo)
{
    double applyDiscount = mTraderDiscount;

    //检查特例价格政策
    if ( (mTable.startsWith(QStringLiteral("pf")) || mTable == QStringLiteral("lsd")) &&
         !mTraderName.isEmpty() ) {

        for ( int i = 0, iLen = pricePolicies.length(); i < iLen; ++i ) {

            BsPolicy *pol = pricePolicies.at(i);

            QRegExp *traderExp = new QRegExp("^" + pol->mTraderExp + "$", Qt::CaseInsensitive);
            QRegExp *cargoExp = new QRegExp("^" + pol->mCargoExp + "$", Qt::CaseInsensitive);

            if ( pol->mTraderExp.isEmpty() ||
                 traderExp->exactMatch(mTraderName.toUpper()) ||
                 traderExp->exactMatch(mTraderName.toLower()) ) {

                if ( pol->mCargoExp.isEmpty() ||
                     cargoExp->exactMatch(cargo.toUpper()) ||
                     cargoExp->exactMatch(cargo.toLower()) ) {

                    applyDiscount = pol->mPolicyDis;
                    break;
                }
            }
        }
    }

    QString usePriceName;
    if ( mTable.startsWith(QStringLiteral("cg")) )
        usePriceName = QStringLiteral("buyprice");
    else if ( mTable.startsWith(QStringLiteral("pf")) )
        usePriceName = QStringLiteral("lotprice");
    else if ( mTable.startsWith(QStringLiteral("ls")) )
        usePriceName = QStringLiteral("retprice");
    else
        usePriceName = QStringLiteral("setprice");

    double usePrice = applyDiscount * (dsCargo->getValue(cargo, usePriceName).toLongLong() / 10000.0);
    double setPrice = dsCargo->getValue(cargo, QStringLiteral("setprice")).toLongLong() / 10000.0;
    int priceCol = getColumnIndexByFieldName(QStringLiteral("price"));
    int discountCol = getColumnIndexByFieldName(QStringLiteral("discount"));
    double discount = (setPrice > 0.0001 || setPrice < -0.0001) ? usePrice / setPrice : 0.0;
    item(row, priceCol)->setText(QString::number(usePrice, 'f', mPriceDots));
    item(row, discountCol)->setText(QString::number(discount, 'f', mDiscDots));
}

void BsSheetCargoGrid::readyHpRef(const int row, const QString &cargo)
{
    int hpMarkNum = mapOption.value("sheet_hpmark_define").toInt();
    if ( hpMarkNum > 0 && hpMarkNum < 7 )
    {
        QString hpMark = dsCargo->getValue(cargo, QStringLiteral("attr%1").arg(hpMarkNum));
        int hpmarkCol = getColumnIndexByFieldName(QStringLiteral("hpmark"));
        if ( hpmarkCol > 0 ) item(row, hpmarkCol)->setText(hpMark);
    }

    int hpnameCol = getColumnIndexByFieldName(QStringLiteral("hpname"));
    if ( hpnameCol > 0 ) {
        QString hpname = dsCargo->getValue(cargo, QStringLiteral("hpname"));
        item(row, hpnameCol)->setText(hpname);
    }

    int unitCol = getColumnIndexByFieldName(QStringLiteral("unit"));
    if ( unitCol > 0 ) {
        QString unit = dsCargo->getValue(cargo, QStringLiteral("unit"));
        item(row, unitCol)->setText(unit);
    }

    int setpriceCol = getColumnIndexByFieldName(QStringLiteral("setprice"));
    if ( setpriceCol > 0 ) {
        qint64 iSetprice = dsCargo->getValue(cargo, QStringLiteral("setprice")).toLongLong();
        int moneyDots = mapOption.value("dots_of_money").toInt();
        item(row, setpriceCol)->setText(QString::number(iSetprice / 10000.0, 'f', moneyDots));
    }
}

void BsSheetCargoGrid::checkShrinkSizeColCountForNewCargoCancel(const int row)
{
    Q_UNUSED(row)
    //收缩过于动感，影响体验，暂时不用。问题在于还要考虑删除行、删除警告bad尺码数量这两情况，
    //如果都动态收缩，那么还要考虑取消恢复……
    //反正重新打开单据，总是没有多余列，因此，动态收缩功能不设计也罢。
    //用户体会了，也就习惯了，由于这些情况都不常见，所以基本不太影响操作。
}

void BsSheetCargoGrid::recalcRow(const int row, const int byColIndex)
{
    int idxQtyCol       = getColumnIndexByFieldName(QStringLiteral("qty"));
    int idxDiscountCol  = getColumnIndexByFieldName(QStringLiteral("discount"));
    int idxPriceCol     = getColumnIndexByFieldName(QStringLiteral("price"));
    int idxActmoneyCol  = getColumnIndexByFieldName(QStringLiteral("actmoney"));
    int idxDismoneyCol  = getColumnIndexByFieldName(QStringLiteral("dismoney"));

    QString hpcode = item(row, 0)->text();
    double setPrice = dsCargo->getValue(hpcode, QStringLiteral("setprice")).toLongLong() / 10000.0;

    //数量暂按整数，以后用户多了且有需求反映时再改double
    qint64 qty = 0;
    for ( int i = mSizerPrevCol + 1; i < mSizerPrevCol + mSizerColCount + 1; ++i ) {
        qty += item(row, i)->text().toLongLong();
    }

    double discount = abs(item(row, idxDiscountCol)->text().toDouble());
    double price = abs(item(row, idxPriceCol)->text().toDouble());
    double actMoney = item(row, idxActmoneyCol)->text().toDouble();
    actMoney = ( qty > 0 ) ? abs(actMoney) : (0 - abs(actMoney));

    if ( qty != 0 )
    {
        if ( byColIndex == idxActmoneyCol )
        {
            price = actMoney / qty;
            discount = ( setPrice > 0.000001 || setPrice < -0.000001 ) ? price / setPrice : 0.0;
        }
        else if ( byColIndex == idxDiscountCol )
        {
            price = discount * setPrice;
            actMoney = price * qty;
        }
        else
        {
            discount = ( setPrice > 0.000001 || setPrice < -0.000001 ) ? price / setPrice : 0.0;
            actMoney = price * qty;
        }
    }
    else
    {
        actMoney = 0;
    }

    item(row, idxQtyCol)->setText(QString::number(qty));
    item(row, idxDiscountCol)->setText(QString::number(discount, 'f', mDiscDots));
    item(row, idxPriceCol)->setText(QString::number(price, 'f', mPriceDots));
    item(row, idxActmoneyCol)->setText(QString::number(actMoney, 'f', mMoneyDots));
    item(row, idxDismoneyCol)->setText(QString::number(setPrice * qty - actMoney, 'f', mMoneyDots));
}

void BsSheetCargoGrid::updateHideSizersForSave(const int row)
{
    QStringList sizers;
    for ( int i = mSizerPrevCol + 1; i <= mSizerPrevCol + mSizerColCount; ++i )
    {
        QTableWidgetItem *it = item(row, i);
        QString sname = it->toolTip();
        QString saveQty = bsNumForSave(it->text().toDouble());
        if ( !sname.isEmpty() && saveQty.toLongLong() != 0 )
            sizers << QStringLiteral("%1\t%2").arg(sname).arg(saveQty);
    }
    item(row, getDataSizerColumnIdx())->setText(sizers.join(QChar(10)));
}


}
