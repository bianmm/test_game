<?php
header('Access-Control-Allow-Origin: *');
header('Content-type: text/plain');


/*
'<xml><appid><![CDATA[wxe0304753ff2eff1d]]></appid>
<bank_type><![CDATA[CFT]]></bank_type>
<cash_fee><![CDATA[1000]]></cash_fee>
<fee_type><![CDATA[CNY]]></fee_type>
<is_subscribe><![CDATA[Y]]></is_subscribe>
<mch_id><![CDATA[1259853001]]></mch_id>
<nonce_str><![CDATA[bf9b44b0xm50a5ygdxut1dzi9eg56g5w]]></nonce_str>
<openid><![CDATA[oG11Tw944OCqzTIvpTn32eG2COLw]]></openid>
<out_trade_no><![CDATA[1010599]]></out_trade_no>
<result_code><![CDATA[SUCCESS]]></result_code>
<return_code><![CDATA[SUCCESS]]></return_code>
<sign><![CDATA[9522F188AE6376965521D491D138E09E]]></sign>
<time_end><![CDATA[20150915190135]]></time_end>
<total_fee>1000</total_fee>
<trade_type><![CDATA[JSAPI]]></trade_type>
<transaction_id><![CDATA[1009260547201509150889584827]]></transaction_id>
</xml>'

*/

ini_set('date.timezone','Asia/Shanghai');
error_reporting(E_ERROR);

require_once "WxPay.Api.php";
require_once 'WxPay.Notify.php';

require_once 'anypay_iap_sys.class.php';

//初始化日志
require_once("Log.class.php");
log_init("./log/", 'pr', Log::TYPE_TRACE, 30 * 1024 * 1024);

log_trace(var_export($_SERVER['HTTP_USER_AGENT'], true));

class PayNotifyCallBack extends WxPayNotify
{
	//查询订单
	public function Queryorder($transaction_id)
	{
		$input = new WxPayOrderQuery();
		$input->SetTransaction_id($transaction_id);
		$result = WxPayApi::orderQuery($input);
		log_trace("query:" . json_encode($result));
		if(array_key_exists("return_code", $result)
			&& array_key_exists("result_code", $result)
			&& $result["return_code"] == "SUCCESS"
			&& $result["result_code"] == "SUCCESS")
		{
			return true;
		}
		return false;
	}
	
	//重写回调处理函数
	public function NotifyProcess($data, &$msg)
	{
		log_trace("call back:" . json_encode($data));
		$notfiyOutput = array();
		
		if(!array_key_exists("transaction_id", $data)){
			$msg = "输入参数不正确";
			return false;
		}
		//查询订单，判断订单真实性
		if(!$this->Queryorder($data["transaction_id"])){
			$msg = "订单查询失败";
			return false;
		}
		
		// 发货
        $rst = json_decode(file_get_contents('http://192.168.1.21:8080/ipay/dowx?cmd=wxpay_confirm&pass=34adgdasdfsd&blh='.$data['attach'].'&preorder_id='.$data['out_trade_no'].'&exorder_id='.$data['transaction_id'].'&costs='.$data['total_fee']));
		if ($rst.ret != 1) {
			log_trace("wxpay err:" . var_export($rst, true));
			return false;
		}

		log_trace("wxpay succ:" . var_export($rst, true));
		
		return true;
	}
}

log_trace("begin notify");
$notify = new PayNotifyCallBack();
$notify->Handle(false);

?>
