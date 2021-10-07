#include "bssetdeskloginer.h"
#include "main/bailicode.h"
#include "main/bailidata.h"

//静态变量
namespace BailiSoft {

enum BsRightModelType {bsrmtRegis, bsrmtSheet, bsrmtQuery};

//权限名
QStringList              lstDeskRegisRightFlagCNames;
QStringList              lstDeskSheetRightFlagCNames;
QStringList              lstDeskQueryRightFlagCNames;


//以下序号设计必须与bswins单元中bsRightXXX三枚举定义完全一致，并且因为数据库存储，序号不能再变了。
void initDeskRightFlagCNames()
{
    lstDeskRegisRightFlagCNames.clear();
    lstDeskRegisRightFlagCNames << QStringLiteral("查看")
                            << QStringLiteral("新增")
                            << QStringLiteral("更改")
                            << QStringLiteral("删除")
                            << QStringLiteral("导出");

    lstDeskSheetRightFlagCNames.clear();
    lstDeskSheetRightFlagCNames << QStringLiteral("查看")
                            << QStringLiteral("新增")
                            << QStringLiteral("更改")
                            << QStringLiteral("删除")
                            << QStringLiteral("审核")
                            << QStringLiteral("打印")
                            << QStringLiteral("导出");

    lstDeskQueryRightFlagCNames.clear();
    lstDeskQueryRightFlagCNames << QStringLiteral("数量")
                            << QStringLiteral("金额")
                            << QStringLiteral("折扣")
                            << QStringLiteral("收付")
                            << QStringLiteral("欠款")
                            << QStringLiteral("打印")
                            << QStringLiteral("导出");
}

//二底幂函数
uint deskpowtwo(const int n)
{
    uint rst = 1;
   for ( int i = 0; i < n; ++i ) {
       rst *= 2;
   }
   return rst;
}

}


