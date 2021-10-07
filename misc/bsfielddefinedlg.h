#ifndef BSFIELDDEFINEDLG_H
#define BSFIELDDEFINEDLG_H

#include <QtWidgets>

namespace BailiSoft {

class BsField;

// BsFieldDefineDlg
class BsFieldDefineDlg : public QDialog
{
    Q_OBJECT
public:
    explicit BsFieldDefineDlg(QWidget *parent, const QString &tbl, const QList<BsField*> &flds);

    QFormLayout*        mpForm;
    QDialogButtonBox*   mpBox;

    QString             mTable;
    QList<BsField*>     mFields;

protected:
    void keyPressEvent(QKeyEvent *e);

};

}

#endif // BSFIELDDEFINEDLG_H
