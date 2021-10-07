#include "bsbarcodemaker.h"
#include "main/bailicode.h"

#define MAIN_CODE_CARGO     "编码货品"
#define FILL_MAIN_CODE      "待编入主货号"
#define CODE_ISO_PREFIX     "国标码前缀"
#define CODE_TAIL_FIX       "附加后缀"
#define AUTO_FLOW_NO        "%1位流水号"
#define CODE_DIV_CHAR       "规格分隔符"
#define IFNO_DONOT_FILL      "如无则不填"
#define ISO_DISABLE_USE     "国标码禁用"
#define OTHER_PRINT_INFO    "附加打印信息"
#define ONE_CODE_ONE_ROW    "一码一行"
#define BTN_RESET_QTY       "清空数量"
#define BTN_BUILD           "生成编码"
#define CARGO_CODER         "货品编码"

#define APPEND_SPGRP        "添加规格组"
#define NOT_ALL_BARCODE_SET "系统检查到该货号所属码类或色系未设置代码或设置的名码数不一致，因此不能编码。"
#define PLEASE_SELECT       "请选择："
#define MAINCODE_REQUIRED   "未指定货号！"
#define MAINCODE_NOT_FOUND  "系统没有找到登记的%1！"
#define SPVAL_NOT_PICKED    "请选择至少一个%1！"
#define BUILD_FILE_OK       "条形码编码文件%1已经生成并保存为标准文本格式，标签打印软件或印刷厂可以接受此格式文件。"
#define CONTEXT_TOOLTIPA    "鼠标右键移除或改变选择"
#define CONTEXT_TOOLTIPB    "鼠标左键选择"
#define INPUT_LOTNUM_START  "请提供最多%1位起始序列号："

#define ACT_SELECT_ALL      "全选中"
#define ACT_SELECT_CLEAR    "全取消"
#define ACT_SELECT_REVERT   "反选"
#define ACT_DELETE_SPGRP    "移除整组"

