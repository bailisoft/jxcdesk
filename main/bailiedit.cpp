#include "bailiedit.h"
#include "bailigrid.h"
#include "bailidata.h"
#include "bailicode.h"
#include <QtSql>


// QValidator
namespace BailiSoft {

QValidator::State BsDateValidator::validate(QString &input, int &pos) const
{
    const QString pattern = QStringLiteral("2000-11-11");
    QString future = input.left(pos) + pattern.mid(pos);
    QDate futureDate = QDate::fromString(future, QStringLiteral("yyyy-MM-dd"));
    if ( ! futureDate.isValid() ) {
        if ( pos == 6 && input.at(5).isDigit() ) {
            input = input.left(5) + '0' + input.at(5) + '-';
            pos = 8;
            return QValidator::Acceptable;
        }
        if ( pos == 9 && input.at(8).isDigit() ) {
            input = input.left(8) + '0' + input.at(8);
            pos = 11;
            return QValidator::Acceptable;
        }
        return QValidator::Invalid;
    }
    return QValidator::Acceptable;
}

BsCompleter::BsCompleter(QObject *parent, const bool useCode) : QCompleter(parent), mUseCode(useCode)
{
    //Nothing
}

QString BsCompleter::pathFromIndex(const QModelIndex &index) const
{
    if ( index.isValid() ) {
        QString useValue = (mUseCode)
                ? model()->data(index).toString().split(QChar(32), QString::SkipEmptyParts).at(0)
                : model()->data(index).toString();
        //emit pickedValue(useValue);
        return useValue;
    }
    return QString();
}

}

