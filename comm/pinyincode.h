#ifndef PINYINCODE_H
#define PINYINCODE_H

#include <QString>

namespace LxSoft {

extern bool isTextUnicode(const QString &file);

class ChineseConvertor {  // 根据GB2312编码
public:
    static QString GetFirstLetter(const QString &src);

private:
    static const char *kanjiFirstLetters;
};

}

#endif // PINYINCODE_H