namespace BailiSoft {

BsBarcodeMaker::BsBarcodeMaker(QWidget *parent, const QString &winName) : QWidget(parent)
{
    //用于主窗口查找判断
    setProperty(BSWIN_TABLE, winName);

    //左——主货号
    mpBtnCargo = new QToolButton(this);
    mpBtnCargo->setText(QStringLiteral("指定货号"));
    mpBtnCargo->setToolButtonStyle(Qt::ToolButtonTextOnly);
    mpBtnCargo->setFixedSize(80, 24);

    mpLblCargo = new QLabel(this);
    mpLblCargo->setStyleSheet("font-size:20px;");
    mpLblCargo->setProperty("colortype", QString());
    mpLblCargo->setProperty("sizertype", QString());

    //左——规格
    mpListColor = new LxCoderChkList(this);
    mpListSizer = new LxCoderChkList(this);
    mpListColor->setToolTip(QStringLiteral(CONTEXT_TOOLTIPA));
    mpListSizer->setToolTip(QStringLiteral(CONTEXT_TOOLTIPA));

    //左——左布局
    QVBoxLayout *pLaySel = new QVBoxLayout;
    pLaySel->addWidget(mpBtnCargo, 0, Qt::AlignCenter);
    pLaySel->addWidget(mpLblCargo, 0, Qt::AlignCenter);
    pLaySel->addWidget(mpListColor);
    pLaySel->addWidget(mpListSizer);
    pLaySel->addStretch();

    //中
    mpGrid = new QTableWidget(this);
    mpGrid->setSelectionMode(QAbstractItemView::SingleSelection);
    mpGrid->setItemDelegate(new LxQtyEditorDelegate);

    //右——清空数量
    mpBtnEditQty = new QToolButton(this);
    mpBtnEditQty->setText(QStringLiteral(BTN_RESET_QTY));
    mpBtnEditQty->setToolButtonStyle(Qt::ToolButtonTextOnly);
    mpBtnEditQty->setFixedSize(80, 24);
    mpBtnEditQty->setEnabled(false);

    //右——前缀
    QWidget *pPnlPrefix = new QWidget(this);
    QVBoxLayout *pLayPrefix = new QVBoxLayout(pPnlPrefix);
    QLabel *pLblPrefix = new QLabel(QStringLiteral(CODE_ISO_PREFIX), this);
    mpPrefix = new QLineEdit(this);
    mpPrefix->setPlaceholderText(QStringLiteral(IFNO_DONOT_FILL));
    mpPrefix->setMaxLength(8);

    pLayPrefix->addWidget(pLblPrefix);
    pLayPrefix->addWidget(mpPrefix);
    pLayPrefix->setContentsMargins(0, 0, 0, 0);
    pPnlPrefix->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

    //右——后缀
    QWidget *pPnlTailfix = new QWidget(this);
    QVBoxLayout *pLayTailfix = new QVBoxLayout(pPnlTailfix);
    QLabel *pLblTailfix = new QLabel(QStringLiteral(CODE_TAIL_FIX), this);
    mpTailfix = new QComboBox(this);
    mpTailfix->setMaxVisibleItems(11);
    mpTailfix->addItem(QString());
    mpTailfix->addItem(QStringLiteral(AUTO_FLOW_NO).arg(1));
    mpTailfix->addItem(QStringLiteral(AUTO_FLOW_NO).arg(2));
    mpTailfix->addItem(QStringLiteral(AUTO_FLOW_NO).arg(3));
    mpTailfix->addItem(QStringLiteral(AUTO_FLOW_NO).arg(4));
    mpTailfix->addItem(QStringLiteral(AUTO_FLOW_NO).arg(5));
    mpTailfix->addItem(QStringLiteral(AUTO_FLOW_NO).arg(6));
    mpTailfix->addItem(QStringLiteral(AUTO_FLOW_NO).arg(7));
    mpTailfix->addItem(QStringLiteral(AUTO_FLOW_NO).arg(8));
    mpTailfix->addItem(QStringLiteral(AUTO_FLOW_NO).arg(9));

    pLayTailfix->addWidget(pLblTailfix);
    pLayTailfix->addWidget(mpTailfix);
    pLayTailfix->setContentsMargins(0, 10, 0, 0);
    pPnlTailfix->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));

    //右——分隔符
    mpPnlDivChar = new QWidget(this);
    QVBoxLayout *pLayDivChar = new QVBoxLayout(mpPnlDivChar);
    QLabel *pLblDivChar = new QLabel(QStringLiteral(CODE_DIV_CHAR), this);
    mpDivChar = new QLineEdit(this);
    mpDivChar->setPlaceholderText(QStringLiteral(IFNO_DONOT_FILL));
    mpDivChar->setMaxLength(1);
    pLayDivChar->addWidget(pLblDivChar);
    pLayDivChar->addWidget(mpDivChar);
    pLayDivChar->setContentsMargins(0, 10, 0, 0);
    mpPnlDivChar->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
    mpPnlDivChar->hide();

    //右——附加货品信息
    QWidget *pPnlCargoInfo = new QWidget(this);
    QVBoxLayout *pLayCargoInfo = new QVBoxLayout(pPnlCargoInfo);
    QLabel *pLblCargoInfo = new QLabel(QStringLiteral(OTHER_PRINT_INFO));
    mpListCargoInfo = new LxCoderChkList(this);
    mpListCargoInfo->setToolTip(QStringLiteral(CONTEXT_TOOLTIPB));
    pLayCargoInfo->addWidget(pLblCargoInfo);
    pLayCargoInfo->addWidget(mpListCargoInfo);
    pLayCargoInfo->setContentsMargins(0, 10, 0, 0);

    QListWidgetItem *pItem = new QListWidgetItem(QStringLiteral("货号"), mpListCargoInfo);
    pItem->setData(Qt::UserRole, "hpcode");
    pItem->setCheckState(Qt::Unchecked);

    pItem = new QListWidgetItem(QStringLiteral("名称"), mpListCargoInfo);
    pItem->setData(Qt::UserRole, "hpname");
    pItem->setCheckState(Qt::Unchecked);

    pItem = new QListWidgetItem(QStringLiteral("定价"), mpListCargoInfo);
    pItem->setData(Qt::UserRole, "setprice");
    pItem->setCheckState(Qt::Unchecked);

    //右——一行一码
    mpChkOneCodeOne = new QCheckBox(QStringLiteral(ONE_CODE_ONE_ROW), this);

    //右——编码生成按钮
    mpBtnBuild = new QToolButton(this);
    mpBtnBuild->setText(QStringLiteral(BTN_BUILD));
    mpBtnBuild->setIcon(QIcon(":/icons/icons/barcoder.png"));
    mpBtnBuild->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mpBtnBuild->setIconSize(QSize(32, 32));
    mpBtnBuild->setMinimumWidth(90);
    mpBtnBuild->setEnabled(false);

    //右——布局
    QVBoxLayout *pLayDeal = new QVBoxLayout;
    pLayDeal->addWidget(mpBtnEditQty, 0, Qt::AlignCenter);
    pLayDeal->addWidget(pPnlPrefix);
    pLayDeal->addWidget(pPnlTailfix);
    pLayDeal->addWidget(mpPnlDivChar);
    pLayDeal->addWidget(pPnlCargoInfo);
    pLayDeal->addWidget(mpChkOneCodeOne, 0, Qt::AlignCenter);
    pLayDeal->addWidget(mpBtnBuild, 0, Qt::AlignCenter);
    pLayDeal->addStretch();

    //主Layout
    QHBoxLayout *pLayMain = new QHBoxLayout(this);
    pLayMain->addLayout(pLaySel, 1);
    pLayMain->addSpacing(6);
    pLayMain->addWidget(mpGrid, 5);
    pLayMain->addSpacing(6);
    pLayMain->addLayout(pLayDeal, 1);

    //事件
    connect(mpBtnCargo, SIGNAL(clicked()), this, SLOT(doSetCargo()));
    connect(mpListColor, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(doResetEditGrid()));
    connect(mpListSizer, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(doResetEditGrid()));
    connect(mpBtnEditQty, SIGNAL(clicked()), this, SLOT(doResetEditGrid()));
    connect(mpPrefix, SIGNAL(textChanged(QString)), this, SLOT(updateUiEnableState()));
    connect(mpTailfix, SIGNAL(currentIndexChanged(int)), this, SLOT(setTailfixEditable(int)));
    connect(mpBtnBuild, SIGNAL(clicked()), this, SLOT(doBuildBarcodes()));

    mpTailfix->setCurrentIndex(0);

    setWindowTitle(QStringLiteral("条形码批量编码工具"));
}