// BsFldEditor
namespace BailiSoft {

BsFldEditor::BsFldEditor(QWidget *parent, BsField* field, BsAbstractModel *pickModel)
    : QLineEdit(parent), mpField(field), mpModel(pickModel),
      mpView(nullptr), mpCompleter(nullptr), mpCalendar(nullptr)
{
    uint df = field->mFlags;

    //integer
    if ( (df & bsffInt) == bsffInt ) {

        if ( (df & bsffDate) == bsffDate ) {

            setValidator(new BsDateValidator(this));

            mpCalendar = new QCalendarWidget;
            mpCalendar->setMinimumDate(QDate::fromString(QStringLiteral("2001-01-01"), QStringLiteral("yyyy-MM-dd")));
            mpCalendar->setSelectedDate(QDate::currentDate());
            mpCalendar->setFirstDayOfWeek(Qt::Monday);
            mpCalendar->setGridVisible(true);
            mpCalendar->setHorizontalHeaderFormat(QCalendarWidget::LongDayNames);
            mpCalendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
            mpCalendar->setNavigationBarVisible(true);
            mpCalendar->setWindowFlags(Qt::Popup);
            for ( int i = 0, iLen = mpCalendar->children().length(); i < iLen; ++i ) {
                QObject *obj = mpCalendar->children().at(i);
                if ( QString(obj->metaObject()->className()) == QString("QCalendarView") ) {
                    obj->installEventFilter(this);
                }
            }
            connect(mpCalendar, SIGNAL(clicked(QDate)), this, SLOT(setPickedDate(QDate)));
            connect(this, SIGNAL(editingFinished()), this, SLOT(checkDateValid()));
        }

        else if ( (df & bsffNumeric) == bsffNumeric ) {
            QDoubleValidator *val = new QDoubleValidator(this);
            val->setNotation(QDoubleValidator::StandardNotation);
            val->setDecimals(field->mLenDots);
            setValidator(val);
        }

        else if ( (df & bsffBool) != bsffBool ) {
            setValidator(new QIntValidator(this));
        }

        //不会有bool型editor
#ifdef Q_OS_WIN
        setAttribute(Qt::WA_InputMethodEnabled, false);
#endif
    }
    //text
    else if ( pickModel ) {
        mpView = new QTableView(this);
        bool useCodee = field->mFldName == QStringLiteral("cargo") || field->mFldName == QStringLiteral("subject");
        mpCompleter = new BsCompleter(this, useCodee);
        mpCompleter->setModel(pickModel);
        mpCompleter->setFilterMode(Qt::MatchContains);
        mpCompleter->setCompletionColumn(0);
        mpCompleter->setCaseSensitivity(Qt::CaseInsensitive);
        mpCompleter->setWrapAround(false);
        mpCompleter->setMaxVisibleItems(15);
        mpCompleter->setPopup(mpView);

        mpView->setMinimumHeight(200);
        mpView->verticalHeader()->hide();
        mpView->verticalHeader()->setDefaultSectionSize(20);
        mpView->setSelectionBehavior(QAbstractItemView::SelectRows);
        mpView->setSelectionMode(QAbstractItemView::SingleSelection);
        mpView->horizontalHeader()->hide();
        mpView->horizontalHeader()->setStretchLastSection(true);
        mpView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
        mpView->setStyleSheet(QLatin1String("QTableView {"
                                            "   background-color: #cce8ff; "
                                            "   selection-color: #000; "
                                            "   selection-background-color: qlineargradient("
                                            "       x1: 0, y1: 0, x2: 0, y2: 1, "
                                            "       stop: 0 #ddd, stop: 0.5 #fff, stop: 1 #ddd); "
                                            "}"));
        setCompleter(mpCompleter);
        connect(this, &QLineEdit::textChanged, this, &BsFldEditor::editingChanged);
#ifdef Q_OS_WIN
        if ( field->mFldName != QStringLiteral("colortype") ) {
            setAttribute(Qt::WA_InputMethodEnabled, false);
        }
#endif
    }
}

BsFldEditor::~BsFldEditor()
{
    if ( mpCalendar )
        delete mpCalendar;
}

void BsFldEditor::setDataValue(const QVariant &value)
{
    QString newText = getTextFromValue(value);
    if ( newText != text() ) {
        mOldTextValue = newText;
        setText(newText);
    }
}

QString BsFldEditor::getDataValue() const
{
    return getStringForSave(text().trimmed());
}

void BsFldEditor::restoreValue()
{
    setText(mOldTextValue);
}

QString BsFldEditor::getDataValueForSql() const
{
    QString txt = getStringForSave(text()).replace(QChar(39), QString()).trimmed();
    return ( mpField->mFlags & bsffText ) ? QChar(39) + txt + QChar(39) : txt;
}

bool BsFldEditor::isDirty() const
{
    return text() != mOldTextValue;
}

bool BsFldEditor::inputIsValid() const
{
    if ( mpModel ) {
        return mpModel->keyValueRowExists(text());
    }
    else if ( (mpField->mFlags & bsffNumeric) == bsffNumeric ) {
        bool ok;
        if ( mpField->mLenDots > 0 ) {
            double v = text().toDouble(&ok);
            Q_UNUSED(v);
        }
        else {
            qint64 v = text().toLongLong(&ok);
            Q_UNUSED(v);
        }
        return ok;
    }
    else if ( (mpField->mFlags & bsffDate) == bsffDate ) {
        QDate v = QDate::fromString(text());
        return v.isValid();
    }
    else if ( (mpField->mFlags & bsffDateTime) == bsffDateTime ) {
        QDateTime v = QDateTime::fromString(text());
        return v.isValid();
    }

    return true;
}

void BsFldEditor::setMyPlaceText(const QString &txt)
{
    mPlaceText = txt;
}

void BsFldEditor::setMyPlaceColor(const QColor &clr)
{
    mPlaceColor = clr;
}

bool BsFldEditor::eventFilter(QObject *obj, QEvent *ev)
{
    if ( mpCalendar ) {
        if ( ev->type() == QEvent::KeyPress ) {
            QKeyEvent *kev = static_cast<QKeyEvent*>(ev);
            if ( kev->key() == Qt::Key_Enter || kev->key() == Qt::Key_Return ) {
                setText(mpCalendar->selectedDate().toString("yyyy-MM-dd"));
                mpCalendar->hide();
                return true;
            }
            else if ( kev->key() == Qt::Key_Backspace || kev->key() == Qt::Key_Delete ) {
                mpCalendar->hide();
                return true;
            }
        }
    }
    return QLineEdit::eventFilter(obj, ev);
}

void BsFldEditor::paintEvent(QPaintEvent *e)
{
    QLineEdit::paintEvent(e);
    if ( !hasFocus() && text().isEmpty() && !mPlaceText.isEmpty() ) {

        //为了自定有色placeholder，不能使用原setPlacehoderText()
        if ( !placeholderText().isEmpty() ) {
            qDebug() << "Don't use old setPlaceholder().";
            Q_ASSERT(1==2);
        }

        QPainter p(this);
        p.setPen(mPlaceColor);
        QFontMetrics fm = fontMetrics();
        int minLB = qMax(0, -fm.minLeftBearing());
        QRect lineRect = rect();
        QRect ph = lineRect.adjusted(minLB + 3, 0, 0, 0);
        QString etxt = fm.elidedText(mPlaceText, Qt::ElideRight, ph.width());
        p.drawText(ph, Qt::AlignVCenter, etxt);
    }
}

void BsFldEditor::keyPressEvent(QKeyEvent *e)
{
    if ( !isReadOnly() && mpCompleter ) {

        if ( mNewing ) {
            checkShowAllPopList();
            if ( e->key() == Qt::Key_Space ) {
                return;
            } else {
                mNewing = false;
            }
        }
        else if ( text().isEmpty() && (e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete) ) {
            checkShowAllPopList();
            mNewing = true;
            return;
        }
    }

    mNewing = false;

    QLineEdit::keyPressEvent(e);
}

void BsFldEditor::mouseDoubleClickEvent(QMouseEvent *e)
{
    QLineEdit::mouseDoubleClickEvent(e);

    if ( !isReadOnly() ) {

        checkShowAllPopList();

        if ( mpCalendar ) {
            mpCalendar->show();
            mpCalendar->setSelectedDate(QDate::currentDate());
            mpCalendar->setFocus();
            QPoint p = mapToGlobal(QPoint(0, 0));
            mpCalendar->move(p.x(), p.y() + height());
        }
    }
}

void BsFldEditor::checkDateValid()
{
    QDate d = QDate::fromString(text(), "yyyy-MM-dd");
    if ( d.isValid() )
        setText(d.toString("yyyy-MM-dd"));
    else if ( isReadOnly() )
        setText(QString());
    else
        setText(QDate::currentDate().toString("yyyy-MM-dd"));
}

void BsFldEditor::setPickedDate(const QDate &date)
{
    setText(date.toString("yyyy-MM-dd"));
    mpCalendar->hide();
}

void BsFldEditor::checkShowAllPopList()
{
    //去掉筛选并显示
    if ( mpCompleter ) {
        mpCompleter->setCompletionPrefix(QString());
        mpCompleter->complete();
    }
}

QString BsFldEditor::getTextFromValue(const QVariant &value) const
{
    uint df = mpField->mFlags;

    if ( (df & bsffInt) == bsffInt ) {

        if ( (df & bsffDate) == bsffDate ) {
            if ( value.toLongLong() == 0 )
                return QString();
            QDate dt = QDateTime::fromMSecsSinceEpoch(1000 * value.toLongLong()).date();
            return dt.toString("yyyy-MM-dd");
        }

        else if ( (df & bsffNumeric) == bsffNumeric ) {
            qint64 intx = value.toLongLong();
            return bsNumForRead(intx, mpField->mLenDots);
        }

        else {
            return QString::number(value.toLongLong());
        }
    }

    else {
        return value.toString();
    }
}

QString BsFldEditor::getStringForSave(const QString &text) const
{
    uint df = mpField->mFlags;

    if ( (df & bsffInt) == bsffInt ) {

        if ( (df & bsffDate) == bsffDate ) {
            QDate dt = QDate::fromString(text, QStringLiteral("yyyy-MM-dd"));
            return QString::number(QDateTime(dt).toMSecsSinceEpoch() / 1000);
        }

        else if ( (df & bsffNumeric) == bsffNumeric ) {
            return bsNumForSave(text.toDouble());
        }

        else {
            int v = text.toInt();
            return QString::number(v);
        }
    }

    else {
        return text;
    }
}

}


