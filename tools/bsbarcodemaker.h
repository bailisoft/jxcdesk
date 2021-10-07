#ifndef BSBARCODEMAKER_H
#define BSBARCODEMAKER_H

#include <QtWidgets>
#include <QtSql>

#define BSPLUG_SHOW_NAME     QStringLiteral("条形码编码器")
#define BSPLUG_TABLE_NAME    QStringLiteral("main")

namespace BailiSoft {

class LxCoderChkList;
class LxQtyEditorDelegate;

class BsBarcodeMaker : public QWidget
{
    Q_OBJECT
public:
    explicit BsBarcodeMaker(QWidget *parent, const QString &winName);

    QLabel          *mpLblCargo;
    QToolButton     *mpBtnCargo;

    LxCoderChkList  *mpListColor;
    LxCoderChkList  *mpListSizer;

    QToolButton     *mpBtnEditQty;
    QTableWidget    *mpGrid;

    QLineEdit       *mpPrefix;

    QComboBox       *mpTailfix;

    QWidget         *mpPnlDivChar;
    QLineEdit       *mpDivChar;

    LxCoderChkList  *mpListCargoInfo;

    QCheckBox       *mpChkOneCodeOne;

    QToolButton     *mpBtnBuild;

private slots:
    void doSetCargo();
    void doResetEditGrid();
    void doBuildBarcodes();

    void updateUiEnableState();
    void setTailfixEditable(const int idx);

private:
    int  fillSpval(const QString &prSpvalName, const QString &prSpType, LxCoderChkList *prList);
    void copyPickedSpvals(QList<QPair<QString, QString> > &prChecked, LxCoderChkList *prAll);
    void resetGridOne();
    void resetGridLine();
    void resetGridCross();

    QList<QPair<QString, QString> > mPickedColors;
    QList<QPair<QString, QString> > mPickedSizes;
};


// LxCoderChkList =============================================================================
class LxCoderChkList : public QListWidget
{
    Q_OBJECT
public:
    LxCoderChkList(QWidget *parent = 0);

protected:
    void mouseReleaseEvent(QMouseEvent *event);

private slots:
    void doSelAll();
    void doSelClear();
    void doSelRevert();

private:
    QMenu   *mpMenuPop;
    QAction *mpActSelAll;
    QAction *mpActSelClear;
    QAction *mpActSelRevert;

};


// LxQtyEditorDelegate =========================================================================
class LxQtyEditorDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    LxQtyEditorDelegate(QObject *parent = 0);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool eventFilter(QObject *object, QEvent *event);
};

}

#endif // BSBARCODEMAKER_H
