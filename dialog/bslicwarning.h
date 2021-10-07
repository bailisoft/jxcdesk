#ifndef BSLICWARNING_H
#define BSLICWARNING_H

#include <QtWidgets>

namespace BailiSoft {

class BsLicWarning : public QLabel
{
    Q_OBJECT
public:
    BsLicWarning(QWidget *parent);
    ~BsLicWarning(){}

    static void popShow(QWidget *parent);
};

}

#endif // BSLICWARNING_H