// BsSheetIdLabel
namespace BailiSoft {

BsSheetIdLabel::BsSheetIdLabel(QWidget *parent, const QString &sheetTable)
    : QLabel(parent), mSheetTable(sheetTable), mSheetId(0)
{
    setStyleSheet(QLatin1String("QLabel{font-size:10pt;}"));
}

void BsSheetIdLabel::setDataValue(const QVariant &value)
{
    int iValue = value.toInt();
    if ( iValue < 0 ) {
        QString txt = mapMsg.value("i_new_sheet_to_be_created");
        setText(QString("<font color='gray'>%1</font><font color='green'>%2</font>").arg(mapMsg.value("i_sheet_id_label")).arg(txt));
    } else if ( iValue == 0 ) {
        setText(QStringLiteral("-"));
    } else {
        QString txt = QStringLiteral("%1%2").arg(mSheetTable.toUpper()).arg(value.toInt(), 8, 10, QLatin1Char('0'));
        setText(QString("<font color='gray'>%1</font><b>%2</b>").arg(mapMsg.value("i_sheet_id_label")).arg(txt));
    }
    mSheetId = value.toInt();
}

QString BsSheetIdLabel::getDisplayText() const
{
    return QStringLiteral("%1%2").arg(mSheetTable.toUpper()).arg(mSheetId, 8, 10, QLatin1Char('0'));
}

}


