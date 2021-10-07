#include "bailisql.h"
#include "bailicode.h"

#include <QtSql>

namespace BailiSoft {

QString getListValueTip(const int totalLen, const int itemCountMin, const int itemCountMax) {
    return QStringLiteral("逗号分隔各项，字数不限，但一般不应超过%1字，建议%2个到%3项为宜。")
            .arg(totalLen).arg(itemCountMin).arg(itemCountMax);
}

QStringList createSysRelTableSql(const QString &bookName, const bool forImport)
{
    QStringList ls;

    ls << QLatin1String("create table if not exists bailifldname("
                        "   tblname         text not null,"
                        "   fldname         text not null,"
                        "   cname           text default '',"
                        "   primary key(tblname, fldname));");

    ls << QLatin1String("create table if not exists bailiLoginer("
                        "    loginer		text primary key,"
                        "    deskPassword	text default '',"       //后台登录密码
                        "    passHash		text default '',"       //曾被用为netAesKey => 实际被用为netVcode
                        "    loginMobile    text default '',"       //未用，保留
                        "    mobiSalt		text default '',"       //曾被用为netVcode => 未用，保留
                        "    bindShop       text default '',"
                        "    bindCustomer   text default '',"
                        "    bindSupplier   text default '',"       //尽管有此设计预留，就算网络功能完全开发也尽量不设计厂商终端。用处不大但麻烦。
                        "    customAllow    text default '',"       //例外允许（未用，保留）
                        "    customDeny     text default '',"       //例外禁止（未用，保留）
                        "    limCargoExp    text default '',"       //仅用于service控制移动终端，后台不限制
                        "    retprice    integer default 0,"
                        "    lotprice    integer default 0,"
                        "    buyprice    integer default 0,"
                        "    sizertype   integer default 0,"
                        "    colortype   integer default 0,"
                        "    cargo       integer default 0,"
                        "    staff       integer default 0,"
                        "    subject     integer default 0,"
                        "    shop        integer default 0,"
                        "    supplier    integer default 0,"
                        "    customer    integer default 0,"
                        "    cgd         integer default 0,"
                        "    cgj         integer default 0,"
                        "    cgt         integer default 0,"
                        "    pfd         integer default 0,"
                        "    pff         integer default 0,"
                        "    pft         integer default 0,"
                        "    lsd         integer default 0,"
                        "    dbd         integer default 0,"
                        "    syd         integer default 0,"
                        "    szd         integer default 0,"
                        "    vicgd       integer default 0,"
                        "    vicgj       integer default 0,"
                        "    vicgt       integer default 0,"
                        "    vipfd       integer default 0,"
                        "    vipff       integer default 0,"
                        "    vipft       integer default 0,"
                        "    vilsd       integer default 0,"
                        "    vidbd       integer default 0,"
                        "    visyd       integer default 0,"
                        "    viszd       integer default 0,"
                        "    vicg        integer default 0,"
                        "    vipf        integer default 0,"
                        "    vixs        integer default 0,"
                        "    vicgcash    integer default 0,"
                        "    vipfcash    integer default 0,"
                        "    vixscash    integer default 0,"
                        "    vicgrest    integer default 0,"
                        "    vipfrest    integer default 0,"
                        "    vistock     integer default 0,"
                        "    viall       integer default 0"
                        ");");

    ls << QLatin1String("create table if not exists bailiOption("
                        "    optcode			text primary key,"
                        "    optname			text not null,"
                        "    vsetting           text not null,"
                        "    vdefault           text not null,"
                        "    vformat			text not null"
                        ");");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'app_book_name', '账册名称', '%1', '%1', '账册名称，用于登录向导识别。');").arg(bookName);

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'app_company_name', '公司名称', '百利服饰公司', '百利服饰公司', '您公司名称，用于打印标识等场合。');");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'app_company_plogo', '公司LOGO', '', '', '请选择LOGO图片，300x300像素为佳。');");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'app_company_pcolor', '公司主题色', '119900', '119900', '请选择公司主题色');");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'app_company_plway', 'LOGO显示方式', '居中', '居中', '请填“重叠平铺”或“居中一块”');");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'app_image_path', '货品图片根目录', '', '', '货品图片集中放置文件夹。注意每个图片文件名必须与货号一致，且需为jpg格式。');");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'dots_of_qty', '数量小数位数', '0', '0', '0~4位');");
    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'dots_of_price', '价格小数位数', '2', '2', '0~4位');");
    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'dots_of_discount', '折扣小数位数', '3', '3', '0~4位');");
    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'dots_of_money', '金额小数位数', '2', '2', '0~4位');");


    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'sheet_hpmark_define', '货备注列默认填充值', '0', '0', '填数字1~6以代表货品自定义分类第几列。无效值表示不需要货备注列。');");
    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'sheet_hpmark_editable', '货备注列是否允许手动更改', '否', '否', '请填“是”或“否”');");


    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'cargo_attr1_name', '货品分类一角度名称', '品牌', '品牌', '角度名，空名称停用。');");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'cargo_attr2_name', '货品分类二角度名称', '材料', '材料', '角度名，空名称停用。');");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'cargo_attr3_name', '货品分类三角度名称', '风格', '风格', '角度名，空名称停用。');");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'cargo_attr4_name', '货品分类四角度名称', '档次', '档次', '角度名，空名称停用。');");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'cargo_attr5_name', '货品分类五角度名称', '季节', '季节', '角度名，空名称停用。');");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'cargo_attr6_name', '货品分类六角度名称', '设计师', '设计师', '角度名，空名称停用。');");


    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                  "'cargo_attr1_list', '货品分类一分类项', 'DIOR,LV,雅莹,雅鹿', 'DIOR,LV,雅莹,雅鹿', '%1');")
          .arg(getListValueTip(300, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                  "'cargo_attr2_list', '货品分类二分类项', '棉,毛,麻,丝', '棉,毛,麻,丝', '%1');")
          .arg(getListValueTip(300, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                  "'cargo_attr3_list', '货品分类三分类项', '朋克,嬉皮,淑女,精致,职业', '朋克,嬉皮,淑女,精致,职业', '%1');")
          .arg(getListValueTip(300, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                  "'cargo_attr4_list', '货品分类四分类项', '高,中,低', '高,中,低', '%1');")
          .arg(getListValueTip(300, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                  "'cargo_attr5_list', '货品分类五分类项', '春秋,夏,冬,四季', '春秋,夏,冬,四季', '%1');")
          .arg(getListValueTip(300, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                  "'cargo_attr6_list', '货品分类六分类项', '张然,Lydia,Tony,老板娘', '张然,Lydia,Tony,老板娘', '%1');")
          .arg(getListValueTip(300, 3, 30));


    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'subject_attr1_name', '账目分类一角度名称', '一级科目', '一级科目', '角度名，空名称停用。');");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'subject_attr2_name', '账目分类二角度名称', '二级科目', '二级科目', '角度名，空名称停用。');");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'subject_attr3_name', '账目分类三角度名称', '三级科目', '三级科目', '角度名，空名称停用。');");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'subject_attr4_name', '账目分类四角度名称', '', '', '角度名，空名称停用。');");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'subject_attr5_name', '账目分类五角度名称', '', '', '角度名，空名称停用。');");

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'subject_attr6_name', '账目分类六角度名称', '', '', '角度名，空名称停用。');");


    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'subject_attr1_list', '账目分类一分类项', '资产,负债,权益,收入,费用', '资产,负债,权益,收入,费用', '%1');")
          .arg(getListValueTip(300, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'subject_attr2_list', '账目分类二分类项', '', '', '%1');")
          .arg(getListValueTip(300, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'subject_attr3_list', '账目分类三分类项', '', '', '%1');")
          .arg(getListValueTip(300, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'subject_attr4_list', '账目分类四分类项', '', '', '%1');")
          .arg(getListValueTip(300, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'subject_attr5_list', '账目分类五分类项', '', '', '%1');")
          .arg(getListValueTip(300, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'subject_attr6_list', '账目分类六分类项', '', '', '%1');")
          .arg(getListValueTip(300, 3, 30));


    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'stypes_cgd', '采购定货单业务分类', '意向,指标保证金,定金,全款', '意向,指标保证金,定金,全款', '%1');")
          .arg(getListValueTip(100, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'stypes_cgj', '采购进货单业务分类', '自产,外购,代销', '自产,外购,代销', '%1');")
          .arg(getListValueTip(100, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'stypes_cgt', '采购退货单业务分类', '自产,外购,代销', '自产,外购,代销', '%1');")
          .arg(getListValueTip(100, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'stypes_pfd', '批发定货单业务分类', '意向,指标保证金,定金,全款', '意向,指标保证金,定金,全款', '%1');")
          .arg(getListValueTip(100, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'stypes_pff', '批发发货单业务分类', '首铺,补货,买断', '首铺,补货,买断', '%1');")
          .arg(getListValueTip(100, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'stypes_pft', '批发退货单业务分类', '首铺,补货,买断', '首铺,补货,买断', '%1');")
          .arg(getListValueTip(100, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'stypes_lsd', '零售单业务分类', '员购,代销,自营', '员购,代销,自营', '%1');")
          .arg(getListValueTip(100, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'stypes_dbd', '调拨单业务分类', '店间,铺货,退库', '店间,铺货,退库', '%1');")
          .arg(getListValueTip(100, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'stypes_syd', '损益单业务分类', '串色串码,疵品报损', '串色串码,疵品报损', '%1');")
          .arg(getListValueTip(100, 3, 30));

    ls << QStringLiteral("insert or ignore into bailiOption(optcode, optname, vsetting, vdefault, vformat) values("
                         "'stypes_szd', '收支单业务分类', '日常,结转', '日常,结转', '%1');")
          .arg(getListValueTip(100, 3, 30));

    if ( forImport ) {
        //Nothing
    }
    else {
        //Nothing
    }

    return ls;
}

QStringList createPrintRelTableSql(const bool forImport)
{
    QStringList ls;

    ls << QStringLiteral("CREATE TABLE IF NOT EXISTS lxprintpage( "
                         "    nId			integer primary key autoincrement, "
                         "    sBizName		text, "
                         "    sPrinterName	text, "
                         "    sPaperName	text "
                         "); ");

    ls << QStringLiteral( "CREATE TABLE IF NOT EXISTS lxprintsec( "
                          "    nId			integer primary key autoincrement, "
                          "    sBizName		text, "
                          "    nSecType		integer, "
                          "    nSecHeight	integer, "
                          "    nRowLine		integer, "
                          "    nColLine		integer, "
                          "    nColTitle	integer "
                          "); ");

    ls << QStringLiteral("CREATE TABLE IF NOT EXISTS lxprintobj( "
                         "    nId			integer primary key autoincrement, "
                         "    sBizName		text, "
                         "    nSecType		integer, "
                         "    sValue		text, "
                         "    sExp			text, "
                         "    nPosX			integer, "
                         "    nPosY			integer, "
                         "    nWidth		integer, "
                         "    sFontName		text, "
                         "    nFontPoint	integer, "
                         "    nFontAlign	integer "
                         "); ");

    if ( forImport ) {
        //Nothing
    }
    else {
        //Nothing
    }

    return ls;
}

QStringList createCargoRelTableSql(const bool forImport)
{
    QStringList ls;

    ls << QLatin1String("create table if not exists sizerType("
                        "    tname	text primary key,"
                        "    namelist	text default '',"
                        "    codelist	text default '',"
                        "    upman		text default '',"
                        "    uptime		integer default 0"
                        ");");

    ls << QLatin1String("create table if not exists colorType("
                        "    tname	text primary key,"
                        "    namelist	text default '',"
                        "    codelist	text default '',"
                        "    upman		text default '',"
                        "    uptime		integer default 0"
                        ");");

    ls << QLatin1String("create table if not exists buyPolicy("
                        "    policyName		text primary key,"
                        "    traderExp		text default '',"
                        "    cargoExp		text default '',"
                        "    policyDis		integer default 5000,"
                        "    useLevel       integer default 1,"
                        "    startDate		integer default 0,"
                        "    endDate		integer default 99999999999,"
                        "    upman			text default '',"
                        "    uptime			integer default 0"
                        ");");

    ls << QLatin1String("create table if not exists lotPolicy("
                        "    policyName		text primary key,"
                        "    traderExp		text default '',"
                        "    cargoExp		text default '',"
                        "    policyDis		integer default 5000,"
                        "    useLevel       integer default 1,"
                        "    startDate		integer default 0,"
                        "    endDate		integer default 99999999999,"
                        "    upman			text default '',"
                        "    uptime			integer default 0"
                        ");");

    ls << QLatin1String("create table if not exists retPolicy("
                        "    policyName		text primary key,"
                        "    traderExp		text default '',"
                        "    cargoExp		text default '',"
                        "    policyDis		integer default 5000,"
                        "    useLevel       integer default 1,"
                        "    startDate		integer default 0,"
                        "    endDate		integer default 99999999999,"
                        "    upman			text default '',"
                        "    uptime			integer default 0"
                        ");");

    ls << QLatin1String("create table if not exists movPolicy("
                        "    policyName		text primary key,"
                        "    traderExp		text default '',"
                        "    cargoExp		text default '',"
                        "    policyDis		integer default 5000,"
                        "    useLevel       integer default 1,"
                        "    startDate		integer default 0,"
                        "    endDate		integer default 99999999999,"
                        "    upman			text default '',"
                        "    uptime			integer default 0"
                        ");");

    ls << QLatin1String("create table if not exists barcodeRule("
                        "    barcodexp      text primary key,"
                        "    sizermiddlee   integer default 0,"
                        "    barcodemark    text default '',"
                        "    upman			text default '',"
                        "    uptime			integer default 0"
                        ");");

    if ( forImport ) {
        //Nothing
    }
    else {
        //Nothing
    }

    return ls;
}

QString createStockAlarmTableSql()
{
    QStringList fields;
    fields << "cargo" << "color" << "minsizers" << "maxsizers";

    QString sql = QStringLiteral("CREATE TABLE IF NOT EXISTS stockalarm (");
    QStringList fs;
    for ( int i = 0, iLen = fields.length(); i < iLen; ++i ) {
        QString fld = fields.at(i);
        if ( ! mapMsg.contains(QStringLiteral("fld_%1").arg(fld)) ) {
            qDebug() << "createStockAlarmTableSql not found fld: " << fld;
            return "-ERR-";
        }
        if ( ! mapMsg.contains(QStringLiteral("fld_%1").arg(fld)) ) {
            qDebug() << "createStockAlarmTableSql not found fld sql: " << fld;
            return "-ERR-";
        }
        fs << fld + " " + mapMsg.value(QStringLiteral("fld_%1").arg(fld)).split(QChar(9)).at(1);
    }

    sql += fs.join(QChar(44));
    sql += QStringLiteral(", primary key(cargo, color)); ");

    return sql;
}

QString createRegTableSql(const QString &table, const QStringList &fields)
{
    if ( ! mapMsg.contains(QStringLiteral("win_%1").arg(table)) ) {
        qDebug() << "createRegTableSql not found table: " << table;
        return "-ERR-";
    }

    QString sql = QStringLiteral("CREATE TABLE IF NOT EXISTS %1 (").arg(table);
    QStringList fs;
    for ( int i = 0, iLen = fields.length(); i < iLen; ++i ) {
        QString fld = fields.at(i);
        if ( ! mapMsg.contains(QStringLiteral("fld_%1").arg(fld)) ) {
            qDebug() << "createRegTableSql not found fld: " << fld;
            return "-ERR-";
        }
        if ( ! mapMsg.contains(QStringLiteral("fld_%1").arg(fld)) ) {
            qDebug() << "createRegTableSql not found fld sql: " << fld;
            return "-ERR-";
        }
        fs << fld + QString(" ") + mapMsg.value(QStringLiteral("fld_%1").arg(fld)).split(QChar(9)).at(1);
    }
    sql += fs.join(QChar(44));
    sql += "); ";
    return sql;
}

QString createSheetTableSql(const QString &table, const bool financee = false)
{
    if ( ! mapMsg.contains(QStringLiteral("win_%1").arg(table)) ) {
        qDebug() << "createSheetTableSql not found table: " << table;
        return "-ERR-";
    }

    QStringList fields;
    fields << "sheetid" << "proof" << "dated" << "shop" << "trader"
           << "stype" << "staff" << "remark";

    if ( !financee )
        fields << "sumqty" << "summoney" << "sumdis"
               << "actpay" << "actowe";

    fields << "checker" << "chktime" << "upman"
           << "uptime";

    QString sql = QStringLiteral("CREATE TABLE IF NOT EXISTS %1 (").arg(table);
    QStringList fs;
    for ( int i = 0, iLen = fields.length(); i < iLen; ++i ) {
        QString fld = fields.at(i);
        if ( ! mapMsg.contains(QStringLiteral("fld_%1").arg(fld)) ) {
            qDebug() << "createSheetTableSql not found fld: " << fld;
            return "-ERR-";
        }
        if ( ! mapMsg.contains(QStringLiteral("fld_%1").arg(fld)) ) {
            qDebug() << "createSheetTableSql not found fld sql: " << fld;
            return "-ERR-";
        }
        fs << fld + " " + mapMsg.value(QStringLiteral("fld_%1").arg(fld)).split(QChar(9)).at(1);
    }
    sql += fs.join(QChar(44));
    sql += "); ";
    return sql;
}

QString createSheetDetailTableSql(const QString &mainTable, const bool financee = false)
{
    if ( ! mapMsg.contains(QStringLiteral("win_%1").arg(mainTable)) ) {
        qDebug() << "createSheetDetailTableSql not found table: " << mainTable;
        return "-ERR-";
    }

    QStringList fields;
    fields << "parentid" << "rowtime";
    if ( financee )
        fields << "subject" << "rowmark" << "income" << "expense";
    else
        fields << "cargo" << "color" << "sizers" << "qty" << "price" << "discount"
               << "actmoney" << "dismoney" << "hpmark" << "rowmark";

    QString sql = QStringLiteral("CREATE TABLE IF NOT EXISTS %1dtl (").arg(mainTable);
    QStringList fs;
    for ( int i = 0, iLen = fields.length(); i < iLen; ++i ) {
        QString fld = fields.at(i);
        if ( ! mapMsg.contains(QStringLiteral("fld_%1").arg(fld)) ) {
            qDebug() << "createSheetDetailTableSql not found fld: " << fld;
            return "-ERR-";
        }
        if ( ! mapMsg.contains(QStringLiteral("fld_%1").arg(fld)) ) {
            qDebug() << "createSheetDetailTableSql not found fld sql: " << fld;
            return "-ERR-";
        }
        fs << fld + " " + mapMsg.value(QStringLiteral("fld_%1").arg(fld)).split(QChar(9)).at(1);
    }

    sql += fs.join(QChar(44));
    sql += QStringLiteral(", primary key(parentid, rowtime)); ");
    return sql;
}


//注意统计值字段命名必须有此四特征：fld.endsWith("qty") || fld.endsWith("money") || fld.startsWith("act") || fld.startsWith("sum")
QString selectSheetCashSql(const QString &table, const bool minus)
{
    QStringList mflds;
    mflds << "sheetid" << "proof" << "dated" << "shop" << "trader" << "stype" << "staff"
          << "sumqty" << "summoney" << "sumdis" << "actpay" << "actowe" << "chktime";

    for ( int i = 0, iLen = mflds.length(); i < iLen; ++i ) {
        QString fld = mflds.at(i);
        if ( ! mapMsg.contains(QStringLiteral("fld_%1").arg(fld)) ) {
            qDebug() << "selectSheetDetailSql not found fld: " << fld;
            return "-ERR-";
        }
    }

    QString sql = QStringLiteral("SELECT ");
    QStringList fs;
    fs << QStringLiteral("'%1' AS sheetname").arg(table.toUpper());
    fs << QStringLiteral("strftime('%Y', datetime(dated, 'unixepoch', 'localtime')) as yeard");     //注意%Y必须大写
    fs << QStringLiteral("strftime('%Y%m', datetime(dated, 'unixepoch', 'localtime')) as monthd "); //注意%m必须小写

    for ( int i = 0, iLen = mflds.length(); i < iLen; ++i ) {
        QString fld = mflds.at(i);
        QString f;
        if ( minus ) {
            f = ( fld.endsWith("qty") || fld.endsWith("money") || fld.startsWith("act") || fld.startsWith("sum") )
                    ? QStringLiteral("(0-%1.%2) AS %2").arg(table).arg(fld)         //数值
                    : QStringLiteral("%1.%2").arg(table).arg(fld);                  //非数值
        } else {
            f = QStringLiteral("%1.%2").arg(table).arg(fld);
        }
        fs << f;
    }

    sql += fs.join(QChar(44));
    sql += QStringLiteral(" FROM %1").arg(table);      //注意不要加分号，因为别处要UNION ALL
    return sql;
}


//注意统计值字段命名必须有此四特征：fld.endsWith("qty") || fld.endsWith("money") || fld.startsWith("act") || fld.startsWith("sum")
QString selectSheetDetailSql(const QString &mainTable, const bool minus, const bool joinCargo)
{
    QStringList mflds;
    mflds << "sheetid" << "proof" << "dated" << "shop" << "trader" << "stype" << "staff" << "chktime";

    for ( int i = 0, iLen = mflds.length(); i < iLen; ++i ) {
        QString fld = mflds.at(i);
        if ( ! mapMsg.contains(QStringLiteral("fld_%1").arg(fld)) ) {
            qDebug() << "selectSheetDetailSql not found fld: " << fld;
            return "-ERR-";
        }
    }

    QStringList dflds;
    dflds << "parentid" << "cargo" << "color" << "sizers" << "qty"
          << "price" << "actmoney" << "discount" << "dismoney";

    for ( int i = 0, iLen = dflds.length(); i < iLen; ++i ) {
        QString fld = dflds.at(i);
        if ( ! mapMsg.contains(QStringLiteral("fld_%1").arg(fld)) ) {
            qDebug() << "selectSheetDetailSql not found fld: " << fld;
            return "-ERR-";
        }
    }

    QString sql = QStringLiteral("SELECT ");
    QStringList fs;
    fs << QStringLiteral("'%1' AS sheetname").arg(mainTable.toUpper());
    fs << QStringLiteral("strftime('%Y', datetime(%1.dated, 'unixepoch', 'localtime')) as yeard").arg(mainTable);     //注意%Y必须大写
    fs << QStringLiteral("strftime('%Y%m', datetime(%1.dated, 'unixepoch', 'localtime')) as monthd ").arg(mainTable); //注意%m必须小写

    for ( int i = 0, iLen = mflds.length(); i < iLen; ++i ) {
        QString fld = mflds.at(i);
        fs << QStringLiteral("%1.%2").arg(mainTable).arg(fld);
    }

    for ( int i = 0, iLen = dflds.length(); i < iLen; ++i ) {
        QString fld = dflds.at(i);
        QString f;
        if ( minus ) {
            f = ( fld.endsWith("qty") || fld.endsWith("money") || fld.startsWith("act") || fld.startsWith("sum") )
                    ? QStringLiteral("(0-%1dtl.%2) AS %2").arg(mainTable).arg(fld)
                    : QStringLiteral("%1dtl.%2").arg(mainTable).arg(fld);

            if ( fld == QStringLiteral("sizers") )
                f = QStringLiteral("('\r\f' || %1dtl.sizers) AS sizers").arg(mainTable);      //\r\f重要标志
        }
        else {
            f = QStringLiteral("%1dtl.%2").arg(mainTable).arg(fld);

            if ( fld == QStringLiteral("sizers") )
                f = QStringLiteral("('\r\v' || %1dtl.sizers) AS sizers").arg(mainTable);      //\r\v重要标志
        }
        fs << f;
    }

    if ( joinCargo ) {
        fs << QStringLiteral("cargo.hpname");
        fs << QStringLiteral("cargo.colortype");
        fs << QStringLiteral("cargo.sizertype");
        fs << QStringLiteral("cargo.setprice");
        fs << QStringLiteral("cargo.unit");
        for ( int i = 1; i <= 6; ++i ) {
            fs << QStringLiteral("cargo.attr%1").arg(i);
        }
    }

    sql += fs.join(QChar(44));
    sql += QStringLiteral(" FROM (%1 LEFT JOIN %1dtl ON %1.sheetid=%1dtl.parentid) ").arg(mainTable);
    if ( joinCargo )
        sql += QStringLiteral(" LEFT JOIN cargo ON cargo.hpcode=%1dtl.cargo").arg(mainTable);
    //注意不要加分号，因为别处要UNION ALL
    return sql;
}


QString selectFinanceDetailSql(const bool joinSubject)
{
    QStringList mflds;
    mflds << "sheetid" << "proof" << "dated" << "shop" << "trader" << "stype" << "staff" << "chktime";

    for ( int i = 0, iLen = mflds.length(); i < iLen; ++i ) {
        QString fld = mflds.at(i);
        if ( ! mapMsg.contains(QStringLiteral("fld_%1").arg(fld)) ) {
            qDebug() << "selectFinanceDetailSql not found fld: " << fld;
            return "-ERR-";
        }
    }

    QStringList dflds;
    dflds << "parentid" << "subject" << "income" << "expense";

    for ( int i = 0, iLen = dflds.length(); i < iLen; ++i ) {
        QString fld = dflds.at(i);
        if ( ! mapMsg.contains(QStringLiteral("fld_%1").arg(fld)) ) {
            qDebug() << "selectFinanceDetailSql not found fld: " << fld;
            return "-ERR-";
        }
    }

    QString sql = QStringLiteral("SELECT ");
    QStringList fs;
    fs << QStringLiteral("'SZD' AS sheetname");
    fs << QStringLiteral("strftime('%Y', datetime(szd.dated, 'unixepoch', 'localtime')) as yeard");     //注意%Y必须大写
    fs << QStringLiteral("strftime('%Y%m', datetime(szd.dated, 'unixepoch', 'localtime')) as monthd "); //注意%m必须小写

    for ( int i = 0, iLen = mflds.length(); i < iLen; ++i ) {
        QString fld = mflds.at(i);
        fs << QStringLiteral("szd.%1").arg(fld);
    }

    for ( int i = 0, iLen = dflds.length(); i < iLen; ++i ) {
        QString fld = dflds.at(i);
        fs << QStringLiteral("szddtl.%1").arg(fld);
    }

    if ( joinSubject ) {
        for ( int i = 1; i <= 6; ++i ) {
            fs << QStringLiteral("subject.attr%1").arg(i);
        }
    }

    sql += fs.join(QChar(44));
    sql += QStringLiteral(" FROM (szd LEFT JOIN szddtl ON szd.sheetid=szddtl.parentid) ");
    if ( joinSubject )
        sql += QStringLiteral(" LEFT JOIN subject ON subject.kname=szddtl.subject");
    //注意不要加分号，调用处会加
    return sql;
}


QString selectSheetJoinForStockSql(const QString &mainTable, const bool minus, const bool stSwitch,
                                   const bool joinCargo, const bool forHistoryList)
{
    QStringList flds;
    flds  << "sheetid" << "dated" << "shop" << "chktime" << "proof" << "stype" << "staff"
          << "cargo" << "color" << "sizers" << "qty" << "actmoney" << "dismoney";

    for ( int i = 0, iLen = flds.length(); i < iLen; ++i ) {
        QString fld = flds.at(i);
        if ( ! mapMsg.contains(QStringLiteral("fld_%1").arg(fld)) ) {
            qDebug() << "selectSheetJoinForStockSql not found fld: " << fld;
            return "-ERR-";
        }
    }

    QString sql = QStringLiteral("SELECT ");
    QStringList fs;

    bool useMinus = minus;
    QString sheetName = mainTable.toUpper();
    if ( mainTable.toUpper() == "DBD" ) {
        sheetName = ( stSwitch ) ? "DBJ" : "DBC";
        useMinus  = ( stSwitch ) ? false : true;
    }
    QString fromView = (joinCargo) ? QStringLiteral("vi_%1_attr").arg(mainTable) : QStringLiteral("vi_%1").arg(mainTable);

    fs << QStringLiteral("'%1' AS sheetname").arg(sheetName);
    fs << "sheetid" << "dated" << "chktime";

    if ( stSwitch ) {
        fs << "trader AS shop";
    } else {
        fs << "shop";
    }

    if ( forHistoryList ) {
        fs << "proof" << "stype" << "staff";

        if ( stSwitch ) {
            fs << "shop AS trader";
        } else {
            fs << "trader";
        }
    }

    fs << "cargo" << "color";

    if ( useMinus ) {
        //\r\f重要标志（因为from的单据对应视图中已有正\r\v所以substr）
        fs << QStringLiteral("('\r\f' || substr(%1.sizers,3)) AS sizers").arg(fromView)
           << QStringLiteral("(0-%1.qty) AS qty").arg(fromView)
           << QStringLiteral("(0-%1.actmoney) AS actmoney").arg(fromView)
           << QStringLiteral("(0-%1.dismoney) AS dismoney").arg(fromView);
    } else {
        //因为from的单据对应视图中已有正\r\v
        fs << "sizers"
           << "qty"
           << "actmoney"
           << "dismoney";
    }

    if ( joinCargo ) {
        fs << QStringLiteral("hpname");
        fs << QStringLiteral("colortype");
        fs << QStringLiteral("sizertype");
        fs << QStringLiteral("setprice");
        fs << QStringLiteral("unit");
        for ( int i = 1; i <= 6; ++i ) {
            fs << QStringLiteral("attr%1").arg(i);
        }
    }

    //注意不要加分号，因为别处要UNION ALL
    sql += fs.join(QChar(44)) + QStringLiteral(" FROM %1").arg(fromView);

    return sql;
}

QStringList sqliteInitSqls(const QString &bookName, const bool forImport)
{
    QStringList sqls;

    sqls << QStringLiteral("create table if not exists meeting ("
                           "meetid      integer primary key,"
                           "meetname	text unique not null"
                           ");");

    sqls << QStringLiteral("create table if not exists msglog ("
                           "msgid         integer primary key, "  //milliSecondsSinceEpoch，处理中如真有重复冲突则放弃。
                           "senderId      text not null,"
                           "senderName    text not null,"
                           "receiverId    text not null,"
                           "receiverName  text not null,"
                           "content       text default '',"
                           "openRead      integer default 0);");

    sqls << QStringLiteral("create table if not exists msgfail ("
                           "msgid       integer not null,"
                           "toFrontId   text not null,"
                           "primary key(msgid, toFrontId));");

    sqls << createSysRelTableSql(bookName, forImport);
    sqls << createPrintRelTableSql(forImport);
    sqls << createCargoRelTableSql(forImport);
    sqls << createStockAlarmTableSql();

    QStringList flds;
    flds << "hpcode" << "hpname" << "sizertype" << "colortype" << "unit" << "setprice" << "retprice" << "lotprice" << "buyprice"
         << "almin" << "almax" << "attr1" << "attr2" << "attr3" << "attr4" << "attr5" << "attr6" << "upman" << "uptime";
    sqls << createRegTableSql("cargo", flds);

    flds.clear();
    flds << "kname" << "attr1" << "attr2" << "attr3" << "attr4" << "attr5" << "attr6" << "adminboss" << "upman" << "uptime";
    sqls << createRegTableSql("subject", flds);

    flds.clear();
    flds << "kname" << "cancg" << "canpf" << "canls" << "candb" << "cansy" << "upman" << "uptime";
    sqls << createRegTableSql("staff", flds);

    flds.clear();
    flds << "kname" << "regdis" << "regman" << "regaddr" << "regtele" << "regmark" << "upman" << "uptime";
    sqls << createRegTableSql("shop", flds);

    flds.clear();
    flds << "kname" << "regdis" << "regman" << "regaddr" << "regtele" << "regmark" << "upman" << "uptime";
    sqls << createRegTableSql("customer", flds);

    flds.clear();
    flds << "kname" << "regdis" << "regman" << "regaddr" << "regtele" << "regmark" << "upman" << "uptime";
    sqls << createRegTableSql("supplier", flds);

    sqls << createSheetTableSql("cgd");
    sqls << createSheetTableSql("cgj");
    sqls << createSheetTableSql("cgt");
    sqls << createSheetTableSql("pfd");
    sqls << createSheetTableSql("pff");
    sqls << createSheetTableSql("pft");
    sqls << createSheetTableSql("lsd");
    sqls << createSheetTableSql("dbd");
    sqls << createSheetTableSql("syd");
    sqls << createSheetTableSql("szd", true);

    sqls << createSheetDetailTableSql("cgd");
    sqls << createSheetDetailTableSql("cgj");
    sqls << createSheetDetailTableSql("cgt");
    sqls << createSheetDetailTableSql("pfd");
    sqls << createSheetDetailTableSql("pff");
    sqls << createSheetDetailTableSql("pft");
    sqls << createSheetDetailTableSql("lsd");
    sqls << createSheetDetailTableSql("dbd");
    sqls << createSheetDetailTableSql("syd");
    sqls << createSheetDetailTableSql("szd", true);

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_cgd AS %1;").arg(selectSheetDetailSql("cgd", false, false));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_cgj AS %1;").arg(selectSheetDetailSql("cgj", false, false));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_cgt AS %1;").arg(selectSheetDetailSql("cgt", false, false));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_pfd AS %1;").arg(selectSheetDetailSql("pfd", false, false));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_pff AS %1;").arg(selectSheetDetailSql("pff", false, false));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_pft AS %1;").arg(selectSheetDetailSql("pft", false, false));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_lsd AS %1;").arg(selectSheetDetailSql("lsd", false, false));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_dbd AS %1;").arg(selectSheetDetailSql("dbd", false, false));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_syd AS %1;").arg(selectSheetDetailSql("syd", false, false));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_szd AS %1;").arg(selectFinanceDetailSql(false));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_cgd_attr AS %1;").arg(selectSheetDetailSql("cgd", false, true));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_cgj_attr AS %1;").arg(selectSheetDetailSql("cgj", false, true));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_cgt_attr AS %1;").arg(selectSheetDetailSql("cgt", false, true));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_pfd_attr AS %1;").arg(selectSheetDetailSql("pfd", false, true));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_pff_attr AS %1;").arg(selectSheetDetailSql("pff", false, true));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_pft_attr AS %1;").arg(selectSheetDetailSql("pft", false, true));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_lsd_attr AS %1;").arg(selectSheetDetailSql("lsd", false, true));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_dbd_attr AS %1;").arg(selectSheetDetailSql("dbd", false, true));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_syd_attr AS %1;").arg(selectSheetDetailSql("syd", false, true));
    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_szd_attr AS %1;").arg(selectFinanceDetailSql(true));


    QString sqlViDbd = QStringLiteral("CREATE VIEW IF NOT EXISTS vi_dbr AS %1;").arg(selectSheetDetailSql("dbd", false, false));
    sqls << sqlViDbd.replace(QStringLiteral("shop"), QStringLiteral("trader as shop"));

    QString sqlViDbdAttr = QStringLiteral("CREATE VIEW IF NOT EXISTS vi_dbr_attr AS %1;").arg(selectSheetDetailSql("dbd", false, true));
    sqls << sqlViDbdAttr.replace(QStringLiteral("shop"), QStringLiteral("trader as shop"));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_cg_cash AS %1 UNION ALL %2;")
            .arg(selectSheetCashSql("cgj", false))
            .arg(selectSheetCashSql("cgt", true));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_pf_cash AS %1 UNION ALL %2;")
            .arg(selectSheetCashSql("pff", false))
            .arg(selectSheetCashSql("pft", true));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_xs_cash AS %1 UNION ALL %2 UNION ALL %3;")
            .arg(selectSheetCashSql("pff", false))
            .arg(selectSheetCashSql("lsd", false))
            .arg(selectSheetCashSql("pft", true));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_cg AS %1 UNION ALL %2;")
            .arg(selectSheetDetailSql("cgj", false, false))
            .arg(selectSheetDetailSql("cgt", true, false));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_pf AS %1 UNION ALL %2;")
            .arg(selectSheetDetailSql("pff", false, false))
            .arg(selectSheetDetailSql("pft", true, false));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_xs AS %1 UNION ALL %2 UNION ALL %3;")
            .arg(selectSheetDetailSql("pff", false, false))
            .arg(selectSheetDetailSql("lsd", false, false))
            .arg(selectSheetDetailSql("pft", true, false));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_cg_attr AS %1 UNION ALL %2;")
            .arg(selectSheetDetailSql("cgj", false, true))
            .arg(selectSheetDetailSql("cgt", true, true));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_pf_attr AS %1 UNION ALL %2;")
            .arg(selectSheetDetailSql("pff", false, true))
            .arg(selectSheetDetailSql("pft", true, true));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_xs_attr AS %1 UNION ALL %2 UNION ALL %3;")
            .arg(selectSheetDetailSql("pff", false, true))
            .arg(selectSheetDetailSql("lsd", false, true))
            .arg(selectSheetDetailSql("pft", true, true));


    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_cg_rest AS %1 UNION ALL %2;")
            .arg(selectSheetDetailSql("cgd", false, false))
            .arg(selectSheetDetailSql("cgj", true, false));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_cg_rest_attr AS %1 UNION ALL %2;")
            .arg(selectSheetDetailSql("cgd", false, true))
            .arg(selectSheetDetailSql("cgj", true, true));


    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_pf_rest AS %1 UNION ALL %2;")
            .arg(selectSheetDetailSql("pfd", false, false))
            .arg(selectSheetDetailSql("pff", true, false));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_pf_rest_attr AS %1 UNION ALL %2;")
            .arg(selectSheetDetailSql("pfd", false, true))
            .arg(selectSheetDetailSql("pff", true, true));


    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_stock AS %1 "
                           "UNION ALL %2 "
                           "UNION ALL %3 "
                           "UNION ALL %4 "
                           "UNION ALL %5 "
                           "UNION ALL %6 "
                           "UNION ALL %7 "
                           "UNION ALL %8;")
            .arg(selectSheetJoinForStockSql("syd", false, false, false, false))
            .arg(selectSheetJoinForStockSql("cgj", false, false, false, false))
            .arg(selectSheetJoinForStockSql("cgt", true, false, false, false))
            .arg(selectSheetJoinForStockSql("pff", true, false, false, false))
            .arg(selectSheetJoinForStockSql("pft", false, false, false, false))
            .arg(selectSheetJoinForStockSql("lsd", true, false, false, false))
            .arg(selectSheetJoinForStockSql("dbd", true, false, false, false))
            .arg(selectSheetJoinForStockSql("dbd", false, true, false, false));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_stock_attr AS %1 "
                           "UNION ALL %2 "
                           "UNION ALL %3 "
                           "UNION ALL %4 "
                           "UNION ALL %5 "
                           "UNION ALL %6 "
                           "UNION ALL %7 "
                           "UNION ALL %8;")
            .arg(selectSheetJoinForStockSql("syd", false, false, true, false))
            .arg(selectSheetJoinForStockSql("cgj", false, false, true, false))
            .arg(selectSheetJoinForStockSql("cgt", true, false, true, false))
            .arg(selectSheetJoinForStockSql("pff", true, false, true, false))
            .arg(selectSheetJoinForStockSql("pft", false, false, true, false))
            .arg(selectSheetJoinForStockSql("lsd", true, false, true, false))
            .arg(selectSheetJoinForStockSql("dbd", true, false, true, false))
            .arg(selectSheetJoinForStockSql("dbd", false, true, true, false));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_stock_nodb AS %1 "
                           "UNION ALL %2 "
                           "UNION ALL %3 "
                           "UNION ALL %4 "
                           "UNION ALL %5 "
                           "UNION ALL %6;")
            .arg(selectSheetJoinForStockSql("syd", false, false, false, false))
            .arg(selectSheetJoinForStockSql("cgj", false, false, false, false))
            .arg(selectSheetJoinForStockSql("cgt", true, false, false, false))
            .arg(selectSheetJoinForStockSql("pff", true, false, false, false))
            .arg(selectSheetJoinForStockSql("pft", false, false, false, false))
            .arg(selectSheetJoinForStockSql("lsd", true, false, false, false));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_stock_nodb_attr AS %1 "
                           "UNION ALL %2 "
                           "UNION ALL %3 "
                           "UNION ALL %4 "
                           "UNION ALL %5 "
                           "UNION ALL %6;")
            .arg(selectSheetJoinForStockSql("syd", false, false, true, false))
            .arg(selectSheetJoinForStockSql("cgj", false, false, true, false))
            .arg(selectSheetJoinForStockSql("cgt", true, false, true, false))
            .arg(selectSheetJoinForStockSql("pff", true, false, true, false))
            .arg(selectSheetJoinForStockSql("pft", false, false, true, false))
            .arg(selectSheetJoinForStockSql("lsd", true, false, true, false));

    sqls << QStringLiteral("CREATE VIEW IF NOT EXISTS vi_stock_history AS %1 "
                           "UNION ALL %2 "
                           "UNION ALL %3 "
                           "UNION ALL %4 "
                           "UNION ALL %5 "
                           "UNION ALL %6 "
                           "UNION ALL %7 "
                           "UNION ALL %8;")
            .arg(selectSheetJoinForStockSql("syd", false, false, false, true))
            .arg(selectSheetJoinForStockSql("cgj", false, false, false, true))
            .arg(selectSheetJoinForStockSql("cgt", true, false, false, true))
            .arg(selectSheetJoinForStockSql("pff", true, false, false, true))
            .arg(selectSheetJoinForStockSql("pft", false, false, false, true))
            .arg(selectSheetJoinForStockSql("lsd", true, false, false, true))
            .arg(selectSheetJoinForStockSql("dbd", true, false, false, true))
            .arg(selectSheetJoinForStockSql("dbd", false, true, false, true));

    return sqls;
}


