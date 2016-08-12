<?
require_once("Log.class.php");

log_init("./log/", 'pr', Log::TYPE_TRACE, 30 * 1024 * 1024);

if (isset($_REQUEST['openid']) && strlen($_REQUEST['openid']) > 0) {
	// 从memcached中获取uid
	$mem = new Memcached();
	$mem->addServer('127.0.0.1', 11211);
	$memcached_uid = $mem->get($_REQUEST['openid'].'/uid');
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
<title>捕鱼无双OL充值</title>
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
      	<p class="tip">进入游戏后，点击左上角头像查看游戏ID号<br />切勿输入账号！！ID号不是账号！！</p>
     </h3>
    <article>
      <p class="nickname">游戏ID号：
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
          <p class="tab_pay">
          	<a class="bor_rig on" title="充金币" id="title_2" href="javascript:BydrPay.tabChange(2);">充金币</a>
          	<a class="bor_rig" title="充钻石" id="title_3" href="javascript:BydrPay.tabChange(3);">充钻石</a>
          	<a title="充贵族礼包" id="title_4" href="javascript:BydrPay.tabChange(4);">充特选礼包</a>
          </p>
          <ul id="tab_2">
            <!-- <li onclick="javascript:BydrPay.select(2, 0, '金币礼包', '金币礼包', 6);">   <span class="fl">6元</span><em class="fr">金币礼包</em></li> -->
            <li onclick="javascript:BydrPay.select(2, 0, '10万金币包', '10万金币包', 10);">  <span class="fl">10元</span><em class="fr">10万金币包</em></li>
            <li onclick="javascript:BydrPay.select(2, 0, '35万金币包', '35万金币包', 30);">  <span class="fl">30元</span><em class="fr">35万金币包</em></li>
            <li onclick="javascript:BydrPay.select(2, 0, '60万金币包', '60万金币包', 50);">  <span class="fl">50元</span><em class="fr">60万金币包</em></li>
            <li onclick="javascript:BydrPay.select(2, 0, '125万金币包', '125万金币包', 100);"> <span class="fl">100元</span><em class="fr">125万金币包</em></li>
            <li onclick="javascript:BydrPay.select(2, 0, '256万金币包', '256万金币包', 200);"> <span class="fl">200元</span><em class="fr">256万金币包</em></li>
            <li onclick="javascript:BydrPay.select(2, 0, '830万金币包', '830万金币包', 600);"> <span class="fl">600元</span><em class="fr">830万金币包</em></li>
          </ul>
          <ul style="display:none" id="tab_3">
            <!-- <li onclick="javascript:BydrPay.select(3, 0, '钻石礼包', '钻石礼包', 6);"><span class="fl">6元</span><em class="fr">钻石礼包</em></li> -->
            <li onclick="javascript:BydrPay.select(3, 0, '100钻石包', '100钻石包', 10);"><span class="fl">10元</span><em class="fr">100钻石包</em></li>
            <li onclick="javascript:BydrPay.select(3, 0, '300钻石包', '300钻石包', 30);"><span class="fl">30元</span><em class="fr">300钻石包</em></li>
            <li onclick="javascript:BydrPay.select(3, 0, '500钻石包', '500钻石包', 50);"><span class="fl">50元</span><em class="fr">500钻石包</em></li>
            <li onclick="javascript:BydrPay.select(3, 0, '1000钻石包', '1000钻石包', 100);"><span class="fl">100元</span><em class="fr">1000钻石包</em></li>
            <li onclick="javascript:BydrPay.select(3, 0, '2000钻石包', '2000钻石包', 200);"><span class="fl">200元</span><em class="fr">2000钻石包</em></li>
            <li onclick="javascript:BydrPay.select(3, 0, '6000钻石包', '6000钻石包', 600);"><span class="fl">600元</span><em class="fr">6000钻石包</em></li>
          </ul>
          <ul style="display:none" id="tab_4">
            <li onclick="javascript:BydrPay.select(4, 0, '特选礼包(寒冰炮)', '特选礼包(寒冰炮)', 30);"><span class="fl">30元</span><em class="fr">特选礼包(寒冰炮)</em></li>
            <li onclick="javascript:BydrPay.select(4, 1, '特选礼包(电磁炮)', '特选礼包(电磁炮)', 30);"><span class="fl">30元</span><em class="fr">特选礼包(电磁炮)</em></li>
          </ul>
        </div>
      </div>
      <a class="btn_recharge" title="微信支付" href="javascript:pay.wechatPrePay();">微信支付</a>
      
      <p class="method mt"></p>
      <h3 class="caution"><center>
      <font size="3" color="red"><br /><b>微信充值金币钻石，额外再送20%！<br />（赠送将与首冲及VIP奖励叠加！）</b></font>
      </center>
      </h3> 
      <p class="method ht">
      </p>

      <h3 class="caution">
      	<i>注1：</i>
      	<span>请确保您填写的游戏ID号<strong>正确无误</strong></span>
      </h3>
      <h3 class="caution">
      	<i>注2：</i>
      	<span>特选礼包里<strong>没有金币</strong>，需要购买金币的玩家不要买特选礼包</span>
      </h3>
      <h3 class="caution">
      	<i>注3：</i>
      	<span>首次充值30元及以上，额外<strong>双倍返利</strong>，双倍返利不可与游戏中重复使用，游戏中获得过双倍返利，则这里充值不再双倍，返利数量以APP中显示为准</span>
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
