#ifndef BSPICKDATEDLG_H
#define BSPICKDATEDLG_H

#include <QtWidgets>

namespace BailiSoft {

class BsField;
class BsFldEditor;

class BsPickDateDlg : public QDialog
{
    Q_OBJECT
public:
    explicit BsPickDateDlg(QWidget *parent);
    ~BsPickDateDlg();

    QGroupBox*          mpHisType;
    QRadioButton*           mpTypeSpec;
    QRadioButton*           mpTypeLeft;
    QLabel*             mpLabel;
    BsFldEditor*      mpEdtDate;

protected:
    void keyPressEvent(QKeyEvent * e);

private slots:
    void typeLeftToggled(bool checked);

private:
    BsField*    mFldDate;

};

}

#endif // BSPICKDATEDLG_H
