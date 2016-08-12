<?
require_once("Log.class.php");

log_init("./log/", 'pr', Log::TYPE_TRACE, 30 * 1024 * 1024);

if (isset($_REQUEST['openid']) && strlen($_REQUEST['openid']) > 0) {
    // 从memcached中获取uid
    $mem = new Memcached();
    $mem->addServer('127.0.0.1', 11211);
    $memcached_uid = $mem->get($_REQUEST['openid'].'/uslwh');
    log_trace('get openid '.$_REQUEST['openid'].' -> uid '.$memcached_uid);
}

?>

<!DOCTYPE html>
<html><head>
<meta charset="utf-8">
<meta content="target- densitydpi =device-dpi, width=device-width, initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=no" name="viewport">
<meta content="yes" name="apple-mobile-web-app-capable">
<meta content="blank" name="apple-mobile-web-app-status-bar-style">
<meta content="telephone=no" name="format-detection">
<title>百乐森林舞会充值</title>
<link href="wx.common.css" rel="stylesheet">
<link href="wx.pay.1.4.css" rel="stylesheet">
</head>
<body>
<div style="display:none;" class="TB_overlayBG" id="TB_overlay"></div>
<p style="display:none;font-size:18px;position:absolute;top:20%;width:100%;text-align:center;color:#fff;z-index:9999" id="loading_p">
<img style="margin-right:10px;" src="http://png.gc73.com.cn/website_img/opay/loading1.gif">支付请求中，请稍后...</p>
<p style="display:none" class="hint_01" id="hint"></p>
<div class="pay">
  <section>
  	 <h3 class="caution">
      	<!-- <p class="tip">这里购买"金币"或"钻石"，额外再返利10%</p> -->
      	<p class="tip">请正确输入自己的百乐号，切勿输入账号！<br />百乐号不是账号！！！！！</p>
     </h3>
    <article>
      <p class="nickname">百乐号：
        <input type="text" value="<? if ($memcached_uid != false) echo $memcached_uid; ?>" onblur="javascript:BydrPay.trimInputField(this);" onkeyup="javascript:BydrPay.trimInputField(this);" onkeydown="javascript:BydrPay.trimInputField(this);" id="inputName">
      </p>
      <p>
   	   	<input type="hidden" value="0" id="infullAmount">
   	   	<input type="hidden" value="12" id="platform">
   	   	<input type="hidden" value="49" id="infullType">
   	   	<input type="hidden" id="propType">
   	   	<input type="hidden" id="propId">
   	   	<input type="hidden" id="propName">
   	   	<input type="hidden" id="propDesc">
        <input type="hidden" id="openid" value="<? echo $_REQUEST['openid'];  ?>">
   	   </p>	
   	   
      <div class="pay_list">
        <a class="pr btn_pay" title="点我选择货币和购买金额" href="javascript:BydrPay.showGoods();" id="init_click_btn">点我选择货币和购买金额<span class="pa"></span></a> 
        <a style="display:none" class="pr btn_pay pay_selected" title="点我选择货币和购买金额" href="javascript:BydrPay.showGoods();" id="after_click_btn">
        	<em id="amount_title"></em><em id="prop_name_title" class="fr"></em><span class="pa"></span>
        </a>
        <div id="goods_list" style="display: none;" class="list">
          <ul id="tab_2">
            <!-- <li onclick="javascript:BydrPay.select(2, 0, '金币礼包', '金币礼包', 6);">   <span class="fl">6元</span><em class="fr">金币礼包</em></li> -->
            <li onclick="javascript:BydrPay.select(2, 0, '10万金币包', '10万金币包', 10);">  <span class="fl">10元</span><em class="fr">10万金币包</em></li>
            <li onclick="javascript:BydrPay.select(2, 0, '30万金币包', '30万金币包', 30);">  <span class="fl">30元</span><em class="fr">30万金币包</em></li>
            <li onclick="javascript:BydrPay.select(2, 0, '50万金币包', '50万金币包', 50);">  <span class="fl">50元</span><em class="fr">50万金币包</em></li>
            <li onclick="javascript:BydrPay.select(2, 0, '100万金币包', '100万金币包', 100);"> <span class="fl">100元</span><em class="fr">100万金币包</em></li>
            <li onclick="javascript:BydrPay.select(2, 0, '200万金币包', '200万金币包', 200);"> <span class="fl">200元</span><em class="fr">200万金币包</em></li>
            <li onclick="javascript:BydrPay.select(2, 0, '600万金币包', '600万金币包', 600);"> <span class="fl">600元</span><em class="fr">600万金币包</em></li>
          </ul>
        </div>
      </div>
      <a class="btn_recharge" title="微信支付" href="javascript:pay.wechatPrePay();">微信支付</a>
      
      <p class="method mt"></p>
      <h3 class="caution"><center>
      <font size="3" color="red"><br />微信充值金币，额外再送20%！</font>
      </center>
      </h3> 
      <p class="method ht">
      </p>

      <h3 class="caution">
      	<i>注1：</i>
      	<span>请确保您填写的百乐号<strong>正确无误</strong>，点击您的头像打开个人资料界面中，在您的头像下方可以看到自己的百乐号！</span>
      </h3>
     </article>
  </section>
</div>
<script type="text/javascript">
	!function(){
		var cw=document.documentElement.clientWidth||document.body.clientWidth;
		cw=cw>720?720:cw;
		var zoom=cw/360;
		document.write('<style id="htmlzoom">html{font-size:'+(zoom*50)+'px;}</style>');
		window.addEventListener('resize',function(){
			cw=document.documentElement.clientWidth||document.body.clientWidth;
			if(cw>720)cw=720;
			zoom=cw/360;
			document.getElementById('htmlzoom').innerHTML='html{font-size:'+(zoom*50)+'px;}';
		});
	}();
</script>    
<script src="base/jquery.js" type="text/javascript"></script>
<script src="common/htmlReqHtml.js" type="text/javascript"></script>
<script src="common/util.js" type="text/javascript"></script>
<script src="common/pay-1.4.js" type="text/javascript"></script>
<script src="wechat/bydr.pay-2.2.js" type="text/javascript"></script>
<script src="http://res.wx.qq.com/open/js/jweixin-1.0.0.js" type="text/javascript"></script>
</body></html>
