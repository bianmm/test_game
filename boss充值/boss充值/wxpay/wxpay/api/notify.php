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
		define('IP', '192.168.1.81');
		define('PORT', 12001);
		
		$json['order_id'] = $data['transaction_id'];
		$json['preorder_id'] = $data['out_trade_no'];
		$json['product_count'] = 0;//$notify_data['product_count'];//none
		$json['pay_costs'] = $data['total_fee'];
		$json['pay_status'] = "1";
		$json['pay_time'] = time();//$notify_data['pay_time'];//none
		$json['channel_user_id'] = "0";//$notify_data['user_id'];//none
		$json['pay_channel_id'] = "74";//none
		$json['game_user_id'] = $data['attach'];
		$json['product_name'] = "none";//$notify_data['product_name'];//none
		$json['product_id'] = "0";//$notify_data['product_id'];//none
		$json['private_data'] = 0;//$notify_data['private_data'];//none
		$json['channel_id'] = "1001";//none
		$my_json = json_encode($json); 

		$sys = new anypay_iap_sys(IP, PORT);
		$rst = $sys->receipt_verify($my_json, $data['attach']);
		if ($rst == false) {
			log_trace("anypay err:" . var_export($rst, true));
			return false;
		}

		if ($rst['status_code'] != 0) {
			log_trace("anypay fail:" . var_export($rst, true));
			return false;
		}
		
		log_trace("anypay succ:" . var_export($rst, true));
		
		return true;
	}
}

log_trace("begin notify");
$notify = new PayNotifyCallBack();
$notify->Handle(false);

?>
