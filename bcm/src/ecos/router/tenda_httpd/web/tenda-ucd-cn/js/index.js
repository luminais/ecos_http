(function (window, document, $) {
	"use strict";
	
	var G = {};
	
	G.firstInit = true;
	G.ajaxOn = false;
	G.changeView = false;

	function showConnectType(conType) {
		var typeObj = {
				"0": "type_static",
				"1": "type_dhcp",
				"2": "type_pppoe"
			},
			$placeholderElem = $('.placeholder-content'),
			$placeholderTarget = $('input[placeholder]'),
			id,
			i;
		id = typeObj[conType];
		$('.connect-type').addClass('none');
		$('#' + id).removeClass('none');
		$placeholderElem.hide();
		for(i = 0; i< $placeholderElem.length; i++) {
			if($placeholderTarget[i].value === "") {
				$($placeholderElem[i]).show();
			}
		}
	}
	
	function fillDataById(obj) {
		var id,
			elem;

		for (id in G.data) {
			if(G.data.hasOwnProperty(id) && (elem = document.getElementById(id))) {
				if (elem.id === id) {
					if (elem.val) {
						elem.val(G.data[id]);
					} else {
						elem.value = G.data[id];
					}
				}			
			}
		}
	}
	
	function saveDialogStatus(noprompt) {
		$.cookie.set('dialogSameNet', noprompt, '/');
	}
	function wispSameNetWithSuper() {
		var ipnum = G.data['lan-ip'].split("."),
			ipval = parseInt(ipnum[2]),
			m,
			newIp,
			conMassage,
			loc;
		
		m = (parseInt(ipnum[2]) + 1) % 256;
		newIp = ipnum[0] + "." + ipnum[1] + "." + m + "." + ipnum[3];
		
		conMassage = "检测到IP地址冲突，登录 IP 地址将自动修改为" +
				newIp + "，请以" + newIp + "重新登录界面。";
		
		if ($.cookie.get('dialogSameNet') &&
			$.cookie.get('dialogSameNet') === 'true') {
			
			return ;
		}
		if (!G.dialog) {
			G.dialog = $.dialog({
				title: '来自网页的消息',
				showNoprompt: true,
				content: '<p>' + conMassage + '</p>',
				apply: function () {
					if(this.noprompt === "true") {
						saveDialogStatus(this.noprompt);
					}
					G.ajaxMsg = $.ajaxMassage('数据保存中，请稍候...');
					loc = "/goform/AdvSetLanip?GO=index.htm" + "&LANIP=" + newIp + "&LANMASK=255.255.255.0";
					window.location = loc;
				},
				cancel: function () {
					if(this.noprompt === "true") {
						saveDialogStatus(this.noprompt);
					}
				}
			});
		} else {
			G.dialog.open();
		}
	}
	
	function showErrorMassage() {
		var $errCheckElem = $("#errcheck"),
			$errConextElem = $("#errconext"),
			internetStat = parseInt(G.internetStat, 10),
			stateObj = ['<span class="text-error">未联网</span>',
				'<span class="text-tips">联网中</span><img class="img-loading" alt="加载中" src="img/loading.gif">',
				'已联网，<a href="http://ipcamera.tenda.com.cn/default.html" style="text-decoration: underline; color:#468847;">上网试试</a>'],
			$connectInfo = $("#connectInfo"),
			errorMassage = {
                "0": '<span class="info-block text-error">检测到WAN口网线未连接，请检查并连接好您的WAN口网线。</span>',
                "1": '<span class="info-block text-error">正在诊断您输入的宽带用户名和宽带密码是否正确，请稍等，整个过程约1-5分钟。</span>',
                "2": '<span class="info-block text-error">用户名密码验证失败，请确认您的宽带用户名与宽带密码并重新输入。</span>',
                "3": '<span class="info-block text-error">网络运营商远端无响应，请确认不接路由器时是否可以正常上网，如不能，请联系当地网络运营商解决。</span>',
                "4": '<span class="info-block text-error"> 拨号成功，但无法连接至互联网。<br/>请确认不使用路由器时是否可以正常上网，如不能，请联系您的宽带运营商。</span>'
            },
			errorTip = {
                "0": '<span class="info-block text-error">系统检测到您的联网方式可能为ADSL拨号，请填写完整的宽带用户名和密码，<br/>点击“确定”尝试联网。</span>',
                "1": '<span class="info-block text-error">系统检测到您的联网方式可能为静态IP，请手动选择并配置静态IP，尝试联网。</span>',
                "2": '<span class="info-block text-error">系统已获取到IP，但无法连接至互联网，请依次尝试以下操作：<br/>1. <a id="cloneMac" style="text-decoration:underline;"class="text-success" href="#">克隆MAC地址</a>，并等待30秒。' +
                        '<br/>2. 在不使用路由器时可正常上网的电脑上重新配置(电脑与路由器间需用网线连接)。<br/>3. 请确认不使用路由器时是否可以正常上网，如不能，请联系您的宽带运营商。</span>',
		 "3": '<span class="info-block text-error">系统检测到您的联网方式可能为ADSL拨号，请点击“确定”尝试联网。</span>'
            };
		
		if(getValueByName('con-type') !== G.data['con-type']) {
			$connectInfo.hide();
		} else {
			$connectInfo.show();
		}
	
		$errCheckElem.addClass('none');
		
		if (G.conStat == 1) {
			$('#con-stat').html(stateObj[G.conStat]);
		} else {
			$('#con-stat').html(stateObj[internetStat]);
		}
		if(G.wanLink == 0) {
			if (G.wlMode == "0") {//no G.netCheck == 4 &&
				$errCheckElem.removeClass('none');
				$errConextElem.html(errorMassage[G.errorInfo]);
			}
		} else if(G.internetStat != 2 ){//未联网 +
			if(G.conType == 1) {//dhcp  + 
				if(G.netCheck != 4 && G.netCheck != 3){
					$errCheckElem.removeClass('none');
				}
				if(G.netCheck == 2) {
					// (未获得wanip 或者已经获取到wanip) + 存在pppoe服务器
					//此种情况已经不存在！怎么又跑出来了？
					if(G.changeView == false) {//只自动切换一次
						document.getElementById('adsl').click();
						G.changeView = true;
						$connectInfo.show();
						$errCheckElem.removeClass('none');
						if(document.getElementById('adsl-pwd').value == '' || document.getElementById('adsl-user').value == '') {
							$errConextElem.html(errorTip[0]);
							clearTimeout(G.ajaxOn);
						} else {
							$errConextElem.html(errorTip[3]);
						}
					}
				} else if (G.conStat != 2 && G.netCheck != 2) {
					//未获得wanip + bu存在pppoe服务器
					if(G.internetStat == 1) {
						/*$errCheckElem.addClass('none');
					} else {*/
						$errConextElem.html(errorTip[1]);
					}
				} else if(G.conStat == 2 && G.netCheck != 2) {
					//获得wanip + bu存在pppoe服务器
					$errConextElem.html(errorTip[2]);
				}
			} else { //非dhcp
				if(G.conType == 2) {//pppoe
					//测试有宽带用户名和密码完整性
					if(document.getElementById('adsl-pwd').value == '' || document.getElementById('adsl-user').value == '') {
						$errCheckElem.removeClass('none');
						$errConextElem.html(errorTip[0]);
						clearTimeout(G.ajaxOn);
					} else {
						if (G.conStat != 2) {
							if (G.errorInfo > 0 && G.errorInfo < 4) {
								$errCheckElem.removeClass('none');
								$errConextElem.html(errorMassage[G.errorInfo]);
							}
						} else if(G.conStat == 2) {
							$errCheckElem.removeClass('none');
							$errConextElem.html(errorMassage[4]);
						}
					}
				}
			}
		}
			
		if (G.errorInfo == "11") {
			wispSameNetWithSuper();
		}
	}
	
	function initControler() {
		var $connectInfo = $("#connectInfo");
		
		$('#type-select input').on('click', function (){
			showConnectType(this.value);
			
			if(this.value !== G.data['con-type']) {
				$connectInfo.hide();
				if(this.value === '0' ) {
					$('#ipval')[0].val("");
					$('#submask')[0].val("");
					$('#gateway')[0].val("");
					$('#dns1')[0].val("");
					$('#dns2')[0].val("");
					
				}
			} else {
				$connectInfo.show();
			}
		});
		
		G.validate = $.validate({
			custom: function () {
				var wanIp = $('#ipval')[0].val(),
					wanGateway = $('#gateway')[0].val(),
					wanDns1 = $('#dns1')[0].val(),
					wanDns2 = $('#dns2')[0].val(),
					lanIp = G.data['lan-ip'],
					lanMask = '255.255.255.0',
					wanMask = $('#submask')[0].val(),
					i = 0,
					ssid = $('#ssid')[0];
				if($('#static')[0].checked) {
					var wanipArr = wanIp.split('.'),
						  gatewayArr = wanGateway.split('.'),
						  submaskArr = wanMask.split('.');
						
					if (isSameNet(lanIp, wanIp, lanMask, wanMask)) {
						return 'IP 地址不能与登录 IP(' + lanIp + ') 在同一网段！';
					}
					if (wanGateway === lanIp) {
						return '默认网关不能与登录 IP(' + lanIp + ') 相同！';
					}
					
					if (wanMask == '0.0.0.0') {
							return "子网掩码不能为0";
					}
					for (i = 0; i < 4; i++) {
						if((gatewayArr[i] & submaskArr[i]) != (wanipArr[i] & submaskArr[i])){
							return "WAN口IP和网关IP必须在同一个网段！";
						}
					}
					if ((wanIp === wanGateway  || wanIp === wanDns1 ||
							wanIp === wanDns2) && !$.isHidden($('#ipval')[0])) {
						return 'IP 地址不能与默认网关或 DNS 服务器相同！';
					}
				}
				if(!(/^[ -~]+$/g).test(ssid.value) && ssid.value.length > 10) {
					ssid.focus();
					return '当无线信号名称中含有中文字符时，最长只能输入10个字符';
				}
			},
			
			success: function () {
				var formElem = $('#index-form')[0],
					submitData = $.serialize(formElem, 'textboxs'),
					url = "/goform/NetWorkSetupSave?" + Math.random(),
					data = "Go=index.htm&" + submitData,
					wirelessPwd = document.getElementById('ssid-pwd').value,
					wanIp = $('#ipval')[0].val(),
					wanMask = $('#submask')[0].val();
					
				if(getValueByName('con-type') == '0' && !ipMaskMergeOk(wanIp, wanMask)) {
					return;
				}
				if(G.data['transmitter'] === '1' && wirelessPwd !== G.data['ssid-pwd']) {
					if(!confirm("无线更改密码为" + wirelessPwd +", 需用新密码重新连接路由器！确认更改吗？")) {
						return;
					}
				}
				clearTimeout(G.ajaxOn);
				G.ajaxMsg = $.ajaxMassage('数据保存中，请稍候...');
				$('#submit')[0].disabled = true;
				$.post(url, data, function (req) {
					setTimeout(function () {
						getData();
						G.ajaxMsg.text("数据保存成功！");
						setTimeout(function () {
							$('#submit')[0].disabled = '';
							G.ajaxMsg.hide();
						}, 500);
					}, 1500);
				});
			},
			
			error: function (msg) {
				if (msg && typeof msg === 'string') {
					alert(msg);
				}
			}
		});
		
		$('#submit').on('click', function () {
			G.validate.checkAll();
		});
		
		//Cancel
		$('#cancel').on('click',function() {
			window.location.reload();
		});
		
		$('#adsl-pwd').on('keyup', function () {
			if (this.value === '' && this.showText !== 'changed') {
				setTimeout(function() {
					var $adslPsw_ = $('#adsl-pwd_');
					
					if ($adslPsw_[0]) {
						G.validate.addElems($adslPsw_);
						if (!$.isHidden($adslPsw_[0])) {
							$adslPsw_[0].focus();
						}
					}
					
					
				}, 20);
				
				this.showText = 'changed';
			}
		});
	}
	
	function updateStatus() {
		G.ajaxOn = setTimeout(updateStatus, 4000);
		$.getJSON('/goform/NetWorkSetupInit?'+ Math.random(), function (json) {
			if (this.responseText.indexOf('!DOCTYPE ') !== -1) {
				window.location.reload();
				return;
			}
			G.data = json;
			transformData(json);
			showErrorMassage();
			//cloneMac
			$('#cloneMac').on('click', function() {
				var f = document.macClone;
				f.WMAC.value = G.data['mac-pc'];
				f.submit();
				$.ajaxMassage('mac地址克隆中，请等待页面响应...');
				return false;
			});
		});
	}
    
	function showPwdMsg() {//修改无线密码未修改ssid时提示
		var ossid = document.getElementById('ssid').value,
			ossid_pwd = document.getElementById('ssid-pwd').value;
			
		if(ossid === G.data['ssid'] && ossid_pwd !== G.data['ssid-pwd'] &&
				G.data['ssid-pwd'] !== '') {
				
			if(ossid_pwd.length >= 8 && ossid_pwd.length <= 63 &&
					(/^[ -~]+$/g).test(ossid_pwd) && ossid !== '') {
					
				$('#ssid').removeValidateTip(true)
					.addValidateTip("为避免因修改密码而导致无线连接不上，建议同时修改无线信号名称", true)
					.showValidateTip();
			}
		}
		if (ossid_pwd === G.data['ssid-pwd']  && ossid !== '') {
			$('#ssid').removeValidateTip(true);
		}
	}
	function initView() {
		var conType = G.data['con-type'],
			textboxsIp = $('.textboxs-ip').toTextboxs(),
			elem;

		$('.textboxs-mask').toTextboxs('ip', '255.255.255.0');
		if(+G.wifiPower) {
			 $(".icons-signal").parent().removeClass("none");	
		} else {
			 $(".icons-signal").parent().addClass("none");
		}
		fillDataById(G.data);
		setValueByName('con-type', conType);
		showConnectType(conType);
		
		if (G.firstInit) {
			$('#adsl-user').addPlaceholder('请输入宽带运营商提供的账号');
			$('#adsl-pwd').initPassword('请输入宽带运营商提供的密码', true, true);
			//$('#ssid-pwd').addPlaceholder('此项为空表示不加密');
			$('#ssid-pwd').initPassword('此项为空表示不加密');
			initControler();
			G.firstInit = false;
		}
		
		$('#ssid-pwd').on('keyup', showPwdMsg);
		$('#ssid-pwd').on('focus', function () {
			setTimeout(showPwdMsg, 200);
		});
		$('#ssid').on('blur', function () {
			setTimeout(showPwdMsg, 200);
		});
	}
	function transformData(data) {
		G.wifiPower = data['wifi-power'];
		G.wanLink = data['wan-link'];
		G.conType = data['con-type'];
		G.conStat = data['con-stat'];
		G.internetStat = data['internet-state'];
		G.errorInfo = data['error-info'];
		G.wlMode = data['wl-mod'];
		G.netCheck = data['internet-check-type'];
		
		data.ssid = decodeSSID(data.ssid);
		data['ssid-pwd'] = decodeSSID(data['ssid-pwd']);
		data['adsl-pwd'] = decodeSSID(data['adsl-pwd']);
		data['adsl-user'] = decodeSSID(data['adsl-user']);
		
		var ssidPwd = document.getElementById('ssid-pwd'),
			ssidElm = document.getElementById('ssid');
		if(data['enwps'] === '1') {
			ssidPwd.disabled = true;
			ssidElm.disabled = true;
			ssidElm.title = "在系统设置中关闭wps后，才可修改无线信号名称";
			ssidPwd.title = "在系统设置中关闭wps后，才可修改密码";
		} else {
			ssidPwd.disabled = false;
			ssidElm.disabled = false;
		}
	}
	function init(jsonData) {
		G.data = jsonData;
		transformData(G.data);
		initView();
		showErrorMassage();
		G.ajaxOn = setTimeout(updateStatus, 5000);
	}
	
	function getData() {
		$.getJSON('/goform/NetWorkSetupInit?'+ Math.random(), init);
	}

	// 页面DOM元素加载完成，图片未加载时运行
	$(function () {
		getData();
	});

}(window, document, $));