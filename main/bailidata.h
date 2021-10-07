#ifndef BAILIDATA_H
#define BAILIDATA_H

#include <QtCore>
#include <QObject>
#include <QtWidgets>
#include <QtSql>


// BsAbstractModel
namespace BailiSoft {

class BsAbstractModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    BsAbstractModel(QWidget *parent) : QAbstractTableModel(parent) {}
    int columnCount(const QModelIndex &parent = QModelIndex()) const { Q_UNUSED(parent) return 1;}
    virtual void reload() = 0;
    virtual bool keyValueRowExists(const QString& keyValue) = 0;
};

}


// BsSizerTypeModel
namespace BailiSoft {

class BsSizerTypeModel : public BsAbstractModel
{
    Q_OBJECT
public:
    BsSizerTypeModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void reload();
    bool keyValueRowExists(const QString& keyValue);
    QStringList getSizerList(const QString &sizerType);
    QString getSizerNameByIndex(const QString &sizerType, const int idx);
    QString getSizerNameByCode(const QString &sizerType, const QString &code);
    int getColIndexBySizerCode(const QString &sizerType, const QString &code);
    int getColIndexBySizerName(const QString &sizerType, const QString &name);
    int getMaxColCount() { return mMaxCount; }
    QStringList getWholeUniqueNames();
    QStringList getAllTypes() const { return lstTypeName; }

private:
    QStringList                     lstTypeName;
    QStringList                     lstTypePinyin;
    QMap<QString, bool>             mapScan;
    QMap<QString, QStringList>      mapName;
    QMap<QString, QStringList>      mapCode;
    int                             mMaxCount;
};

}


// BsSizerListModel
namespace BailiSoft {

class BsSizerListModel : public BsAbstractModel
{
    Q_OBJECT
public:
    BsSizerListModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void reload();
    bool keyValueRowExists(const QString& keyValue);
    void setFilterByCargoType(const QString &sizerType);
    bool foundSizerInType(const QString &sizer, const QString &sizerType);
    QString getSizerByCodeInType(const QString &code, const QString &sizerType);
    QStringList getSizerListByType(const QString &sizerType);
    QString getFirstSizerByType(const QString &sizerType);

private:
    QString         mFilteringType;
    QMap<QString, QStringList>  mapName;
    QMap<QString, QStringList>  mapCode;
};
}


// BsColorTypeModel
namespace BailiSoft {

class BsColorTypeModel : public BsAbstractModel
{
    Q_OBJECT
public:
    BsColorTypeModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    void reload();
    bool keyValueRowExists(const QString& keyValue);
private:
    QStringList     lstTypeName;
    QStringList     lstTypePinyin;
};
}

// BsColorListModel
namespace BailiSoft {

class BsColorListModel : public BsAbstractModel
{
    Q_OBJECT
public:
    BsColorListModel();
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void reload();
    bool keyValueRowExists(const QString& keyValue);
    void setFilterByCargoType(const QString &colorType);
    bool foundColorInType(const QString &color, const QString &colorType);
    QString getColorByCodeInType(const QString &code, const QString &colorType);
    QStringList getColorListByType(const QString &colorType);
    QString getFirstColorByType(const QString &colorType);

private:
    QString         mFilteringType;
    QMap<QString, QStringList>  mapName;
    QMap<QString, QStringList>  mapCode;
    QMap<QString, bool>         mapScan;
};
}

// BsListModel
namespace BailiSoft {

class BsListModel : public BsAbstractModel
{
    Q_OBJECT
public:
    BsListModel(QWidget *parent, const QString &sql, const bool commaList = true);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    void reload();
    bool keyValueRowExists(const QString& keyValue);
private:
    QString         mSql;
    QStringList     mValues;
    QStringList     mPinyins;
    bool            mCommaList = true;
};

}


// BsRegModel
namespace BailiSoft {

class BsRegModel : public BsAbstractModel
{
    Q_OBJECT
public:
    BsRegModel(const QString &table, const QStringList &fields);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void reload();
    bool keyValueRowExists(const QString& keyValue);
    void switchBookLogin();
    QString getTableName() { return mTable; }

    QString getValue(const QString &keyValue, const QString &fldName);
    bool    foundKey(const QString &keyValue);

    QString getCargoBasicInfo(const QString &keyValue);

private:
    bool        mUseCode;
    int         mCargoSizeIdx;
    int         mCargoColorIdx;
    int         mCargoNameIdx;
    int         mCargoSetPriceIdx;


    QString                     mTable;
    QStringList                 mFields;
    int                         mPriceDots;

    QMap<QString, QStringList>  mRecords;
    QStringList                 mRecIndex;

    qint64               mReloadEpochSecs;
};

}


// BsSqlModel
namespace BailiSoft {

class BsSqlModel : public BsAbstractModel
{
    Q_OBJECT
public:
    BsSqlModel(QWidget *parent, const QString &sql);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    void reload();
    bool keyValueRowExists(const QString& keyValue);
    void switchBookLogin();

    QString getValue(const QString &keyValue, const QString &fldName);
    bool    foundKey(const QString &keyValue);

private:
    QString                     mSql;
    QStringList                 mFields;

    QMap<QString, QStringList>  mRecords;
    QStringList                 mRecIndex;
};

}


// BsPolicy
namespace BailiSoft {

class BsPolicy
{
public:
    BsPolicy(const QString &traderExp, const QString &cargoExp, const double policyDis,
             const QDate startDate, const QDate endDate)
        : mTraderExp(traderExp), mCargoExp(cargoExp), mPolicyDis(policyDis),
          mStartDate(startDate), mEndDate(endDate) {}

    QString     mTraderExp;
    QString     mCargoExp;
    double      mPolicyDis;
    QDate       mStartDate;
    QDate       mEndDate;
};

}

// init
namespace BailiSoft {

int checkDataDir();
QString getDataPathFile(const QString &bookName);
int openDefaultSqliteConn();
int loginLoadOptions();
int loginLoadRights();
int loginLoadRegis();
bool canDo(const QString &winBaseName, const uint rightFlag = 0);

}


// extern global
namespace BailiSoft {

extern QString                  dataDir;
extern QString                  backupDir;
extern QString                  imageDir;

extern QStringList              cargoSheetCommonFields;
extern QStringList              financeSheetCommonFields;
extern QStringList              cargoQueryCommonFields;
extern QStringList              financeQueryCommonFields;

extern QString                  loginLink;
extern QString                  loginer;
extern QString                  loginTrader;
extern QString                  loginPassword;

extern QString                  bossAccount;
extern bool                     canRett;
extern bool                     canLott;
extern bool                     canBuyy;

extern BsSizerTypeModel*                dsSizerType;
extern BsSizerListModel*                dsSizerList;
extern BsColorTypeModel*                dsColorType;
extern BsColorListModel*                dsColorList;
extern BsRegModel*                      dsCargo;
extern BsRegModel*                      dsSubject;
extern BsRegModel*                      dsShop;
extern BsRegModel*                      dsCustomer;
extern BsRegModel*                      dsSupplier;

extern QMap<QString, QString>                   mapOption;
extern QMap<QString, QString>                   mapFldUserSetName;         //指用户额外自定义得字段名，可为空
extern QVector<QPair<QString, bool> >           vecBarcodeRule;
extern QList<BsPolicy*>                         pricePolicies;

}


#endif // BAILIDATA_H
