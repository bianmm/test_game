#include <string.h>

#include "proto.xml.h"

const char* GET_SVR_PROTO_STR_ZIP_CMD = "GET_SERVER_PROTO_XML_ZIP\r\n";
const char* GET_SVR_PROTO_STR_RAW_CMD = "GET_SERVER_PROTO_XML_RAW\r\n";

const char* get_server_proto_xml_str();

static const uLongf g_zlib_buf_len = 1024 * 32;
static Bytef g_zlib_buffer[g_zlib_buf_len] = {0};

int get_server_proto_xml_zip(Bytef** buf, uLongf* len)
{
    const char* xml_str = get_server_proto_xml_str();
    *len = g_zlib_buf_len;
    if (Z_OK != compress(g_zlib_buffer, len, (Bytef*)xml_str, (uLongf)strlen(xml_str))) {
        *buf = NULL;
        *len = 0;
        return -1;
    }

    *buf = g_zlib_buffer;
    return 0;
}

#define GAMEFLAG_DEFINITION "1:摩尔 2:赛尔 3:大玩国 4:DUDU 5:小花仙 6:功夫 7:mole2(勇士) 8:摩尔宝宝 9:校巴 10:赛尔2 21:哈奇"

const char* get_server_proto_xml_str()
{
    return "<?xml version=\"1.0\"?>\n\
    <service name=\"mb_account\" version=\"1.9.8\" compile_time=\""__DATE__" "__TIME__"\" compiler_ver=\""__VERSION__"\">\n\
    <proto_header> \n\
        <item name=\"pkg_len\" type=\"S\" id=\"pkg_len\" desc=\"包长\"/> \n\
        <item name=\"cmd_id\" type=\"S\" id=\"cmd_id\" desc=\"命令号\"/> \n\
        <item name=\"result\" type=\"S\" id=\"result\" desc=\"返回码\"/> \n\
        <item name=\"seq_num\" type=\"L\" id=\"seq_num\" desc=\"包序号\"/> \n\
    </proto_header> \n\
    <proto name=\"charge_mb\" cmd_id=\"2001\" desc=\"米币帐号充值\" need_verify=\"true\"> \n\
        <req> \n\
            <item name=\"account_id\" type=\"L\" desc=\"米米号\" /> \n\
            <item name=\"mb_num\" type=\"L\" desc=\"米币数额(100倍)\" /> \n\
            <item name=\"pay_gate_trans_id\" type=\"L\" desc=\"充值网关事务ID\" /> \n\
            <item name=\"private_data_01\" type=\"L\" desc=\"充值接口私有数据\" /> \n\
            <item name=\"private_data_02\" type=\"a64\" desc=\"充值接口私有数据\" /> \n\
        </req> \n\
        <ack> \n\
            <item name=\"transaction_id_low\" type=\"L\" desc=\"交易流水号(低位)\" /> \n\
            <item name=\"transaction_id_high\" type=\"L\" desc=\"交易流水号(高位)\" /> \n\
        </ack> \n\
    </proto> \n\
    <proto name=\"consume_mb\" cmd_id=\"2002\" desc=\"米币消费\" need_verify=\"true\"> \n\
        <req> \n\
            <item name=\"account_id\" type=\"L\" desc=\"米米号\" /> \n\
            <item name=\"account_pwd\" type=\"a16\" desc=\"支付密码\" /> \n\
            <item name=\"dest_account_id\" type=\"L\" desc=\"目标米米号\" /> \n\
            <item name=\"product_id\" type=\"L\" desc=\"购买产品ID\" /> \n\
            <item name=\"product_count\" type=\"S\" desc=\"购买产品数量\" /> \n\
            <item name=\"mb_num\" type=\"L\" desc=\"所需米币(100倍)\" /> \n\
            <item name=\"consume_trans_id\" type=\"L\" desc=\"本次消费的事务ID\" /> \n\
        </req> \n\
        <ack> \n\
            <item name=\"transaction_id_low\" type=\"L\" desc=\"交易流水号(低位)\" /> \n\
            <item name=\"transaction_id_high\" type=\"L\" desc=\"交易流水号(高位)\" /> \n\
            <item name=\"mb_num_balance\" type=\"L\" desc=\"米币余额(100倍)\" /> \n\
        </ack> \n\
    </proto> \n\
    <proto name=\"query_balance\" cmd_id=\"3001\" desc=\"查询用户米币余额\"> \n\
        <req> \n\
            <item name=\"account_id\" type=\"L\" desc=\"米米号\" /> \n\
            <item name=\"account_pwd\" type=\"a16\" desc=\"用户密码\" /> \n\
        </req> \n\
        <ack> \n\
            <item name=\"mb_num_balance\" type=\"L\" desc=\"米币余额(100倍)\" /> \n\
            <item name=\"account_status\" type=\"L\" desc=\"账户状态\" /> \n\
        </ack> \n\
    </proto> \n\
    <proto name=\"query_transaction_id\" cmd_id=\"3002\" desc=\"事务状态查询\"> \n\
        <req> \n\
            <item name=\"transaction_id_low\" type=\"L\" desc=\"交易流水号(低位)\" /> \n\
            <item name=\"transaction_id_high\" type=\"L\" desc=\"交易流水号(高位)\" /> \n\
        </req> \n\
        <ack> \n\
            <item name=\"account_id\" type=\"L\" desc=\"米币账户\" /> \n\
            <item name=\"dest_account_id\" type=\"L\" desc=\"目标米币账户\" /> \n\
            <item name=\"transaction_time\" type=\"L\" desc=\"事务时间\" /> \n\
            <item name=\"channel_id\" type=\"S\" desc=\"渠道ID\" /> \n\
            <item name=\"pay_gate_trans_id\" type=\"L\" desc=\"充值事务ID/消费事务ID\" /> \n\
            <item name=\"product_id\" type=\"L\" desc=\"产品ID\" /> \n\
            <item name=\"product_count\" type=\"S\" desc=\"产品数量\" /> \n\
            <item name=\"mb_num\" type=\"L\" desc=\"此次事务关联的米币数(100倍)(消费时为负数)\" /> \n\
        </ack> \n\
    </proto> \n\
    <result_info> \n\
        <item name=\"ok\" value=\"0\" desc=\"操作成功\" /> \n\
        <item name=\"false\" value=\"1\" desc=\"数据已经处于所要求的状态\" /> \n\
        <item name=\"unknown\" value=\"2\" desc=\"未知错误\" /> \n\
        <item name=\"invalid_param\" value=\"3\" desc=\"数据包参数错误\" /> \n\
        <item name=\"operation_failed\" value=\"4\" desc=\"操作失败\" /> \n\
        <item name=\"account_already_exists\" value=\"101\" desc=\"米币账户已经存在，不能创建\" /> \n\
        <item name=\"account_not_exists\" value=\"102\" desc=\"米币账户不存在\" /> \n\
        <item name=\"invalid_pwd\" value=\"103\" desc=\"密码错误\" /> \n\
        <item name=\"account_not_activated\" value=\"104\" desc=\"米币账户还未激活\" /> \n\
        <item name=\"not_enough_mb\" value=\"105\" desc=\"米币账户余额不足\" /> \n\
        <item name=\"invalid_product_count\" value=\"106\" desc=\"购买产品数量错误\" /> \n\
        <item name=\"beyond_month_consume_limit\" value=\"107\" desc=\"超过每月消费上限\" /> \n\
        <item name=\"beyond_per_consume_limit\" value=\"108\" desc=\"超过单笔消费上限\" /> \n\
        <item name=\"not_allow_transfer\" value=\"109\" desc=\"账户不允许转帐\" /> \n\
        <item name=\"invalid_seq\" value=\"110\" desc=\"数据包序列号错误\" /> \n\
        <item name=\"invalid_channel\" value=\"111\" desc=\"渠道不存在或接口验证错误\" /> \n\
        <item name=\"trans_already_dealed\" value=\"112\" desc=\"充值或消费事务已经处理（重复包）\" /> \n\
        <item name=\"account_locked\" value=\"113\" desc=\"米币账户已经被锁定\" /> \n\
        <item name=\"invalid_trans_id\" value=\"114\" desc=\"事务ID错误或不存在\" /> \n\
        <item name=\"trans_smaller_than_init\" value=\"115\" desc=\"事务ID比初始事务ID还要小\" /> \n\
    </result_info> \n\
    </service>\n";
}