QString sqlInitBook(const QString &bookFilePath, const QString &bookName)
{
    QString strErr;

    //检查创建数据库文件
    QSqlDatabase sdb = QSqlDatabase::database();
    if ( sdb.isOpen() ) sdb.close();
    sdb.setDatabaseName(bookFilePath);
    if ( !sdb.open() ) {
        return sdb.lastError().text();
    }

    //初始化新数据库表结构完整SQL
    QStringList sqls = sqliteInitSqls(bookName, false);

    //批处理
    sdb.transaction();
    foreach (QString sql, sqls) {
        if ( sql.trimmed().length() > 10 ) {
            sdb.exec(sql);
            if ( sdb.lastError().isValid() ) {
                strErr = QStringLiteral("%1\n%2").arg(sdb.lastError().text()).arg(sql);
                qDebug() << strErr;
                break;
            }
        }
    }

    //提交
    if ( strErr.isEmpty() )
        sdb.commit();
    else
        sdb.rollback();

    //返回
    return strErr;
}


QVariant readValueFromSqliteFile(const QString &sql, const QString &sqliteFile)
{
    //注意：要保证bookFile的路径必须已经创建好，但文件却不能存在。否则，QSqlDatabase.open()会产生“out of memory”错误。
    const QString sqlite_temp_conn = "SqliteTempConn";
    QVariant v;

    {
        QSqlDatabase db;
        if ( ! sqliteFile.isEmpty() ) {
            db = QSqlDatabase::addDatabase(QString("QSQLITE"), sqlite_temp_conn);
            db.setDatabaseName(sqliteFile);
            if ( !db.open() )
                return v;
        }
        else {
            db = QSqlDatabase::database();
        }

        QSqlQuery qry(db);
        qry.exec(sql);
        if ( qry.next() ) {
            v = qry.value(0);
        }
        qry.finish();

        if ( ! sqliteFile.isEmpty() )
            db.close();
    }

    //清除数据库连接
    if ( ! sqliteFile.isEmpty() )
        QSqlDatabase::removeDatabase(sqlite_temp_conn);

    return v;
}