// BsSetDeskLoginer
namespace BailiSoft {

BsSetDeskLoginer::BsSetDeskLoginer(QWidget *parent) : QDialog(parent)
{
    //初始化静态变量
    initDeskRightFlagCNames();

    //数据模型
    mpModelRegis = new BsDeskRightModel(bsrmtRegis, this);
    mpModelSheet = new BsDeskRightModel(bsrmtSheet, this);
    mpModelQuery = new BsDeskRightModel(bsrmtQuery, this);

    //左功能
    mpBtnAddLoginer = new QPushButton(QStringLiteral("添加用户"), this);
    mpBtnAddLoginer->setFixedWidth(100);

    //左列表
    mpList = new BsDeskUserList(this);
    mpList->setFixedWidth(200);

    //左布局
    mpLeft = new QWidget(this);
    QVBoxLayout *layLeft = new QVBoxLayout(mpLeft);
    layLeft->addWidget(mpBtnAddLoginer, 0, Qt::AlignCenter);
    layLeft->addWidget(mpList, 1);

    //右头
    mpCurrentLoginer = new QLabel(this);
    mpCurrentLoginer->setAlignment(Qt::AlignCenter);

    //右页————角色
    mpLoginRole = new QComboBox(this);
    mpLoginRole->setMinimumWidth(200);
    mpLoginRole->setMaximumWidth(300);

    mpRetPrice = new QCheckBox(QStringLiteral("开放零售价"), this);

    mpLotPrice = new QCheckBox(QStringLiteral("开放批发价"), this);

    mpBuyPrice = new QCheckBox(QStringLiteral("开放进货价"), this);

    mpRole = new QWidget(this);
    QVBoxLayout *layRole = new QVBoxLayout(mpRole);
    layRole->addStretch(3);
    layRole->addWidget(mpLoginRole, 0, Qt::AlignCenter);
    layRole->addStretch(1);
    layRole->addWidget(mpRetPrice, 0, Qt::AlignCenter);
    layRole->addWidget(mpLotPrice, 0, Qt::AlignCenter);
    layRole->addWidget(mpBuyPrice, 0, Qt::AlignCenter);
    layRole->addStretch(3);

    //右页————登记权限
    mpViewRegis = new BsDeskRightView(this);
    mpViewRegis->setModel(mpModelRegis);
    mpViewRegis->setItemDelegateForColumn(0, new BsDeskRightDelegate(mpViewRegis, lstDeskRegisRightFlagCNames));

    //右页————单据权限
    mpViewSheet = new BsDeskRightView(this);
    mpViewSheet->setModel(mpModelSheet);
    mpViewSheet->setItemDelegateForColumn(0, new BsDeskRightDelegate(mpViewSheet, lstDeskSheetRightFlagCNames));

    //右页————查询权限
    mpViewQuery = new BsDeskRightView(this);
    mpViewQuery->setModel(mpModelQuery);
    mpViewQuery->setItemDelegateForColumn(0, new BsDeskRightDelegate(mpViewQuery, lstDeskQueryRightFlagCNames));
    for ( int i = 0, iLen = lstQueryWinTableNames.length(); i < iLen; ++i ) {
        QString wintname = lstQueryWinTableNames.at(i);
        if ( wintname == QStringLiteral("viall") )
            mpViewQuery->setItemDelegateForRow(i, new BsDeskRightDelegate(mpViewQuery,
                                                                      lstDeskQueryRightFlagCNames, true, true, true));
        else if ( wintname.endsWith(QStringLiteral("rest")) )
            mpViewQuery->setItemDelegateForRow(i, new BsDeskRightDelegate(mpViewQuery,
                                                                      lstDeskQueryRightFlagCNames, false, true, true));
        else if ( !wintname.endsWith(QStringLiteral("cash")) )
            mpViewQuery->setItemDelegateForRow(i, new BsDeskRightDelegate(mpViewQuery,
                                                                      lstDeskQueryRightFlagCNames, false, false, true));
    }

    //右页控件
    mpTab = new QTabWidget(this);
    mpTab->setMinimumSize(660, 500);
    mpTab->addTab(mpRole, QStringLiteral("角色前提"));
    mpTab->addTab(mpViewRegis, QStringLiteral("登记权限"));
    mpTab->addTab(mpViewSheet, QStringLiteral("单据权限"));
    mpTab->addTab(mpViewQuery, QStringLiteral("查询权限"));

    //右尾
    mpBtnSaveChange = new QPushButton(mapMsg.value("btn_apply"), this);
    mpBtnCancelChange = new QPushButton(mapMsg.value("btn_cancel"), this);

    mpSaveBox = new QWidget(this);
    QHBoxLayout *laySave = new QHBoxLayout(mpSaveBox);
    laySave->addStretch();
    laySave->addWidget(mpBtnSaveChange);
    laySave->addWidget(mpBtnCancelChange);
    laySave->addStretch();

    //右布局
    mpRight = new QWidget(this);
    QVBoxLayout *layRight = new QVBoxLayout(mpRight);
    layRight->addWidget(mpCurrentLoginer);
    layRight->addWidget(mpTab, 1);
    layRight->addWidget(mpSaveBox);

    //总布局
    QHBoxLayout *layWin = new QHBoxLayout(this);
    layWin->addWidget(mpLeft);
    layWin->addWidget(mpRight, 1);
    setWindowFlags(windowFlags() &~ Qt::WindowContextHelpButtonHint);

    //加载
    connect(mpBtnAddLoginer, SIGNAL(clicked(bool)), this, SLOT(addLoginer()));
    connect(mpBtnSaveChange, SIGNAL(clicked(bool)), this, SLOT(saveChange()));
    connect(mpBtnCancelChange, SIGNAL(clicked(bool)), this, SLOT(cancelChange()));
    connect(mpList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(pickLoginerChanged(QListWidgetItem*,QListWidgetItem*)));

    mpLoginRole->addItem(QStringLiteral("★总部管理★"), QString());
    QSqlQuery qry;
    qry.setForwardOnly(true);
    qry.exec(QStringLiteral("select kname from shop order by kname;"));
    while ( qry.next() ) {
        //此绑定“%1”格式要与setCurrentText()处一致
        mpLoginRole->addItem(QStringLiteral("绑定“%1”").arg(qry.value(0).toString()), qry.value(0).toString());
    }
    qry.exec(QStringLiteral("select loginer from baililoginer "
                            "where loginer<>'%1' and loginer<>'%2' "
                            "order by loginer;")
             .arg(mapMsg.value("word_boss"))
             .arg(mapMsg.value("word_admin")));
    while ( qry.next() ) {
        mpList->addItem(qry.value(0).toString());
    }
    qry.finish();

    mpSaveBox->hide();
}

void BsSetDeskLoginer::addLoginer()
{
    QInputDialog dlg(this);
    dlg.setWindowTitle(QStringLiteral("添加登录用户"));
    dlg.setLabelText(QStringLiteral("新用户名："));
    dlg.setOkButtonText(mapMsg.value("btn_ok"));
    dlg.setCancelButtonText(mapMsg.value("btn_cancel"));
    dlg.setWindowFlags(dlg.windowFlags() &~ Qt::WindowContextHelpButtonHint);
    dlg.setMinimumSize(400, 200);
    if ( dlg.exec() != QDialog::Accepted )
        return;
    if ( dlg.textValue().isEmpty() )
        return;

    QSqlQuery qry;
    QString sql = QStringLiteral("insert into baililoginer(loginer) values('%1');")
            .arg(dlg.textValue().replace(QChar(39), QString()));
    qry.exec(sql);

    if ( !qry.lastError().isValid() ) {
        mpList->addItem(dlg.textValue());
        mpList->setCurrentRow(mpList->count() - 1);
        loadLoginer(dlg.textValue());
    }
    else {
        QMessageBox::information(this, QString(), qry.lastError().text());
    }
}

void BsSetDeskLoginer::delLoginer()
{
    QListWidgetItem *it = mpList->currentItem();
    if ( !it )
        return;

    QSqlQuery qry;
    QString sql = QStringLiteral("delete from baililoginer where loginer='%1';").arg(it->text());
    qry.exec(sql);

    if ( !qry.lastError().isValid() ) {
        mpList->takeItem(mpList->currentRow());
        delete it;
        mpModelRegis->clearData();
        mpModelSheet->clearData();
        mpModelQuery->clearData();
    }
    else {
        QMessageBox::information(this, QString(), qry.lastError().text());
    }
}

void BsSetDeskLoginer::clearPassword()
{
    QSqlQuery qry;
    qry.exec(QStringLiteral("update baililoginer set deskpassword='' where loginer='%1';")
             .arg(mpList->currentItem()->text()));
    if ( !qry.lastError().isValid() )
        QMessageBox::information(this, QString(), QStringLiteral("“%1”的密码已经清除成功！")
                                 .arg(mpList->currentItem()->text()));
    else
        QMessageBox::information(this, QString(), QStringLiteral("“%1”的密码清除失败！")
                                 .arg(mpList->currentItem()->text()));
}

void BsSetDeskLoginer::presetMinRight()
{
    presetSaveRight(false);
    mpList->currentItem()->setForeground(QColor(0, 0, 0));
    mpList->currentItem()->setData(Qt::UserRole, 0);
    loadLoginer(mpList->currentItem()->text());
}

void BsSetDeskLoginer::presetMaxRight()
{
    presetSaveRight(true);
    mpList->currentItem()->setForeground(QColor(0, 0, 0));
    mpList->currentItem()->setData(Qt::UserRole, 0);
    loadLoginer(mpList->currentItem()->text());
}

void BsSetDeskLoginer::saveChange()
{
    QStringList setExps;

    QString bindShop = (mpLoginRole->currentIndex() > 0) ? mpLoginRole->currentData(Qt::UserRole).toString() : QString();
    setExps << QStringLiteral("bindshop='%1'").arg(bindShop);

    setExps << QStringLiteral("retprice=%1").arg((mpRetPrice->isChecked()) ? -1 : 0);
    setExps << QStringLiteral("lotprice=%1").arg((mpLotPrice->isChecked()) ? -1 : 0);
    setExps << QStringLiteral("buyprice=%1").arg((mpBuyPrice->isChecked()) ? -1 : 0);

    for ( int i = 0, iLen = lstRegisWinTableNames.length(); i < iLen; ++i ) {
        QModelIndex dataIdx = mpModelRegis->index(i, 0);
        setExps << QStringLiteral("%1=%2").arg(lstRegisWinTableNames.at(i))
                   .arg(mpModelRegis->data(dataIdx, Qt::EditRole).toUInt());
    }

    for ( int i = 0, iLen = lstSheetWinTableNames.length(); i < iLen; ++i ) {
        QModelIndex dataIdx = mpModelSheet->index(i, 0);
        setExps << QStringLiteral("%1=%2").arg(lstSheetWinTableNames.at(i))
                   .arg(mpModelSheet->data(dataIdx, Qt::EditRole).toUInt());
    }

    for ( int i = 0, iLen = lstQueryWinTableNames.length(); i < iLen; ++i ) {
        QModelIndex dataIdx = mpModelQuery->index(i, 0);
        setExps << QStringLiteral("%1=%2").arg(lstQueryWinTableNames.at(i))
                   .arg(mpModelQuery->data(dataIdx, Qt::EditRole).toUInt());
    }

    QString sql = QStringLiteral("update baililoginer set %1 where loginer='%2';")
            .arg(setExps.join(QChar(44))).arg(mpList->currentItem()->text());
    QSqlQuery qry;
    qry.exec(sql);
    if ( qry.lastError().isValid() ) {
        qDebug() << qry.lastError().text() << "\n" << sql;
    }
    else {
        mpList->currentItem()->setForeground(QColor(0, 0, 0));
        mpList->currentItem()->setData(Qt::UserRole, 0);
        mpSaveBox->hide();
    }
}

void BsSetDeskLoginer::cancelChange()
{
    mpList->currentItem()->setForeground(QColor(0, 0, 0));
    mpList->currentItem()->setData(Qt::UserRole, 0);
    mpSaveBox->hide();

    loadLoginer(mpList->currentItem()->text());
}

void BsSetDeskLoginer::pickLoginerChanged(QListWidgetItem *current, QListWidgetItem *)
{
    if ( current )
        loadLoginer(current->text());
}

void BsSetDeskLoginer::rightChanged()
{
    if ( mpList->currentItem() ) {
        mpList->currentItem()->setForeground(QColor(255, 0, 0));
        mpList->currentItem()->setData(Qt::UserRole, 1);
        mpSaveBox->show();
    }
}

void BsSetDeskLoginer::loadLoginer(const QString &loginer)
{
    disconnect(mpLoginRole, 0, 0, 0);
    disconnect(mpRetPrice, 0, 0, 0);
    disconnect(mpLotPrice, 0, 0, 0);
    disconnect(mpBuyPrice, 0, 0, 0);
    disconnect(mpModelRegis, 0, 0, 0);
    disconnect(mpModelSheet, 0, 0, 0);
    disconnect(mpModelQuery, 0, 0, 0);

    mpCurrentLoginer->setText(QStringLiteral("<b>%1</b><font color='#666'> 的权限：</font>").arg(loginer));
    mpLoginRole->setCurrentIndex(0);

    QSqlQuery qry;
    QString sql = QStringLiteral("select deskpassword, bindshop, retprice, lotprice, buyprice, "
                                 "%1, %2, %3 from baililoginer where loginer='%4';")
            .arg(lstRegisWinTableNames.join(QChar(44)))
            .arg(lstSheetWinTableNames.join(QChar(44)))
            .arg(lstQueryWinTableNames.join(QChar(44)))
            .arg(loginer);
    qry.exec(sql);
    if ( qry.lastError().isValid() ) qDebug() << qry.lastError().text() << "\n" << sql;
    qry.next();

    mpLoginRole->setCurrentText(QStringLiteral("绑定“%1”").arg(qry.value(1).toString()));     //与addItem()处要一致
    mpRetPrice->setChecked(qry.value(2).toBool());
    mpLotPrice->setChecked(qry.value(3).toBool());
    mpBuyPrice->setChecked(qry.value(4).toBool());

    int idxBase = 5;
    QList<uint> regisDatas;
    for ( int i = idxBase; i < idxBase + lstRegisWinTableNames.length(); ++i )
        regisDatas << qry.value(i).toUInt();
    mpModelRegis->loadData(regisDatas);

    idxBase += lstRegisWinTableNames.length();
    QList<uint> sheetDatas;
    for ( int i = idxBase; i < idxBase + lstSheetWinTableNames.length(); ++i )
        sheetDatas << qry.value(i).toUInt();
    mpModelSheet->loadData(sheetDatas);

    idxBase += lstSheetWinTableNames.length();
    QList<uint> queryDatas;
    for ( int i = idxBase; i < idxBase + lstQueryWinTableNames.length(); ++i )
        queryDatas << qry.value(i).toUInt();
    mpModelQuery->loadData(queryDatas);

    mpTab->setCurrentIndex(0);
    mpSaveBox->setVisible(mpList->currentItem()->data(Qt::UserRole).toInt() > 0);

    connect(mpLoginRole, SIGNAL(currentTextChanged(QString)), this, SLOT(rightChanged()));
    connect(mpRetPrice, SIGNAL(clicked(bool)), this, SLOT(rightChanged()));
    connect(mpLotPrice, SIGNAL(clicked(bool)), this, SLOT(rightChanged()));
    connect(mpBuyPrice, SIGNAL(clicked(bool)), this, SLOT(rightChanged()));
    connect(mpModelRegis, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(rightChanged()));
    connect(mpModelSheet, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(rightChanged()));
    connect(mpModelQuery, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(rightChanged()));
}

void BsSetDeskLoginer::presetSaveRight(const bool maxx)
{
    QStringList setExps;

    setExps << QStringLiteral("bindshop='%1'").arg((maxx) ? QString() : QStringLiteral("NOTEXISTSANYSTR"));

    setExps << QStringLiteral("retprice=%1").arg((maxx) ? -1 : 0);
    setExps << QStringLiteral("lotprice=%1").arg((maxx) ? -1 : 0);
    setExps << QStringLiteral("buyprice=%1").arg((maxx) ? -1 : 0);

    for ( int i = 0, iLen = lstRegisWinTableNames.length(); i < iLen; ++i ) {
        setExps << QStringLiteral("%1=%2").arg(lstRegisWinTableNames.at(i))
                   .arg((maxx) ? 0x0fffffff : 0);
    }

    for ( int i = 0, iLen = lstSheetWinTableNames.length(); i < iLen; ++i ) {
        setExps << QStringLiteral("%1=%2").arg(lstSheetWinTableNames.at(i))
                   .arg((maxx) ? 0x0fffffff : 0);
    }

    for ( int i = 0, iLen = lstQueryWinTableNames.length(); i < iLen; ++i ) {
        setExps << QStringLiteral("%1=%2").arg(lstQueryWinTableNames.at(i))
                   .arg((maxx) ? 0x0fffffff : 0);
    }

    QString sql = QStringLiteral("update baililoginer set %1 where loginer='%2';")
            .arg(setExps.join(QChar(44))).arg(mpList->currentItem()->text());
    QSqlQuery qry;
    qry.exec(sql);
}

}


