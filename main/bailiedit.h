#ifndef BAILIEDIT_H
#define BAILIEDIT_H

#include <QtCore>
#include <QObject>
#include <QtWidgets>

// class
namespace BailiSoft {
class BsField;
class BsAbstractModel;
}


// BsDateValidator
namespace BailiSoft {

class BsDateValidator : public QValidator {
    Q_OBJECT
public:
    explicit BsDateValidator(QObject *parent) : QValidator(parent) {}
    State validate(QString &input, int &pos) const;
};

}


// BsCompleter
namespace BailiSoft {

class BsCompleter : public QCompleter
{
    Q_OBJECT
public:
    explicit BsCompleter(QObject *parent, const bool useCode = false);
    QString pathFromIndex(const QModelIndex &index) const;
//signals:
//    void pickedValue(const QString &text) const;
private:
    bool mUseCode;
};

}


// BsFldEditor
namespace BailiSoft {

class BsFldEditor : public QLineEdit
{
    Q_OBJECT
public:
    BsFldEditor(QWidget *parent, BsField* field, BsAbstractModel *pickModel = nullptr);
    ~BsFldEditor();

    void        setDataValue(const QVariant &value);
    QString     getDataValue() const;
    void        restoreValue();
    QString     getDataValueForSql() const;
    bool        isDirty() const;
    bool        inputIsValid() const;

    void setMyPlaceText(const QString &txt);
    void setMyPlaceColor(const QColor &clr);

    BsField             *mpField;
    bool                mNewing;

signals:
    void editingChanged(const QString &text);

protected:
    bool eventFilter(QObject *obj, QEvent *ev);
    void paintEvent(QPaintEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);

    BsAbstractModel     *mpModel;
    QTableView          *mpView;
    BsCompleter         *mpCompleter;
    QCalendarWidget     *mpCalendar;

private slots:
    void checkDateValid();
    void setPickedDate(const QDate &date);

private:
    void        checkShowAllPopList();
    QString     getTextFromValue(const QVariant &value) const;
    QString     getStringForSave(const QString &text) const;

    QString             mOldTextValue;
    QString             mPlaceText;
    QColor              mPlaceColor;
};

}


// BsSheetIdLabel
namespace BailiSoft {

class BsSheetIdLabel : public QLabel
{
    Q_OBJECT
public:
    BsSheetIdLabel(QWidget *parent, const QString &sheetTable);
    void setDataValue(const QVariant &value);
    QString getDisplayText() const;
private:
    QString mSheetTable;
    int mSheetId;
};

}


// BsSheetCheckLabel
namespace BailiSoft {

class BsSheetCheckLabel : public QLabel
{
    Q_OBJECT
public:
    BsSheetCheckLabel(QWidget *parent);
    void setDataValue(const QVariant &value, const int sheetId);
    bool getDataCheckedValue();
private:
    bool mChecked;
};

}


//BsConCheck
namespace BailiSoft {

class BsConCheck : public QCheckBox
{
    Q_OBJECT
public:
    explicit BsConCheck(QWidget *parent, const bool useTristate);
    QString getConExp();

private slots:
    void stateChanged(int state);
};

}


// BsNoBorderDelegate
namespace BailiSoft {

class BsNoBorderDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit BsNoBorderDelegate(QObject *parent = Q_NULLPTR);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

}


// BsPickDelegate
namespace BailiSoft {

class BsPickDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit BsPickDelegate(QObject *parent, BsField* field, BsAbstractModel *pickModel);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

private:
    BsField             *mpField;
    BsAbstractModel     *mpModel;
};

}


// BsCheckDelegate
namespace BailiSoft {

class BsCheckDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit BsCheckDelegate(QObject *parent);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget*editor, const QStyleOptionViewItem& option, const QModelIndex&index) const;
};

}


// BsFldBox
namespace BailiSoft {

class BsFldBox : public QWidget
{
    Q_OBJECT
public:
    explicit BsFldBox(QWidget *parent, BsField* field, BsAbstractModel *pickModel = nullptr, const bool asQryCon = false);
    QString     getFieldName();
    QString     getFieldCnName();
    void        disableShow();

    QLabel        *mpLabel;
    BsFldEditor   *mpEditor;

signals:
    void editingFinished();
    void editingChanged(const QString &text);
};

}


// BsQryCheckor
namespace BailiSoft {

class BsQryCheckor : public QCheckBox
{
    Q_OBJECT
public:
    explicit BsQryCheckor(QWidget *parent, BsField* field, const bool fontBoldd = false);
    BsField     *mField;
    uint         mFlags;
};

}


//// BsQryRadio
//namespace BailiSoft {

//class BsQryRadio : public QRadioButton
//{
//    Q_OBJECT
//public:
//    explicit BsQryRadio(QWidget *parent, BsField* field);
//    BsField     *mField;
//    uint         mFlags;
//};

//}


////factory method
//namespace BailiSoft {

//extern QAbstractButton *newBsQryButton(QWidget *parent, BsField* field, const bool radioo = false, const bool fontBoldd = false);

//}


#endif // BAILIEDIT_H
