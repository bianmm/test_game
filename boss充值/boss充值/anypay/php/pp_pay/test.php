<?php
/*
	pp充值平台：http://pay.25pp.com/
	以下为示例代码
*/
include('Rsa.php');
include('MyRsa.php');

testController::Appreturn();

class testController
{
	const PP_NOTIFY_CMD_ID = 0xA049;
	public function Appreturn()
	{
		$notify_data = $_POST;
		$chkres = self::chk($notify_data);
		error_log(date("Y-m-d h:i:s")."result ".serialize($chkres)."\r\n",3,'rsa.log');
		if($chkres) 
		{
            //验证通过
            //对数据做统一格式处理，发给充值服务器
            $json['order_id'] = $notify_data['order_id'];
            $json['preorder_id'] = $notify_data['billno'];
            $json['product_count'] = 0;//$notify_data['product_count'];//none
            $jaon['costs'] = $notify_data['amount'];
            $json['pay_status'] = $notify_data['status'];
            $json['pay_time'] = time();//$notify_data['pay_time'];//none
            $json['channel_user_id'] = 0;//$notify_data['user_id'];//none
            $json['pay_channel_id'] = 74;//none
            $json['game_user_id'] = $notify_data['roleid'];
            $json['product_name'] = "none";//$notify_data['product_name'];//none
            $json['product_id'] = 0;//$notify_data['product_id'];//none
            $json['private_data'] = 0;//$notify_data['private_data'];//none
            $json['channel_id'] = 9;//none
            $my_json = json_encode($json); 
            
            //error_log(date("Y-m-d h:i:s")."send ".serialize($my_json)."\r\n",3,'rsa.log');
            $user_id = $notify_data['roleid'];
            $req_body = pack('Sa'.(strlen($my_json)), strlen($my_json) , $my_json);
            $req_header = pack('LLSLL', 18 + strlen($req_body), 0,self::PP_NOTIFY_CMD_ID, 0, $user_id);
            $send_buf = $req_header . $req_body;

            //模拟http请求
            include_once 'socket_handler.class.php';
            $sock = new socket_handler();
            $ret = $sock->connect("192.168.2.207", 12001);
            if(false == $ret)
            {
                error_log(date("Y-m-d h:i:s")."connect failed"."\r\n",3,'rsa.log');
                return false;
            }
            $ret = $sock->send($send_buf);
            if (false === $ret)
            {
                $logFile = './anysdk.pay.error.log'; 
                file_put_contents($logFile, "send request to anypay server failed" . "\n", FILE_APPEND);
                echo "fail";
                return false;
            }

            $ret = $sock->receive();
            error_log(date("Y-m-d h:i:s")."anypay return". serial ."\r\n",3,'rsa.log');
            $ack_header = substr($ret, 0, 18);
            $ack_body   = substr($ret, 18);
            $res_header = unpack('Lpkg_len/Lseq_num/Scmd_id/Lstatus_code/Luser_id', $ack_header);
            $res_body = array();
            
            if($res_header['status_code'] != 0)
            {
                echo "fail";
                return false;
            }
            if ($res_header['pkg_len'] > 18)
            {
                $res_body = unpack('Lverify_result/Ctrans_id', $ack_body);
                error_log(date("Y-m-d h:i:s")."recv ".serialize($res_header).serialize($res_body)."\r\n",3,'rsa.log');
            }
            $sock->close();
                          
            echo "success"; //业务处理完成返回success
        }
        else
        {
            echo "fail";
        }
        
}
	
public function chk($notify_data)
{
    $privatedata = $notify_data['sign'];
    error_log(date("Y-m-d h:i:s")." ".serialize($privatedata)."\r\n",3,'rsa.log');
    
    $privatebackdata = base64_decode($privatedata);
    error_log(date("Y-m-d h:i:s")."base64_decode ".serialize($privatebackdata)."\r\n",3,'rsa.log');
    $MyRsa = new MyRsa;
    $data = $MyRsa->publickey_decodeing($privatebackdata, MyRsa::public_key);
    error_log(date("Y-m-d h:i:s")."publickey_decodeing ".$data."\r\n",3,'rsa.log');
    $rs = json_decode($data,true);
    error_log(date("Y-m-d h:i:s")."rs ".serialize($rs)."\r\n",3,'rsa.log');
    if(empty($rs)||empty($notify_data))return false;
    //解密出来的数据和接收到的明文数据对比
    if($rs["billno"] == $notify_data['billno']&&$rs["amount"] == $notify_data['amount']&&$rs["status"] == $notify_data['status']) {
        return true; 
    }else{
        $logFile = './anysdk.pay.error.log';
        file_put_contents($logFile, "billno: ". print_r($rs["billno"], true) . "amount: ". print_r($rs["amount"], true). "status: ". print_r($rs["status"], true) . "\n", FILE_APPEND);
        return false;
    }
}

}
?>
