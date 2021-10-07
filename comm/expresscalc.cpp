#include "expresscalc.h"

#include <math.h>
#include <QString>
#include <QStringList>
#include <QDebug>


namespace BailiSoft {

//单元私有函数
//foundPos指(-  +-  --  *-  /-  %-六种情况出现的位置
QString replaceMinusSign(const QString &prExp, const int foundPos)
{
    int i = foundPos + 2;
    while ( i < prExp.size() && (prExp.at(i).isNumber() || prExp.at(i) == QChar('.')) )
        ++i;

    QString str(prExp.left(foundPos + 1));
    str += QStringLiteral("(0");
    str += prExp.mid(foundPos + 1, (i - foundPos - 1));
    str += QStringLiteral(")");

    if ( i < prExp.size() )
        str += prExp.mid(i);

    return str;
}

//单元私有函数
bool lxOutestParse(const QString &prExp, QString &prOp, QStringList &prParams)
{
    QString str = prExp.toUpper();

    //去空格
    str.replace(QChar(32), QString());
    if (str.isEmpty())
        return false;

    //去头部等于号
    if (str.at(0) == '=')
        str = str.mid(1);

    //负数开头，前面补0
    if (str.at(0) == QChar('-'))
        str.prepend("0");

    //其它负数情况替换
    int pos;
    while ((pos = str.indexOf("(-")) >= 0)
        str = replaceMinusSign(str, pos);
    while ((pos = str.indexOf("+-")) >= 0)
        str = replaceMinusSign(str, pos);
    while ((pos = str.indexOf("--")) >= 0)
        str = replaceMinusSign(str, pos);
    while ((pos = str.indexOf("*-")) >= 0)
        str = replaceMinusSign(str, pos);
    while ((pos = str.indexOf("/-")) >= 0)
        str = replaceMinusSign(str, pos);
    while ((pos = str.indexOf("%-")) >= 0)
        str = replaceMinusSign(str, pos);

    //去最外层多余括号
    while (str.at(0) == '(' && str.at(str.size()-1) == ')' )
    {
        bool outestCanRemove = true;

        int iHalfLetfs = 0;
        for (int i = 1; i < str.size() - 1; ++i) {
            if (str.at(i) == '(')
                ++iHalfLetfs;
            if (str.at(i) == ')')
                --iHalfLetfs;
            if ( iHalfLetfs < 0 ) {
                outestCanRemove = false;
                break;
            }
        }

        if ( outestCanRemove )
            str = str.mid(1, str.size() - 2);
        else
            break;
    }
    if (str.isEmpty())
        return false;

    //开头仅允许数字或字母开头，或者左括号
    if (    (str.at(0).unicode() < 48 && str.at(0) != 40)
            || (str.at(0).unicode() > 57 && str.at(0).unicode() < 65)
            || (str.at(0).unicode() > 90)     )
        return false;

    //准备
    prOp.clear();
    prParams.clear();

    //先加减（最外层）
    int iInBrckets = 0;
    for (int i = 0; i < str.size(); ++i) {

        if (str.at(i) == '(')
            ++iInBrckets;
        if (str.at(i) == ')')
            --iInBrckets;

        if ( (str.at(i) == '+' || str.at(i) == '-')  &&  iInBrckets == 0 && i > 0) {
            if (str.at(i) == '+')
                prOp = QStringLiteral("+");
            else
                prOp = QStringLiteral("-");
            prParams.append(str.left(i));
            prParams.append(str.mid(i + 1));
            return true;
        }
    }

    //再乘除（最外层）
    iInBrckets = 0;
    for (int i = 0; i < str.size(); ++i) {

        if (str.at(i) == '(')
            ++iInBrckets;
        if (str.at(i) == ')')
            --iInBrckets;

        if ( (str.at(i) == '*' || str.at(i) == '/')  &&  iInBrckets == 0 && i > 0) {
            if (str.at(i) == '*')
                prOp = QStringLiteral("*");
            else
                prOp = QStringLiteral("/");
            prParams.append(str.left(i));
            prParams.append(str.mid(i + 1));
            return true;
        }
    }

    //再求余（最外层）
    iInBrckets = 0;
    for (int i = 0; i < str.size(); ++i) {

        if (str.at(i) == '(')
            ++iInBrckets;
        if (str.at(i) == ')')
            --iInBrckets;

        if (str.at(i) == '%'  &&  iInBrckets == 0 && i > 0) {
            prOp = QStringLiteral("%");
            prParams.append(str.left(i));
            prParams.append(str.mid(i + 1));
            return true;
        }
    }

    //最后函数（排除了外层二元运算符，必然以函数名开头，括号结尾）
    int iStart = str.indexOf('(');
    prOp = str.left(iStart);

    str = str.mid(iStart + 1);          //去函数名头和其头括号
    str = str.left(str.size() - 1);     //去尾括号

    int iPos = 0;
    iInBrckets = 0;
    int iCountLen = 0;
    for (int i = 0; i < str.size(); ++i) {

        if (str.at(i) == '(')
            ++iInBrckets;
        if (str.at(i) == ')')
            --iInBrckets;

        ++iCountLen;

        if ( (str.at(i) == ',' || i == str.size() - 1) &&  iInBrckets == 0) {
            if (i == str.size() - 1) {
                prParams.append(str.mid(iPos));
                return true;
            } else {
                prParams.append(str.mid(iPos, iCountLen - 1));
                iPos = i + 1;
                iCountLen = 0;
            }
        }
    }

    //意外
    return false;
}

//单元私有函数
int lxGetExpComplexity(const QString &prExp) {
    QString str = prExp.toUpper();
    int iCount = 0;
    iCount += str.count(QChar('+'));
    iCount += str.count(QChar('-'));
    iCount += str.count(QChar('*'));
    iCount += str.count(QChar('/'));
    iCount += str.count("FMOD(");
    iCount += str.count("FLOOR(");
    iCount += str.count("CEIL(");
    iCount += str.count("ABS(");
    iCount += str.count("FABS(");
    iCount += str.count("SQRT(");
    iCount += str.count("POW(");
    iCount += str.count("POWER(");
    iCount += str.count("LOG(");
    iCount += str.count("LN(");
    iCount += str.count("LOG10(");
    iCount += str.count("LG(");
    iCount += str.count("LG(");
    iCount += str.count("SIN(");
    iCount += str.count("COS(");
    iCount += str.count("TAN(");
    iCount += str.count("ASIN(");
    iCount += str.count("ACOS(");
    iCount += str.count("ATAN(");
    iCount += str.count("ATAN2(");
    iCount += str.count("ROUND(");
    return iCount;
}

//单元私有函数
double lxRound(double dVal, short iDigits)
{
    double dRst;
    double dMod = (dVal < 0) ? -0.0000001 : 0.0000001;

    dRst = dVal;
    dRst += (5.0 / pow(10.0, iDigits + 1.0));
    dRst *= pow(10.0, iDigits);
    dRst = floor(dRst + dMod);
    dRst /= pow(10.0, iDigits);

    return dRst;
}


//开放接口函数
double lxMathEval(const QString &prExp, bool *prSuccess) {

    bool ok = false;
    double dRst = prExp.toDouble(&ok);
    if (ok) {
        *prSuccess = true;
        return dRst;
    } else if (prExp.size() < 3) { //任何运算，至少3字符。
        *prSuccess = false;
        return 0;
    } else
        *prSuccess = true;          //所有意外，都另外赋予false

    QString strOp("");
    QStringList lsParams;
    if ( !lxOutestParse(prExp, strOp, lsParams) ) {
        *prSuccess = false;
        return 0.0;
    }

    //    qDebug() << "lxMathEval exp NOW:" << prExp << "       parsed op:" << strOp << "           params:" << lsParams;


    if (strOp == QStringLiteral("+")) {
        if (lsParams.size() == 2) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            double param2 = lxMathEval(lsParams.at(1), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return param1 + param2;

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("-")) {
        if (lsParams.size() == 2) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            double param2 = lxMathEval(lsParams.at(1), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return param1 - param2;

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("*")) {
        if (lsParams.size() == 2) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            double param2 = lxMathEval(lsParams.at(1), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return param1 * param2;

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("/")) {
        if (lsParams.size() == 2) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            double param2 = lxMathEval(lsParams.at(1), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return param1 / param2;

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("%")) {
        if (lsParams.size() == 2) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            double param2 = lxMathEval(lsParams.at(1), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            int iParam1 = int(param1);
            int iParam2 = int(param2);
            int iRst = iParam1 % iParam2;

            return iRst;

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("FMOD")) {
        if (lsParams.size() == 2) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            double param2 = lxMathEval(lsParams.at(1), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return fmod(param1, param2);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("FLOOR")) {            //FLOOR(x+0.5)相当于四舍五入
        if (lsParams.size() == 1) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return floor(param1);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("CEIL")) {
        if (lsParams.size() == 1) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return ceil(param1);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("ABS")) {
        if (lsParams.size() == 1) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return fabs(param1);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("SQRT")) {
        if (lsParams.size() == 1) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return sqrt(param1);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("POW")) {
        if (lsParams.size() == 2) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            double param2 = lxMathEval(lsParams.at(1), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return pow(param1, param2);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("LOG10")) {
        if (lsParams.size() == 1) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return log10(param1);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("LN")) {
        if (lsParams.size() == 1) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return log(param1);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("EXP")) {
        if (lsParams.size() == 1) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return exp(param1);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("SIN")) {
        if (lsParams.size() == 1) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return sin(param1);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("ASIN")) {
        if (lsParams.size() == 1) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return asin(param1);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("COS")) {
        if (lsParams.size() == 1) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return cos(param1);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("ACOS")) {
        if (lsParams.size() == 1) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return acos(param1);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("TAN")) {
        if (lsParams.size() == 1) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return tan(param1);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("ATAN")) {
        if (lsParams.size() == 1) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return atan(param1);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }


    else if (strOp == QStringLiteral("ATAN2")) {
        if (lsParams.size() == 2) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            double param2 = lxMathEval(lsParams.at(1), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return atan2(param1, param2);

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }

    else if (strOp == QStringLiteral("ROUND")) {
        if (lsParams.size() == 2) {

            bool blSucc;

            double param1 = lxMathEval(lsParams.at(0), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            double param2 = lxMathEval(lsParams.at(1), &blSucc);
            if (!blSucc) {
                *prSuccess = false;
                return 0.0;
            }

            return lxRound(param1, short(param2));

        } else {
            *prSuccess = false;
            return 0.0;
        }
    }



    *prSuccess = false;
    return 0.0;

}


}