void BsBarcodeMaker::doSetCargo()
{
    QInputDialog dlg(this);
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.setWindowTitle(windowTitle());
    dlg.setLabelText(QStringLiteral("请指定货号"));
    dlg.setOkButtonText(QStringLiteral("确定"));
    dlg.setCancelButtonText(QStringLiteral("取消"));
    dlg.setMinimumSize(dlg.sizeHint());
    if (dlg.exec() != QDialog::Accepted)
        return;
    QString cargo = dlg.textValue().trimmed();
    if ( cargo.isEmpty() )
        return;

    //货号
    QSqlQuery qry;
    QString sql = QString("select sizertype, colortype from cargo where upper(hpcode)='%1';").arg(cargo.toUpper());
    qry.exec(sql);
    if ( qry.next() ) {

        QString sizertype = qry.value(0).toString();
        QString colortype = qry.value(1).toString();

        mpLblCargo->setText(cargo);
        mpLblCargo->setProperty("sizertype", sizertype);
        mpLblCargo->setProperty("colortype", colortype);

        if ( !sizertype.isEmpty() )
            fillSpval(QStringLiteral("尺码品类"), sizertype, mpListSizer);
        mpListSizer->setVisible(!sizertype.isEmpty());

        if ( !colortype.isEmpty() )
            fillSpval(QStringLiteral("颜色系列"), colortype, mpListColor);
        mpListColor->setVisible(!colortype.isEmpty());

        doResetEditGrid();
    }
    else {
        QMessageBox::information(this, windowTitle(), QStringLiteral("无此货号！"));
    }
}