// BsDeskUserList
namespace BailiSoft {

BsDeskUserList::BsDeskUserList(BsSetDeskLoginer *parent) : QListWidget(parent), mppWin(parent)
{
    mpMenu = new QMenu(this);
    mpAcAddLoginer  = mpMenu->addAction(QStringLiteral("添加用户"), mppWin, SLOT(addLoginer()));
    mpAcDelLoginer  = mpMenu->addAction(QStringLiteral("删除用户"), mppWin, SLOT(delLoginer()));
    mpAcResetPwd    = mpMenu->addAction(QStringLiteral("清除密码"), mppWin, SLOT(clearPassword()));
    mpAcPresetMin   = mpMenu->addAction(QStringLiteral("预设最小权限"), mppWin, SLOT(presetMinRight()));
    mpAcPresetMax   = mpMenu->addAction(QStringLiteral("预设最大权限"), mppWin, SLOT(presetMaxRight()));
}

void BsDeskUserList::mousePressEvent(QMouseEvent *e)
{
    QListWidget::mousePressEvent(e);

    if ( e->button() != Qt::RightButton )
        return;

    QPoint pt = viewport()->mapFromGlobal(e->globalPos());
    QListWidgetItem *it = itemAt(pt);
    if ( it == currentItem() ) {
        mpAcAddLoginer->setEnabled(true);
        mpAcDelLoginer->setEnabled(currentItem());
        mpAcResetPwd->setEnabled(currentItem());
        mpAcPresetMin->setEnabled(currentItem());
        mpAcPresetMax->setEnabled(currentItem());
        mpMenu->popup(e->globalPos());
    }
}

void BsDeskUserList::paintEvent(QPaintEvent *e)
{
    QListWidget::paintEvent(e);
    if ( count() <= 10 ) {
        QPainter p(viewport());
        p.setPen(QColor(160, 160, 160));
        p.drawText(viewport()->rect(), Qt::AlignHCenter | Qt::AlignBottom,
                   QStringLiteral("更多操作请鼠标右击用户…"));
    }
}

}