// BsSheetCheckLabel
namespace BailiSoft {

BsSheetCheckLabel::BsSheetCheckLabel(QWidget *parent) : QLabel(parent), mChecked(false)
{
    setAlignment(Qt::AlignCenter);
}

void BsSheetCheckLabel::setDataValue(const QVariant &value, const int sheetId)
{
    if ( sheetId > 0 )
    {
        if ( value.toBool() )
        {
            mChecked = true;
            setText(mapMsg.value("i_sheet_checked"));
            setStyleSheet(QLatin1String("QLabel{border:2px solid red; color:red; font-size:12pt; padding:-2px 6px 0 6px;}"));
        }
        else
        {
            mChecked = false;
            setText(mapMsg.value("i_sheet_unchecked"));
            setStyleSheet(QLatin1String("QLabel{border:2px solid silver; color:silver; font-size:12pt; padding:-2px 6px 0 6px;}"));
        }
    }
    else
    {
        mChecked = false;
        setText(QString());
        setStyleSheet(QLatin1String("QLabel{border-style:none;}"));
    }
}

bool BsSheetCheckLabel::getDataCheckedValue()
{
    return mChecked;
}

}


//BsConCheck
namespace BailiSoft {

BsConCheck::BsConCheck(QWidget *parent, const bool useTristate) : QCheckBox(parent)
{
    setText(mapMsg.value("i_con_check_noway"));
    setTristate(useTristate);
    if ( useTristate ) {
        setCheckState(Qt::PartiallyChecked);
        setStyleSheet(QLatin1String("QCheckBox:checked{color:#f00;} "
                                    "QCheckBox:unchecked{color:#290;}"
                                    "QCheckBox:indeterminate{color:#000;} "));
    }
    else {
        setChecked(Qt::Unchecked);
    }
    connect(this, SIGNAL(stateChanged(int)), this, SLOT(stateChanged(int)));
}

QString BsConCheck::getConExp()
{
    if ( isTristate() ) {
        switch ( checkState() ) {
        case Qt::Checked:
            return QStringLiteral("chktime<>0");
        case Qt::Unchecked:
            return QStringLiteral("chktime=0");
        default:
            return QString();
        }
    }
    else {
        return ( checkState() == Qt::Checked ) ? QStringLiteral("chktime<>0") : QString();
    }
}

void BsConCheck::stateChanged(int state)
{
    if ( isTristate() ) {
        switch ( state ) {
        case Qt::Checked:
            setText(mapMsg.value("i_con_checked"));
            break;
        case Qt::Unchecked:
            setText(mapMsg.value("i_con_uncheck"));
            break;
        default:
            setText(mapMsg.value("i_con_check_noway"));
            break;
        }
    }
    else {
        setText((state == Qt::Checked) ? mapMsg.value("i_con_checked") : mapMsg.value("i_con_check_noway"));
    }
}

}


// BsNoBorderDelegate
namespace BailiSoft {

BsNoBorderDelegate::BsNoBorderDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

void BsNoBorderDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //draw option
    QStyleOptionViewItem  opt(option);

    //去焦点虚线框
    if (opt.state & QStyle::State_HasFocus)
        opt.state = opt.state ^ QStyle::State_HasFocus;

    //使用原项前景色，不使用缺少反色。这样如果原色多样，选择后，仍然是多样色
    QColor fColor = index.data(Qt::ForegroundRole).value<QColor>();
    if (fColor.isValid())
        opt.palette.setColor(QPalette::HighlightedText, fColor);
    else
        opt.palette.setColor(QPalette::HighlightedText, QColor(Qt::black));

    //draw start
    QStyledItemDelegate::paint(painter, opt, index);
}

}


// BsPickDelegate
namespace BailiSoft {

BsPickDelegate::BsPickDelegate(QObject *parent, BsField *field, BsAbstractModel *pickModel)
    : QStyledItemDelegate(parent), mpField(field), mpModel(pickModel)
{
    //Nothing
}

QWidget *BsPickDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    BsFldEditor    *edt = new BsFldEditor(parent, mpField, mpModel);
    edt->mNewing = true;
    return edt;
}

void BsPickDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    BsFldEditor *edt = qobject_cast<BsFldEditor *>(editor);
    edt->setText(index.model()->data(index).toString());
}

void BsPickDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    BsFldEditor *edt = qobject_cast<BsFldEditor *>(editor);
    model->setData(index, edt->text());
}

}