void BsBarcodeMaker::doResetEditGrid()
{
    mpGrid->clearContents();
    mpGrid->setRowCount(0);
    mpGrid->setColumnCount(0);
    mpBtnBuild->setEnabled(false);

    copyPickedSpvals(mPickedColors, mpListColor);
    copyPickedSpvals(mPickedSizes, mpListSizer);

    if (mPickedColors.size() > 0 && mPickedSizes.size() > 0) {
        resetGridCross();
    } else if (mPickedColors.size() > 0 || mPickedSizes.size() > 0) {
        resetGridLine();
    } else {
        resetGridOne();
    }

    updateUiEnableState();
}

void BsBarcodeMaker::doBuildBarcodes()
{
    if ( mpLblCargo->text().isEmpty() ) {
        QMessageBox::information(this, windowTitle(), QStringLiteral(MAINCODE_REQUIRED));
        return;
    }

    if ( !mpListColor->isHidden() && mpListColor->count() > 0 && mPickedColors.size() == 0 ) {
        QMessageBox::information(this, windowTitle(), QStringLiteral("至少需要一个颜色！"));
        return;
    }

    if ( !mpListSizer->isHidden() && mpListSizer->count() > 0 && mPickedSizes.size() == 0 )  {
        QMessageBox::information(this, windowTitle(), QStringLiteral("至少需要一个尺码！"));
        return;
    }

    //约定iFlowDigits后面待用
    //-1    无批号后缀
    //0     固定字符后缀（如日期做批号）
    //1~N   为流水整数号占据位数
    int intLotFlowNo = 0;
    int iFlowDigits = -1;
    if (mpTailfix->currentIndex() == 0 && !mpTailfix->currentText().trimmed().isEmpty())
        iFlowDigits = 0;
    else if (mpTailfix->currentIndex() > 0) {
        iFlowDigits = mpTailfix->currentText().left(1).toInt();
        //序列号超始编号
        QInputDialog dlg(this);
        dlg.setWindowTitle(QStringLiteral(CARGO_CODER));
        dlg.setLabelText(QString(QStringLiteral(INPUT_LOTNUM_START)).arg(iFlowDigits));
        dlg.setOkButtonText(QStringLiteral("确定"));
        dlg.setCancelButtonText(QStringLiteral("取消"));
        dlg.setIntMinimum(1);
        dlg.setIntMaximum(int(pow(10.0, iFlowDigits)) - 1);
        dlg.setIntValue(0);
        if ( dlg.exec() == QDialog::Accepted )
            intLotFlowNo = dlg.intValue();
        if ( intLotFlowNo == 0 )
            return;
    }

    //文本存储器
    QString txts;

    //取标题
    QString sTitleLine = QStringLiteral("条形码");
    sTitleLine += QString(",%1").arg(QStringLiteral("数量"));
    if (mPickedColors.size() > 0)
        sTitleLine += QString(",%1").arg(mpListColor->whatsThis());
    if (mPickedSizes.size() > 0)
        sTitleLine += QString(",%1").arg(mpListSizer->whatsThis());

    //提取保存附加打印信息，及继续取标题
    int iPrintCols = 0;
    QString sCargoPrintSql;
    for (int i = 0; i < mpListCargoInfo->count(); ++i) {
        QListWidgetItem *pItem = mpListCargoInfo->item(i);
        if (pItem->checkState() == Qt::Checked) {
            ++iPrintCols;
            sTitleLine += QString(",%1").arg(pItem->text());
            sCargoPrintSql += QString(",%1").arg(pItem->data(Qt::UserRole).toString());
        }
    }

    //存储标题
    txts += sTitleLine;

    //附加打印信息，以后每行都要复制
    QString sAttFixLine;
    QSqlQuery qry;
    qry.exec(QString("SELECT hpcode%1 FROM cargo WHERE hpcode='%2';").arg(sCargoPrintSql).arg(mpLblCargo->text()));
    QSqlRecord rec = qry.record();
    while (qry.next()) {
        for (int i = 1; i <= iPrintCols; ++i) {
            QString fvalue = qry.value(i).toString();
            if ( rec.fieldName(i).contains(QStringLiteral("price")) )
                fvalue = bsNumForRead(fvalue.toLongLong(), 0);
            sAttFixLine += QString(",%1").arg(fvalue);
        }
    }

    //根据表格行列遍历处理行内容
    QString sBasic;
    sBasic += mpPrefix->text();
    sBasic += mpLblCargo->text();

    for (int row = 0; row < mpGrid->rowCount(); ++row) {

        for (int col = 0; col < mpGrid->columnCount(); ++col) {

            int iQty = 0;
            if (mpGrid->item(row, col))
                iQty = mpGrid->item(row, col)->text().toInt();

            if (iQty == 0) continue;

            int idxColor = -1;
            int idxSizer = -1;

            if ( mPickedColors.size() > 0 && mPickedSizes.size() > 0 ) {
                idxColor = row;
                idxSizer = col;
            }
            else if (mPickedColors.size() > 0) {
                idxColor = row;
            } else  if (mPickedColors.size() > 0) {
                idxSizer = col;
            }

            QString sBarBase(sBasic);

            if (idxColor >= 0) {
                sBarBase += mpDivChar->text();
                sBarBase += mPickedColors.at(idxColor).second;
            }
            if (idxSizer >= 0) {
                sBarBase += mpDivChar->text();
                sBarBase += mPickedSizes.at(idxSizer).second;
            }

            for (int i = 0; i < iQty; ++i) {
                QString sLine(sBarBase);
                if (iFlowDigits > 0) {
                    ++intLotFlowNo;
                    sLine += mpDivChar->text();
                    sLine += QString("%1").arg(intLotFlowNo, iFlowDigits, 10, QChar('0'));
                } else if (iFlowDigits == 0) {
                    sLine += mpDivChar->text();
                    sLine += mpTailfix->currentText();
                }

                if (mpChkOneCodeOne->isChecked())
                    sLine += QString(",1");
                else
                    sLine += QString(",%1").arg(iQty);

                if (idxColor >= 0)
                    sLine += QString(",%1").arg(mPickedColors.at(idxColor).first);
                if (idxSizer >= 0)
                    sLine += QString(",%1").arg(mPickedSizes.at(idxSizer).first);

                sLine += sAttFixLine;

                txts += QString("\n%1").arg(sLine);

                if (!mpChkOneCodeOne->isChecked())
                    break;
            }
        }
    }

    //提示用户，保存文件
    QString deskPath = QStandardPaths::locate(QStandardPaths::DesktopLocation,"", QStandardPaths::LocateDirectory);
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    windowTitle(),
                                                    deskPath,
                                                    QStringLiteral("逗号格式文本(*.txt)"));
    if (fileName.isEmpty())
        return;

    //执行文件写入
    QFile f(fileName);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream strm(&f);
