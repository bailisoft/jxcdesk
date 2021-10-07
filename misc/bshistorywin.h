#ifndef BSHISTORYWIN_H
#define BSHISTORYWIN_H

#include <QtWidgets>

namespace BailiSoft {

class BsQryWin;
class BsQueryGrid;

class BsHistoryWin : public QWidget
{
    Q_OBJECT
public:
    explicit BsHistoryWin(QWidget *parent, BsQryWin *qryWin, const QStringList &labelPairs, const QString &sql,
                          const QStringList &colTitles, const QString &sizerType = QString(),
                          const bool hasQcValue = false);
    ~BsHistoryWin(){}

protected:
    void closeEvent(QCloseEvent *e);

private slots:
    void doPrint();
    void doExport();

private:
    QLabel*         mpCons;
    BsQueryGrid*    mpGrid;

    BsQryWin*       mppQryWin;
    QStringList     mLabelPairs;
};

}

#endif // BSHISTORYWIN_H
