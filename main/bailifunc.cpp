#include "bailifunc.h"
#include "comm/pinyincode.h"

#include <math.h>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QtWidgets>

namespace BailiSoft {

QNetworkAccessManager       netManager;

//小端判断
bool is_little_endian()
{
  unsigned short flag = 0x4321;
  unsigned char * p = reinterpret_cast<unsigned char *>(&flag);
  return (*p == 0x21);
}

//判断使用系统是否Win64
bool osIsWin64 ()
{
    SYSTEM_INFO si;
    memset(&si, 0, sizeof(si));
    typedef void (WINAPI *LPFN_PGNSI)(LPSYSTEM_INFO);
    LPFN_PGNSI pGNSI = LPFN_PGNSI(GetProcAddress(GetModuleHandleA(("kernel32.dll")),"GetNativeSystemInfo"));

    if (pGNSI)
        pGNSI(&si);

    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
        return true;

    return false;
}

//产生随机字符串
QString generateReadableRandomString(const quint8 byteLen)
{
    qsrand(uint(QDateTime::currentMSecsSinceEpoch()));
    const int COUNT = 54;
    static const char charset[COUNT + 1] = "abcdefghjkmnpqrstuvwxyzABCDEFGHJKMNPQRSTUVWXYZ23456789";
    QString result;
    for (int i = 0; i < byteLen; i++) {
        result += charset[ qrand() % COUNT ];
    }
    return result;
}

QString generateRandomString(const quint8 byteLen)
{
    qsrand(uint(QDateTime::currentMSecsSinceEpoch()));
    const int COUNT = 62;
    static const char charset[COUNT + 1] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    QString result;
    for (int i = 0; i < byteLen; i++) {
        result += charset[ qrand() % COUNT ];
    }
    return result;
}

QByteArray generateRandomAsciiBytes(const quint32 byteLen)
{
    qsrand(uint(QDateTime::currentMSecsSinceEpoch()));
    const int COUNT = 92;
    static const char charset[COUNT + 1] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~`!@#$%^&*()_+-={}|[]:;'<>?,./";
    QString result;
    for (int i = 0; i < int(byteLen); i++) {
        result += charset[ qrand() % COUNT ];
    }
    return result.toLatin1();
}

QByteArray generateRandomBytes(const quint32 byteLen)
{
    qsrand(uint(QDateTime::currentMSecsSinceEpoch()));
    QByteArray result;
    for (int i = 0; i < int(byteLen); i++) {
        result += char((qrand() % 256));
    }
    return result;
}

QDate dateOfFormattedText(const QString &text, const QChar splittor)
{
    QDate date;
    QStringList ls = text.split(splittor);
    if (ls.length() == 3) {
        bool yOk, mOk, dOk;
        int y = QString(ls.at(0)).toInt(&yOk);
        int m = QString(ls.at(1)).toInt(&mOk);
        int d = QString(ls.at(2)).toInt(&dOk);
        if ( yOk && mOk && dOk ) {
            date = QDate(y, m, d);
        }
    }
    if ( date.isValid() )
        return date;
    else
        return QDateTime::currentDateTime().date();
}

QString fetchCodePrefix(const QString text)
{
    QString codes;
    for ( int i = 0, iLen = text.length(); i < iLen; ++i ) {
        QChar c = text.at(i);
        int x = c.unicode();
        if ( (x >= 48 && x <= 57) || (x >= 65 && x <= 90) || (x >= 97 && x <= 122) ) {
            codes += c;
        } else {
            break;
        }
    }
    return codes;
}

bool copyDir(const QString &source, const QString &destination, bool forceOverride)
{
    QDir directory(source);
    if (!directory.exists()) {
        return false;
    }

    QString srcPath = QDir::toNativeSeparators(source);
    if (!srcPath.endsWith(QDir::separator()))
        srcPath += QDir::separator();
    QString dstPath = QDir::toNativeSeparators(destination);
    if (!dstPath.endsWith(QDir::separator()))
        dstPath += QDir::separator();

    bool foundError = false;
    QStringList fileNames = directory.entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    for ( QStringList::size_type i = 0; i != fileNames.size(); ++i ) {
        QString fileName = fileNames.at(i);
        QString srcFilePath = srcPath + fileName;
        QString dstFilePath = dstPath + fileName;
        QFileInfo fileInfo(srcFilePath);
        if (fileInfo.isFile() || fileInfo.isSymLink()) {
            if (forceOverride) {
                QFile::setPermissions(dstFilePath, QFile::WriteOwner);
            }
            QFile::copy(srcFilePath, dstFilePath);
        }
        else if (fileInfo.isDir()) {
            QDir dstDir(dstFilePath);
            dstDir.mkpath(dstFilePath);
            if (!copyDir(srcFilePath, dstFilePath, forceOverride)) {
                foundError = true;
            }
        }
    }

    return !foundError;
}

void disableEditInputMethod(QLineEdit *edt)
{
#ifdef Q_OS_WIN
    edt->setAttribute(Qt::WA_InputMethodEnabled, false);
#else
    Q_UNUSED(edt);
#endif
}

//QMessageBox::warning(...);
//QMessageBox::critical(...);
//QMessageBox::information(...);
bool confirmDialog(QWidget *parent,
                   const QString &intro,
                   const QString &ask,
                   const QString &yes,
                   const QString &no,
                   QMessageBox::Icon icon)
{
    QMessageBox msgBox(parent);
    msgBox.setText(intro);
    msgBox.setInformativeText(ask);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.button(QMessageBox::Ok)->setText(yes);
    msgBox.button(QMessageBox::Cancel)->setText(no);
    msgBox.setIcon(icon);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.setFixedSize(msgBox.sizeHint());  //仅仅为了去掉讨厌的Unable to set geometry...的提示
    return (msgBox.exec() == QMessageBox::Ok);
}


QString openLoadTextFile(const QString &openWinTitle,
                         const QString &openFileInitDir,
                         const QString &openFileType,
                         QWidget *parentWin)
{
    QString fileName = QFileDialog::getOpenFileName(parentWin,
                                                    openWinTitle,
                                                    openFileInitDir,
                                                    openFileType
#ifdef Q_OS_MAC
                                                    ,0
                                                    ,QFileDialog::DontUseNativeDialog
#endif
                                                    );
    if (fileName.length() < 1)
        return QString();

    //判断文件是否为unicode编码
    if ( LxSoft::isTextUnicode(fileName) ) {
        QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    }

    //打开数据文件，并读入表格
    QString fileData;
    QFile f(fileName);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream strm(&f);
        fileData = strm.readAll();
        f.close();
    }
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("System"));

    //跨平台处理
    fileData.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    fileData.replace(QStringLiteral("\r"), QStringLiteral("\n"));

    //返回
    return fileData;
}


}

