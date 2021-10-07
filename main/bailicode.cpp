#include "bailicode.h"
#include "bailigrid.h"

#include <QSqlDatabase>
#include <QSqlQuery>

namespace BailiSoft {

//表基名（英）
QStringList              lstRegisWinTableNames;
QStringList              lstSheetWinTableNames;
QStringList              lstQueryWinTableNames;

//表基名（中）
QStringList              lstRegisWinTableCNames;
QStringList              lstSheetWinTableCNames;
QStringList              lstQueryWinTableCNames;

//中文字符串
QMap<QString, QString>      mapMsg;

//中英顺序务必一致！！！仅用于权限！！！
void initWinTableNames()
{
    lstRegisWinTableNames << QStringLiteral("sizertype")
                          << QStringLiteral("colortype")
                          << QStringLiteral("cargo")
                          << QStringLiteral("staff")
                          << QStringLiteral("shop")
                          << QStringLiteral("customer")
                          << QStringLiteral("supplier");

    lstRegisWinTableCNames << QStringLiteral("尺码登记")
                           << QStringLiteral("颜色登记")
                           << QStringLiteral("货品登记")
                           << QStringLiteral("员工登记")
                           << QStringLiteral("门店登记")
                           << QStringLiteral("客户登记")
                           << QStringLiteral("厂商登记");


    lstSheetWinTableNames << QStringLiteral("cgd")
                          << QStringLiteral("cgj")
                          << QStringLiteral("cgt")
                          << QStringLiteral("pfd")
                          << QStringLiteral("pff")
                          << QStringLiteral("pft")
                          << QStringLiteral("lsd")
                          << QStringLiteral("dbd")
                          << QStringLiteral("syd");

    lstSheetWinTableCNames << QStringLiteral("采购订货单")
                           << QStringLiteral("采购进货单")
                           << QStringLiteral("采购退货单")
                           << QStringLiteral("批发订货单")
                           << QStringLiteral("批发发货单")
                           << QStringLiteral("批发退货单")
                           << QStringLiteral("零售单")
                           << QStringLiteral("调拨单")
                           << QStringLiteral("损益单");

    lstQueryWinTableNames << QStringLiteral("vicgd")
                          << QStringLiteral("vicgrest")
                          << QStringLiteral("vicgj")
                          << QStringLiteral("vicgt")
                          << QStringLiteral("vicg")
                          << QStringLiteral("vicgcash")
                          << QStringLiteral("vipfd")
                          << QStringLiteral("vipfrest")
                          << QStringLiteral("vipff")
                          << QStringLiteral("vipft")
                          << QStringLiteral("vipf")
                          << QStringLiteral("vipfcash")
                          << QStringLiteral("vilsd")
                          << QStringLiteral("vixs")
                          << QStringLiteral("vixscash")
                          << QStringLiteral("vidbd")
                          << QStringLiteral("visyd")
                          << QStringLiteral("vistock")
                          << QStringLiteral("viall");

    lstQueryWinTableCNames << QStringLiteral("采购订货单统计")
                           << QStringLiteral("采购订欠货")
                           << QStringLiteral("采购进货单统计")
                           << QStringLiteral("采购退货单统计")
                           << QStringLiteral("净采购货品统计")
                           << QStringLiteral("净采购欠款")
                           << QStringLiteral("批发订货单统计")
                           << QStringLiteral("批发订欠货")
                           << QStringLiteral("批发发货单统计")
                           << QStringLiteral("批发退货单统计")
                           << QStringLiteral("净批发货品统计")
                           << QStringLiteral("净批发欠款")
                           << QStringLiteral("零售单统计")
                           << QStringLiteral("净销售货品统计")
                           << QStringLiteral("净销售欠款")
                           << QStringLiteral("调拨单统计")
                           << QStringLiteral("损益单统计")
                           << QStringLiteral("货品库存")
                           << QStringLiteral("进销存一览");

}

//大量登记
void initMapMsg()
{
    //重要约定，不能变
    mapMsg.insert("mix_size_name", QStringLiteral("无码"));

    mapMsg.insert("app_name", QStringLiteral("百利进销存敬业版桌面前端"));
    mapMsg.insert("app_sheetid_placeholer", QStringLiteral("sFH~Y0gs#F-2a%52d"));
    mapMsg.insert("app_sample_file_name", QStringLiteral("百利样例账册.jyb"));
    mapMsg.insert("app_sample_book_name", QStringLiteral("百利样例账册"));
    mapMsg.insert("app_ver_try", QStringLiteral("试用版"));
    mapMsg.insert("app_ver_lic", QStringLiteral("企业注册版"));
    mapMsg.insert("app_ver_r15", QStringLiteral("客户体验版"));
    mapMsg.insert("app_ver_sky", QStringLiteral("定制验收测试版"));
    mapMsg.insert("app_dog_user_r15", QStringLiteral("体验客户"));
    mapMsg.insert("app_dog_user_try", QStringLiteral("试用用户"));


    //主菜单====================================================================

    mapMsg.insert("main_register", QStringLiteral("登记"));
    mapMsg.insert("main_input", QStringLiteral("录单"));
    mapMsg.insert("main_summary", QStringLiteral("统计"));
    mapMsg.insert("main_balance", QStringLiteral("结存"));
    mapMsg.insert("main_tool", QStringLiteral("工具"));
    mapMsg.insert("main_help", QStringLiteral("帮助"));

    //窗口====================================================================\t隔开分别为：cname, statusTip

    mapMsg.insert("win_setpassword", QStringLiteral("更换密码\t更换后台登录密码"));
    mapMsg.insert("win_setloginer", QStringLiteral("用户与权限\t增删用户与配置权限"));
    mapMsg.insert("win_setbarcode", QStringLiteral("条码识别规则\t条码识别规则说明"));
    mapMsg.insert("win_setsystem", QStringLiteral("系统参数\t系统参数说明"));
    mapMsg.insert("win_setnet", QStringLiteral("网络后台\t网络服务器设置管理"));

    mapMsg.insert("win_<reg>", QStringLiteral("登记对象\t直接在表格中“添”、“改”、“删”，然后一次性全部“保存”或“取消”。"));
    mapMsg.insert("win_sizertype", QStringLiteral("尺码\t尺码跟随品类走，因此系统多处简称“码类”。一个品类太多尺码但通常一款产品的尺码并不多，那么建议分设多个“码类”。"));
    mapMsg.insert("win_colortype", QStringLiteral("颜色\t颜色建议同系列多款通用，因此系统多处简称为“色系”。"));
    mapMsg.insert("win_cargo", QStringLiteral("货品\t指可能拥有多码多色的同款同价商品。"));
    mapMsg.insert("win_subject", QStringLiteral("账目\t收支账目编制。"));
    mapMsg.insert("win_shop", QStringLiteral("门店\t门店概念包括仓库，店即库、库即店。"));
    mapMsg.insert("win_staff", QStringLiteral("员工\t员工登记用于业绩统计。"));
    mapMsg.insert("win_supplier", QStringLiteral("厂商\t供货单位。"));
    mapMsg.insert("win_customer", QStringLiteral("客户\t购货单位或个人。零售顾客建议登记手机号代替名称。"));
    mapMsg.insert("win_barcoderule", QStringLiteral("条码识别规则\t条形码字符特征，扫描必须设置。"));
    mapMsg.insert("win_buypolicy", QStringLiteral("采购价格政策\t设计保留..."));
    mapMsg.insert("win_lotpolicy", QStringLiteral("批发价格政策\t设计保留..."));
    mapMsg.insert("win_retpolicy", QStringLiteral("零售价格政策\t设计保留..."));
    mapMsg.insert("win_movpolicy", QStringLiteral("调拨价格政策\t设计保留..."));

    mapMsg.insert("win_<sheet>", QStringLiteral("业务单据\t单据是查询统计的数据基础。按钮与菜单如为灰色，要么是因为状态，要么是没有权限。"));
    mapMsg.insert("win_cgd", QStringLiteral("采购订货单\t填制订货单的目的，结合实际采购进货单，可以统计订单完成情况。订货不会入库。"));
    mapMsg.insert("win_cgj", QStringLiteral("采购进货单\t实际采购入库，数量、金额与付款都应流水记录。"));
    mapMsg.insert("win_cgt", QStringLiteral("采购退货单\t采购退货，真实数量出库。所有数值应填正值。"));
    mapMsg.insert("win_pfd", QStringLiteral("批发订货单\t填制订货单的目的，结合实际批发发货单，可以统计订单完成情况。订货不会出库。"));
    mapMsg.insert("win_pff", QStringLiteral("批发发货单\t实际批发出库，数量、金额与付款都应流水记录。"));
    mapMsg.insert("win_pft", QStringLiteral("批发退货单\t批发退货，真实数量入库。所有数值应填正值。"));
    mapMsg.insert("win_lsd", QStringLiteral("零售单\t实际零售出库，数量、金额与付款都应流水记录。"));
    mapMsg.insert("win_syd", QStringLiteral("损益单\t通常用于盘点色码纠正。正数增加库存，负数减少库存，相当于采购入库。"));
    mapMsg.insert("win_dbd", QStringLiteral("调拨单\t调出库减少库存，调入库增加库存。"));
    mapMsg.insert("win_szd", QStringLiteral("报销\t收支钱款进出记录。"));

    mapMsg.insert("win_<query>", QStringLiteral("业务统计\t单据加减汇总查询。"));
    mapMsg.insert("win_vi_cgd", QStringLiteral("采购订货单统计\t采购订货单纯加法汇总。"));
    mapMsg.insert("win_vi_cgj", QStringLiteral("采购进货单统计\t采购进货单纯加法汇总。"));
    mapMsg.insert("win_vi_cgt", QStringLiteral("采购退货单统计\t采购退货单纯加法汇总。"));
    mapMsg.insert("win_vi_pfd", QStringLiteral("批发订货单统计\t批发订货单纯加法汇总。"));
    mapMsg.insert("win_vi_pff", QStringLiteral("批发发货单统计\t批发发货单纯加法汇总。"));
    mapMsg.insert("win_vi_pft", QStringLiteral("批发退货单统计\t批发退货单纯加法汇总。"));
    mapMsg.insert("win_vi_lsd", QStringLiteral("零售单统计\t零售单纯加法汇总。"));
    mapMsg.insert("win_vi_syd", QStringLiteral("损益单统计\t损益单纯加法汇总。"));
    mapMsg.insert("win_vi_dbd", QStringLiteral("调拨单统计\t调拨单纯加法汇总。"));
    mapMsg.insert("win_vi_szd", QStringLiteral("收支单统计\t收支单纯加法汇总。"));
    mapMsg.insert("win_vi_cg",      QStringLiteral("净采购货品统计\t采购进货单加、采购退货单减，之汇总。"));
    mapMsg.insert("win_vi_cg_cash", QStringLiteral("净采购欠款\t采购进货单加、采购退货单减，之收付欠款为重点汇总"));
    mapMsg.insert("win_vi_cg_rest", QStringLiteral("采购订欠货\t采购订货单加、采购进货单减，之汇总。"));
    mapMsg.insert("win_vi_pf",      QStringLiteral("净批发货品统计\t批发发货单加、批发发货单减，之汇总。"));
    mapMsg.insert("win_vi_pf_cash", QStringLiteral("净批发欠款\t批发发货单加、批发发货单减，之收付欠款为重点汇总。"));
    mapMsg.insert("win_vi_pf_rest", QStringLiteral("批发订欠货\t批发订货单加、批发发货单减，之汇总。"));
    mapMsg.insert("win_vi_xs",      QStringLiteral("净销售货品统计\t批发发货单加、零售单加、批发退货单减，之汇总。"));
    mapMsg.insert("win_vi_xs_cash", QStringLiteral("净销售欠款\t批发发货单加、零售单加、批发退货单减，之收付欠款为重点汇总。"));
    mapMsg.insert("win_vi_stock",   QStringLiteral("货品库存\t采购进货单加、采购退货单减、采购退货单加、批发发货单减、"
                                                   "批发退货单加、零售单减、损益单加、调拨单有加有减，之汇总。"));
    mapMsg.insert("win_vi_all",     QStringLiteral("进销存一览\t一段时间内，按期初、各单进出、期末数之汇总。"));


    //字段===============================================\t隔开各项分别为：cname, defSQL, statusTip, flags, len_dots
    //特注：此处定义仅仅为一般性定义，具体特别化更正，在各窗口子类中，trader.cname等。
    //特注：字符串长度10、20、50、100四级跳。

    mapMsg.insert("fld_upman", QStringLiteral("最后更新人\tTEXT DEFAULT ''\t单据最后保存的登录人\t%1\t%2")
                  .arg(bsffText | bsffHideSys | bsffAggNone).arg(10));

    mapMsg.insert("fld_uptime", QStringLiteral("最后更新时间\tINTEGER DEFAULT 0\t单据最后保存的系统时间\t%1\t%2")
                  .arg(bsffDateTime | bsffHideSys | bsffAggNone).arg(0));

    mapMsg.insert("fld_sheetid", QStringLiteral("单据号\tINTEGER PRIMARY KEY AUTOINCREMENT\t单据流水编号\t%1\t%2")
                  .arg(bsffInt | bsffHead | bsffAggNone | bsffPKey).arg(0));

    mapMsg.insert("fld_proof", QStringLiteral("原单号\tTEXT DEFAULT ''\t备用选填\t%1\t%2")
                  .arg(bsffText | bsffHead | bsffAggNone).arg(20));

    mapMsg.insert("fld_dated", QStringLiteral("日期\tINTEGER DEFAULT 0\t业务发生日\t%1\t%2")
                  .arg(bsffDate | bsffHead | bsffAggNone | bsffQryAsCon | bsffQryAsSel).arg(0));

    mapMsg.insert("fld_shop", QStringLiteral("门店\tTEXT DEFAULT ''\t业务记录门店\t%1\t%2")
                  .arg(bsffText | bsffHead | bsffAggCount | bsffQryAsCon | bsffQryAsSel | bsffQrySelBold).arg(20));

    mapMsg.insert("fld_trader", QStringLiteral("往来对方\tTEXT DEFAULT ''\t业务往来方\t%1\t%2")
                  .arg(bsffText | bsffHead | bsffAggCount | bsffQryAsCon | bsffQryAsSel | bsffQrySelBold).arg(20));

    mapMsg.insert("fld_stype", QStringLiteral("业务类型\tTEXT DEFAULT ''\t本业务细分类型\t%1\t%2")
                  .arg(bsffText | bsffHead | bsffAggCount | bsffQryAsCon | bsffQryAsSel).arg(10));

    mapMsg.insert("fld_staff", QStringLiteral("关联员工\tTEXT DEFAULT ''\t业务关联员工\t%1\t%2")
                  .arg(bsffText | bsffHead | bsffAggCount | bsffQryAsCon | bsffQryAsSel).arg(20));

    mapMsg.insert("fld_remark", QStringLiteral("备注\tTEXT DEFAULT ''\t选填记录，最多100字。\t%1\t%2")
                  .arg(bsffText | bsffHead | bsffAggNone).arg(100));

    mapMsg.insert("fld_sumqty", QStringLiteral("数量合计\tINTEGER DEFAULT 0\t本单货品数量合计\t%1\t%2")
                  .arg(bsffNumeric | bsffHead | bsffAggSum | bsffReadOnly | bsffQryAsVal).arg(0));

    mapMsg.insert("fld_summoney", QStringLiteral("金额合计\tINTEGER DEFAULT 0\t本单货品金额合计\t%1\t%2")
                  .arg(bsffNumeric | bsffHead | bsffAggSum | bsffReadOnly | bsffQryAsVal).arg(2));

    mapMsg.insert("fld_sumdis", QStringLiteral("折扣合计\tINTEGER DEFAULT 0\t以标牌价为参照，本单折扣掉的金额合计\t%1\t%2")
                  .arg(bsffNumeric | bsffHead | bsffAggSum | bsffReadOnly | bsffQryAsVal).arg(2));

    mapMsg.insert("fld_actpay", QStringLiteral("actpay\tINTEGER DEFAULT 0\t本单实际收付货款\t%1\t%2")
                  .arg(bsffNumeric | bsffHead | bsffAggSum | bsffQryAsVal).arg(2));

    mapMsg.insert("fld_actowe", QStringLiteral("本次欠款\tINTEGER DEFAULT 0\t本单欠款金额\t%1\t%2")
                  .arg(bsffNumeric | bsffHead | bsffAggSum | bsffReadOnly | bsffQryAsVal).arg(2));

    mapMsg.insert("fld_chktime", QStringLiteral("审核时间\tINTEGER DEFAULT 0\t本单审核时间\t%1\t%2")
                  .arg(bsffInt | bsffHead | bsffAggNone | bsffReadOnly | bsffQryAsCon).arg(0));

    mapMsg.insert("fld_checker", QStringLiteral("审核人\tTEXT DEFAULT ''\t本单审核登录人\t%1\t%2")
                  .arg(bsffText | bsffHead | bsffAggNone | bsffReadOnly).arg(10));

    mapMsg.insert("fld_parentid", QStringLiteral("单据号\tINTEGER NOT NULL\t父单号\t%1\t%2")
                  .arg(bsffInt | bsffFKey).arg(0));

    mapMsg.insert("fld_rowtime", QStringLiteral("行时序号\tINTEGER NOT NULL\t行时间\t%1\t%2")
                  .arg(bsffInt | bsffGrid | bsffAggNone | bsffSKey | bsffHideSys).arg(0));        //注意不能设为bsffDateTime

    mapMsg.insert("fld_cargo", QStringLiteral("货号\tTEXT NOT NULL\t货品主列\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggCount | bsffQryAsCon | bsffQryAsSel | bsffQrySelBold).arg(20));

    mapMsg.insert("fld_color", QStringLiteral("颜色\tTEXT NOT NULL\t颜色规格\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggCount | bsffQryAsSel).arg(10));

    mapMsg.insert("fld_sizers", QStringLiteral("尺码明细\tTEXT DEFAULT ''\t尺码数量明细\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone | bsffQryAsSel | bsffHideSys).arg(999999999));       //不限长TEXT

    mapMsg.insert("fld_qty", QStringLiteral("数量\tINTEGER DEFAULT 0\t各尺码数量合计\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal).arg(0));

    mapMsg.insert("fld_discount", QStringLiteral("折扣\tINTEGER DEFAULT 0\t针对标牌价的折扣\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggNone).arg(3));

    mapMsg.insert("fld_price", QStringLiteral("本单价\tINTEGER DEFAULT 0\t本单单价\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggNone).arg(2));

    mapMsg.insert("fld_actmoney", QStringLiteral("本单金额\tINTEGER DEFAULT 0\t本单金额\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffQryAsVal).arg(2));

    mapMsg.insert("fld_dismoney", QStringLiteral("折扣金额\tINTEGER DEFAULT 0\t折扣金额\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal).arg(2));

    mapMsg.insert("fld_rowmark", QStringLiteral("行备注\tTEXT DEFAULT ''\t本行备注\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone).arg(50));

    mapMsg.insert("fld_hpmark", QStringLiteral("货备注\tTEXT DEFAULT ''\t本行货品备注\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone).arg(50));

    mapMsg.insert("fld_minsizers", QStringLiteral("最少数量明细\tTEXT DEFAULT ''\t尺码数量明细\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone | bsffHideSys).arg(999999999));       //不限长TEXT

    mapMsg.insert("fld_maxsizers", QStringLiteral("最少数量明细\tTEXT DEFAULT ''\t尺码数量明细\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone | bsffHideSys).arg(999999999));       //不限长TEXT

    mapMsg.insert("fld_sheetname", QStringLiteral("单据名\tTEXT\t业务名简拼\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone).arg(3));                          //仅查询

    mapMsg.insert("fld_subject", QStringLiteral("账目\tTEXT NOT NULL\t账目主列\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggCount | bsffQryAsCon | bsffQryAsSel | bsffQrySelBold).arg(20));

    mapMsg.insert("fld_income", QStringLiteral("记收\tINTEGER DEFAULT 0\t记收金额\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffQryAsVal | bsffBlankZero).arg(2));

    mapMsg.insert("fld_expense", QStringLiteral("记支\tINTEGER DEFAULT 0\t记支金额\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffQryAsVal | bsffBlankZero).arg(2));

    //由于事务提交，多行会在同毫秒之内完成，因此，rowtime只能在QTableWiget表格中生成时间。
    //因此sqlite中默认值写法：(CAST((julianday('now')-2440587.5)*86400000 AS INTEGER))不能用。

    mapMsg.insert("fld_kname", QStringLiteral("名称\tTEXT PRIMARY KEY\t名称\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone | bsffPKey).arg(20));

    mapMsg.insert("fld_regdis", QStringLiteral("默认折扣\tINTEGER DEFAULT 0\t一般折扣\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggNone).arg(3));

    mapMsg.insert("fld_regman", QStringLiteral("联系人\tTEXT DEFAULT ''\t收件人\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone).arg(10));

    mapMsg.insert("fld_regaddr", QStringLiteral("地址\tTEXT DEFAULT ''\t收件地址\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone).arg(50));

    mapMsg.insert("fld_regtele", QStringLiteral("电话\tTEXT DEFAULT ''\t收件电话\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone).arg(50));

    mapMsg.insert("fld_regmark", QStringLiteral("备注\tTEXT DEFAULT ''\t备注\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone).arg(100));

    mapMsg.insert("fld_hpcode", QStringLiteral("货号\tTEXT PRIMARY KEY\t货号\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone | bsffPKey | bsffCargoRel).arg(20));

    mapMsg.insert("fld_hpname", QStringLiteral("品名\tTEXT DEFAULT ''\t品名\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggCount | bsffCargoRel).arg(20));

    mapMsg.insert("fld_sizertype", QStringLiteral("码类\tTEXT DEFAULT ''\t登记的尺码品类\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggCount | bsffCargoRel).arg(10));

    mapMsg.insert("fld_colortype", QStringLiteral("色系色表\tTEXT DEFAULT ''\t登记的颜色系列或直接逗号列举各色\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggCount | bsffCargoRel).arg(200));

    mapMsg.insert("fld_unit", QStringLiteral("单位\tTEXT DEFAULT ''\t计量单位\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggCount | bsffQryAsSel | bsffCargoRel).arg(10));

    mapMsg.insert("fld_setprice", QStringLiteral("标牌价\tINTEGER DEFAULT 0\t统一原始定价，为折扣计算依据。\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggNone | bsffQryAsSel | bsffCargoRel).arg(2));

    mapMsg.insert("fld_retprice", QStringLiteral("零售价\tINTEGER DEFAULT 0\t当前零售价，零售单默认价。\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggNone | bsffCargoRel).arg(2));

    mapMsg.insert("fld_lotprice", QStringLiteral("批发价\tINTEGER DEFAULT 0\t当前批发价，批发单默认价。\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggNone | bsffCargoRel).arg(2));

    mapMsg.insert("fld_buyprice", QStringLiteral("进货价\tINTEGER DEFAULT 0\t当前进货价，采购单默认价。\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggNone | bsffCargoRel).arg(2));

    mapMsg.insert("fld_setmoney", QStringLiteral("标价金额\tINTEGER DEFAULT 0\t按标牌价计算的金额\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal).arg(2));

    mapMsg.insert("fld_retmoney", QStringLiteral("零售价金额\tINTEGER DEFAULT 0\t按零售价计算的金额\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal).arg(2));

    mapMsg.insert("fld_lotmoney", QStringLiteral("批发价金额\tINTEGER DEFAULT 0\t按批发价计算的金额\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal).arg(2));

    mapMsg.insert("fld_buymoney", QStringLiteral("进货价金额\tINTEGER DEFAULT 0\t按进货价计算的金额\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal).arg(2));

    mapMsg.insert("fld_almin", QStringLiteral("最低数\tINTEGER DEFAULT 0\t库存数最低警报值，工具箱管理员或总经理设置。\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggNone | bsffBlankZero | bsffCargoRel | bsffReadOnly).arg(0));

    mapMsg.insert("fld_almax", QStringLiteral("最高数\tINTEGER DEFAULT 0\t库存数最高警报值，工具箱管理员或总经理设置。\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggNone | bsffBlankZero | bsffCargoRel | bsffReadOnly).arg(0));

    mapMsg.insert("fld_attr1", QStringLiteral("自设分类一\tTEXT DEFAULT ''\t自定义分类，系统参数窗口设置名称和各子项。\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggCount | bsffQryAsSel | bsffCargoRel).arg(10));

    mapMsg.insert("fld_attr2", QStringLiteral("自设分类二\tTEXT DEFAULT ''\t自定义分类，系统参数窗口设置名称和各子项。\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggCount | bsffQryAsSel | bsffCargoRel).arg(10));

    mapMsg.insert("fld_attr3", QStringLiteral("自设分类三\tTEXT DEFAULT ''\t自定义分类，系统参数窗口设置名称和各子项。\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggCount | bsffQryAsSel | bsffCargoRel).arg(10));

    mapMsg.insert("fld_attr4", QStringLiteral("自设分类四\tTEXT DEFAULT ''\t自定义分类，系统参数窗口设置名称和各子项。\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggCount | bsffQryAsSel | bsffCargoRel).arg(10));

    mapMsg.insert("fld_attr5", QStringLiteral("自设分类五\tTEXT DEFAULT ''\t自定义分类，系统参数窗口设置名称和各子项。\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggCount | bsffQryAsSel | bsffCargoRel).arg(10));

    mapMsg.insert("fld_attr6", QStringLiteral("自设分类六\tTEXT DEFAULT ''\t自定义分类，系统参数窗口设置名称和各子项。\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggCount | bsffQryAsSel | bsffCargoRel).arg(10));

    mapMsg.insert("fld_cancg", QStringLiteral("采购\tINTEGER DEFAULT 0\t采购单关联员工\t%1\t%2")
                  .arg(bsffBool | bsffGrid | bsffAggNone).arg(0));

    mapMsg.insert("fld_canpf", QStringLiteral("批发\tINTEGER DEFAULT 0\t批发单关联员工\t%1\t%2")
                  .arg(bsffBool | bsffGrid | bsffAggNone).arg(0));

    mapMsg.insert("fld_canls", QStringLiteral("零售\tINTEGER DEFAULT 0\t零售单关联员工\t%1\t%2")
                  .arg(bsffBool | bsffGrid | bsffAggNone).arg(0));

    mapMsg.insert("fld_candb", QStringLiteral("调拨\tINTEGER DEFAULT 0\t调拨单关联员工\t%1\t%2")
                  .arg(bsffBool | bsffGrid | bsffAggNone).arg(0));

    mapMsg.insert("fld_cansy", QStringLiteral("盘点\tINTEGER DEFAULT 0\t盘点单关联员工\t%1\t%2")
                  .arg(bsffBool | bsffGrid | bsffAggNone));

    mapMsg.insert("fld_adminboss", QStringLiteral("老总专用\tINTEGER DEFAULT 0\t总经理与管理员专用\t%1\t%2")
                  .arg(bsffBool | bsffGrid | bsffAggNone).arg(0));

    mapMsg.insert("fld_tname", QStringLiteral("品类或系列\tTEXT PRIMARY KEY\t简短命名即可。\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone | bsffPKey).arg(10));

    mapMsg.insert("fld_namelist", QStringLiteral("名称列举\tTEXT DEFAULT ''\t直接逗号分隔列举填写。\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone).arg(100));

    mapMsg.insert("fld_codelist", QStringLiteral("对应编码列举\tTEXT DEFAULT ''\t条码扫描必填；不扫描可不填。列举数目需一致。\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone).arg(100));


    mapMsg.insert("fld_barcodexp", QStringLiteral("条码规则（正则表达式）\tTEXT PRIMARY KEY\t识别解析条码提取货号色号尺码三要素的正则表达式。语法看帮助。\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone | bsffPKey).arg(100));

    mapMsg.insert("fld_sizermiddlee", QStringLiteral("尺码在色号前\tINTEGER DEFAULT 0\t条码字符串里通常色号编在尺码前，如果不是，这里指定。\t%1\t%2")
                  .arg(bsffBool | bsffGrid | bsffAggNone).arg(0));

    mapMsg.insert("fld_barcodemark", QStringLiteral("备注\tTEXT DEFAULT ''\t备注此规则是哪类商品。\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone).arg(20));


    mapMsg.insert("fld_policyname", QStringLiteral("政策名称\tTEXT PRIMARY KEY\t...\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone | bsffPKey).arg(20));

    mapMsg.insert("fld_traderexp", QStringLiteral("享受对象\tTEXT DEFAULT ''\t...\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone).arg(20000));

    mapMsg.insert("fld_cargoexp", QStringLiteral("实施货号\tTEXT DEFAULT ''\t...\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone).arg(20000));

    mapMsg.insert("fld_policydis", QStringLiteral("执行折扣\tINTEGER DEFAULT 5000\t...\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggNone).arg(3));

    mapMsg.insert("fld_uselevel", QStringLiteral("优先级别\tINTEGER DEFAULT 1\t...\t%1\t%2")
                  .arg(bsffInt | bsffGrid | bsffAggNone).arg(0));

    mapMsg.insert("fld_startdate", QStringLiteral("开始日期\tINTEGER DEFAULT 0\t...\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggNone).arg(3));

    mapMsg.insert("fld_enddate", QStringLiteral("结束日期\tINTEGER DEFAULT 99999999999\t...\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggNone).arg(3));


    mapMsg.insert("fld_yeard", QStringLiteral("年份\tINTEGER DEFAULT 0\t公元纪年\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggCount | bsffQryAsSel).arg(0));      //视图是text

    mapMsg.insert("fld_monthd", QStringLiteral("月份\tINTEGER DEFAULT 0\t年月份\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggCount | bsffQryAsSel).arg(0));      //视图是text


    mapMsg.insert("fld_sizer", QStringLiteral("尺码\tTEXT DEFAULT ''\t尺码\t%1\t%2")
                  .arg(bsffText | bsffGrid | bsffAggNone | bsffQryAsSel).arg(10));

    mapMsg.insert("fld_base", QStringLiteral("期初\tINTEGER DEFAULT 0\t期初库存数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal).arg(0));

    mapMsg.insert("fld_cgj", QStringLiteral("购进\tINTEGER DEFAULT 0\t期初库存数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal | bsffBlankZero).arg(0));

    mapMsg.insert("fld_cgt", QStringLiteral("购退\tINTEGER DEFAULT 0\t期初库存数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal | bsffBlankZero).arg(0));

    mapMsg.insert("fld_pff", QStringLiteral("批发\tINTEGER DEFAULT 0\t期初库存数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal | bsffBlankZero).arg(0));

    mapMsg.insert("fld_pft", QStringLiteral("批退\tINTEGER DEFAULT 0\t期初库存数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal | bsffBlankZero).arg(0));

    mapMsg.insert("fld_lsd", QStringLiteral("零售\tINTEGER DEFAULT 0\t期初库存数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal | bsffBlankZero).arg(0));

    mapMsg.insert("fld_syd", QStringLiteral("损益\tINTEGER DEFAULT 0\t期初库存数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal | bsffBlankZero).arg(0));

    mapMsg.insert("fld_dbd", QStringLiteral("调出\tINTEGER DEFAULT 0\t期初库存数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal | bsffBlankZero).arg(0));

    mapMsg.insert("fld_dbr", QStringLiteral("调入\tINTEGER DEFAULT 0\t期初库存数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal | bsffBlankZero).arg(0));

    mapMsg.insert("fld_stock", QStringLiteral("期末\tINTEGER DEFAULT 0\t期初库存数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly | bsffQryAsVal).arg(0));

    mapMsg.insert("fld_nowstock", QStringLiteral("当前库存\tINTEGER DEFAULT 0\t期初库存数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));

    mapMsg.insert("fld_limqty", QStringLiteral("预警数量\tINTEGER DEFAULT 0\t期初库存数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));

    //用与查询返回动态建表
    mapMsg.insert("fld_stockqty", QStringLiteral("数量\tINTEGER DEFAULT 0\t库存数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));
    mapMsg.insert("fld_stockmny", QStringLiteral("金额\tINTEGER DEFAULT 0\t库存金额\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(2));
    mapMsg.insert("fld_stockdis", QStringLiteral("折扣\tINTEGER DEFAULT 0\t库存折扣\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(2));

    mapMsg.insert("fld_summqty", QStringLiteral("数量\tINTEGER DEFAULT 0\t数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));
    mapMsg.insert("fld_summmny", QStringLiteral("金额\tINTEGER DEFAULT 0\t金额\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(2));
    mapMsg.insert("fld_summdis", QStringLiteral("折扣\tINTEGER DEFAULT 0\t折扣\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(2));

    mapMsg.insert("fld_cashqty", QStringLiteral("数量\tINTEGER DEFAULT 0\t数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));
    mapMsg.insert("fld_cashmny", QStringLiteral("金额\tINTEGER DEFAULT 0\t金额\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(2));
    mapMsg.insert("fld_cashdis", QStringLiteral("折扣\tINTEGER DEFAULT 0\t折扣\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(2));
    mapMsg.insert("fld_cashpay", QStringLiteral("实付\tINTEGER DEFAULT 0\t实付\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(2));
    mapMsg.insert("fld_cashowe", QStringLiteral("欠款\tINTEGER DEFAULT 0\t欠款\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(2));

    mapMsg.insert("fld_restqty", QStringLiteral("数量\tINTEGER DEFAULT 0\t数量\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));
    mapMsg.insert("fld_restmny", QStringLiteral("金额\tINTEGER DEFAULT 0\t金额\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(2));
    mapMsg.insert("fld_sumowe", QStringLiteral("总欠款\tINTEGER DEFAULT 0\t总欠款\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(2));

    mapMsg.insert("fld_qc", QStringLiteral("期初\tINTEGER DEFAULT 0\t期初\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));
    mapMsg.insert("fld_gj", QStringLiteral("购进\tINTEGER DEFAULT 0\t购进\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));
    mapMsg.insert("fld_gt", QStringLiteral("购退\tINTEGER DEFAULT 0\t购退\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));
    mapMsg.insert("fld_sy", QStringLiteral("损益\tINTEGER DEFAULT 0\t损益\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));
    mapMsg.insert("fld_dr", QStringLiteral("调入\tINTEGER DEFAULT 0\t调入\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));
    mapMsg.insert("fld_dc", QStringLiteral("调出\tINTEGER DEFAULT 0\t调出\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));
    mapMsg.insert("fld_pf", QStringLiteral("批发\tINTEGER DEFAULT 0\t批发\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));
    mapMsg.insert("fld_pt", QStringLiteral("批退\tINTEGER DEFAULT 0\t批退\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));
    mapMsg.insert("fld_ls", QStringLiteral("零售\tINTEGER DEFAULT 0\t零售\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));
    mapMsg.insert("fld_qm", QStringLiteral("期末\tINTEGER DEFAULT 0\t期末\t%1\t%2")
                  .arg(bsffNumeric | bsffGrid | bsffAggSum | bsffReadOnly).arg(0));

    mapMsg.insert("fldcname_cgd_shop", QStringLiteral("订货门店"));
    mapMsg.insert("fldcname_cgj_shop", QStringLiteral("入库门店"));
    mapMsg.insert("fldcname_cgt_shop", QStringLiteral("出库门店"));
    mapMsg.insert("fldcname_pfd_shop", QStringLiteral("订货门店"));
    mapMsg.insert("fldcname_pff_shop", QStringLiteral("出库门店"));
    mapMsg.insert("fldcname_pft_shop", QStringLiteral("入库门店"));
    mapMsg.insert("fldcname_lsd_shop", QStringLiteral("销售门店"));
    mapMsg.insert("fldcname_syd_shop", QStringLiteral("调整门店"));
    mapMsg.insert("fldcname_dbd_shop", QStringLiteral("调出门店"));
    mapMsg.insert("fldcname_szd_shop", QStringLiteral("关联门店"));
    mapMsg.insert("fldcname_cgd_trader", QStringLiteral("供货厂商"));
    mapMsg.insert("fldcname_cgj_trader", QStringLiteral("供货厂商"));
    mapMsg.insert("fldcname_cgt_trader", QStringLiteral("供货厂商"));
    mapMsg.insert("fldcname_pfd_trader", QStringLiteral("批发客户"));
    mapMsg.insert("fldcname_pff_trader", QStringLiteral("批发客户"));
    mapMsg.insert("fldcname_pft_trader", QStringLiteral("批发客户"));
    mapMsg.insert("fldcname_lsd_trader", QStringLiteral("零售顾客"));
    mapMsg.insert("fldcname_syd_trader", QStringLiteral("trader"));
    mapMsg.insert("fldcname_dbd_trader", QStringLiteral("调入门店"));
    mapMsg.insert("fldcname_szd_trader", QStringLiteral("关联客商"));
    mapMsg.insert("fldcname_cgd_actpay", QStringLiteral("本次实付"));
    mapMsg.insert("fldcname_cgj_actpay", QStringLiteral("本次实付"));
    mapMsg.insert("fldcname_cgt_actpay", QStringLiteral("本次实退"));
    mapMsg.insert("fldcname_pfd_actpay", QStringLiteral("本次实收"));
    mapMsg.insert("fldcname_pff_actpay", QStringLiteral("本次实收"));
    mapMsg.insert("fldcname_pft_actpay", QStringLiteral("本次实退"));
    mapMsg.insert("fldcname_lsd_actpay", QStringLiteral("本次实收"));
    mapMsg.insert("fldcname_dbd_actpay", QStringLiteral("本次实调"));
    mapMsg.insert("fldcname_syd_actpay", QStringLiteral("actpay"));
    mapMsg.insert("fldcname_szd_actpay", QStringLiteral("actpay"));



    //主按钮====================================================================

    //caption
    mapMsg.insert("btn_toolcase", QStringLiteral("工具箱\t辅助工具"));
    mapMsg.insert("btn_optionbox", QStringLiteral("开关盒\t选项开关"));

    mapMsg.insert("btn_reg_edit", QStringLiteral("开启编辑\t编辑模式用于一行一行增、删、改数据；“保存”或“取消”结束编辑模式。"));
    mapMsg.insert("btn_reg_save", QStringLiteral("保存\t将表格中所做的“增”、“删”、“改”编辑，一次性保存。"));
    mapMsg.insert("btn_reg_cancel", QStringLiteral("取消\t取消尚未保存的“增”、“删”、“改”编辑过程。"));

    mapMsg.insert("btn_sheet_new", QStringLiteral("新建\t新建空白单据，等待填写。"));
    mapMsg.insert("btn_sheet_edit", QStringLiteral("修改\t进入编辑态。因为单据阅读状态是不可编辑的，目的是为防止不小心误碰键盘。"));
    mapMsg.insert("btn_sheet_del", QStringLiteral("删除\t删除该张单据。"));
    mapMsg.insert("btn_sheet_save", QStringLiteral("保存\t保存该张单据"));
    mapMsg.insert("btn_sheet_cancel", QStringLiteral("取消\t取消该张单据"));
    mapMsg.insert("btn_sheet_open", QStringLiteral("打开\t查找单据"));
    mapMsg.insert("btn_sheet_check", QStringLiteral("审核\t审核单据以防止修改"));

    mapMsg.insert("btn_history", QStringLiteral("核对\t具体货品的详细色码或对账清单"));
    mapMsg.insert("btn_print", QStringLiteral("打印\t打印前需要设计格式"));
    mapMsg.insert("btn_export", QStringLiteral("导出\t导出到通用csv格式。"));
    mapMsg.insert("btn_help", QStringLiteral("帮助\t官网 www.bailisoft.com 帮助"));

    mapMsg.insert("btn_back_requery", QStringLiteral("返回重查\t重新设置查询。"));



    //工具箱菜单====================================================================

    //caption
    mapMsg.insert("tool_export", QStringLiteral("导出数据"));
    mapMsg.insert("tool_set_right", QStringLiteral("设置权限"));
    mapMsg.insert("tool_define_name", QStringLiteral("定义字段名称"));
    mapMsg.insert("tool_print_setting", QStringLiteral("打印设计"));
    mapMsg.insert("tool_sheet_uncheck", QStringLiteral("撤销审核"));
    mapMsg.insert("tool_import_data", QStringLiteral("导入数据"));
    mapMsg.insert("tool_setmoney_calc", QStringLiteral("标牌价金额演算"));
    mapMsg.insert("tool_retmoney_calc", QStringLiteral("零售价金额演算"));
    mapMsg.insert("tool_lotmoney_calc", QStringLiteral("批发价金额演算"));
    mapMsg.insert("tool_buymoney_calc", QStringLiteral("进货价金额演算"));
    mapMsg.insert("tool_import_batch_barcodes", QStringLiteral("批量导入条形码"));
    mapMsg.insert("tool_copy_import_sheet", QStringLiteral("复制导入单据"));
    mapMsg.insert("tool_alarm_setting", QStringLiteral("设置库存警报"));
    mapMsg.insert("tool_alarm_remove", QStringLiteral("解除库存警报"));
    mapMsg.insert("tool_hide_current_col", QStringLiteral("隐藏表格当前列"));
    mapMsg.insert("tool_show_all_cols", QStringLiteral("显示表格全部列"));
    mapMsg.insert("tool_adjust_current_row_position", QStringLiteral("调整当前行位序"));



    //开关盒菜单====================================================================

    mapMsg.insert("opt_dont_show_guide_anymore", QStringLiteral("不再显示顶部黄色提示条"));
    mapMsg.insert("opt_hide_drop_red_row", QStringLiteral("不要显示待删除红色行"));
    mapMsg.insert("opt_sort_befor_print", QStringLiteral("打印前强制排序行顺序"));
    mapMsg.insert("opt_hide_noqty_sizercol_when_open", QStringLiteral("打开单据后自动隐藏全无数量尺码列"));
    mapMsg.insert("opt_hide_noqty_sizercol_when_print", QStringLiteral("打印前自动隐藏全无数量尺码列"));
    mapMsg.insert("opt_print_zero_size_qty", QStringLiteral("零数量尺码也要打印"));
    mapMsg.insert("opt_auto_use_first_color", QStringLiteral("录入新行货号后自动填第一个颜色"));



    //杂类按钮菜单====================================================================
    mapMsg.insert("word_admin", QStringLiteral("管理员"));
    mapMsg.insert("word_yes", QStringLiteral("是"));
    mapMsg.insert("word_execute", QStringLiteral("执行"));
    mapMsg.insert("word_list_object", QStringLiteral("列出对象"));
    mapMsg.insert("word_list_belong", QStringLiteral("对于"));
    mapMsg.insert("word_list_relative", QStringLiteral("关联方"));
    mapMsg.insert("word_i_have_know", QStringLiteral("知道了"));
    mapMsg.insert("word_baili_sheet_paper", QStringLiteral("百利单据用纸"));
    mapMsg.insert("word_check_range", QStringLiteral("审核范围"));
    mapMsg.insert("word_not_checked", QStringLiteral("仅未审核"));
    mapMsg.insert("word_only_checked", QStringLiteral("仅已审核"));
    mapMsg.insert("word_any_checked", QStringLiteral("全部无论审核"));
    mapMsg.insert("word_sum_qty", QStringLiteral("总数"));
    mapMsg.insert("word_sum_mny", QStringLiteral("总额"));
    mapMsg.insert("word_sum_payi", QStringLiteral("总收"));
    mapMsg.insert("word_sum_payo", QStringLiteral("总付"));
    mapMsg.insert("word_sum_owe", QStringLiteral("总欠"));
    mapMsg.insert("word_print_total_owe", QStringLiteral("当前总欠款"));
    mapMsg.insert("word_payi", QStringLiteral("实收"));
    mapMsg.insert("word_payo", QStringLiteral("实付"));
    mapMsg.insert("word_history", QStringLiteral("核对明细"));
    mapMsg.insert("word_scan_assitant", QStringLiteral("条码扫描"));
    mapMsg.insert("word_pick_assitant", QStringLiteral("筛选拣货"));
    mapMsg.insert("word_pick_trader_stock", QStringLiteral("查看调入库"));
    mapMsg.insert("word_query", QStringLiteral("查询"));
    mapMsg.insert("word_start", QStringLiteral("启动"));
    mapMsg.insert("word_stop", QStringLiteral("停止"));
    mapMsg.insert("word_total", QStringLiteral("合计"));


    mapMsg.insert("btn_ok", QStringLiteral("确定"));
    mapMsg.insert("btn_cancel", QStringLiteral("取消"));
    mapMsg.insert("btn_apply", QStringLiteral("提交生效"));
    mapMsg.insert("btn_giveup_save", QStringLiteral("放弃保存"));
    mapMsg.insert("btn_help", QStringLiteral("帮助"));
    mapMsg.insert("btn_reload", QStringLiteral("刷新"));

    mapMsg.insert("menu_filter_in", QStringLiteral("选择保留..."));
    mapMsg.insert("menu_filter_out", QStringLiteral("剔除"));
    mapMsg.insert("menu_filter_restore_col", QStringLiteral("恢复本列"));
    mapMsg.insert("menu_filter_restore_all", QStringLiteral("恢复全部"));

    mapMsg.insert("menu_today", QStringLiteral("今天"));
    mapMsg.insert("menu_yesterday", QStringLiteral("昨天"));
    mapMsg.insert("menu_this_week", QStringLiteral("本周"));
    mapMsg.insert("menu_last_week", QStringLiteral("上周"));
    mapMsg.insert("menu_this_month", QStringLiteral("本月"));
    mapMsg.insert("menu_last_month", QStringLiteral("上月"));
    mapMsg.insert("menu_this_year", QStringLiteral("今年"));
    mapMsg.insert("menu_last_year", QStringLiteral("去年"));
    mapMsg.insert("menu_batch_edit", QStringLiteral("登记名批量修改器"));
    mapMsg.insert("menu_barcode_maker", QStringLiteral("条形码批量编码器"));
    mapMsg.insert("menu_custom", QStringLiteral("更多定制…"));

    mapMsg.insert("qry_panel_con", QStringLiteral("条件范围"));
    mapMsg.insert("qry_panel_sel", QStringLiteral("统计角度"));
    mapMsg.insert("qry_panel_val", QStringLiteral("计算数值"));
    mapMsg.insert("qry_stock_shop_cname", QStringLiteral("库存门店"));
    mapMsg.insert("qry_pff_lsd_trader_cname", QStringLiteral("批零客户"));
    mapMsg.insert("qry_stock_qty", QStringLiteral("库存数量"));
    mapMsg.insert("qry_money_occupied", QStringLiteral("资金占用"));
    mapMsg.insert("qry_dismoney_gain", QStringLiteral("折扣盈利"));
    mapMsg.insert("qry_qty_cname", QStringLiteral("数量"));
    mapMsg.insert("qry_money_cname", QStringLiteral("金额"));
    mapMsg.insert("qry_dismoney_cname", QStringLiteral("折扣额"));
    mapMsg.insert("qry_actpayout_cname", QStringLiteral("实付"));
    mapMsg.insert("qry_actpayin_cname", QStringLiteral("实收"));
    mapMsg.insert("qry_actowe_cname", QStringLiteral("欠款"));
    mapMsg.insert("qry_show_zero_stock", QStringLiteral("显示零库存"));
    mapMsg.insert("qry_show_finished_items", QStringLiteral("显示已完成部分"));


    //信息文字====================================================================

    mapMsg.insert("i_footer_sum", QStringLiteral("合计"));
    mapMsg.insert("i_none_size", QStringLiteral("单码"));
    mapMsg.insert("i_sheet_checked", QStringLiteral("已审"));
    mapMsg.insert("i_sheet_unchecked", QStringLiteral("未审"));
    mapMsg.insert("i_period_quick_select", QStringLiteral("日期快选"));
    mapMsg.insert("i_date_begin_text", QStringLiteral("开始日期"));
    mapMsg.insert("i_date_end_text", QStringLiteral("截至日期"));
    mapMsg.insert("i_date_begin_label", QStringLiteral("开始日期："));
    mapMsg.insert("i_date_end_label", QStringLiteral("截至日期："));
    mapMsg.insert("i_con_check_noway", QStringLiteral("不论审核"));
    mapMsg.insert("i_con_checked", QStringLiteral("仅限审核"));
    mapMsg.insert("i_con_uncheck", QStringLiteral("仅限未审"));
    mapMsg.insert("i_sheet_id_label", QStringLiteral("单据号："));
    mapMsg.insert("i_new_sheet_to_be_created", QStringLiteral("新单待建"));
    mapMsg.insert("i_status_check_time", QStringLiteral("审核时间"));

    mapMsg.insert("i_app_make_datadir_fail", QStringLiteral("创建账册文件夹失败！"));
    mapMsg.insert("i_app_make_imagedir_fail", QStringLiteral("创建默认图片文件夹失败！"));
    mapMsg.insert("i_app_make_bakdir_fail", QStringLiteral("创建备份文件夹失败！"));
    mapMsg.insert("i_app_sqlite_driver_drror", QStringLiteral("系统运行sqlite失败！"));
    mapMsg.insert("i_reg_edit_save_hint", QStringLiteral("记得最后要统一【保存】"));
    mapMsg.insert("i_no_more_help_msg", QStringLiteral("更多内容，请参考www.bailisoft.com官网。"));

    mapMsg.insert("i_cannot_filter_in_because_few", QStringLiteral("本列值只有一项，无需筛选。"));
    mapMsg.insert("i_cannot_filter_out_because_few", QStringLiteral("本列值只有一项，不能再剔除。"));
    mapMsg.insert("i_cannot_filter_when_dirty", QStringLiteral("<font color='red'>请保存或取消，然后才可以筛选表格。</font>"));
    mapMsg.insert("i_this_col_cannot_filter", QStringLiteral("<font color='red'>主列、数值列、以及无分类意义的列，只能排序，不能筛选。</font>"));
    mapMsg.insert("i_edit_mode_tip", QStringLiteral("【增】在表格最后一行按键盘下箭头键；【删】一行头部红叉叉按钮；【改】直接打字。"));
    mapMsg.insert("i_read_mode_tip", QStringLiteral("点击列头排序；右击表格短文字列内容筛选。"));
    mapMsg.insert("i_sheet_query_open_tip", QStringLiteral("双击具体行打开该单据。"));
    mapMsg.insert("i_barcode_scan_tip", QStringLiteral("条码枪扫描，请先将当前光标放到表格数量列（灰色不可编辑的数量列）"));

    mapMsg.insert("i_error_invalid_number", QStringLiteral("无效数值或公式！"));
    mapMsg.insert("i_warning_too_long_text", QStringLiteral("文字超长！"));
    mapMsg.insert("i_keycol_value_duplicated", QStringLiteral("重复登记！"));
    mapMsg.insert("i_cargo_not_registered", QStringLiteral("该货号未登记！"));
    mapMsg.insert("i_subject_not_registered", QStringLiteral("该账目未登记！"));
    mapMsg.insert("i_cargo_not_registered_or_not_complete", QStringLiteral("该货号未登记或色码类型登记不完整！"));
    mapMsg.insert("i_color_not_found_by_cargo", QStringLiteral("该货号下无此色号！"));
    mapMsg.insert("i_unknown_color_of_unknow_cargo", QStringLiteral("未登记货号下的未知色号！"));
    mapMsg.insert("i_invalid_barcode", QStringLiteral("无效或不可识别的条码！"));
    mapMsg.insert("i_cargo_has_no_colortype", QStringLiteral("识别的色号不在识别的货号登记颜色系列内！"));
    mapMsg.insert("i_cargo_has_no_sizertype", QStringLiteral("识别的尺码不在识别的货号登记尺码品类内！"));

    mapMsg.insert("i_head_col_must_edit_first", QStringLiteral("第一列主列必须先填！"));
    mapMsg.insert("i_cannot_edit_exists_key_col", QStringLiteral("主列禁止更改。您可删除该行，然后重录新行。"));

    mapMsg.insert("i_save_found_error", QStringLiteral("发现红色错误标识，请排除后再保存！"));
    mapMsg.insert("i_save_found_warning", QStringLiteral("发现黄色警告标识！"));
    mapMsg.insert("i_save_ask_warning", QStringLiteral("确定不用管它直接保存吗？"));
    mapMsg.insert("i_delete_sheet_notice", QStringLiteral("本操作不可撤销，但只删内容，而保留单据号与操作人操作时间。"));
    mapMsg.insert("i_delete_sheet_confirm", QStringLiteral("您确定要删除本单吗？"));
    mapMsg.insert("i_check_sheet_notice", QStringLiteral("本操作不可撤销，一经审核，不能再改。如有错需另外开单冲平。"));
    mapMsg.insert("i_check_sheet_confirm", QStringLiteral("您确定要审核本单吗？"));
    mapMsg.insert("i_check_sheet_failed", QStringLiteral("审核不成功，请稍后重试。"));
    mapMsg.insert("i_uncheck_sheet_failed", QStringLiteral("撤审不成功，请稍后重试。"));
    mapMsg.insert("i_check_sheet_success", QStringLiteral("审核本单成功！"));
    mapMsg.insert("i_uncheck_sheet_success", QStringLiteral("撤审成功！"));
    mapMsg.insert("i_window_in_editing", QStringLiteral("窗口处于编辑状态，请“保存”或“取消”后关闭。"));
    mapMsg.insert("i_found_editing_win", QStringLiteral("有打开的窗口处于未保存编辑态，请先保存或取消。"));
    mapMsg.insert("i_need_close_other_wins_first", QStringLiteral("本操作需要关闭所有打开的窗口。"));
    mapMsg.insert("i_close_all_other_win_now", QStringLiteral("是否立即关闭？"));
    mapMsg.insert("i_please_pick_a_row", QStringLiteral("请先选择具体一行！"));
    mapMsg.insert("i_scan_assistant", QStringLiteral("请将光标移到尺码合计数量列（任意行），就可开枪扫描，不用专门的输入框。"));
    mapMsg.insert("i_please_pick_shop_first", QStringLiteral("请先指定门店！"));
    mapMsg.insert("i_please_pick_trader_first", QStringLiteral("请先指定调入门店！"));
    mapMsg.insert("i_please_pick_sizertype_first", QStringLiteral("码类必须指定！"));
    mapMsg.insert("i_pick_query_first", QStringLiteral("请先选择码类查询库存"));
    mapMsg.insert("i_need_close_input_method", QStringLiteral("请关闭输入法"));
    mapMsg.insert("i_pick_keypress_hint", QStringLiteral("请在表格中任意处打货号字符或品名首字母进行筛选。"));
    mapMsg.insert("i_pick_back_space_hint", QStringLiteral("退格键减少字符，空格键取消筛选，具体色码数量上双击或加号键拣货。"));


    mapMsg.insert("i_printer_not_support_this_paper_size", QStringLiteral("此打印机无法支持此尺寸规格！"));
    mapMsg.insert("i_reg_import_over_msg", QStringLiteral("导入完成，可以核对后决定保存或取消。"));
    mapMsg.insert("i_common_text_file", QStringLiteral("通用文本文件(*.txt)"));
    mapMsg.insert("i_formatted_csv_file", QStringLiteral("通用格式文本文件(*.csv *.txt)"));
    mapMsg.insert("i_invliad_baili_sheet_data", QStringLiteral("无效的百利单据格式文件！"));
    mapMsg.insert("i_import_sheet_finished_ok", QStringLiteral("单据明细导入完成！"));
    mapMsg.insert("i_import_sheet_too_many_lost", QStringLiteral("单据导入完成，但有太多未登记货品未填入！"));
    mapMsg.insert("i_import_sheet_lost_following", QStringLiteral("单据导入完成，如下货品登记登记不正确，未填入：\n"));
    mapMsg.insert("i_qry_execute_failed", QStringLiteral("查询出错！"));
    mapMsg.insert("i_need_pick_one_grid_row", QStringLiteral("本操作需要先点击表格具体某行数据。"));
    mapMsg.insert("i_need_sizertype_befor_alarm_setting", QStringLiteral("每个设置警报的货号，都必须登记色码类型。一个色、一个码也要登记指定。"));
    mapMsg.insert("i_update_demo_book_date", QStringLiteral("您已登录百利样例账册，为便于观摩，所有单据日期调整为最新日期。"));
    mapMsg.insert("i_pick_one_row_first", QStringLiteral("请选择具体一行！"));
    mapMsg.insert("i_tobe_put_row_num", QStringLiteral("调整为第几行："));
    mapMsg.insert("i_require_restart_net_server", QStringLiteral("更改保密码后，需要重新启动网络服务器才能生效。"));
    mapMsg.insert("i_adjust_rowtime_need_clean_sort", QStringLiteral("数据已经变序，请重新打开单据保持原序，然后调整。"));
    mapMsg.insert("i_r15user_lic_warning",  QStringLiteral("仅限体验，录入请购买新版软件狗！"));


    //样式====================================================================

    mapMsg.insert("css_vertical_gradient", QLatin1String("background-color: qlineargradient("
                                                         "x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #f5f5f5, stop: 1 #ddd);"));

    mapMsg.insert("css_grid_editable",  QLatin1String("QTableWidget { "
                                                      "selection-background-color: #170; "
                                                      "selection-color: #fff; }"));
    mapMsg.insert("css_grid_readonly",  QLatin1String("QTableWidget { "
                                                      "background: #fff; "
                                                      "alternate-background-color: #eee; "
                                                      "selection-background-color: #170; "
                                                      "selection-color: #fff; }"));
    mapMsg.insert("css_grid_filtering",  QLatin1String("QTableWidget { "
                                                       "background: #ffd; "
                                                       "alternate-background-color: #cef; "
                                                       "selection-background-color: #170; "
                                                       "selection-color: #fff; }"));

//    mapMsg.insert("", QLatin1String("selection-background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
//                                                      "stop: 0   rgba(200, 200, 200, 255), "
//                                                      "stop: 0.5 rgba(255, 255, 255, 255), "
//                                                      "stop: 1   rgba(200, 200, 200, 255)); "
//                                                      "selection-color: #000; "));

    //====================================================================


    //检查确保键名无大写（防止粗心）
#ifdef QT_DEBUG
    QMapIterator<QString, QString> it(mapMsg);
    while ( it.hasNext() ) {
        it.next();
        QString key = it.key();
        if ( key.indexOf(QRegularExpression("[A-Z]")) >= 0 ) {
            qDebug() << QStringLiteral("编码警告: Found upper-case letter %1 in mapFld.").arg(key);
        }
    }
#endif
}

//批量提交
QString sqliteCommit(const QStringList sqls)
{
    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery qry(db);
    db.transaction();
    foreach (QString sql, sqls) {
        if ( sql.trimmed().length() > 10 )
        {
            qry.exec(sql);
            if ( qry.lastError().isValid() )
            {
                db.rollback();
                qDebug() << qry.lastError().text();
                qDebug() << sql;
                return QStringLiteral("%1\n%2").arg(qry.lastError().text()).arg(sql);
            }
        }
    }
    db.commit();
    return QString();
}


}
