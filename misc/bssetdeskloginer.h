#ifndef BSSETDESKLOGINER_H
#define BSSETDESKLOGINER_H

#include <QtWidgets>


// BsSetDeskLoginer
namespace BailiSoft {

class BsDeskUserList;
class BsDeskRightModel;
class BsDeskRightView;

class BsSetDeskLoginer : public QDialog
{
    Q_OBJECT
public:
    BsSetDeskLoginer(QWidget *parent);
    ~BsSetDeskLoginer() {}

public slots:
    void addLoginer();
    void delLoginer();
    void clearPassword();
    void presetMinRight();
    void presetMaxRight();

    void saveChange();
    void cancelChange();

    void pickLoginerChanged(QListWidgetItem *current, QListWidgetItem *);
    void rightChanged();

private:
    void loadLoginer(const QString &loginer);
    void presetSaveRight(const bool maxx);

    QWidget*            mpLeft;
    QPushButton*            mpBtnAddLoginer;
    BsDeskUserList*             mpList;

    QWidget*            mpRight;
    QLabel*                 mpCurrentLoginer;
    QTabWidget*             mpTab;
    QWidget*                    mpRole;
    QComboBox*                      mpLoginRole;
    QCheckBox*                      mpRetPrice;
    QCheckBox*                      mpLotPrice;
    QCheckBox*                      mpBuyPrice;
    BsDeskRightView*            mpViewRegis;
    BsDeskRightView*            mpViewSheet;
    BsDeskRightView*            mpViewQuery;
    QWidget*                mpSaveBox;
    QPushButton*                mpBtnSaveChange;
    QPushButton*                mpBtnCancelChange;

    BsDeskRightModel*       mpModelRegis;
    BsDeskRightModel*       mpModelSheet;
    BsDeskRightModel*       mpModelQuery;

};

}


// BsDeskUserList
namespace BailiSoft {

class BsDeskUserList : public QListWidget
{
    Q_OBJECT
public:
    BsDeskUserList(BsSetDeskLoginer *parent);
    ~BsDeskUserList(){}
protected:
    void mousePressEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *e);
private:
    BsSetDeskLoginer*   mppWin;
    QMenu*          mpMenu;
    QAction*        mpAcAddLoginer;
    QAction*        mpAcDelLoginer;
    QAction*        mpAcResetPwd;
    QAction*        mpAcPresetMin;
    QAction*        mpAcPresetMax;
};

}


// BsDeskRightModel
namespace BailiSoft {

class BsDeskRightModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    BsDeskRightModel(const uint rmType, QObject *parent);
    ~BsDeskRightModel(){}

    void clearData();
    void loadData(const QList<uint> &datas);
    int rowCount(const QModelIndex &) const;
    int columnCount(const QModelIndex &) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    QStringList         mTvNames;
    QStringList         mFlagNames;
    uint                mRmType;
    QList<uint>         mDatas;
};

}


// BsDeskRightView
namespace BailiSoft {

class BsDeskRightView : public QTableView
{
    Q_OBJECT
public:
    BsDeskRightView(QWidget *parent);
    ~BsDeskRightView(){}

};

}


// BsDeskRightDelegate
namespace BailiSoft {

class BsDeskRightDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit BsDeskRightDelegate(QWidget *parent, const QStringList &options, const bool disableActMoney = false,
                             const bool disableDisMoney = false, const bool disablePayOwe = false);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
private:
    QCheckBox *getCheckBoxByText(QWidget *pnl, const QString &txt) const;

    QStringList         mOptions;
    bool                mDisableActMoney;
    bool                mDisableDisMoney;
    bool                mDisablePayOwe;
};

}


#endif // BSSETDESKLOGINER_H