#ifdef Q_OS_WIN
        //main.cpp中setCodecForLocale设为UTF-8就表示stream的输出输入都是UTF-8的。
        //而EXCEL打不开无BOM头的UTF-8文件（记事本带智能检测，故可以）。
        strm.setGenerateByteOrderMark(true);
#endif
        strm << txts;
        f.close();
    }

    QMessageBox::information(this, windowTitle(), QStringLiteral(BUILD_FILE_OK).arg(fileName));
}

void BsBarcodeMaker::updateUiEnableState()
{
    int iSpecs = 0;
    if (mpListColor->count() > 0) ++iSpecs;
    if (mpListSizer->count() > 0) ++iSpecs;

    if (mpPrefix->text().isEmpty() && iSpecs > 0)
        mpPnlDivChar->show();
    else {
        mpDivChar->clear();
        mpPnlDivChar->hide();
    }

    mpBtnEditQty->setEnabled( !mpLblCargo->text().isEmpty() );
    mpBtnBuild->setEnabled( !mpLblCargo->text().isEmpty() && mpGrid->rowCount() > 0 );
}

void BsBarcodeMaker::setTailfixEditable(const int idx)
{
    if (idx == 0) {
        mpTailfix->setEditable(true);
        mpTailfix->lineEdit()->setPlaceholderText(QStringLiteral(IFNO_DONOT_FILL));
        mpTailfix->lineEdit()->setMaxLength(50);
        mpChkOneCodeOne->setEnabled(true);
        mpChkOneCodeOne->setChecked(false);
    } else {
        mpTailfix->setEditable(false);
        mpChkOneCodeOne->setChecked(true);
        mpChkOneCodeOne->setEnabled(false);
    }
}

