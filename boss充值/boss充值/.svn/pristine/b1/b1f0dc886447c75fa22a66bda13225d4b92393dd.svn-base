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
$appid = 100021950;
$appkey = "pxwKvFOwXGCLwRLWloARcsNMn0ay71Lv";
$paykey = "ZeAIm0XNE88QlYdgRw904PLbCxQVX4aj";

// =======================================  下面是回调接口代码逻辑  =====================================================

/**
 * 签名校验
 */
$sign = $_POST['sign'];
$mysign = _gen_safe_sign($_POST, $appkey);

if($sign != $mysign) {
	die("fail");
	/*
	$arr = array('ret'=>6, 'msg'=>'App签名错误');
	echo json_encode($arr);
	exit;
	*/
}

/**
 * 如果在dev.xyzs.com申请了paykey，需要验证sig签名
 * 如果没有paykey，不会收到sig参数
 * 建议开发商申请paykey，验证sig签名参数
 */
$sig = $_POST['sig'];
$mysig = _gen_safe_sign($_POST, $paykey);
if($sig != $mysig) {
	die("fail");
	/*
	$arr = array('ret'=>6, 'msg'=>'支付签名错误');
	echo json_encode($arr);
	exit;
	*/
}

//  =========================================  下面处理发货逻辑
	    $notify_data = $_POST;	
		//extra字段由 [游戏id,预交易id]组成
		$self_info = explode(",", $notify_data['extra']);
		$json['order_id'] = $notify_data['order_id'];
		$json['preorder_id'] = $self_info[1];
		$json['product_count'] = 0;//none
		$jaon['costs'] = $notify_data['amount'];
		$json['pay_status'] = 0;
		$json['pay_time'] = $notify_data['ts'];///none
		$json['channel_user_id'] = $notify_data['uid'];//none
		$json['pay_channel_id'] = 143;//none
		$json['game_user_id'] = $self_info[0];
		$json['product_name'] = "none";//none
		$json['product_id'] = 0;//none
		$json['private_data'] = 0;//none
		$json['channel_id'] = 6;//none
		$my_json = json_encode($json); 
		
		error_log(date("Y-m-d h:i:s")."send ".serialize($my_json)."\r\n",3,'xy_debug.log');
		const XY_NOTIFY_CMD_ID = 0xA050;
		$user_id = $$self_info[0];
		$req_body = pack('Sa'.(strlen($my_json)), strlen($my_json) , $my_json);
        $req_header = pack('LLSLL', 18 + strlen($req_body), 0, XY_NOTIFY_CMD_ID, 0, $user_id);
        $send_buf = $req_header . $req_body;

		//模拟http请求
		include_once 'socket_handler.class.php';
		$sock = new socket_handler();
		$ret = $sock->connect("192.168.2.207", 12001);
		if(false == $ret)
		{
			error_log(date("Y-m-d h:i:s")."connect failed"."\r\n",3,'xy_debug.log');
			die("fail");
		}
		$ret = $sock->send($send_buf);
        if (false === $ret)
		{
			error_log(date("Y-m-d h:i:s")."send failed"."\r\n",3,'xy_debug.log');
			die("fail");
        }

        $ret = $sock->receive();
		$ack_header = substr($ret, 0, 18);
        $ack_body   = substr($ret, 18);
        $res_header = unpack('Lpkg_len/Lseq_num/Scmd_id/Lstatus_code/Luser_id', $ack_header);
        $res_body = array();
			
		if($res_header['status_code'] != 0 )
		{
			die("fail");
		}
		if ($res_header['pkg_len'] > 18)
		{
			$res_body = unpack('Lverify_result/Ctrans_id', $ack_body);
			error_log(date("Y-m-d h:i:s")."recv ".serialize($res_header).serialize($res_body)."\r\n",3,'xy_debug.log');
				
		}
		$sock->close();
		die("success"); //业务处理完成返回success



//  ==================================  上面处理发货逻辑 ========================================


/**
 * 支持两种返回格式 success/fail 和 json格式{"ret":0,"msg":"ok"}
 */
if($callback_result === true) {
	die("success");
	/*
	$arr = array('ret'=>0, 'msg'=>'发货成功');
	echo json_encode($arr);
	exit;
	*/
}else {
	die("fail");
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
function _gen_safe_sign($params, $prikey)
{
	ksort($params);
	$query_string = array();
	foreach ($params as $key => $val)
	{
		if($key == "sig" || $key == "sign") {
			continue;
		}
		array_push($query_string, $key . '=' . $val);
	}
	$query_string = join('&', $query_string);
	$sign = md5($prikey . $query_string);
	return $sign;
}
?>

