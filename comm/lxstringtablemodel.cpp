#include "lxstringtablemodel.h"
#include <QtWidgets>

namespace BailiSoft {

LxStringTableModel::LxStringTableModel(const QString &prStrData, QObject *parent) :
    QAbstractTableModel(parent)
{
    resetData(prStrData, true);
    mFirstRowGray = false;
    mSpliter = QChar(',');
}

void LxStringTableModel::resetData(const QString &prStrData, const bool newFirstt)
{
    if (!newFirstt)
        beginResetModel();

    mDataLines = prStrData.split("\n");

    if (prStrData.count(QChar(9)) >= mDataLines.count())
        mSpliter = QChar(9);        //TAB

    if (prStrData.isEmpty()) {
        mRows = 0;
        mCols = 0;
    } else {
        QStringList lsCells = mDataLines.at(0).split(mSpliter);
        mRows = mDataLines.count();
        mCols = lsCells.count();
    }

    if (!newFirstt)
        endResetModel();
}

int LxStringTableModel::rowCount(const QModelIndex &) const
{
    return mRows;
}

int LxStringTableModel::columnCount(const QModelIndex &) const
{
    return mCols;
}

QVariant LxStringTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::BackgroundRole) {
        if (index.row() == 0 ) {
            if (mFirstRowGray)
                return QColor(Qt::gray);
            else
                return QColor(Qt::white);
        }
    }

    if (role == Qt::DisplayRole ) {

        int iRow = index.row();
        int iCol = index.column();

        QStringList sCells = mDataLines.at(iRow).split(mSpliter);
        if (iCol < sCells.count())
            return sCells.at(iCol);
        else
            return QVariant();
    }

    return QVariant();
}

QVariant LxStringTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && mRows > 0) {
        if (orientation == Qt::Horizontal)
            return QString("<%1>").arg(section + 1);
        else if (orientation == Qt::Vertical)
            return section + 1;
    }
    return QVariant();
}

Qt::ItemFlags LxStringTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    flags |= Qt::ItemIsDragEnabled;
    return flags;
}

bool LxStringTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::BackgroundRole && index.isValid()) {
        mFirstRowGray = value.toBool();
        emit dataChanged(index, index);
        return true;
    } else
        return false;
}

void LxStringTableModel::setFirstRowColor(const bool isTitleGreyy)
{
    for (int i = 0; i < mCols; ++i) {
        QModelIndex idx = index(0, i, QModelIndex());
        setData(idx, isTitleGreyy, Qt::BackgroundRole);
    }
}


}