int BsBarcodeMaker::fillSpval(const QString &prSpgrpName, const QString &prSpType, LxCoderChkList *prList)
{
    prList->setWhatsThis(prSpgrpName);
    QString spTable = ( prList == mpListColor ) ? QStringLiteral("colortype") : QStringLiteral("sizertype");

    QSqlQuery qry;
    QString sql = QStringLiteral("select namelist, codelist from %1 where tname='%2';").arg(spTable).arg(prSpType);
    qry.exec(sql);

    bool typeOk = qry.next();
    QStringList names = qry.value(0).toString().split(QChar(44));
    QStringList codes = qry.value(1).toString().split(QChar(44));
    if ( !typeOk || names.length() != codes.length() ) {
        QMessageBox::information(this, QStringLiteral(APPEND_SPGRP), QStringLiteral(NOT_ALL_BARCODE_SET));
        qry.finish();
        return 0;
    }

    prList->clear();
    int iCount = 0;
    for ( int i = 0, iLen = names.length(); i < iLen; ++i ) {
        QListWidgetItem *pItem = new QListWidgetItem(names.at(i), prList);
        pItem->setData(Qt::UserRole, codes.at(i));
        pItem->setCheckState(Qt::Checked);
        ++iCount;
    }

    updateUiEnableState();

    return iCount;
}

void BsBarcodeMaker::copyPickedSpvals(QList<QPair<QString, QString> > &prChecked, LxCoderChkList *prAll)
{
    prChecked.clear();

    if ( prAll->isHidden() )
        return;

    for (int i = 0; i < prAll->count(); ++i) {
        if (prAll->item(i)->checkState() == Qt::Checked) {
            QString str1 = prAll->item(i)->text();
            QString str2 = prAll->item(i)->data(Qt::UserRole).toString();
            prChecked.append(qMakePair(str1, str2));
        }
    }
}

void BsBarcodeMaker::resetGridOne()
{
    QStringList lsHori;
    lsHori << QStringLiteral("数量");
    mpGrid->setColumnCount(1);
    mpGrid->setHorizontalHeaderLabels(lsHori);

    QStringList lsVert;
    lsVert << mpLblCargo->text();
    mpGrid->setRowCount(1);
    mpGrid->setVerticalHeaderLabels(lsVert);
}

void BsBarcodeMaker::resetGridLine()
{
    QStringList lsQty;
    lsQty << QStringLiteral("数量");

    if ( mPickedColors.size() > 0 ) {
        QStringList lsSpecs;
        for (int i = 0; i < mPickedColors.size(); ++i)
            lsSpecs << mPickedColors.at(i).first;

        mpGrid->setRowCount(lsSpecs.size());
        mpGrid->setColumnCount(1);

        mpGrid->setHorizontalHeaderLabels(lsQty);
        mpGrid->setVerticalHeaderLabels(lsSpecs);

        mpGrid->setColumnWidth(0, 100);
        mpGrid->setColumnWidth(1, 60);
    }
    else {
        QStringList lsSpecs;
        for (int i = 0; i < mPickedSizes.size(); ++i)
            lsSpecs << mPickedSizes.at(i).first;

        mpGrid->setRowCount(1);
        mpGrid->setColumnCount(lsSpecs.size());

        mpGrid->setHorizontalHeaderLabels(lsSpecs);
        mpGrid->setVerticalHeaderLabels(lsQty);

        for (int i = 0; i < mpGrid->columnCount(); ++i)
            mpGrid->setColumnWidth(i, 50);
    }
}

