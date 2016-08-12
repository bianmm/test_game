<html> 
<?php   

require_once("Log.class.php");

log_init("./log/", 'pr', Log::TYPE_TRACE, 30 * 1024 * 1024);

log_trace(var_export($_REQUEST, true));

// 先从memcached中读取

$mem = new Memcached();
$mem->addServer('127.0.0.1', 11211);

//log_trace(var_export($mem->get($_REQUEST['code']), true));
$openid = $mem->get($_REQUEST['code']);
if ($openid == false) {
    // 从网页获取
    $rst = file_get_contents("https://api.weixin.qq.com/sns/oauth2/access_token?appid=wxe0304753ff2eff1d&secret=d5d83c14067d6445a54fb61e50c8d248&code={$_REQUEST['code']}&grant_type=authorization_code");
    log_trace(var_export($rst, true));
    $obj = json_decode($rst);
    log_trace(var_export($obj, true));
    if ($obj) {
        $openid = $obj->openid;
        $mem->set($_REQUEST['code'], $openid);
        log_trace("set mem: ".$_REQUEST['code']." -> ".$openid);
    }
}


if (strlen($openid) > 0) {
    $url = 'http://wxyoo.com/wxpay/wxpay.php?openid='.$openid;
    echo "<script language=\"javascript\">"; 
    echo "location.href=\"$url\""; 
    echo "</script>"; 
}
else {
    echo "no openid: ".var_export($obj, true).", req: ".var_export($_REQUEST, true);
    log_trace("no openid: https://api.weixin.qq.com/sns/oauth2/access_token?appid=wxe0304753ff2eff1d&secret=d5d83c14067d6445a54fb61e50c8d248&code={$_REQUEST['code']}&grant_type=authorization_code");
}

// 通过code获取openid
//$code = $_REQUEST['code'];

//重定向浏览器
//header("Location: http://wxyoo.com/wxpay/wxpay.php?openid={$obj['openid']}");
//确保重定向后，后续代码不会被执行   
//exit;  
?> 

</html>