// BsDeskRightModel
namespace BailiSoft {

BsDeskRightModel::BsDeskRightModel(const uint rmType, QObject *parent)
    : QAbstractTableModel(parent), mRmType(rmType)
{
    if ( mRmType == bsrmtRegis )
        mTvNames = lstRegisWinTableCNames;
    else if ( mRmType == bsrmtSheet )
        mTvNames = lstSheetWinTableCNames;
    else
        mTvNames = lstQueryWinTableCNames;

    if ( mRmType == bsrmtRegis )
        mFlagNames = lstDeskRegisRightFlagCNames;
    else if ( mRmType == bsrmtSheet )
        mFlagNames = lstDeskSheetRightFlagCNames;
    else
        mFlagNames = lstDeskQueryRightFlagCNames;
}

void BsDeskRightModel::clearData()
{
    beginResetModel();
    mDatas.clear();
    endResetModel();
}

void BsDeskRightModel::loadData(const QList<uint> &datas)
{
    beginResetModel();
    mDatas.clear();
    mDatas << datas;
    endResetModel();
}

int BsDeskRightModel::rowCount(const QModelIndex &) const
{
    return mTvNames.length();
}

int BsDeskRightModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant BsDeskRightModel::data(const QModelIndex &index, int role) const
{
    if ( role == Qt::EditRole ) {
            return mDatas.at(index.row());
    }
    else if ( mDatas.length() > index.row() && role == Qt::DisplayRole ) {
        QStringList grants;
        uint flags = mDatas.at(index.row());
        for ( int i = 0, iLen = mFlagNames.length(); i < iLen; ++i )
        {
            uint f = deskpowtwo(i);
            if ( (flags & f) == f ) {
                grants << mFlagNames.at(i);
            }
        }
        return grants.join(QStringLiteral("，"));
    }
    else if ( role == Qt::TextAlignmentRole )
        return Qt::AlignCenter;

    return QVariant();
}

QVariant BsDeskRightModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( role == Qt::DisplayRole )
    {
        if ( orientation == Qt::Horizontal )
            return QStringLiteral("权限");
        else
            return QStringLiteral("  %1  ").arg(mTvNames.at(section));
    }