void BsBarcodeMaker::resetGridCross()
{
    QStringList lsColors;
    for (int i = 0; i < mPickedColors.size(); ++i)
        lsColors << mPickedColors.at(i).first;
    mpGrid->setRowCount(lsColors.size());
    mpGrid->setVerticalHeaderLabels(lsColors);

    QStringList lsSizes;
    for (int i = 0; i < mPickedSizes.size(); ++i)
        lsSizes << mPickedSizes.at(i).first;
    mpGrid->setColumnCount(lsSizes.size());
    mpGrid->setHorizontalHeaderLabels(lsSizes);

    for (int i = 0; i < mpGrid->columnCount(); ++i)
        mpGrid->setColumnWidth(i, 50);
}


// LxCoderChkList =============================================================================
LxCoderChkList::LxCoderChkList(QWidget *parent) : QListWidget(parent)
{
    mpMenuPop = new QMenu(this);
    mpActSelAll     = mpMenuPop->addAction(QStringLiteral(ACT_SELECT_ALL));
    mpActSelClear   = mpMenuPop->addAction(QStringLiteral(ACT_SELECT_CLEAR));
    mpActSelRevert  = mpMenuPop->addAction(QStringLiteral(ACT_SELECT_REVERT));

    connect(mpActSelAll,    SIGNAL(triggered()), this, SLOT(doSelAll()));
    connect(mpActSelClear,  SIGNAL(triggered()), this, SLOT(doSelClear()));
    connect(mpActSelRevert, SIGNAL(triggered()), this, SLOT(doSelRevert()));
}

void LxCoderChkList::mouseReleaseEvent(QMouseEvent *event)
{
    QListWidget::mouseReleaseEvent(event);

    if (event->button() == Qt::RightButton) {
        mpActSelAll->setEnabled(count() > 0);
        mpActSelClear->setEnabled(count() > 0);
        mpActSelRevert->setEnabled(count() > 0);
        mpMenuPop->exec(event->globalPos());
    }
}

void LxCoderChkList::doSelAll()
{
    for (int i = 0; i < count(); ++i) {
        item(i)->setCheckState(Qt::Checked);
    }
}

void LxCoderChkList::doSelClear()
{
    for (int i = 0; i < count(); ++i) {
        item(i)->setCheckState(Qt::Unchecked);
    }
}

void LxCoderChkList::doSelRevert()
{
    for (int i = 0; i < count(); ++i) {
        if (item(i)->checkState() == Qt::Checked)
            item(i)->setCheckState(Qt::Unchecked);
        else
            item(i)->setCheckState(Qt::Checked);
    }
}


// LxQtyEditorDelegate =========================================================================
LxQtyEditorDelegate::LxQtyEditorDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    installEventFilter(this);   //也可不是this，那就必须另外建一个继承自QObject的类，实现eventFilter事件。
                                //但此处有内部commitData、closeEditor调用处理，必须用this作为filter对象。
}

QWidget *LxQtyEditorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    QLineEdit *editor = qobject_cast<QLineEdit *>(QStyledItemDelegate::createEditor(parent, option, index));
    editor->setValidator(new QIntValidator(0, 10000));
    return editor;
}

bool LxQtyEditorDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            emit commitData(qobject_cast<QWidget *>(object));
            closeEditor(qobject_cast<QWidget *>(object), QAbstractItemDelegate::EditNextItem);
            return true;
        }
    }
    return QStyledItemDelegate::eventFilter(object, event);
}

}
