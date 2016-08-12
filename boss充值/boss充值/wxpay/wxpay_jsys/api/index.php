<?php
header('Access-Control-Allow-Origin: *');
header('Content-type: text/plain');

require_once "WxPay.Api.php";
require_once "WxPay.Data.php";

require_once("Log.class.php");

log_init("./log/", 'pr', Log::TYPE_TRACE, 30 * 1024 * 1024);

/*
[20150915 14:30:10]array (
  'inputName' => 'UID II',
  'infullAmount' => '12',
  'platform' => '12',
  'infullType' => '49',
  'propType' => '2',
  'propId' => '0',
  'propName' => '12万金币包',
  'propDesc' => '12万金币包',
  'openid' => '',
)
*/

//var_export($_REQUEST);
log_trace(var_export($_REQUEST, true));

log_trace(var_export($_SERVER['HTTP_USER_AGENT'], true));
if (! strstr($_SERVER['HTTP_USER_AGENT'], 'MicroMessenger')) {
    log_trace('not weixin messenger');
	die();
}

$return_data = array();

$pdt_id = '';
// 金币
if ($_REQUEST['infullAmount'] == 10) {
    $pdt_id = '2';
}
else if ($_REQUEST['infullAmount'] == 30) {
    $pdt_id = '3';
}
else if ($_REQUEST['infullAmount'] == 50) {
    $pdt_id = '4';
}
else if ($_REQUEST['infullAmount'] == 100) {
    $pdt_id = '5';
}
else if ($_REQUEST['infullAmount'] == 200) {
    $pdt_id = '6';
}
else if ($_REQUEST['infullAmount'] == 600) {
    $pdt_id = '7';
}
else {
    log_trace("invalid pdt: ". $_REQUEST['infullAmount']);
    $return_data['ret'] = 'E';
    $return_data['msg'] = '无效的商品ID';
    echo json_encode($return_data);
    die();
}

// 1. 查询账号是否存在

if ($_REQUEST['inputName'] <= 50000) {
    $return_data['ret'] = 'E';
    $return_data['msg'] = '百乐号不正确，请检查后再输入！注意百乐号不是账号！！';
    echo json_encode($return_data);
    log_trace('invalid blh: '.$_REQUEST['inputName']);
    die();
}

$rst = json_decode(file_get_contents('http://192.168.1.31:8080/ipay/dowx?cmd=check_blh&blh='.$_REQUEST['inputName']));
if ($rst->ret != 1) {
    // 百乐号不存在
    $return_data['ret'] = 'E';
    $return_data['msg'] = '百乐号不存在，请检查后再输入！注意百乐号不是账号！！';
    echo json_encode($return_data);
    log_trace('check blh fail:'.var_export($rst, true).', ret: '.json_encode($return_data));
    die();
}

// 将openid于ID号绑定
$mem = new Memcached();
$mem->addServer('127.0.0.1', 11211);
$mem->set($_REQUEST['openid'].'/ujsys', $_REQUEST['inputName']);
log_trace('set openid '.$_REQUEST['openid'].' -> uid '.$_REQUEST['inputName']);

// 2. 获得预交易订单号
$rst = json_decode(file_get_contents('http://192.168.1.31:8080/ipay/dowx?cmd=wxpay_preorder&pass=afsf342asf&blh='.$_REQUEST['inputName'].'&pdt_id='.$pdt_id.'&amt='.($_REQUEST['infullAmount'] * 100)));
//log_trace(file_get_contents('http://192.168.1.21:8081/ipay/dowx?cmd=wxpay_preorder&pass=q35dgqtwas&blh='.$_REQUEST['inputName'].'&pdt_id='.$pdt_id.'&amt='.($_REQUEST['infullAmount'] * 100)));
if ($rst->ret != 1) {
	$return_data['ret'] = 'E';
	$return_data['msg'] = '预交易失败';
	echo json_encode($return_data);
    log_trace('preordre fail'.var_export($rst, true).', ret: '.json_encode($return_data));
	die();
}

// 3. 微信统一下单
$subject = $_REQUEST['propDesc'];
$out_trade_no = $rst->preorder_id;

$unifiedOrder = new WxPayUnifiedOrder();
$unifiedOrder->SetBody($subject);
$unifiedOrder->SetOut_trade_no($out_trade_no);
$unifiedOrder->SetTotal_fee($_REQUEST['infullAmount'] * 100);
$unifiedOrder->SetTrade_type("JSAPI");
$unifiedOrder->SetOpenid($_REQUEST['openid']);
$unifiedOrder->SetAttach($_REQUEST['inputName']);

$rst = '';
try {
    $rst = WxPayApi::unifiedOrder($unifiedOrder);
} catch (Exception $e) {
    echo 'Caught exception: ',  $e->getMessage(), "\n";
}

//if (is_array($rst)) {
//    echo json_encode($rst);
//}

/*
{
    "appid": "wx73dd4ef592acbd97", 
    "noncestr": "M1URCNUaW1urOE0S", 
    "package": "Sign=WXPay", 
    "partnerid": "1251586901", 
    "prepayid": "wx201509151504184863528fa00761932725", 
    "timestamp": 1442300554, 
    "sign": "B183B79DFA53A6C536FE8DDE2886CAE6"
}
*/
//var_export($rst);
log_trace(var_export($rst, true));
// 4. 返回数据
$return_data['ret'] = 'S';
$return_data['formInfo']['appId'] = $rst['appId'];
$return_data['formInfo']['timeStamp'] = (string)$rst['timeStamp'];
$return_data['formInfo']['nonceStr'] = $rst['nonceStr'];
$return_data['formInfo']['package'] = $rst['package'];
$return_data['formInfo']['signType'] = 'MD5';
$return_data['formInfo']['paySign'] = $rst['sign'];

log_trace(var_export($return_data, true));

echo json_encode($return_data);
/*
{
code: "30020"
msg: "请使用微信浏览器充值"
ret: "E"/"S"
formInfo: {
"appId" ： "wx2421b1c4370ec43b",     //公众号名称，由商户传入     
           "timeStamp"：" 1395712654",         //时间戳，自1970年以来的秒数     
           "nonceStr" ： "e61463f8efa94090b1f366cccfbbb444", //随机串     
           "package" ： "prepay_id=u802345jgfjsdfgsdg888",     
           "signType" ： "MD5",         //微信签名方式：     
           "paySign" ： "70EA570631E4BB79628FBCA90534C63FF7FADD89" //微信签名 
}
}
*/

?>
