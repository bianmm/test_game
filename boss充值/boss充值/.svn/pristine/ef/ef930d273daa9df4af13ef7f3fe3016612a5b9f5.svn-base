pay = function(me){
	return me = {
		request : function() {
			$.ajax({
				url : '../../msg/payRequest.do',
				async : true,
				type : 'post',
				dataType : 'json',
				data : {
					"userName" : payParams.userName,
					"infullAmount" : payParams.infullAmount,
					"platform" : payParams.platform,
					"infullType" : payParams.infullType,
					"propName" : payParams.propName,
					"propDesc" : payParams.propDesc,
					"sign" : payParams.sign,
					"encryptKey" : payParams.encryptKey,
					"encryptData" : payParams.encryptData
				},
				success : function(data) {
					if (data.ret == 'S') {
						$('#formInfo').html(data.formInfo);
						$('#pookPayFrm').submit();
					} else {
						$('#TB_overlay').hide();
						$('#loading_p').hide();
						alert(data.msg);
					}
				},
				error : function() {
					$('#TB_overlay').hide();
					$('#loading_p').hide();
					alert('很抱歉，支付失败，请稍后再试');
				}
			});
		},
		
		loadNickName : function() {
			var openid = $('#openid').val();
			
			if (openid != null && openid != "" && openid.length > 0) {
				
				$.ajax({
					url : '../user/getNickName.do',
					async : true,
					type : 'post',
					dataType : 'json',
					data : {
						"platform" : 12,
						"openId" : openid
					},
					success : function(data) {
						if (data.ret == 'S') {
							$('#inputName').val(data.nickName);
						}
					}
				});
				
			}			
		},
		
		payRequest : function(infullType) {
			$('#TB_overlay').show();
			$('#loading_p').show();
			// 设置充值类型
			payParams.infullType = infullType;

			// 发起充值请求
			pay.request();
		},
		wechatPay : function(inputName, infullAmount, platform, infullType, 
				propType, propId, propName, propDesc, openid) {
			$.ajax({
				url : './api/index.php',
				async : true,
				type : 'post',
				dataType : 'json',
				data : {
					"inputName" : inputName,
					"infullAmount" : infullAmount,
					"platform" : platform,
					"infullType" : infullType,
					"propType" : propType,
					"propId" : propId,
					"propName" : propName,
					"propDesc" : propDesc,
					"openid" : openid
				},
				success : function(data) {
					if (data.ret == 'S') {
						// 调用JSAPI支付
						WeixinJSBridge.invoke(
							'getBrandWCPayRequest', data.formInfo,
							function(res){
								// 关闭遮罩
								pay.closeLoading();
								
								// 使用以上方式判断前端返回,微信团队郑重提示：res.err_msg将在用户支付成功后返回    ok，但并不保证它绝对可靠。 
								if(res.err_msg == "get_brand_wcpay_request:ok" ) {
									alert("支付成功，请到游戏中查看到账情况");
								} else if (res.err_msg == "get_brand_wcpay_request:cancel") {
									// 用户取消
									alert("您已取消支付");
								} else {
									//alert("很抱歉，支付失败，请稍后再试");
									//alert(res.err_msg);
									alert(JSON.stringify(data));
								}
							}
						);
					} else {
						// 关闭遮罩
						pay.closeLoading();
						
						alert(data.msg);
					}
				},
				error : function() {
					// 关闭遮罩
					pay.closeLoading();
					
					alert('很抱歉，支付失败，请稍后再试');
				}
			});
		},
		
		wechatPrePay : function() {
			// 打开遮罩
			pay.openLoading();
			
			// 充值帐号
			var inputName = $('#inputName').val().trim();
			if (inputName == '') {
				// 关闭遮罩
				pay.closeLoading();
				
				alert('请输入充值帐号信息');
				return;
			}
			// 充值金额
			var infullAmount = $('#infullAmount').val();
			if (infullAmount == '0') {
				// 关闭遮罩
				pay.closeLoading();
				
				alert('请选择购买的道具');
				return;
			}
			
			// 支付
			pay.wechatPay(inputName, infullAmount, $('#platform').val(), $('#infullType').val(), 
					$('#propType').val(), $('#propId').val(), $('#propName').val(), $('#propDesc').val(), $('#openid').val());
		},
		
		bankPrePay : function(infullType, platform) {
			// 打开遮罩
			pay.openLoading();
			
			// 充值帐号
			var inputName = $('#inputName').val().trim();
			if (inputName == '') {
				// 关闭遮罩
				pay.closeLoading();
				
				alert('请输入充值帐号信息');
				return;
			}
			// 充值金额
			var infullAmount = $('#infullAmount').val();
			if (infullAmount == '0') {
				// 关闭遮罩
				pay.closeLoading();
				
				alert('请选择购买的道具');
				return;
			}
			
			// 支付
			pay.bankPay(inputName, infullAmount, platform, infullType, 
					$('#propType').val(), $('#propId').val(), $('#propName').val(), $('#propDesc').val(), $('#openid').val());
		},
		
		aliPrePay : function(infullType) {
			if (util.isWeixin()) {
				pay.showAliHint();
				return;
			}
			var inputName = $('#inputName').val().trim();
			var infullAmount = $('#infullAmount').val();
			if (!pay.loadingAndCheck(inputName, infullAmount)) {
				return;
			}
			
			// 支付
			pay.bankPay(inputName, infullAmount, "18", infullType, 
					$('#propType').val(), $('#propId').val(), $('#propName').val(), $('#propDesc').val(), $('#openid').val());
		},
		
		bankPay : function(inputName, infullAmount, platform, infullType, 
				propType, propId, propName, propDesc, openid) {
			$.ajax({
				url : '../msg/bankPay.do',
				async : true,
				type : 'post',
				dataType : 'json',
				data : {
					"inputName" : inputName,
					"infullAmount" : infullAmount,
					"platform" : platform,
					"infullType" : infullType,
					"propType" : propType,
					"propId" : propId,
					"propName" : propName,
					"propDesc" : propDesc,
					"openid" : openid
				},
				success : function(data) {
					if (data.ret == 'S') {
						// 创建form表单后提交
						var formInfo = data.formInfo;
						if (formInfo != '') {
							$('.pay').append(formInfo);
//							$('#pookPayFrm').attr("target", "_blank");
							$('#pookPayFrm').attr("method", "POST");
							$('#pookPayFrm').submit();							
							$('#pookPayFrm').remove();
						}
					} else {
						// 关闭遮罩
						pay.closeLoading();						
						alert(data.msg);
					}
				},
				error : function() {
					// 关闭遮罩
					pay.closeLoading();					
					alert('很抱歉，支付失败，请稍后再试');
				}
			});
		},
		
		openLoading : function() {
			// 打开遮罩
			$('#TB_overlay').show();
			$('#loading_p').show();
		},
		
		closeLoading : function() {
			// 关闭遮罩
			$('#TB_overlay').hide();
			$('#loading_p').hide();
		},
		
		showAliHint:function(){
			$("#hint").show();
			$("#TB_overlay").show();
			$(document).click(function(){
		 		pay.closeAliHint();
		 	});
		},
		
		closeAliHint:function(){
			$("#TB_overlay").hide();
			$("#hint").hide();
		},
		
		loadingAndCheck:function(inputName, infullAmount){
			// 打开遮罩
			pay.openLoading();
			
			// 充值帐号
			if (inputName == '') {
				// 关闭遮罩
				pay.closeLoading();
				
				alert('请输入充值帐号信息');
				return false;
			}
			// 充值金额
			if (infullAmount == '0') {
				// 关闭遮罩
				pay.closeLoading();
				
				alert('请选择购买的道具');
				return false;
			}
			return true;
		}
	};
}();