#ifndef BSWINFEE_H
#define BSWINFEE_H

#include <QtWidgets>

namespace BailiSoft {

class BsMain;
class BsSqlModel;
class BsField;
class BsFldBox;
class BsFeeGrid;

class BsWinFee : public QWidget
{
    Q_OBJECT
public:
    explicit BsWinFee(QWidget *parent, const QString &tableName);
    ~BsWinFee();

    QString         mTable;

private:
    void onClickCommit();
    void onRequestOk(const QWidget *sender, const QStringList &fens);

    BsSqlModel*     mpDsStaff;

    BsField*        mpFldShop;
    BsField*        mpFldStaff;
    BsField*        mpFldRemark;

    BsFldBox*       mpEdtShop;
    BsFldBox*       mpEdtStaff;
    BsFldBox*       mpEdtRemark;
    BsFeeGrid*      mpGrid;
    QPushButton*    mpBtnCommit;

    BsMain*         mppMain;
};

// BsFeeGrid
class BsFeeGrid : public QTableWidget
{
    Q_OBJECT
public:
    explicit BsFeeGrid(QWidget *parent);
    ~BsFeeGrid();

protected:
    void keyPressEvent(QKeyEvent *e);
    void commitData(QWidget *editor);

private:
    void onClickAdd();
    void onClickDel();
    void onCurrentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    QWidget*        mpPnlButtons;
    QPushButton*        mpBtnAdd;
    QPushButton*        mpBtnDel;

};

}

#endif // BSWINFEE_H