    return QVariant();
}

Qt::ItemFlags BsDeskRightModel::flags(const QModelIndex &index) const
{
    if ( mDatas.isEmpty() )
        return Qt::NoItemFlags;
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool BsDeskRightModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && index.isValid()) {
        mDatas[index.row()] = value.toUInt();
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

}


// BsDeskRightView
namespace BailiSoft {

BsDeskRightView::BsDeskRightView(QWidget *parent) : QTableView(parent)
{
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setStyleSheet("QHeaderView::section{border-style:none; border-bottom:1px solid #ccc;}");

    verticalHeader()->setDefaultSectionSize(24);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setStyleSheet("QHeaderView{background-color:#ccc;} "
                                    "QHeaderView::section{border-style:none; border-bottom:1px solid #ccc;}");

    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::SingleSelection);

    setSortingEnabled(false);
}

}


// BsDeskRightDelegate
namespace BailiSoft {

BsDeskRightDelegate::BsDeskRightDelegate(QWidget *parent, const QStringList &options, const bool disableActMoney,
                                 const bool disableDisMoney, const bool disablePayOwe)
    : QStyledItemDelegate(parent), mOptions(options), mDisableActMoney(disableActMoney),
      mDisableDisMoney(disableDisMoney), mDisablePayOwe(disablePayOwe)
{
}

QWidget *BsDeskRightDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    QWidget *pnl = new QWidget(parent);
    QHBoxLayout *lay = new QHBoxLayout(pnl);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(6);
    lay->addStretch();
    for ( int i = 0, iLen = mOptions.length(); i < iLen; ++i ) {
        QCheckBox *chk = new QCheckBox(mOptions.at(i), pnl);
        if ( mDisableActMoney && i == 1 ) chk->setEnabled(false);
        if ( mDisableDisMoney && i == 2 ) chk->setEnabled(false);
        if ( mDisablePayOwe && (i == 3 || i ==4) ) chk->setEnabled(false);
        lay->addWidget(chk, 0, Qt::AlignVCenter);
    }
    lay->addStretch();
    pnl->setStyleSheet(".QWidget{background-color:#fff;}");  //默认为透明，故需如此
    return pnl;
}

void BsDeskRightDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QWidget *pnl = qobject_cast<QWidget *>(editor);
    uint val = index.model()->data(index, Qt::EditRole).toUInt();
    pnl->setProperty("flags", val);
    for ( int i = 0, iLen = mOptions.length(); i < iLen; ++i ) {
        QCheckBox *chk = getCheckBoxByText(pnl, mOptions.at(i));
        if ( chk ) {
            bool picked = (val & deskpowtwo(i));
            chk->setChecked(picked && chk->isEnabled());
        }
    }
}

void BsDeskRightDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QWidget *pnl = qobject_cast<QWidget *>(editor);
    uint oldFlags = pnl->property("flags").toUInt();
    uint newFlags = 0;
    for ( int i = 0, iLen = mOptions.length(); i < iLen; ++i ) {
        QCheckBox *chk = getCheckBoxByText(pnl, mOptions.at(i));
        if ( chk ) {
            if ( chk->isChecked() ) {
                newFlags |= deskpowtwo(i);
            }
        }
    }
    if ( newFlags != oldFlags ) {
        model->setData(index, newFlags, Qt::EditRole);
        emit model->dataChanged(index, index);
    }
}

QCheckBox *BsDeskRightDelegate::getCheckBoxByText(QWidget *pnl, const QString &txt) const
{
    for ( int i = 0, iLen = pnl->children().count(); i < iLen; ++i ) {
        QCheckBox *chk = qobject_cast<QCheckBox *>(pnl->children().at(i));
        if ( chk ) {
            if ( chk->text() == txt ) {
                return chk;
            }
        }
    }
    return 0;
}

}

