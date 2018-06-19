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

		conMassage = _('IP conflict! The login IP address will be changed into %s automatically.Please log in again using %s.',[newIp, newIp]);

		if ($.cookie.get('dialogSameNet') &&
			$.cookie.get('dialogSameNet') === 'true') {

			return ;
		}
		if (!G.dialog) {
			G.dialog = $.dialog({
				title: _('Message from webpage'),
				showNoprompt: true,
				content: '<p>' + conMassage + '</p>',
				apply: function () {
					if(this.noprompt === "true") {
						saveDialogStatus(this.noprompt);
					}
					G.ajaxMsg = $.ajaxMassage(_('Saving…please wait…'));
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
			stateObj = ['<span class="text-error">' + _('No access to Internet') + '</span>',
				'<span class="text-tips">' + _('Connecting Internet') + '</span><img class="img-loading" alt="Loading…" src="img/loading.gif">',
				_('Internet Access')],
			$connectInfo = $("#connectInfo"),
			errorMassage = {
                "0": '<span class="info-block text-error">' + _('There is no Ethernet  cable inserting to WAN port, please plug an Ethernet cable to WAN port.') + '</span>',
                "1": '<span class="info-block text-error">' + _('Checking the username and password, please wait. It will take 1~5 minutes.') + '</span>',
                "2": '<span class="info-block text-error">' + _('Failed. Please confirm your username and password and try again.') + '</span>',
                "3": '<span class="info-block text-error">' + _('Remote server gives no response. Please make sure your PC can access the Internet without a router.If not, consult your ISP for help.') + '</span>',
                "4": '<span class="info-block text-error">' + _('Dial-up succeeded. But the router is not connected to the Internet.') + '<br/>' + _('Please make sure your PC can access the Internet without a router.If not, consult your ISP for help.') +'</span>'
            },
			errorTip = {
                "0": '<span class="info-block text-error">' + _('Your Internet connection type may be PPPoE as detected.Please enter correct username and password.')+ '<br/>' + _('Click OK to try to access the Internet.') + '</span>',
                "1": '<span class="info-block text-error">' + _('Your Internet connection type may be Static IP as detected. Please select Static IP and configure the network parameters manually.') + '</span>',
                "2": '<span class="info-block text-error">' + _('The router has obtained a valid IP address but cannot access the Internet. Please try the solutions below one by one.') +
				'<br/>1. <a id="cloneMac" style="text-decoration:underline;"class="text-success" href="#">' + _('Clone MAC address') + '</a>' + _(', and wait for 30 seconds.') +
                        '<br/>' + _('2. Try to configure the router on a PC which can access the Internet without a router. (Connect  the PC to the router via an Ethernet cable.)') +
						'<br/>' + _('3. Please make sure your PC can access the Internet without a router.If not, consult your ISP for help.'),
		 "3": '<span class="info-block text-error">' +_('Your Internet connection type may be PPPoE as detected.Please click OK to try to access the Internet.') + '</span>'
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
		
		$('#ssid-pwd').on('keyup', showPwdMsg);
		$('#ssid-pwd').on('focus', function () {
			setTimeout(showPwdMsg, 200);
		});
		$('#ssid').on('blur', function () {
			setTimeout(showPwdMsg, 200);
		});
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
					ssid = $('#ssid')[0];
				if($('#static')[0].checked) {
					if (isSameNet(lanIp, wanIp, lanMask, wanMask)) {
						return _('The IP cannot be in the same net segment as the login IP %s!',[lanIp]);
					}
					if (wanGateway === lanIp) {
						return _('The default gateway cannot be the same as the login IP %s!',[lanIp]);
					}
					if ((wanIp === wanGateway  || wanIp === wanDns1 ||
							wanIp === wanDns2) && !$.isHidden($('#ipval')[0])) {
						return _('The IP cannot be the same as default gateway or DNS server.');
					}
				}
				//if(!(/^[ -~]+$/g).test(ssid.value) && ssid.value.length > 10) {
				if($.getUtf8Length(ssid.value) > 32) {
					ssid.focus();
					return _('WiFi Name length exceed 32 Bytes');
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
					if(!confirm(_("WiFi password is changed into %s.You'll need to use the new password to reconnect to the WiFi. Are you sure to change the password?",[wirelessPwd]))) {
						return;
					}
				}
				clearTimeout(G.ajaxOn);
				G.ajaxMsg = $.ajaxMassage(_('Saving…please wait…'));
				$('#submit')[0].disabled = true;
				$.post(url, data, function (req) {
					setTimeout(function () {
						getData();
						G.ajaxMsg.text(_('Saved successfully!'));
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
				$.ajaxMassage(_('Cloning MAC address, please wait…'));
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
					.addValidateTip(_('Suggest changing WiFi name while changing the WiFi password to ensure proper WiFi connection.'), true)
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

		fillDataById(G.data);
		setValueByName('con-type', conType);
		showConnectType(conType);

		if (G.firstInit) {
			$('#adsl-user').addPlaceholder(_('Please enter the username.'));
			$('#adsl-pwd').initPassword(_('Please enter the password.'), true, true);
			$('#ssid-pwd').initPassword(_('Blank indicates no encryption.'));
			initControler();
			G.firstInit = false;
		}
	}
	function transformData(data) {
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
			ssidElm.title = _('You need to disable WPS feature before you change the WiFi name (SSID).');
			ssidPwd.title = _('You need to disable WPS feature before you change the WiFi Password.');
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