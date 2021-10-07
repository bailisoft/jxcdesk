#ifndef BAILIFUNC_H
#define BAILIFUNC_H

#include <QtWidgets>
#include <QMessageBox>
#include <QNetworkAccessManager>

namespace BailiSoft {

//全局http访问器（Splash窗口升级功能共用，此对象Qt文档说明本就应当全局共用）
extern QNetworkAccessManager    netManager;

//小端判断
extern bool is_little_endian();
extern bool osIsWin64 ();

//产生随机字符串
extern QString generateReadableRandomString(const quint8 byteLen);
extern QString generateRandomString(const quint8 byteLen);
extern QByteArray generateRandomAsciiBytes(const quint32 byteLen);
extern QByteArray generateRandomBytes(const quint32 byteLen);

extern QDate dateOfFormattedText(const QString &text, const QChar splittor);
extern QString fetchCodePrefix(const QString text);

extern void disableEditInputMethod(QLineEdit *edt);

//通用对话框
extern bool confirmDialog(QWidget *parent,
                          const QString &intro,
                          const QString &ask,
                          const QString &yes,
                          const QString &no,
                          QMessageBox::Icon icon = QMessageBox::Question);

//递归拷贝目录
extern bool copyDir(const QString &source, const QString &destination, bool forceOverride);

//读取文本文件
extern QString openLoadTextFile(const QString &openWinTitle,
                                const QString &openFileInitDir,
                                const QString &openFileType,
                                QWidget *parentWin = nullptr);
}

#endif // BAILIFUNC_H