QString setValueToSqliteFile(const QStringList &sqls, const QString &sqliteFile)
{
    //注意：要保证bookFile的路径必须已经创建好，但文件却不能存在。否则，QSqlDatabase.open()会产生“out of memory”错误。
    const QString sqlite_temp_conn = "SqliteTempConn";
    QString strErr;
    {
        QSqlDatabase db;
        if ( ! sqliteFile.isEmpty() ) {
            db = QSqlDatabase::addDatabase(QString("QSQLITE"), sqlite_temp_conn);
            db.setDatabaseName(sqliteFile);
            if ( !db.open() )
                return QString("Bad sqlite driver.");
        }
        else {
            db = QSqlDatabase::database();
        }

        db.transaction();
        for ( int i = 0, iLen = sqls.length(); i < iLen; ++i ) {
            QString sql = sqls.at(i);
            if ( sql.length() > 9 ) {
                db.exec(sql);
                if ( db.lastError().isValid() ) {
                    strErr = db.lastError().text();
                    db.rollback();
                    break;
                }
            }
        }
        if ( strErr.isEmpty() )
            db.commit();

        if ( ! sqliteFile.isEmpty() )
            db.close();
    }

    //清除数据库连接
    if ( ! sqliteFile.isEmpty() )
        QSqlDatabase::removeDatabase(sqlite_temp_conn);

    return strErr;
}

QStringList getExistsFieldsOfTable(const QString &table, QSqlDatabase &db)
{
    QSqlQuery qry(db);
    qry.exec(QStringLiteral("PRAGMA table_info(%1)").arg(table));
    int nameIdx = -1;
    for ( int i = 0, iLen = qry.record().count(); i < iLen; ++i ) {
        if ( qry.record().fieldName(i) == QStringLiteral("name") ) {
            nameIdx = i;
            break;
        }
    }
    QStringList flds;
    if ( nameIdx >= 0 ) {
        while ( qry.next() )
            flds << qry.value(nameIdx).toString().toLower();
    }
    return flds;
}

}
