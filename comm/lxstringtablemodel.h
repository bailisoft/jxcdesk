#ifndef LXSTRINGTABLEMODEL_H
#define LXSTRINGTABLEMODEL_H

#include <QAbstractTableModel>

namespace BailiSoft {

class LxStringTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit LxStringTableModel(const QString &prStrData, QObject *parent = nullptr);

    void resetData(const QString &prStrData, const bool newFirstt = false);
    int rowCount(const QModelIndex &) const;
    int columnCount(const QModelIndex &) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    void setFirstRowColor(const bool isTitleGreyy);

    QStringList mDataLines;
    bool        mFirstRowGray;
    QChar       mSpliter;
    
signals:
    
public slots:

private:
    int mCols;
    int mRows;
    
};

}

#endif // LXSTRINGTABLEMODEL_H