// BsCheckDelegate
namespace BailiSoft {

BsCheckDelegate::BsCheckDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    //Nothing
}

QWidget *BsCheckDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    QCheckBox* chk = new QCheckBox(parent);
    return chk;
}

void BsCheckDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    bool checked = !index.model()->data(index).toString().isEmpty();

    QCheckBox* chk = static_cast<QCheckBox*>(editor);
    if (!chk) {
        qCritical("Couldn't retrieve QCheckBox in BsCheckDelegate.");
        return;
    }

    chk->setChecked(checked);
}

void BsCheckDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QCheckBox* chk = static_cast<QCheckBox*>(editor);
    if (!chk) {
        qCritical("Couldn't retrieve QCheckBox in BsCheckDelegate.");
        return;
    }
    model->setData(index, ((chk->isChecked()) ? mapMsg.value("word_yes") : QString()));
}

void BsCheckDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QCheckBox checkbox;
    QSize size = checkbox.sizeHint();

    QStyleOptionViewItem  opt(option);
    opt.rect.setX(option.rect.x() + (option.rect.width() - size.width()) / 2);

    QStyledItemDelegate::updateEditorGeometry(editor, opt, index);
}

}


//BsFldBox
namespace BailiSoft {

BsFldBox::BsFldBox(QWidget *parent, BsField *field, BsAbstractModel *pickModel, const bool asQryCon)
    : QWidget(parent)
{
    QString nameText = field->mFldCnName;
    QString showText;
    if ( nameText.length() == 2 ) {
        showText = nameText.left(1) + QStringLiteral("\u3000\u3000") + nameText.right(1);
    }
    else if ( nameText.length() == 3 ) {
        showText = nameText.left(1) + QStringLiteral("\u2002") + nameText.mid(1, 1) + QStringLiteral("\u2002") + nameText.right(1);
    }
    else {
        showText = nameText;
    }

    QFontMetrics fm(font());
    mpLabel = new QLabel(this);
    mpLabel->setText(QStringLiteral("%1：").arg(showText));
    mpLabel->setFixedWidth(fm.horizontalAdvance(QStringLiteral("字字字字：")));
    mpLabel->setStyleSheet(QLatin1String("color:#666;"));

    mpEditor = new BsFldEditor(this, field, pickModel);
    if ( asQryCon ) {
        mpEditor->setStyleSheet(QLatin1String("QLineEdit{color:#290;}"));
    } else {
        mpEditor->setStyleSheet(QLatin1String("QLineEdit{border-style:none; border-bottom:1px solid #666; color:blue;} "
                                              "QLineEdit:read-only{color:black;}"));
    }

    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    lay->addWidget(mpLabel);
    lay->addWidget(mpEditor, 1);

    connect(mpEditor, SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));      //以后有空全改为新语法
    connect(mpEditor, &BsFldEditor::editingChanged, this, &BsFldBox::editingChanged);   //新语法，好！
}

QString BsFldBox::getFieldName()
{
    return mpEditor->mpField->mFldName;
}

QString BsFldBox::getFieldCnName()
{
    return mpLabel->text().replace(QRegularExpression("[\u3000\u2002：]"), QString());
}

void BsFldBox::disableShow()
{
    mpEditor->setEnabled(false);
    mpEditor->setStyleSheet("color:gray;");
}

}


// BsQryCheckor
namespace BailiSoft {

BsQryCheckor::BsQryCheckor(QWidget *parent, BsField *field, const bool fontBoldd)
    : QCheckBox(parent), mField(field)
{
    mFlags = mField->mFlags;
    setText(field->mFldCnName);
    if ( fontBoldd )
        setStyleSheet(QLatin1String("QCheckBox{font-weight:900;text-decoration:underline;} QCheckBox:checked{color:#290;}"));
    else
        setStyleSheet(QLatin1String("QCheckBox:checked{color:#290;}"));
}

}

//// BsQryRadio
//namespace BailiSoft {

//BsQryRadio::BsQryRadio(QWidget *parent, BsField *field) : QRadioButton(parent), mField(field)
//{
//    mFlags = mField->mFlags;
//    setText(field->mFldCnName);
//    setStyleSheet(QLatin1String("QRadioButton{font-weight:900;text-decoration:underline;}"));
//}

//}

////factory method
//namespace BailiSoft {

//QAbstractButton *newBsQryButton(QWidget *parent, BsField* field, const bool radioo, const bool fontBoldd)
//{
//    if ( radioo )
//        return new BsQryCheckor(parent, field, fontBoldd);
//    else
//        return new BsQryRadio(parent, field);
//}

//}
