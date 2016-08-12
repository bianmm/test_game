$(document).ready(function(){
//	var userAgent = navigator.userAgent;
//	var index = userAgent.search("MicroMessenger");
//	if (index == -1) {
//		window.location = "http://opay.pook.com/wechat/error.html?errMsg=请使用微信浏览器充值";
//	} else {
//		var wechatVersion = userAgent.substring(index + 15, index + 16);
//		if (wechatVersion < 5) {
//			window.location = "http://opay.pook.com/wechat/error.html?errMsg=您当前的微信版本不支持充值，请升级微信";
//		}
//	}
	var openid = htmlRequest.QueryString("openId") == null ? "" : htmlRequest.QueryString("openId");
	$('#openid').val(openid);
	
	//var nickName = htmlRequest.QueryString("nickName")
	//nickName = nickName == null ? "" : nickName;
	//var nickName = decodeURIComponent(nickName);
	//$('#inputName').val(nickName);	
});

BydrPay = function(me){
	return me = {
		select : function(propType, propId, propName, propDesc, amount) {
			// 设置金额和提示
			$('#infullAmount').val(amount);
			$('#amount_title').html(amount + "元");
		
			// 设置道具信息
			$('#propType').val(propType);
			$('#propId').val(propId);
			$('#propName').val(propName);
			$('#propDesc').val(propDesc);
			$('#prop_name_title').html(propName);
			
			// 点击框内容切换
			$('#init_click_btn').hide();
			$('#after_click_btn').show();
			
			// 隐藏商品选择框
			BydrPay.hideGoods();
		},
		
		// 显示商品和价格
		showGoods : function() {
			$('#goods_list').toggle();
		},
		
		hideGoods : function() {
			$('#goods_list').hide();
		},
		
		tabChange : function(type) {
			$("#goods_list ul").each(function(){
				$(this).hide();
			});
			
			$(".tab_pay a").each(function(){
				$(this).removeClass("on");
			});
			
			$('#tab_' + type).show();
			$('#title_' + type).addClass("on");
		},
		
		trimInputField : function(target) {
			var value = $(target).val();
			$(target).val(value.trim());
		},
		
		notOpen : function() {
			alert("即将开放，敬请期待");
		},
		jumpCard : function() {
			// 充值帐号
			var inputName = $('#inputName').val().trim();
			if (!inputName) {
				alert('请输入充值帐号信息');
				return;
			}
			var openid = $('#openid').val().trim();
			if (!openid) {
				alert('页面错误联系开发人员');
				return;
			}
			window.location = "../bydr/carrier_card.html?" + "nickName=" + inputName + "&" + "openId=" + openid;
		}
	};
}();