<?php
/**
 * 支付充值回调接口demo(请保证支付回调接口稳定)
 * 参数为所有post参数
 * 生成签名时要过滤掉 sig 和 sign 两个参数，使用剩余的参数生成签名
 * 生成签名的时候不要写死参数列表，最好是从post参数中动态处理，否则post参数里增加或减少参数，会影响签名结果
 * 如果app没有生成paykey，则不会收到sig签名参数
 * 
 */

/**
 * 以下参数可配置在app的配置文件里
 */
$app_secret = "mPYgxbepKr6yPNerFUvGZtNWGsdfQTTp";

// =======================================  下面是回调接口代码逻辑  =====================================================

/**
 * 签名校验
 */
require_once("Log.class.php");
log_init("./log/", 'pr', Log::TYPE_TRACE, 30 * 1024 * 1024);
$head = getallheaders();
log_trace(var_export($head, true));
$body = $_GET;
log_trace(var_export($body, true));


$sign = $body['sign'];
error_log(date("Y-m-d h:i:s")."notify ". print_r($body, true). "\r\n",3,'debug.log');
$mysign = _gen_safe_sign($body, $app_secret);

if($sign != $mysign) {
	error_log(date("Y-m-d h:i:s")."verify failed ". print_r($sign, true). "mysgin:".print_r($mysign, true)."\r\n",3,'debug.log');
	die("FAILURE");
}

//$content = "{\"tradeId\":\"201508191056024948957\",\"orderId\":\"164\",\"orderStatus\":\"S\",\"tradeTtime\":\"20150819105625\",\"amount\":\"0.01\",\"attachInfo\":\"0\"}";
//$notify_data = json_decode($content, true);
//  ========================================  下面处理发货逻辑
	    $notify_data = $body;
		if($notify_data['orderStatus'] == 'failed')
		{
			die("FAILURE");
		}
		//自定义信息格式  preorder_id,user_id
		$self_info = explode(',', $notify_data['cpdefinepart']);
		$json['order_id'] = $notify_data['orderid'];
		$json['preorder_id'] = $self_info[0];
		$json['product_count'] = "0";//none
		$jaon['costs'] = $notify_data['amount'];
		$json['pay_status'] = "1";
		$json['pay_time'] = time();
		$json['channel_user_id'] = "0";//none
		$json['pay_channel_id'] = "1005";//none
		$json['game_user_id'] = "1234";
		$json['product_name'] = "none";//none
		$json['product_id'] = "0";//none
		$json['private_data'] = 0;//none
		$json['channel_id'] = "7";//none
		$my_json = json_encode($json); 
		
		error_log(date("Y-m-d h:i:s")."send ".serialize($my_json)."\r\n",3,'debug.log');
		const PP_SINGE_NOTIFY_CMD_ID = 0xA051;
		$user_id = $self_info[0];
		$req_body = pack('Sa'.(strlen($my_json)), strlen($my_json) , $my_json);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0, PP_SINGE_NOTIFY_CMD_ID, 0, $user_id);
        $send_buf = $req_header . $req_body;

		//模拟http请求
		include_once 'socket_handler.class.php';
		$sock = new socket_handler();
		$ret = $sock->connect("192.168.2.207", 12001);
		if(false == $ret)
		{
			error_log(date("Y-m-d h:i:s")."connect FAILUREed"."\r\n",3,'debug.log');
			die("FAILURE");
		}
		$ret = $sock->send($send_buf);
        if (false === $ret)
		{
			error_log(date("Y-m-d h:i:s")."send FAILUREed"."\r\n",3,'debug.log');
			die("FAILURE");
        }

        $ret = $sock->receive();
		$ack_header = substr($ret, 0, 18);
        $ack_body   = substr($ret, 18);
        $res_header = unpack('Lpkg_len/Lseq_num/Scmd_id/Lstatus_code/Luser_id', $ack_header);
        $res_body = array();
			
		if($res_header['status_code'] != 0 )
		{
			error_log(date("Y-m-d h:i:s")."recv status_code".$res_header['status_code']."\r\n",3,'debug.log');
			die("FAILURE");
		}
		if ($res_header['pkg_len'] > 18)
		{
			$res_body = unpack('Lverify_result/Ctrans_id', $ack_body);
			error_log(date("Y-m-d h:i:s")."recv ".serialize($res_header).serialize($res_body)."\r\n",3,'debug.log');
		}
		$sock->close();
		die("success"); //业务处理完成返回success



//  ==================================  上面处理发货逻辑 ========================================


/**
 * 支持两种返回格式 success/FAILURE 和 json格式{"ret":0,"msg":"ok"}
 */
if($callback_result === true) {
	die("success");
	/*
	$arr = array('ret'=>0, 'msg'=>'发货成功');
	echo json_encode($arr);
	exit;
	*/
}else {
	die("FAILURE");
	/*
	$arr = array('ret'=>8, 'msg'=>'发货失败');
	echo json_encode($arr);
	exit;
	*/
}

// =======================================  上面是回调接口代码逻辑  ========================================================


/**
 * 生成签名方法
 * @param	array	$_POST	所有post参数，方法内部会过滤 sig和sign 两个参数
 * @param	string	prikey	生成签名用到的key，sign签名使用appkey，sig签名使用paykey
 */
function _gen_safe_sign($params, $app_secret)
{
	$query_string = "";
	foreach ($params as $key => $val)
	{
		if($key == "appid" || $key == "orderid" || $key == "amount" || $key == "unit" ||
				$key == "status" || $key == "paychannel") 
		{
			$query_string .=  $val;
		}
		else
		{
			continue;
		}
	}
	$query_string .=  $app_secret;
	error_log(date("Y-m-d h:i:s")."md5 before ".$query_string . "\r\n",3,'debug.log');
	$sign = md5($query_string);
	error_log(date("Y-m-d h:i:s")."md5 after ".$sign ."\r\n", 3, 'debug.log');
	return $sign;
}
?>

