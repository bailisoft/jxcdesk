#include "bswinfee.h"
#include "main/bailicode.h"
#include "main/bsmain.h"
#include "main/bailiedit.h"
#include "main/bailigrid.h"
#include "main/bailidata.h"
#include "main/bssocket.h"

namespace BailiSoft {

BsWinFee::BsWinFee(QWidget *parent, const QString &tableName, const QStringList &fields)
    : QWidget(parent), mTable(tableName)
{
    //用于主窗口查找判断
    setProperty(BSWIN_TABLE, tableName);

    //主窗
    mppMain = qobject_cast<BsMain*>(parent);
    Q_ASSERT(mppMain);

    //dataset
    dsShop->reload();
    mpDsStaff = new BsSqlModel(this, QStringLiteral("select kname from staff;"));
    mpDsStaff->reload();

    //fields
    mpFldShop = new BsField(QStringLiteral("shop"),
                            QStringLiteral("门店"),
                            (bsffText | bsffHead | bsffAggCount | bsffQryAsCon | bsffQryAsSel | bsffQrySelBold),
                            20,
                            QStringLiteral("关联门店"));

    mpFldStaff = new BsField(QStringLiteral("staff"),
                            QStringLiteral("员工"),
                            (bsffText | bsffHead | bsffAggCount | bsffQryAsCon | bsffQryAsSel | bsffQrySelBold),
                            20,
                            QStringLiteral("关联员工"));

    mpFldRemark = new BsField(QStringLiteral("remark"),
                            QStringLiteral("备注"),
                            (bsffText | bsffHead | bsffAggNone),
                            100,
                            QStringLiteral("备注"));

    //控件
    QLabel* lblTitle = new QLabel(QStringLiteral("报销单"), this);
    lblTitle->setStyleSheet(QLatin1String("QLabel {font-size:32px; color:grey;}"));

    mpEdtShop = new BsFldBox(this, mpFldShop, dsShop, false);

    mpEdtStaff = new BsFldBox(this, mpFldStaff, mpDsStaff, false);

    mpEdtRemark = new BsFldBox(this, mpFldRemark, nullptr, false);

    mpGrid = new BsFeeGrid(this);

    mpBtnCommit = new QPushButton(QStringLiteral("提交"), this);
    mpBtnCommit->setFixedSize(90, 30);

    //布局
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(30, 15, 30, 30);
    lay->addWidget(lblTitle, 0, Qt::AlignCenter);
    lay->addSpacing(10);
    lay->addWidget(mpEdtShop);
    lay->addSpacing(10);
    lay->addWidget(mpEdtStaff);
    lay->addSpacing(10);
    lay->addWidget(mpEdtRemark);
    lay->addSpacing(10);
    lay->addWidget(mpGrid, 1);
    lay->addSpacing(10);
    lay->addWidget(mpBtnCommit, 0, Qt::AlignCenter);

    //win
    setMinimumSize(sizeHint());
    setWindowTitle(QStringLiteral("报销"));
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);

    connect(mpBtnCommit, &QPushButton::clicked, this, &BsWinFee::onClickCommit);
    connect(netSocket, &BsSocket::requestOk, this, &BsWinFee::onRequestOk);
}

BsWinFee::~BsWinFee()
{
    delete mpFldRemark;
    delete mpFldStaff;
    delete mpFldShop;
    delete mpDsStaff;
}

void BsWinFee::onClickCommit()
{
//    //提示文字
//    QString waitMsg = QStringLiteral("正在提交，请稍侯……");

//    //提交
//    QStringList params;
//    params << QStringLiteral("FEEINSERT");
//    params << QString::number(QDateTime::currentMSecsSinceEpoch() * 1000);
//    params << mMainTable;
//    params << mainValues.join(QChar('\t'));
//    params << dtlLines.join(QChar('\n'));
//    netSocket->netRequest(this, params, waitMsg);
}

void BsWinFee::onRequestOk(const QWidget *sender, const QStringList &fens)
{
    if ( sender != this )
        return;

    if ( fens.at(0) == QStringLiteral("FEEINSERT") ) {
        QMessageBox::information(this, QString(), QStringLiteral("提交成功！"));
        mpEdtShop->mpEditor->setDataValue(QString());
        mpEdtStaff->mpEditor->setDataValue(QString());
        mpEdtRemark->mpEditor->setDataValue(QString());
        mpGrid->clearContents();
        mpGrid->setRowCount(0);
    }
}

//===============================================================

BsFeeGrid::BsFeeGrid(QWidget *parent) : QTableWidget(parent)
{
    setColumnCount(2);
    setHorizontalHeaderLabels(QStringList()<<QStringLiteral("细项")<<QStringLiteral("金额"));
    setColumnWidth(0, 400);

    horizontalHeader()->setStyleSheet("QHeaderView{border-style:none; border-bottom:1px solid silver;} ");
    horizontalHeader()->setStretchLastSection(true);

    verticalHeader()->setDefaultSectionSize(24);
    verticalHeader()->setStyleSheet("color:#999;");
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    setMinimumWidth(600);

    mpPnlButtons  = new QWidget(this);
    QHBoxLayout *layButtons = new mpPnlButtons(mpPnlButtons);
    layButtons->addWidget();

    connect(this, &BsFeeGrid::currentCellChanged, this, &BsFeeGrid::onCurrentCellChanged);
}

BsFeeGrid::~BsFeeGrid()
{

}

void BsFeeGrid::keyPressEvent(QKeyEvent *e)
{
    QTableWidget::keyPressEvent(e);
}

void BsFeeGrid::commitData(QWidget *editor)
{

    QTableWidget::commitData(editor);
}

void BsFeeGrid::onClickAdd()
{

}

void BsFeeGrid::onClickDel()
{

}

void BsFeeGrid::onCurrentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{

}

}
